/**********************************************************************
*<
FILE: NWN2Format.h

DESCRIPTION:	NWN File Format

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/
#include "Precomp.h"
#include "NWN2Format.h"

using namespace std;

namespace granny2 {

Matrix3 TOMATRIX(Quat const& q)
{
	Matrix3 m(true);
	float w = q.w, x = q.x, y = q.y, z = q.z;
	float w2 = w * w, x2 = x * x, y2 = y * y, z2 = z * z;
	m.SetRow(0, Point3(w2 + x2- y2 - z2, 2.0f*x*y - 2.0f*w*z, 2.0f*x*z + 2.0f*w*y));
	m.SetRow(1, Point3(2.0f*x*y + 2.0f*w*z, w2 - x2 + y*y - z2, 2.0f*y*z - 2.0f*w*x));
	m.SetRow(2, Point3(2.0f*x*z - 2.0f*w*y, 2.0f*y*z + 2.0f*w*x, w2 - x2 - y2 + z2));
	return m;
}

Point3 TOEULER(const Matrix3 &m)
{
	Point3 rv(0.0f, 0.0f, 0.0f);
	if ( m.GetRow(2)[0] < 1.0 )
	{
		if ( m.GetRow(2)[0] > - 1.0 )
		{
			rv[2] = atan2( - m.GetRow(1)[0], m.GetRow(0)[0] );
			rv[1] = asin( m.GetRow(2)[0] );
			rv[0] = atan2( - m.GetRow(2)[1], m.GetRow(2)[2] );
		}
		else
		{
			rv[2] = - atan2( - m.GetRow(1)[2], m.GetRow(1)[1] );
			rv[1] = - float(M_PI) / 2.0f;
			rv[0] = 0.0;
		}
	}
	else
	{
		rv[2] = atan2( m.GetRow(1)[2], m.GetRow(1)[1] );
		rv[1] = float(M_PI) / 2.0f;
		rv[0] = 0.0;
	}
	return rv;
}


// sprintf for CStr without having to worry about buffer size.
CStr FormatText(const CHAR* format,...)
{
	CStr s;
	va_list ap;

	va_start(ap, format);
	s.printfV(format, ap);
	va_end(ap);
	return s;
}

// sprintf for std::string without having to worry about buffer size.
std::string FormatString(const CHAR* format,...)
{
	CStr s;
	va_list ap;

	va_start(ap, format);
	s.printfV(format, ap);
	va_end(ap);
	return std::string(s.data());
}

CStr& Trim(CStr&p) { 
	while(!p.isNull() && _istspace(p[0])) p.remove(0, 1); 
	int len = p.Length()-1;
	while (len >= 0 && _istspace(p[len])) p.remove(len--, 1);
	return p;
}

//////////////////////////////////////////////////////////////////////////

// Enumeration Support
CStr EnumToString(int value, const EnumLookupType *table) {
	for (const EnumLookupType *itr = table; itr->name != NULL; ++itr) {
		if (itr->value == value) return CStr(itr->name);
	}
	return FormatText("%x", value);
}

int StringToEnum(CStr value, const EnumLookupType *table) {
	Trim(value);
	if (value.isNull()) return 0;

	for (const EnumLookupType *itr = table; itr->name != NULL; ++itr) {
		if (0 == _stricmp(value, itr->name)) return itr->value;
	}
	char *end = NULL;
	return (int)strtol(value, &end, 0);
}

CStr FlagsToString(int value, const EnumLookupType *table) {
	CStr sstr;
	for (const EnumLookupType *itr = table; itr->name != NULL; ++itr) {
		if (itr->value && (itr->value & value) == itr->value) {
			if (!sstr.isNull()) sstr += " | ";
			sstr += itr->name;
			value ^= itr->value;
		}
	}
	if (value == 0 && sstr.isNull()) {
		return EnumToString(value, table);
	}
	if (value != 0) {
		if (!sstr.isNull()) sstr += "|";
		sstr += EnumToString(value, table);
	}
	return sstr;
}

int StringToFlags(CStr value, const EnumLookupType *table) {
	int retval = 0;
	LPCSTR start = value.data();
	LPCSTR end = value.data() + value.Length();
	if (end-start > INT_MAX) throw std::runtime_error( "String too long." );
	while(start < end) {
		LPCSTR bar = strchr(start, '|');
		int len = (bar != NULL) ?  (int)(bar-start) : (int)(end-start);
		CStr subval = value.Substr((int)(start-value.data()), len);
		retval |= StringToEnum(subval, table);
		start += (len + 1);
	}
	return retval;
}

//////////////////////////////////////////////////////////////////////////
void NWStream::Open(const CHAR *file, bool readonly)
{
	if (fh) Close();

	if (file == NULL && readonly)
	{
		fh = stdin;
	}
	else if (file == NULL && !readonly)
	{
		fh = stdout;
	}
	else
	{
		fh = fopen(file, readonly ? "rbSNT" : "wbSN");
		if (!fh) {
			throw runtime_error(FormatString("Unable to open file: %s", file));
		}

		if (readonly) {
			int fd = _fileno(fh);
			HANDLE osfh;
			HANDLE sect;

			osfh = (HANDLE)_get_osfhandle(fd);
			sect = CreateFileMapping(osfh, NULL, PAGE_READONLY, 0, 0, NULL);
			if (sect != NULL) {
				size = TellEnd( );
				view = MapViewOfFile(sect, FILE_MAP_READ, 0, 0, 0);
				CloseHandle(sect);
				if (view != NULL) {
					offset = 0;
				}
			}
		}
		if (!view) {
			setvbuf(fh, NULL, _IOFBF, 0x8000);
		}
	}
}

void NWStream::Close()
{
	if (view) {
		UnmapViewOfFile(view);
		view = NULL;
	}
	if (fh) {
		fclose(fh);
		fh = NULL;
	}
}

uint NWStream::Read(char *buf, uint size, uint count)
{
	if (view) {
		int len = size*count;
		if (((uint)len < size) || ((uint)len < count))
			throw std::runtime_error( "size/count wrapped." );
		if (this->offset+len < this->offset)
			throw std::runtime_error( "File offset wrapped." );
		if (this->offset >= this->size)
			return 0;
		if (this->offset+len > this->size)
			len = this->size-this->offset;
		memcpy(buf, (const char *)view+this->offset, len);
		this->offset += len;
		return (uint)len / size;
	}
	uint n = (uint)fread(buf, size, count, fh);
	int err = errno;
	if (!(n == count && err == 0))
	{
		int t = Tell();
		//ASSERT(n == count && err == 0);
	}
	return n;
}

uint NWStream::Write(const char *buf, uint size, uint count)
{
	return (uint)fwrite(buf, size, count, fh);
}

int NWStream::Seek(int whence, long offset)
{
	if (view) {
		int noffs;
		switch (whence) {
		case SEEK_CUR:
			noffs=this->offset+(int)offset;
			break;
		case SEEK_END:
			noffs=this->size+(int)offset;
			break;
		case SEEK_SET:
			noffs=(int)offset;
			break;
		default:
			return -1;
		}
		if (noffs < 0)
			noffs = 0;
		else if (noffs > this->size)
			noffs = this->size;
		this->offset = noffs;
		return 0;
	}
	return fseek(fh, offset, whence);
}

int NWStream::Tell()
{
	if (view) {
		return this->offset;
	}
	return ftell(fh);
}

int NWStream::TellEnd()
{
	if (view) {
		return this->size;
	}
	struct _stat data;
	memset(&data, 0, sizeof(data));
	_fstat(_fileno(fh), &data);
	return int(data.st_size);
}

bool NWStream::Eof() const
{
	if (view) {
		return (this->offset < this->size);
	}
	return (feof(fh) != 0);
}

#if GRANNY2LIB_ENABLE_DUMP
void NWDumpStream::Print(char const * buf)
{
	Write(buf, (uint)strlen(buf), 1);
}

void NWDumpStream::PrintF(const char *format, ...)
{
	CStr s;
	va_list ap;

	va_start(ap, format);
	s.printfV(format, ap);
	va_end(ap);
	Write(s.data(), (uint)s.length());
}

void NWDumpStream::NewLine()
{
	Write("\n", 1, 1);
}

void NWDumpStream::Indent()
{
	for (int i=0; i<IndentLevel; ++i)
		Write("  ", 2, 1);
}

template<> void Dump(NWDumpStream& out, LPCSTR name, char const & val)
{
	char buf[33];
	StringCbPrintfA(buf, sizeof(buf), "%d", (unsigned int)val);
	Dump(out, name, buf);
}

template<> void Dump(NWDumpStream& out, LPCSTR name, unsigned char const & val)
{
	char buf[33];
	StringCbPrintfA(buf, sizeof(buf), "%d", (unsigned int)val);
	Dump(out, name, buf);
}

template<> void Dump(NWDumpStream& out, LPCSTR name, float const & val)
{
	char buf[33];
	StringCbPrintfA(buf, sizeof(buf), "%g", (double)val);
	Dump(out, name, buf);
}

template<> void Dump(NWDumpStream& out, LPCSTR name, Point2 const & val)
{
	char buffer[256];
	StringCbPrintfA(buffer, sizeof(buffer), "[%f, %f]", val.x, val.y);
	Dump(out, name, buffer);
}

template<> void Dump(NWDumpStream& out, LPCSTR name, Point3 const & val)
{
	char buffer[256];
	StringCbPrintfA(buffer, sizeof(buffer), "[%f, %f, %f]", val.x, val.y, val.z);
	Dump(out, name, buffer);
}

template<> void Dump(NWDumpStream& out, LPCSTR name, Point4 const & val)
{
	char buffer[256];
	StringCbPrintfA(buffer, sizeof(buffer), "[%f, %f, %f, %f]", val.x, val.y, val.z, val.w);
	Dump(out, name, buffer);
}

template<> void Dump(NWDumpStream& out, LPCSTR name, Quat const & val)
{
	char buffer[256];
	StringCbPrintfA(buffer, sizeof(buffer), "[%f, %f, %f, %f]", val.x, val.y, val.z, val.w);
	Dump(out, name, buffer);
}


template<> void Dump(NWDumpStream& out, LPCSTR name, RHMatrix3 const & val)
{
	char buffer[1024];
	StringCbPrintfA(buffer, sizeof(buffer), "[%f, %f, %f] [%f, %f, %f] [%f, %f, %f]"
		, val.m[0][0], val.m[0][1], val.m[0][2]
	, val.m[1][0], val.m[1][1], val.m[1][2]
	, val.m[2][0], val.m[2][1], val.m[2][2]
	);
	Dump(out, name, buffer);
}

template<> void Dump(NWDumpStream& out, LPCSTR name, RHMatrix4 const & val)
{
	char buffer[1024];
	StringCbPrintfA(buffer, sizeof(buffer), "[%f, %f, %f, %f] [%f, %f, %f, %f] [%f, %f, %f, %f] [%f, %f, %f, %f]"
		, val.m[0][0], val.m[0][1], val.m[0][2], val.m[0][3]
	, val.m[1][0], val.m[1][1], val.m[1][2], val.m[1][3]
	, val.m[2][0], val.m[2][1], val.m[2][2], val.m[2][3]
	, val.m[3][0], val.m[3][1], val.m[3][2], val.m[3][3]
	);
	Dump(out, name, buffer);
}

template<> void Dump(NWDumpStream& out, LPCSTR name, ColorRef const & val)
{
	char buffer[256];
	StringCbPrintfA(buffer, sizeof(buffer), "[%d, %d, %d, %d]", (int)val.a, (int)val.r, (int)val.b, (int)val.g);
	Dump(out, name, buffer);
}

template<> void Dump(NWDumpStream& out, LPCSTR name, Color const & val)
{
	char buffer[256];
	StringCbPrintfA(buffer, sizeof(buffer), "[%f, %f, %f]", (int)val.r, (int)val.b, (int)val.g);
	Dump(out, name, buffer);
}
#endif

template<> bool Recall(NWStream& in, CStr& val) {
	uint len = 0;
	bool ok = Recall(in, len);
	if (ok) {
		val.Resize(len);
		ok = (in.Read(&val[0], 1, len) == len);
	}
	return ok;
}

#if GRANNY2LIB_ENABLE_STORE
template<> bool Store(NWStream& out, CStr const & val) {
	bool ok = Store(out, val.Length());
	if (ok) ok = (out.Write(val.data(), 1, val.Length()) == val.Length());
	return ok;
}
#endif

#if GRANNY2LIB_ENABLE_DUMP
template<> void Dump(NWDumpStream& out, LPCSTR name, CStr const & val) {
	out.Indent();
	out.PrintF("%s: %s\n", name, val.data());
}
#endif

void Init(const VARIABLE* defmap, LPVOID val) {
	for ( ; defmap->ShortName; ++defmap)
		memcpy((LPVOID)((LPBYTE)val + defmap->MemberAddr), defmap->DefaultValue, defmap->ValueSize);
}

bool Recall(const VARIABLE* defmap, NWStream& in, LPVOID val) { 
	int cur = in.Tell();
	cur = cur;
	for ( ; defmap->ShortName; ++defmap) {
		if (defmap->SizeType != NULL) {
			uint size = defmap->SizeType->toSize((LPVOID)((LPBYTE)val + defmap->SizeAddr));
			defmap->MemberType->resize(in, (LPVOID)((LPBYTE)val + defmap->MemberAddr), size);
		}
		int mcur = in.Tell();
		if (!defmap->MemberType->recall(in, (LPVOID)((LPBYTE)val + defmap->MemberAddr))) 
			return false;
	}
	return true;
} 
#if GRANNY2LIB_ENABLE_STORE
bool Store(const VARIABLE* defmap, NWStream& out, LPCVOID val){ 
	bool ok = true;
	for ( ; ok && defmap->ShortName; ++defmap) {
		uint size = defmap->MemberType->toSize((LPVOID)((LPBYTE)val + defmap->MemberAddr));
		defmap->SizeType->resize(out, (LPVOID)((LPBYTE)val + defmap->SizeAddr), size);
	}
	for ( ; ok && defmap->ShortName; ++defmap) 
		ok |= defmap->MemberType->store(out, (LPCVOID)((LPCBYTE)val + defmap->MemberAddr)); 
	return ok; 
} 
#endif

#if GRANNY2LIB_ENABLE_DUMP
void Dump(const VARIABLE* defmap, LPCSTR type, NWDumpStream& out, LPCSTR name, LPCVOID val){ 
	out.Indent();
	out.PrintF("%s: %s\n", type, name);
	++out.IndentLevel;
	for ( ; defmap->ShortName; ++defmap) 
		defmap->MemberType->dump(out, defmap->ShortName, (LPCVOID)((LPCBYTE)val + defmap->MemberAddr)); 
	--out.IndentLevel;
} 
#endif

uint SizeOf(const VARIABLE* defmap, NWStream& in, LPCVOID val) {
	uint size = 0;
	for ( ; defmap->ShortName; ++defmap) 
		size += defmap->MemberType->SizeOf(in, (LPCVOID)((LPCBYTE)val + defmap->MemberAddr)); 
	return size; 
}

}
