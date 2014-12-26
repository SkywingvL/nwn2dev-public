/**********************************************************************
*<
FILE: NWN2Format.h

DESCRIPTION:	NWN2 File Formats

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#pragma once
#include "DataHeader.h"
#include <pshpack1.h>

namespace granny2 {

typedef unsigned short	ushort;
typedef void*			VoidPtr;
typedef unsigned int	uint;
typedef const unsigned char * LPCBYTE;

template <typename T>
inline T AlignWord(T val)
{
	T m = (val%4) ? 1 : 0;
	return (val/4 + m) * 4;
}

typedef struct RHMatrix3
{
	Point3 m[3];
} RHMatrix3;

C_ASSERT( sizeof( RHMatrix3 ) == 3 * 3 * 4 );

typedef struct RHMatrix4
{
	Point4 m[4];
} RHMatrix4;

C_ASSERT( sizeof( RHMatrix4 ) == 4 * 4 * 4 );

#include <poppack.h>

#pragma region ColorRef
typedef struct ColorRef
{
	BYTE a,b,g,r;

	// Constructors
	ColorRef(){ a=0xFF, b=0, g=0, r=0; }
	ColorRef(COLORREF argb)  { a=GetA(argb), r=GetR(argb), g=GetG(argb), b=GetB(argb); }
	ColorRef(BYTE R, BYTE G, BYTE B, BYTE A=0xFF)  { a=A, r=R, g=G, b=B; }
	ColorRef(float R, float G, float B, float A=1.0f)  { a=ToByte(A), r=ToByte(R), g=ToByte(G), b=ToByte(B); }
	ColorRef(const ColorRef& c) { a = c.a; r = c.r; g = c.g; b = c.b; } 
	ColorRef(float af[3]) { a = 0xFF, r=ToByte(af[0]), g=ToByte(af[1]), b=ToByte(af[2]); }
	ColorRef(const Point3& pt) { a = 0xFF, r=ToByte(pt.x), g=ToByte(pt.y), b=ToByte(pt.z); }
	//ColorRef(const Color& c) { a = 0xFF, r=ToByte(c.r), g=ToByte(c.g), b=ToByte(c.b); }

	static inline BYTE ToByte(float val) { return BYTE(val * 255.0f); }
	static inline float ToFloat(BYTE val) { return float(val) / 255.0f; }
	static inline BYTE GetR(COLORREF argb) { return ((BYTE)(argb)); }
	static inline BYTE GetG(COLORREF argb) { return (LOBYTE(((WORD)(argb)) >> 8)); }
	static inline BYTE GetB(COLORREF argb) { return (LOBYTE((argb)>>16)); }
	static inline BYTE GetA(COLORREF argb) { return ((BYTE)((argb)>>24)); }
	static inline COLORREF ToARGB(BYTE a, BYTE r, BYTE g, BYTE b) {
		return ((COLORREF)((((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16))|(((DWORD)(BYTE)(a))<<24)));
	}

	// Unary operators
	ColorRef operator-() const { return(ColorRef(a,-r,-g,-b)); } 
	ColorRef operator+() const { return *this; }

	// Relational operators
	int operator==(const ColorRef& c) const { return ((a == c.a) && (r == c.r) && (g == c.g) && (b == c.b)); }
	int operator!=(const ColorRef& c) const { return ((a != c.a) || (r != c.r) || (g != c.g) || (b != c.b)); }

	operator Point3() { return Point3(ToFloat(r), ToFloat(g), ToFloat(b)); }
	//operator Color() { return Color(ToFloat(r), ToFloat(g), ToFloat(b)); }
} ColorRef;
#pragma endregion

#pragma region FixedString
/////////////////////////////////////
// FixedString

template<uint size, int padding='\0'>
class FixedString
{
public:
	FixedString() {
		memset(v_, padding, size);
		v_[size] = 0;
	}
	FixedString(const char *string) {
		assign(string);
	}
	FixedString( const FixedString& other ) {
		memcpy(v_, other.v_, size+1);
	}
	FixedString & operator=( const FixedString& other ) {
		memcpy(v_, other.v_, size+1);
		return *this;
	}
	FixedString & operator=( const char * string ) {
		assign( string );
		return *this;
	}
	void assign( const char * string )
	{
		uint len = string ? strlen(string) : 0;
		// assert(len <= size);
		len = min(len, size);
		strncpy(v_, string, len);
		memset(v_+len, padding, size-len);
		v_[size] = 0;
	}
	void clear()
	{
		memset(v_, padding, size);
		v_[size] = 0;
	}
	operator LPSTR() { return v_; }
	operator LPCSTR() const { return v_; }
	char* data() { return v_; }
	const char *c_str() const { return v_; }

private:
	char v_[size+1];  //! NWArray of data.  Very important that this is only data for this class.
};

template<uint fixedsize, int padding>
inline bool operator==( FixedString<fixedsize, padding>& lhs, const char *rhs) {
	return (strncmp(lhs.data(), rhs, fixedsize) == 0) ? true : false;
}

template<uint fixedsize, int padding>
inline bool operator==( const char *lhs, FixedString<fixedsize, padding>& rhs) {
	return (strncmp(lhs, rhs.data(), fixedsize) == 0) ? true : false;
}

template<uint fixedsize, int padding>
inline bool operator==( FixedString<fixedsize, padding>& lhs, FixedString<fixedsize, padding>& rhs) {
	return (strncmp(lhs.data(), rhs.data(), fixedsize) == 0) ? true : false;
}

template<uint fixedsize, int padding>
inline bool operator!=( FixedString<fixedsize, padding>& lhs, const char *rhs) {
	return (strncmp(lhs.data(), rhs, fixedsize) == 0) ? false : true ;
}

template<uint fixedsize, int padding>
inline bool operator!=( const char *lhs, FixedString<fixedsize, padding>& rhs) {
	return (strncmp(lhs, rhs.data(), fixedsize) == 0) ? false : true;
}

template<uint fixedsize, int padding>
inline bool operator!=( FixedString<fixedsize, padding>& lhs, FixedString<fixedsize, padding>& rhs) {
	return (strncmp(lhs.data(), rhs.data(), fixedsize) == 0) ? false : true;
}
#pragma endregion

#pragma region NWArray
/////////////////////////////////////
// NWArray
template <typename T>
class NWArray : public std::vector<T>
{
public:
	NWArray() { }
	NWArray(int length) : std::vector<T>(length) {}
	NWArray(int capacity, int length) : std::vector<T>(capacity, length) { }
	NWArray(const NWArray& other) : std::vector<T>( other ) {}

	//! Default Destructor
	~NWArray() {}

	T& appendNew()
	{
		int index = size();
		push_back( T() );
		return back();
	}

	void append(T& val) {
		push_back( val );
	}

	int size() const { return int(std::vector<T>::size()); }
	int count() const { return size(); }
	int length() const { return size(); }
};

template <typename T>
void DeleteContents(NWArray<T>& array) {
	for (int i=0, n=array.size(); i<n; ++i)
		delete array[i];
	array.resize(0);
}
#pragma endregion

#pragma region Text
/////////////////////////////////////
// Text
class Text;

class Text
{
public:
	Text() : s(0), n(0) { }

	Text(const char *string) {
		assign(string);
	}

	Text( const Text& other ) : s(0), n(0) {
		assign(other.s);
	}

	Text & operator=( const Text& other ) {
		assign(other.s);
		return *this;
	}
	Text & operator=( const char * string ) {
		assign( string );
		return *this;
	}

	void assign( const char * string )
	{
		size_t len = string ? strlen(string) : 0;
		if (len == 0)
		{
			clear();
		}
		else 
		{
			if (len != (size_t)(unsigned int)len) throw std::bad_alloc( );
			reserve((unsigned int)len);
			StringCbCopyA(s, len+1, string);
		}
	}

	void append( const char * string )
	{
		size_t len = (string ? strlen(string) : 0) +  (s ? strlen(s) : 0);
		if (len == 0)
		{
			clear();
		}
		else 
		{
			if (len != (size_t)(unsigned int)len) throw std::bad_alloc( );
			reserve((unsigned int)len);
			StringCbCatA(s, len+1, string);
		}
	}

	void append( char c )
	{
		size_t l = s ? strlen( s ) : 0;
		unsigned int len = (unsigned int) (1 + (l));
		if ((l < l+1) || (size_t)(1 + l) != (size_t)len) throw std::bad_alloc( );
		if (len == 0)
		{
			clear();
		}
		else 
		{
			reserve(len);
			char *p = s;
			*p++ = c;
			*p = 0;
		}
	}

	void append( char *string, uint len )
	{
		size_t l1 = strlen(string);
		size_t l = l1 + size();
		if (l < l1) throw std::bad_alloc( );
		if (l == 0)
		{
			clear();
		}
		else 
		{
			if (l != (size_t)(unsigned int)l) throw std::bad_alloc( );
			reserve((unsigned int)l);
			StringCbCatA(s, l+1, string);
		}
	}

	void reserve( unsigned int len )
	{
		if (len >= n)
		{
			char *olds = s;
			unsigned int oldn = n;
			unsigned int a = AlignWord(oldn * 3 / 2), b = AlignWord(len+1);
			if (a < oldn) throw std::bad_alloc( );
			if (b < len) throw std::bad_alloc( );
			n = max(a, b);
			s = (char*)calloc(n, sizeof(char));
			if (s == NULL) throw std::bad_alloc( );
			if (olds)
			{
				StringCbCopyA(s, oldn, olds);
				free(olds);
			}
		}
	}

	void clear()
	{
		if (s)
		{
			free(s);
			s = NULL;
		}
		n = 0;
	}

	unsigned int size() const 
	{
		return (s ? (unsigned int)strlen(s) : 0);
	}

	unsigned int capacity() const 
	{
		return n;
	}

	operator LPSTR() { return s; }
	operator LPCSTR() const { return s; }
	char* data() { return s; }
	const char *c_str() const { return s; }

private:
	unsigned int n;
	char *s;
};


inline bool operator==( Text& lhs, const char *rhs) {
	return (strcmp(lhs.data(), rhs) == 0) ? true : false;
}

inline bool operator==( const char *lhs, Text& rhs) {
	return (strcmp(lhs, rhs.data()) == 0) ? true : false;
}

inline bool operator==( Text& lhs, Text& rhs) {
	return (strcmp(lhs.data(), rhs.data()) == 0) ? true : false;
}

inline bool operator!=( Text& lhs, const char *rhs) {
	return (strcmp(lhs.data(), rhs) == 0) ? false : true ;
}

inline bool operator!=( const char *lhs, Text& rhs) {
	return (strcmp(lhs, rhs.data()) == 0) ? false : true;
}

inline bool operator!=( Text& lhs, Text& rhs) {
	return (strcmp(lhs.data(), rhs.data()) == 0) ? false : true;
}
#pragma endregion

#pragma region ValuePtr
// --------------------------------------------------------------------
//
// Auxiliary traits class template providing default clone()
//   Users should specialize this template for types that have their
//   own self-copy operations; failure to do so may lead to slicing!
//
// --------------------------------------------------------------------
template< class T >
struct VPTraits  {
	static  T *  create()  { return new T(); }
	static  T *  clone( T const * p )  { return new T( *p ); }
};  // VPTraits


// --------------------------------------------------------------------
//
// Copyable smart pointer class template
//
// --------------------------------------------------------------------
template< class T >
class ValuePtr  {
public:
	// --------------------------------------------------
	// Default constructor/destructor:
	// --------------------------------------------------
	explicit ValuePtr( ) : myP ( VPTraits<T>::create() ) { ; }
	explicit ValuePtr( T * p ) : myP( p )  { ; }
	~ValuePtr()  { delete myP; myP = 0; }

	// --------------------------------------------------
	// Copy constructor/copy assignment:
	// --------------------------------------------------

	ValuePtr( ValuePtr const & orig )
		: myP( createFrom( orig.myP ) )
	{}

	ValuePtr &  operator = ( ValuePtr const & orig )  {
		ValuePtr<T>  temp( orig );
		swap( temp );
		return *this;
	}

	// --------------------------------------------------
	// Access mechanisms:
	// --------------------------------------------------
	bool isNull() const { return (myP == NULL); }

	T &  operator *  () const  { return *myP; }
	T *  operator -> () const  { return  myP; }

	T *  ToPointer() const     { return  myP; }

	// --------------------------------------------------
	// Manipulation:
	// --------------------------------------------------

	void  swap( ValuePtr & orig )  { std::swap( myP, orig.myP ); }

	// --------------------------------------------------
	// Copy-like construct/assign from compatible ValuePtr<>:
	// --------------------------------------------------

	template< class U >
	ValuePtr( ValuePtr<U> const & orig )
		: myP( createFrom( orig.operator->() ) )
	{}

	template< class U >
	ValuePtr &  operator = ( ValuePtr<U> const & orig )  {
		ValuePtr<T>  temp( orig );
		swap( temp );
		return *this;
	}

private:

	// --------------------------------------------------
	// Implementation aid:
	// --------------------------------------------------

	template< class U >
	T *  createFrom( U const * p ) const  {
		return p
			? VPTraits<U>::clone( p )
			: 0;
	}

	// --------------------------------------------------
	// Member data:
	// --------------------------------------------------

	T *  myP;

};  // ValuePtr


// --------------------------------------------------------------------
//
// Free-standing swap()
//
// --------------------------------------------------------------------

template< class T >
inline
	void  swap(  ValuePtr<T> & vp1, ValuePtr<T> & vp2 )  {
		vp1.swap( vp2 );
}
#pragma endregion

#pragma region array
/*! Used to enable static arrays to be members of vectors */
template<int size, class T>
struct array {
	array() {
		for ( uint i = 0; i < size; ++i ) {
			data[i] = T();
		}
	}
	~array() {}
	T & operator[]( unsigned int index ) {
		return data[index];
	}
	const T & operator[]( unsigned int index ) const {
		return data[index];
	}

