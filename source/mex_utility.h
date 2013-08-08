#ifndef __MEX_UTILITY_H
#define __MEX_UTILITY_H
#ifdef LINUX 
#include<vasnprintf.h>
#include<stdarg.h>
#endif
#include<string>

std::string mcvm_vasprintf(const char* fmt, va_list args);
#endif 
