// This file is part of Glest <https://github.com/Glest>
//
// Copyright (C) 2018  The Glest team
//
// Glest is a fork of MegaGlest <https://megaglest.org/>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>

#ifndef _LEAKDUMPER_H_
#define _LEAKDUMPER_H_

//#define SL_LEAK_DUMP
//SL_LEAK_DUMP controls if leak dumping is enabled or not

#ifdef SL_LEAK_DUMP
#include <new>

// START - Special includes because the use a special new operator that we cannot override
#include <string>
#include "rapidxml.hpp"

#include <xercesc/dom/DOM.hpp>
// END - Special includes because the use a special new operator that we cannot override

#include <memory>
#include <cstdio>
#include <cstdlib>

#include "thread.h"
using Shared::Platform::Mutex;
using Shared::Platform::MutexSafeWrapper;

// START - For gcc backtrace
//#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__FreeBSD__) && !defined(BSD)
#if defined(HAS_GCC_BACKTRACE)
#include <execinfo.h>
#include <cxxabi.h>
#include <signal.h>
#endif
// END - For gcc backtrace

using namespace std;

//including this header in any file of a project will cause all
//leaks to be dumped into leak_dump.txt, but only allocations that
//occurred in a file where this header is included will have
//file and line number

static bool want_full_leak_stacktrace = false;
static bool want_full_leak_stacktrace_line_numbers = false;

struct AllocInfo {
private:
	static bool application_binary_initialized;

public:

	inline static string &get_application_binary() {
		static string application_binary = "";
		return application_binary;
	}

	int line;
	const char *file;
	size_t bytes;
	void *ptr;
	bool freetouse;
	bool array;
	bool inuse;
	string stack;

	inline AllocInfo()
		: ptr(0), file(""), line(-1), bytes(0), array(false), freetouse(false), inuse(false), stack("") {
	}

	inline AllocInfo(void* ptr, const char* file, int line, string stacktrace, size_t bytes, bool array)
		: ptr(ptr), file(file), line(line), bytes(bytes), array(array), freetouse(false), inuse(true), stack(stacktrace) {
	}

	//#if defined(__GNUC__) && !defined(__FreeBSD__) && !defined(BSD)
#if defined(HAS_GCC_BACKTRACE)
	inline static int getFileAndLine(char *function, void *address, char *file, size_t flen) {
		int line = -1;
		if (want_full_leak_stacktrace_line_numbers == true && AllocInfo::get_application_binary() != "") {
			const int maxbufSize = 8094;
			char buf[maxbufSize + 1] = "";
			//char *p=NULL;

			// prepare command to be executed
			// our program need to be passed after the -e parameter
			//sprintf (buf, "/usr/bin/addr2line -C -e ./a.out -f -i %lx", addr);
			snprintf(buf, 8096, "addr2line -C -e %s -f -i %p", AllocInfo::get_application_binary().c_str(), address);

			FILE* f = popen(buf, "r");
			if (f == NULL) {
				perror(buf);
				return 0;
			}

			if (function != NULL && function[0] != '\0') {
				line = 0;
				for (; function != NULL && function[0] != '\0';) {
					// get function name
					char *ret = fgets(buf, maxbufSize, f);
					if (ret == NULL) {
						pclose(f);
						return line;
					}
					//printf("Looking for [%s] Found [%s]\n",function,ret);
					if (strstr(ret, function) != NULL) {
						break;
					}

					// get file and line
					ret = fgets(buf, maxbufSize, f);
					if (ret == NULL) {
						pclose(f);
						return line;
					}

					if (strlen(buf) > 0 && buf[0] != '?') {
						//int l;
						char *p = buf;

						// file name is until ':'
						while (*p != 0 && *p != ':') {
							p++;
						}

						*p++ = 0;
						// after file name follows line number
						strcpy(file, buf);
						sscanf(p, "%d", &line);
					} else {
						strcpy(file, "unknown");
						line = 0;
					}
				}
			}

			// get file and line
			char *ret = fgets(buf, maxbufSize, f);
			if (ret == NULL) {
				pclose(f);
				return line;
			}

			if (strlen(buf) > 0 && buf[0] != '?') {
				//int l;
				char *p = buf;

				// file name is until ':'
				while (*p != 0 && *p != ':') {
					p++;
				}

				*p++ = 0;
				// after file name follows line number
				strcpy(file, buf);
				sscanf(p, "%d", &line);
			} else {
				strcpy(file, "unknown");
				line = 0;
			}
			pclose(f);
		}
		return line;
	}
#endif

