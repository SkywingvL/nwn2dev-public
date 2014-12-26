/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.
Portions Copyright (c) Andreas Hansson (Adron). All rights reserved.

Module Name:

    hdlcommon.h

Abstract:

    This module defines various common definitions used by the .hdl loader
	format, such as the Patch class.

--*/

#ifndef _HDLCOMMON_H
#define _HDLCOMMON_H

// DLL relocation class
class Relocation {
	char *dll;
	char *func;
	DWORD base;
	DWORD offs;
	Relocation();
public:
	~Relocation() {};
	Relocation(char *ndll, char *nfunc, DWORD noffs) : dll(ndll), func(nfunc), offs(noffs) {};
	operator DWORD() {return base; };
	void Relocate();
};

typedef void (*relativefunc)();
typedef struct _TAGABSOLUTEFUNC *absolutefunc;

// Memory patch class
class Patch {
	char *orig;
	char *repl;
	char *offs;
	int len;
	Relocation *reloc;
public:
	~Patch();
	Patch(DWORD offset, char *replace, int l, Relocation *nreloc = 0);
	Patch(DWORD offset, relativefunc func, Relocation *nreloc = 0);
	Patch(DWORD offset, absolutefunc func, Relocation *nreloc = 0);
	Patch();
	int Apply(void);
	int Remove(void);
private:
	bool relocdata;
	bool enabled;
};

#endif