	uint count() const 
	{
		return uint(size);
	}

private:
	T data[size];
	static const int _size = size;
};
#pragma endregion

//
// XXX: Replace with memory-mapped I/O as the seek load is horrendously
//      detrimental to performance.
//

#pragma region NWStream
class NWStream
{
public:
	NWStream() : fh(NULL), view(NULL){}

	NWStream(const CHAR *file, bool readonly) : fh(NULL),view(NULL) { 
		Open(file, readonly); 
	}
	~NWStream() { 
		Close(); 
	}

	void Open(const CHAR *file, bool readonly);
	void Close();

	uint Read(char *buf, uint size, uint count = 1);
	uint Write(char const * buf, uint size, uint count = 1);
	int Seek(int whence, long offset);
	int Tell();
	int TellEnd();

	bool Eof() const;

	void *&pdata() { return data; }

	void* data;
	FILE* fh;
	void* view;
	int   offset;
	int   size;
};

#if GRANNY2LIB_ENABLE_DUMP
class NWDumpStream : public NWStream
{
public:
	NWDumpStream() : NWStream(), IndentLevel(0) {}

	NWDumpStream(const CHAR *file, bool readonly) : NWStream(file, readonly), IndentLevel(0) { 
	}

	~NWDumpStream() { 
	}