	inline static string getStackTrace() {
		//#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__FreeBSD__) && !defined(BSD)
#if defined(HAS_GCC_BACKTRACE)
		if (want_full_leak_stacktrace == true) {
			string errMsg = "\nStack Trace:\n";
			//errMsg += "To find line #'s use:\n";
			//errMsg += "readelf --debug-dump=decodedline %s | egrep 0xaddress-of-stack\n";

			const size_t max_depth = 6;
			void *stack_addrs[max_depth];
			size_t stack_depth = backtrace(stack_addrs, max_depth);
			char **stack_strings = backtrace_symbols(stack_addrs, stack_depth);
			//for (size_t i = 1; i < stack_depth; i++) {
			//    errMsg += string(stack_strings[i]) + "\n";
			//}

			//if(SystemFlags::VERBOSE_MODE_ENABLED) printf("In [%s::%s Line: %d]\n",__FILE__,__FUNCTION__,__LINE__);

			if (stack_depth > 0) {
				char szBuf[8096] = "";
				for (size_t i = 1; i < stack_depth; i++) {
					void *lineAddress = stack_addrs[i]; //getStackAddress(stackIndex);

					size_t sz = 8096; // just a guess, template names will go much wider
					char *function = static_cast<char *>(malloc(sz));
					function[0] = '\0';

					char *begin = 0;
					char *end = 0;

					// find the parentheses and address offset surrounding the mangled name
					for (char *j = stack_strings[i]; *j; ++j) {
						if (*j == '(') {
							begin = j;
						} else if (*j == '+') {
							end = j;
						}
					}
					if (begin && end) {
						*begin++ = '\0';
						*end = '\0';
						// found our mangled name, now in [begin, end)

						int status;
						char *ret = abi::__cxa_demangle(begin, function, &sz, &status);
						if (ret) {
							// return value may be a realloc() of the input
							function = ret;
						} else {
							// demangling failed, just pretend it's a C function with no args
							strncpy(function, begin, sz);
							strncat(function, "()", sz);
							function[sz - 1] = '\0';
						}
						//fprintf(out, "    %s:%s\n", stack.strings[i], function);

						snprintf(szBuf, 8096, "%s:%s address [%p]", stack_strings[i], function, lineAddress);
					} else {
						// didn't find the mangled name, just print the whole line
						//fprintf(out, "    %s\n", stack.strings[i]);
						snprintf(szBuf, 8096, "%s address [%p]", stack_strings[i], lineAddress);
					}

					errMsg += string(szBuf);

					if (want_full_leak_stacktrace_line_numbers == true && AllocInfo::get_application_binary() != "") {
						char file[8096] = "";
						int line = getFileAndLine(function, lineAddress, file, 8096);
						if (line >= 0) {
							char lineBuf[1024] = "";
							snprintf(lineBuf, 1024, "%d", line);
							errMsg += " line: " + string(lineBuf);
						}
					}
					errMsg += "\n";

					free(function);
				}
			}
			free(stack_strings); // malloc()ed by backtrace_symbols

			//printf("%s",errMsg.c_str());
			return errMsg;
		} else {
			static string empty = "";
			return empty;
		}
#else
		static string empty = "";
		return empty;
#endif
	}
};

// =====================================================
//	class AllocRegistry
// =====================================================

class AllocRegistry {
private:
	static const size_t maxAllocs = 100000;
	Mutex *mutex;

private:

	inline AllocRegistry() {
		// manual memory allocation
		void *mymutex_mem = malloc(sizeof(Mutex));
		// in place new operator
		mutex = new(mymutex_mem) Mutex;

		string value = AllocInfo::get_application_binary();
		reset();
	}

private:
	AllocInfo allocs[maxAllocs];	//array to store allocation info
	int allocCount;					//allocations
	size_t allocBytes;				//bytes allocated
	int nonMonitoredCount;
	size_t nonMonitoredBytes;
	int nextFreeIndex;

public:
	~AllocRegistry();
	inline static AllocRegistry &getInstance() {
		static AllocRegistry allocRegistry;
		return allocRegistry;
	}

