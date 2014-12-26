/*++

Copyright (c) Andreas Hansson (Adron). All rights reserved.
Portions Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    Patch.cpp

Abstract:

    This module houses the Patch class implementation, which wraps the process
	of editing the memory of a target program is a reversible fashion.  The
	Patch class also implements automatic support for calculating delta for
	instruction-relative patches.

--*/

#include "precomp.h"
#include "hdlcommon.h"

//
// Intentionally disable 64-bit portability warnings as the Patch class is by
// nature 32-bit specific presently.  It is not used on 64-bit platforms.
//

#pragma warning(disable:4311)
#pragma warning(disable:4312)

Patch::~Patch()
{
	if(orig)
		delete [] orig;
	if(repl)
		delete [] repl;
}

Patch::Patch(DWORD offset, char *replace, int l, Relocation *nreloc)
{
	offs = reinterpret_cast<char*>(offset);
	repl = new char[l];
	memcpy(repl, replace, l);
	reloc = nreloc;
	relocdata = false;
	enabled = false;
	orig = 0;
	len = l;
}

Patch::Patch(DWORD offset, relativefunc func, Relocation *nreloc)
{
	offs = reinterpret_cast<char*>(offset);
	repl = new char[4];
	*(DWORD*)repl = reinterpret_cast<DWORD>(func) - offset - 4;
	reloc = nreloc;
	relocdata = false;
	enabled = false;
	orig = 0;
	len = 4;
}

Patch::Patch(DWORD offset, absolutefunc func, Relocation *nreloc)
{
	offs = reinterpret_cast<char*>(offset);
	repl = new char[4];
	*(DWORD*)repl = reinterpret_cast<DWORD>(func);
	reloc = nreloc;
	relocdata = false;
	enabled = false;
	orig = 0;
	len = 4;
}

Patch::Patch()
{
	offs = repl = orig = 0;
	reloc = 0;
	len = 0;
	enabled = false;
}

int Patch::Apply(void)
{
	if (enabled)
		return 0;
	if(!offs)
		return 0;
	DWORD oldprot;
	char *trueoffs = offs;
	if(reloc) {
		trueoffs += *reloc;
		if (relocdata)
			*(DWORD*)repl -= *reloc;
	}
	VirtualProtect(trueoffs, len, PAGE_EXECUTE_READWRITE, &oldprot);
	if(!orig) {
		orig = new char[len];
		memcpy(orig, trueoffs, len);
	}
	memcpy(trueoffs, repl, len);
	VirtualProtect(trueoffs, len, oldprot, &oldprot);
	enabled = true;
	return 1;
}


int Patch::Remove(void)
{
	if (!enabled)
		return 0;
	if(!offs)
		return 0;
	DWORD oldprot;
	char *trueoffs = offs;
	if(reloc) {
		trueoffs += *reloc;
		if (relocdata)
			*(DWORD*)repl += *reloc;
	}
	VirtualProtect(trueoffs, len, PAGE_EXECUTE_READWRITE, &oldprot);
	if(orig) {
		memcpy(trueoffs, orig, len);
	}
	VirtualProtect(trueoffs, len, oldprot, &oldprot);
	enabled = false;
	return 1;
}

void Relocation::Relocate()
{
	HINSTANCE hLib = LoadLibraryA(dll);
	if(hLib != 0) {
		base = reinterpret_cast<DWORD>(GetProcAddress(hLib, func)) - offs;
		FreeLibrary(hLib);
	} else {
		base = 0x10000000;
	}
}
