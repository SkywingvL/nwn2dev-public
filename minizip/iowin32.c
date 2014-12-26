#include <direct.h>

#define mkdir _mkdirthunk
#define chdir _chdirthunk

#include "../zlib/contrib/minizip/iowin32.c"

//
// Compatibility wrappers with USE_MSVCRT=1
//

_CRTIMP __checkReturn int __cdecl _mkdir(__in_z const char * _Path);
_CRTIMP __checkReturn int __cdecl _chdir(__in_z const char * _Path);

int
_mkdirthunk(
	__in const char * dirname
	)
{
	return _mkdir( dirname );
}

int
_chdirthunk(
	__in const char * dirname
	)
{
	return _chdir( dirname );
}

