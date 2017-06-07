LIBZMQ = $(top_builddir)/src/3rd/linux/lib/libzmq.a
PROTOBUF = $(top_builddir)/src/3rd/linux/lib/libprotobuf.a
ROCKSDB = $(top_builddir)/src/3rd/linux/lib/librocksdb.a
JSONCPP = $(top_builddir)/src/3rd/linux/lib/libjson.a
LIBSSL = $(top_builddir)/src/3rd/linux/lib/libssl.a
LIBCRYPTO = $(top_builddir)/src/3rd/linux/lib/libcrypto.a
LIBZ = $(top_builddir)/src/3rd/linux/lib/libz.a
LIBBZ2 = $(top_builddir)/src/3rd/linux/lib/libbz2.a
POSTGRE = $(top_builddir)/src/3rd/linux/lib/libpq.a 

bin_PROGRAMS = bubi_slave

bubi_slave_SOURCES = \
				main.cpp \
				../api/web_server.cpp \
				slave_service.cpp \
				slave_executor.cpp \
				../overlay/peer_message.cpp \
				../monitor/monitor_slave.cpp \
				../monitor/monitoragent.cpp \
				../monitor/system_manager.cpp

bubi_slave_CXXFLAGS = -std=c++11 -DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STL_ -DBUBI_SLAVE\
					-I$(top_srcdir)/src \
					-I$(top_builddir)/src/3rd/linux/include \
					 $(NULL)

bubi_slave_LDFLAGS = -static-libstdc++ -static-libgcc 

bubi_slave_LDADD =	\
				$(top_builddir)/src/common/libbubi_common.a \
				$(top_builddir)/src/utils/libbubi_utils.a \
				$(top_builddir)/src/proto/libbubi_proto.a \
				$(top_builddir)/src/3rd/linux/lib/libbubi_http.a \
				$(top_builddir)/src/3rd/linux/lib/libed25519_donna.a \
				$(LIBZMQ) \
				$(PROTOBUF) \
				$(ROCKSDB) \
				$(JSONCPP) \
				$(LIBSSL) \
				$(LIBCRYPTO) \
				$(LIBZ) \
				$(LIBBZ2) \
				$(POSTGRE) \
				$(MYSQL) \
				-lpthread -lrt -ldl \
				$(NULL)
					