	void Print(char const * buf);
	void PrintF(const char *format, ...);
	void NewLine();

	void Indent();

	int IndentLevel;
};
#endif
#pragma endregion

#pragma region Recall|Store|Dump|SizeOf
//////////////////////////////////////////////////////////////////////////

template<typename T>
bool Recall(NWStream& in, T& val) {
	return (in.Read((char*)&val, sizeof(T)) == 1);
}

template<uint size, int padding>
bool Recall(NWStream& in, FixedString<size, padding>& val) {
	return (in.Read(val, size) == 1);
}

template<typename T>
bool Recall(NWStream& in, NWArray<T>& val) 
{
	bool ok = true;
	for (int i=0, n=val.count(); i<n && ok; ++i) {
		ok = Recall(in, val[i]);
	}
	return ok;
}

template<typename T>
bool Recall(NWStream& in, ValuePtr<T>& val);

#if GRANNY2LIB_ENABLE_STORE
template<typename T>
bool Store(NWStream& out, T const & val) {
	return (out.Write((const char*)&val, sizeof(T)) == 1);
}

template<uint size, int padding>
bool Store(NWStream& out, FixedString<size, padding> const & val) {
	return (out.Write(val, size) == 1);
}

template<typename T>
bool Store(NWStream& out, NWArray<T> const & val) 
{
	bool ok = true;
	for (int i=0, n=val.count(); i<n && ok; ++i) {
		ok = ::Store(out, val[i]);
	}
	return ok;
}


