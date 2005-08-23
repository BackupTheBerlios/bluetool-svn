#include <stdarg.h>
#include "debug.h"

#ifdef _DEBUG
void debugp(const char* prefix, const char* format,...)
{	
	fprintf(stderr, "%s|",prefix);

	va_list v;
	va_start(v,format);
	vfprintf(stderr, format, v);
	va_end(v);

	fprintf(stderr, "\n");
}
#endif
