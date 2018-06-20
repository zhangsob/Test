#ifndef ZUTIL_H
#define ZUTIL_H

#include <string>

#ifdef _DEBUG
void log(const char *fmt, ...);
#endif

std::string format(const char* format, ...);

#endif