	inline void reset() {
		allocCount = 0;
		allocBytes = 0;
		nonMonitoredCount = 0;
		nonMonitoredBytes = 0;
		nextFreeIndex = 0;

		for (int i = 0; i < maxAllocs; ++i) {
			allocs[i].freetouse = true;
			allocs[i].inuse = false;
		}
	}

	inline void allocate(AllocInfo info) {
		//if(info.line == 0) return;

		MutexSafeWrapper safeMutexMasterList(mutex);
		//printf("ALLOCATE.\tfile: %s, line: %d, bytes: " SIZE_T_SPECIFIER ", array: %d inuse: %d\n", info.file, info.line, info.bytes, info.array, info.inuse);

		if (info.line > 0) {
			++allocCount;
			allocBytes += info.bytes;
		}
		for (int i = (nextFreeIndex >= 0 ? nextFreeIndex : 0); i < maxAllocs; ++i) {
			if (allocs[i].freetouse == true && allocs[i].inuse == false) {
				allocs[i] = info;
				nextFreeIndex = -1;

				return;
			}
		}

		printf("ALLOCATE NOT MONITORED.\tfile: %s, line: %d, ptr [%p], bytes: " SIZE_T_SPECIFIER ", array: %d inuse: %d, \n%s\n", info.file, info.line, info.ptr, info.bytes, info.array, info.inuse, info.stack.c_str());

		++nonMonitoredCount;
		nonMonitoredBytes += info.bytes;
	}

	inline void deallocate(void* ptr, bool array, const char* file, int line) {
		//if(line == 0) return;

		MutexSafeWrapper safeMutexMasterList(mutex);

		for (int i = 0; i < maxAllocs; ++i) {
			if (allocs[i].freetouse == false && allocs[i].inuse == true &&
				allocs[i].ptr == ptr && allocs[i].array == array) {
				allocs[i].freetouse = true;
				allocs[i].inuse = false;
				nextFreeIndex = i;

				return;
			}
		}

		if (line > 0) printf("WARNING, possible error on pointer delete for ptr [%p] array = %d, file [%s] line: %d\n%s\n", ptr, array, file, line, AllocInfo::getStackTrace().c_str());
	}

	void dump(const char *path);
};

//if an allocation ocurrs in a file where "leaks_dumper.h" is not included
//this operator new is called and file and line will be unknown
inline void * operator new (size_t bytes) {
	void *ptr = malloc(bytes);
	AllocRegistry::getInstance().allocate(AllocInfo(ptr, "unknown", 0, "", bytes, false));
	return ptr;
}

inline void operator delete(void *ptr) {
	AllocRegistry::getInstance().deallocate(ptr, false, "unknown", 0);
	free(ptr);
}

inline void * operator new[](size_t bytes) {
	void *ptr = malloc(bytes);
	AllocRegistry::getInstance().allocate(AllocInfo(ptr, "unknown", 0, "", bytes, true));
	return ptr;
}

inline void operator delete [](void *ptr) {
	AllocRegistry::getInstance().deallocate(ptr, true, "unknown", 0);
	free(ptr);
}

//if an allocation ocurrs in a file where "leaks_dumper.h" is included 
//this operator new is called and file and line will be known
inline void * operator new (size_t bytes, const char* file, int line, string stack) {
	void *ptr = malloc(bytes);
	AllocRegistry::getInstance().allocate(AllocInfo(ptr, file, line, stack, bytes, false));
	return ptr;
}

inline void operator delete(void *ptr, const char* file, int line) {
	AllocRegistry::getInstance().deallocate(ptr, false, file, line);
	free(ptr);
}

inline void * operator new[](size_t bytes, const char* file, int line, string stack) {
	void *ptr = malloc(bytes);
	AllocRegistry::getInstance().allocate(AllocInfo(ptr, file, line, stack, bytes, true));
	return ptr;
}

inline void operator delete [](void *ptr, const char* file, int line) {
	AllocRegistry::getInstance().deallocate(ptr, true, file, line);
	free(ptr);
}

#define new new(__FILE__, __LINE__,AllocInfo::getStackTrace())
//#define new new(__FILE__, __LINE__,"")

#endif

#endif
