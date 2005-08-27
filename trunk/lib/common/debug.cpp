#include <stdarg.h>
#include "debug.h"

int nesting = 0;

#ifdef _DEBUG
void debugp(const char* prefix, const char* format,...)
{	
	fprintf(stderr, "%s|",prefix);

	int n=nesting;	while(n--) fprintf(stderr," ");

	va_list v;
	va_start(v,format);
	vfprintf(stderr, format, v);
	va_end(v);

	fprintf(stderr, "\n");
}
#endif
