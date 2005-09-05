#include <common/debug.h>

#ifdef _DEBUG

static void default_vdebugp( const char* format, va_list args )
{
	vfprintf(stderr,format,args);
}

void (*vdebugp)(const char*, va_list) = &default_vdebugp;

int nesting = 0;

void debugp(const char* prefix, const char* format,...)
{	
	fprintf(stderr, "%s|",prefix);

	int n=nesting;	while(n--) fprintf(stderr," ");

	va_list v;
	va_start(v,format);
	(*vdebugp)(format, v);
	va_end(v);

	fprintf(stderr, "\n");
}
#endif