template<typename T>
bool Store(NWStream& out, ValuePtr<T>& val);
#endif

#if GRANNY2LIB_ENABLE_DUMP
template<typename T>
void Dump(NWDumpStream& out, LPCSTR name, T const & val)
{
	std::stringstream sstr;
	sstr << val;
	out.Indent();
	out.PrintF("%s: %s\n", name, sstr.str().c_str());
}

template<typename T>
void DumpArray(NWDumpStream& out, LPCSTR name, T* const val, uint size) 
{
	out.Indent();
	out.PrintF("%s: (size=%d)\n", name, size);
	++out.IndentLevel;
	uint len = 0;
	char buffer[100];
	for (uint i=0, n=size; i<n; ++i) {
		StringCbPrintfA(buffer, sizeof(buffer), "[%d]", i);
		Dump(out, buffer, val[i]);
	}
	--out.IndentLevel;
}

template<typename T> 
void Dump(NWDumpStream& out, LPCSTR name, const NWArray<T> & val)
{
	if (val.size() == 0)
		out.Indent(), out.PrintF("%s: (size=%d)\n", name, val.size());
	else
		DumpArray(out, name, &val[0], val.count());
}

template<typename T>
void Dump(NWDumpStream& out, LPCSTR name, const ValuePtr<T>& val);
#endif

