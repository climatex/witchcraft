#pragma once                     // #ifndef _GOTO_H_ ... #define... #endif
#pragma runtime_checks("s", off) // For Visual Studio (or, compile without /RTC)

#include <string>
#include <map>			 // g_cMap assigns code offsets to labels 

#ifndef DWORD
#define DWORD unsigned __int32
#endif

#define goto(name)		{ \
					DWORD nOfs = g_cMap[(name)]; \
					if (nOfs) \
						__asm jmp nOfs \
				}

#define prep(label, name)	{ \
					DWORD nOfs; \
					__asm { \
						__asm push dword ptr label \
						__asm pop dword ptr nOfs \
					} \
					g_cMap[(name)] = nOfs; \
				}

std::map<std::string, DWORD> g_cMap;