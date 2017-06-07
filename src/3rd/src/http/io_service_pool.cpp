//
// io_service_pool.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include <stdexcept>
#ifndef WIN32
#include <sys/prctl.h>
#endif

namespace http {
namespace server {

io_service_pool::io_service_pool(std::size_t pool_size)
  : next_io_service_(2)
{
  if (pool_size < 2)
    throw std::runtime_error("io_service_pool size is less 2");

  // Give all the io_services work to do so that their run() functions will not
  // exit until they are explicitly stopped.
  for (std::size_t i = 0; i < pool_size; ++i)
  {
    io_service_ptr io_service(new asio::io_service);
    work_ptr work(new asio::io_service::work(*io_service));
    io_services_.push_back(io_service);
    work_.push_back(work);
  }
}

void io_service_pool::run()
{
  // Create a pool of threads to run all of the io_services.
  for (std::size_t i = 0; i < io_services_.size(); ++i)
  {
	  io_service_ptr cur_ptr = io_services_[i];
	  std::shared_ptr<asio::thread> thread(new asio::thread([this, cur_ptr, i](){
#ifndef WIN32
          char name[16] = { 0 };
          sprintf(name, "http-%d", int(i));
          prctl(PR_SET_NAME, name, 0, 0, 0);
#endif //WIN32
		  cur_ptr->run();
	  }));
	  threads_.push_back(thread);
  }

}

void io_service_pool::stop()
{
  // Explicitly stop all io_services.
  for (std::size_t i = 0; i < io_services_.size(); ++i)
    io_services_[i]->stop();

  // Wait for all threads in the pool to exit.
  for (std::size_t i = 0; i < threads_.size(); ++i)
	  threads_[i]->join();
}

asio::io_service& io_service_pool::get_other_service()
{
  // Use a round-robin scheme to choose the next io_service to use.
  asio::io_service& io_service = *io_services_[next_io_service_];
  ++next_io_service_;
  if (next_io_service_ == io_services_.size())
    next_io_service_ = 1;
  return io_service;
}

asio::io_service& io_service_pool::get_first_service()
{
	return *io_services_[0];
}

} // namespace server2
} // namespace http