#pragma region array
template<int size, typename T>
bool Recall(NWStream& in, array<size, T>& val) 
{
	bool ok = true;
	for (int i=0; i<size && ok; ++i) {
		ok = Recall(in, val[i]);
	}
	return ok;
}

#if GRANNY2LIB_ENABLE_STORE
template<int size, typename T>
bool Store(NWStream& out, array<size, T> const & val) 
{
	bool ok = true;
	for (int i=0; i<size && ok; ++i) {
		ok = Store(out, val[i]);
	}
	return ok;
}
#endif

#if GRANNY2LIB_ENABLE_DUMP
template<int size, typename T> 
void Dump(NWDumpStream& out, LPCSTR name, array<size, T> const & val) 
{
	if (size == 0)
		out.Indent(), out.PrintF("%s: (size=%d)\n", name, size);
	else
		DumpArray(out, name, &val[0], size);
}
#endif

#pragma endregion

#if GRANNY2LIB_ENABLE_DUMP
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, char const & val);
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, unsigned char const & val);
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, float const & val);
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, Point2 const & val);
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, Point3 const & val);
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, Point4 const & val);
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, Quat const & val);
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, RHMatrix3 const & val);
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, RHMatrix4 const & val);
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, ColorRef const & val);
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, Color const & val);
#endif

template<> bool granny2::Recall(NWStream& in, CStr& val);
#if GRANNY2LIB_ENABLE_STORE
template<> bool granny2::Store(NWStream& out, CStr const & val);
#endif
#if GRANNY2LIB_ENABLE_DUMP
template<> void granny2::Dump(NWDumpStream& out, LPCSTR name, CStr const & val);
#endif

#pragma endregion 

#pragma region SizeOf

template<typename T>
unsigned int SizeOf(NWStream& in, T const & val) {
	return sizeof(T);
}

