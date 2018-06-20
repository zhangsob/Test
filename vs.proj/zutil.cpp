#include "zutil.h"

#ifdef ZUTIL_H
#include <stdio.h>
#include <stdarg.h>

#ifdef _DEBUG
void log(const char *fmt, ...)
{
	FILE *fp = nullptr ;
	fopen_s(&fp, "D:\\temp\\log.txt", "a+t") ;
	if(fp) {
		va_list args;
		va_start(args, fmt);
		vfprintf(fp, fmt, args) ;
		va_end(args) ;
		fclose(fp) ;
	}
	return ;
}
#endif

std::string format(const char* format, ...)
{
    va_list args;
    va_start(args, format);
#ifndef _MSC_VER
	size_t size = std::snprintf( nullptr, 0, format, args) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf( new char[ size ] ); 
	std::vsnprintf( buf.get(), size, format, args);
	va_end(args);
	return std::string(buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
#else
	int size = _vscprintf(format, args);
	if(size <= 0)	return "" ;
	std::string result(++size, 0);
	vsnprintf_s((char*)result.data(), size, _TRUNCATE, format, args);
	va_end(args);
	result.resize(size-1);
	return result ;
#endif    
}
#endif