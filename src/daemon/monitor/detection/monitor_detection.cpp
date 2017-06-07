/*
Copyright © Bubi Technologies Co., Ltd. 2017 All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
		 http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifdef WIN32
#else
#include <unistd.h>  
#include <fcntl.h>    
#include <string.h>  
#include <libgen.h>  
#include <dirent.h>  
#include <stdio.h>  
#include <stdlib.h>
#endif
#include <iostream>
#include <string>
#include <zmq.h>

bool is_prog_run();
void sleep(int time_milli);

int main(int argc, char* argv[]) {
	do {
		if (is_prog_run()) {
			std::cout << "only allowing one program to run at the same time" << std::endl;
			break;
		}

		if (argc != 2) {
			std::cout << "please enter a parameter." << std::endl;
			break;
		}

		std::string method = argv[1];
		if (method != "bubi" && method != "ledger" && method != "system" && method != "block_status") {
			std::cout << "parameter is wrong" << std::endl;
			break;
		}

		void* pcontext = zmq_ctx_new();
		void* psocket_push = zmq_socket(pcontext, ZMQ_PUSH);
		zmq_connect(psocket_push, "");
		void* psocket_pull = zmq_socket(pcontext, ZMQ_PULL);

		sleep(5000);

	} while (false);
	return 0;
}
#ifdef WIN32
bool is_prog_run() {
	bool bret = false;
	HANDLE m_hMutex = ::CreateMutex(NULL, FALSE, "[Guid(\"4A45E6F4-F030-4136-B138-72C5A7D14A4A\")]");
	if (GetLastError() == ERROR_ALREADY_EXISTS){
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		bret = true;
	}
	return bret;
}
#else
bool is_prog_run() {
	long pid;
	char full_name[1024] = { 0 };
	char proc_name[1024] = { 0 };
	int fd;

	pid = getpid();
	sprintf(full_name, "/proc/%ld/cmdline", pid);
	if (access(full_name, F_OK) == 0) {
		fd = open(full_name, O_RDONLY);
		if (fd == -1)
			return false;
		read(fd, proc_name, 1024);
	}
	else {
		return false;
	}
	char self_proc_name[512] = { 0 };
	char *p = proc_name;
	int pt = 0;
	while (*p != ' ' && *p != '\0') {
		self_proc_name[pt] = *p;
		p++;
		pt++;
	}
	std::string self_final_name = basename(self_proc_name);
	DIR *dir;
	struct dirent *result;
	dir = opendir("/proc");
	while ((result = readdir(dir)) != NULL) {
		if (!strcmp(result->d_name, ".") || !strcmp(result->d_name, "..") || !strcmp(result->d_name, "self") || atol(result->d_name) == pid)
			continue;
		memset(full_name, 0, sizeof(full_name));
		memset(proc_name, 0, sizeof(proc_name));
		sprintf(full_name, "/proc/%s/cmdline", result->d_name);
		if (access(full_name, F_OK) == 0) {
			fd = open(full_name, O_RDONLY);
			if (fd == -1)
				continue;  
			read(fd, proc_name, 1024);

			char *q = proc_name;
			pt = 0;
			memset(self_proc_name, 0, sizeof(self_proc_name));
			while (*q != ' ' && *q != '\0') {
				self_proc_name[pt] = *q;
				q++;
				pt++;
			}
			std::string other_final_name = basename(self_proc_name);
			if (self_final_name == other_final_name) {
				return true;
			}
		}
	}
	return false;
}
#endif // WIN32

void sleep(int time_milli)
{
#ifdef WIN32
	::Sleep(time_milli);
#else
	::usleep(((__useconds_t)time_milli) * 1000);
#endif //WIN32
}