template<uint size, int padding>
unsigned int SizeOf(NWStream& in, FixedString<size, padding> const & val) {
	return size;
}

template<typename T>
unsigned int SizeOf(NWStream& in, NWArray<T> const & val) 
{
	uint len = 0;
	for (int i=0, n=val.count(); i<n; ++i) {
		len += SizeOf(in, val[i]);
	}
	return len;
}

template<> inline 
unsigned int SizeOf(NWStream& in, CStr const& val) {
	return sizeof(int) + val.Length();
}

#pragma endregion 

#pragma region Resize

template<typename T>
inline void Resize(NWStream& in, T & val, uint cnt) {}

template<typename T>
inline void Resize(NWStream& in, NWArray<T> & val, uint cnt) {
	val.resize(cnt);
}

template<> inline void Resize<char>(NWStream& in, char & val, uint cnt) { val = cnt; }
template<> inline void Resize<short>(NWStream& in, short & val, uint cnt) { val = cnt; }
template<> inline void Resize<int>(NWStream& in, int & val, uint cnt) { val = cnt; }
template<> inline void Resize<unsigned char>(NWStream& in, unsigned char & val, uint cnt) { val = cnt; }
template<> inline void Resize<unsigned short>(NWStream& in, unsigned short & val, uint cnt) { val = cnt; }
template<> inline void Resize<unsigned int>(NWStream& in, unsigned int & val, uint cnt) { val = cnt; }

#pragma endregion 

#pragma region ToSize

template<typename T> uint ToSize(T const & val) { return 0; }
template<> inline uint ToSize<char>(char const & val) { return val; }
template<> inline uint ToSize<short>(short const & val) { return val; }
template<> inline uint ToSize<int>(int const & val) { return val; }
template<> inline uint ToSize<unsigned char>(unsigned char const & val) { return val; }
template<> inline uint ToSize<unsigned short>(unsigned short const & val) { return val; }
template<> inline uint ToSize<unsigned int>(unsigned int const & val) { return val; }

template<typename T>
uint ToSize(NWArray<T> const & val) {
	return val.size();
}

#pragma endregion 

#pragma region Enumeration support
// Enumeration support
typedef struct EnumLookupType {
	int value;
	const char *name;
} EnumLookupType;

extern CStr EnumToString(int value, const EnumLookupType *table);
extern int StringToEnum(CStr value, const EnumLookupType *table);
extern CStr FlagsToString(int value, const EnumLookupType *table);
extern int StringToFlags(CStr value, const EnumLookupType *table);
#pragma endregion

#pragma region Helpers
// sprintf for CStr without having to worry about buffer size.
extern CStr FormatText(const CHAR* format,...);

// sprintf for std::string without having to worry about buffer size.
extern std::string FormatString(const CHAR* format,...);

extern CStr& Trim(CStr&p);

#ifndef M_PI
const float M_PI = 3.14159265358979323846f;
#endif

inline BYTE GetR(COLORREF argb) { return ((BYTE)(argb)); }
inline BYTE GetG(COLORREF argb) { return (LOBYTE(((WORD)(argb)) >> 8)); }
inline BYTE GetB(COLORREF argb) { return (LOBYTE((argb)>>16)); }
inline BYTE GetA(COLORREF argb) { return ((BYTE)((argb)>>24)); }

