#include <direct.h>

#define mkdir _mkdirthunk
#define chdir _chdirthunk

int
_mkdirthunk(
	__in const char * dirname
	);

int
_chdirthunk(
	__in const char * dirname
	);

#include "../zlib/contrib/minizip/miniunz.c"