inline COLORREF ToARGB(BYTE a, BYTE r, BYTE g, BYTE b)
{
	return ((COLORREF)((((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16))|(((DWORD)(BYTE)(a))<<24)));
}

extern Matrix3 TOMATRIX(Quat const& q);

extern Point3 TOEULER(const Matrix3 &m);

static inline float TODEG(float x) { return float(x * 180.0f / M_PI); }
static inline float TORAD(float x) { return float(x * M_PI / 180.0f); }

inline Point3 TODEG(const Point3& p){
	return Point3(TODEG(p[0]), TODEG(p[1]), TODEG(p[2]));
}

inline Point3 TORAD(const Point3& p){
	return Point3(TORAD(p[0]), TORAD(p[1]), TORAD(p[2]));
}
#pragma endregion

#pragma region TYPE

struct TYPE
{
	virtual bool recall(NWStream& in, LPVOID val) = 0;
#if GRANNY2LIB_ENABLE_STORE
	virtual bool store(NWStream& out, LPCVOID val) = 0;
#endif
#if GRANNY2LIB_ENABLE_DUMP
	virtual void dump(NWDumpStream& out, LPCSTR name, LPCVOID val) = 0;
#endif
	virtual unsigned int SizeOf(NWStream& in, LPCVOID val) = 0;
	virtual void resize(NWStream& in, LPVOID val, uint cnt) = 0;
	virtual uint toSize(LPCVOID val) = 0;
};

template<typename U>
struct VTYPE : public TYPE
{
	bool recall(NWStream& in, LPVOID val)				{ return granny2::Recall(in, *((U*)val)); }
#if GRANNY2LIB_ENABLE_STORE
	bool store(NWStream& out, LPCVOID val)				{ return granny2::Store(out, *((U const *)val)); }
#endif
#if GRANNY2LIB_ENABLE_DUMP
	void dump(NWDumpStream& out, LPCSTR name, LPCVOID val){ granny2::Dump(out, name, *((U const *)val)); }
#endif
	unsigned int SizeOf(NWStream& str, LPCVOID val)			{ return granny2::SizeOf(str, *((U const *)val)); }
	void resize(NWStream& in, LPVOID val, uint cnt)	{ granny2::Resize(in, *((U *)val), cnt); }
	uint toSize(LPCVOID val)							{ return granny2::ToSize(*((U const *)val)); }
};

struct VARIABLE
{
	VARIABLE() : ShortName(NULL), MemberAddr(0), Description(NULL), DefaultValue(NULL), ValueSize(0), MemberType(NULL) {}

	template<typename U>
	VARIABLE(LPCSTR sName, U const & member, U const & default, LPCSTR desc) 
		: ShortName(sName), Description(desc)
		, MemberAddr((LPCBYTE)&member - (LPCBYTE)0), MemberType(new VTYPE<U>()) 
		, SizeAddr(0), SizeType(NULL)
	{
		SetDefault(default);
	}

	template<typename U>
	VARIABLE(LPCSTR sName, U const & member, LPCSTR desc) 
		: ShortName(sName), Description(desc)
		, MemberAddr((LPCBYTE)&member - (LPCBYTE)0), MemberType(new VTYPE<U>()) 
		, SizeAddr(0), SizeType(NULL)
	{}

	template<typename T, typename U>
	VARIABLE(LPCSTR sName, NWArray<U> const & member, T const & size, LPCSTR desc) 
		: ShortName(sName), Description(desc)
		, MemberAddr((LPCBYTE)&member - (LPCBYTE)0), MemberType(new VTYPE<NWArray<U> >()) 
		, SizeAddr((LPCBYTE)&size - (LPCBYTE)0), SizeType(new VTYPE<T>())
	{}

	~VARIABLE() {
		if (ValueSize && DefaultValue) {
			delete DefaultValue; DefaultValue = NULL; ValueSize = 0;
		}
		if (MemberType) {
			delete MemberType; MemberType = NULL;
		}
	}

	template<typename U> void SetDefault(U default) { ValueSize = sizeof(U); DefaultValue = new U(default); }
	template <typename U> const U& GetDefault() const { return *(U*)DefaultValue; }

	LPCSTR ShortName;
	size_t MemberAddr;
	TYPE *MemberType;
	LPCSTR Description;
	LPVOID DefaultValue;
	DWORD ValueSize;
	DWORD SizeAddr;
	TYPE *SizeType;
};

#define INITVARTYPE(T) \
	T() { granny2::Init(this->GetInfDefmap(), (LPVOID)this); } \

#define BEGIN_INI_MAP_NOINIT(T) \
	LPCSTR GetTypeName() const { return #T; } \
	void *GetMember(const VARIABLE* memptr) const { return (void*)(((char*)this) + memptr->MemberAddr); } \
	const VARIABLE* GetInfDefmap() const { return InternalGetInfDefmap(); } \
	static VARIABLE* InternalGetInfDefmap() { \
	const T* pThis = 0; static VARIABLE map[] = { \

#define BEGIN_INI_MAP(T) \
	INITVARTYPE(T) \
	BEGIN_INI_MAP_NOINIT(T) \

#define BEGIN_BASE_MAP(T) \
	T() { granny2::Init(this->GetInfDefmap(), (LPVOID)this); } \
	virtual LPCSTR GetTypeName() const { return #T; } \
	virtual void *GetMember(const VARIABLE* memptr) const { return (void*)(((char*)this) + memptr->MemberAddr); } \
	virtual const VARIABLE* GetInfDefmap() const { return InternalGetInfDefmap(); } \
	static VARIABLE* InternalGetInfDefmap() { \
	const T* pThis = 0; static VARIABLE map[] = { \

#define END_INI_MAP() \
	VARIABLE() };\
	return map;} \

#define ADDITEM(member, defval) \
	VARIABLE(#member, pThis->member, defval, #member), \

#define ADDITEMEX(sName, member, defval, desc) \
	VARIABLE(sName, pThis->member, defval, desc), \

#define ADDCLASS(member) \
	VARIABLE(#member, pThis->member, #member), \

#define ADDARRAY(member, size) \
	VARIABLE(#member, pThis->member, pThis->size, #member), \

#define ROOTVARTYPE(T) \
	template<> bool granny2::Recall<T>(NWStream& in, T& val); \
/*	template<> bool granny2::Store<T>(NWStream& out, T const & val); */\
/*	template<> void granny2::Dump<T>(NWDumpStream& out,  LPCSTR name, T const & val); */\
	template<> unsigned int granny2::SizeOf<T>(NWStream& in, T const & val); \

#define DEFVARTYPE(N, T) \
	namespace N { struct T; } \
	template<> bool granny2::Recall<N::T>(NWStream& in, N::T& val); \
/*	template<> bool granny2::Store<N::T>(NWStream& out, N::T const & val); */\
/*	template<> void granny2::Dump<N::T>(NWDumpStream& out,  LPCSTR name, N::T const & val); */\
	template<> unsigned int granny2::SizeOf<N::T>(NWStream& in, N::T const & val); \

#define IMPLVARTYPE(N, T) \
	template<> bool granny2::Recall<N::T>(NWStream& in, N::T& val) { \
	return Recall(val.GetInfDefmap(), in, static_cast<LPVOID>(&val)); \
	} \
/*	template<> bool granny2::Store<N::T>(NWStream& out, N::T const & val) {*/\
/*	return Store(val.GetInfDefmap(), out, static_cast<LPCVOID>(&val)); */\
/*	} */\
/*	template<> void granny2::Dump<N::T>(NWDumpStream& out,  LPCSTR name, N::T const & val) { */\
/*	Dump(val.GetInfDefmap(), val.GetTypeName(), out, name, static_cast<LPCVOID>(&val)); */\
/*	} */\
	template<> unsigned int granny2::SizeOf<N::T>(NWStream& in, N::T const & val) { \
	return SizeOf(val.GetInfDefmap(), in, static_cast<LPCVOID>(&val)); \
	} \

//Find:  [a-zA-Z_0-9 \<\>\*]+:b+{:i}:b*;.*
//Repl:	 ADDCLASS(\1)

extern void Init(const VARIABLE* defmap, LPVOID val);
extern bool Recall(const VARIABLE* defmap, NWStream& in, LPVOID val);
#if GRANNY2LIB_ENABLE_STORE
extern bool Store(const VARIABLE* defmap, NWStream& out, LPCVOID val);
#endif
#if GRANNY2LIB_ENABLE_DUMP
extern void Dump(const VARIABLE* defmap, LPCSTR type, NWDumpStream& out,  LPCSTR name, LPCVOID val); 
#endif
extern unsigned int SizeOf(const VARIABLE* defmap, NWStream& in, LPCVOID val); 

#pragma endregion

}


