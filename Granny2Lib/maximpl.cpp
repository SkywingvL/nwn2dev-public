#include "Precomp.h"
#include "strclass.h"
#include "point2.h"
#include "point3.h"

using namespace granny2;

#ifdef _MSC_VER
#pragma warning(disable:4995) // deprecation warning (ugh, what a total mess...)
#endif


CStr::CStr() : buf(NULL) { }

CStr::CStr(const char *cs) : buf(NULL) 
{ 
   operator=(cs);
}

CStr::CStr(const CStr& ws) : buf(NULL)
{
   operator=(ws);
}
CStr::~CStr()
{
   Resize(0);
}

char *CStr::data() { return buf; }

const char *CStr::data() const { return buf; }

CStr::operator char * (void) { return buf; }

// realloc to nchars (padding with blanks)
void CStr::Resize(int nchars)
{
   if (nchars < 0) throw std::bad_alloc( );
   if (nchars == 0)
   {
      if (buf) free(buf);
      buf = NULL;
   }
   else if (buf)
   {
      size_t len = strlen(buf);
	  if (nchars+1 < nchars) throw std::bad_alloc( );
	  char *newbuf = (char*)realloc(buf, nchars+1);
	  if (newbuf == NULL)
		  throw std::bad_alloc( );
	  buf = newbuf;
      if (len < (size_t)nchars)
         memset(buf+len, 0, (size_t)nchars-len);
   }
   else
   {
      if (nchars+1 < nchars) throw std::bad_alloc( );
      buf = (char*)calloc(nchars+1, 1);
   }
}

int CStr::Length() const { return buf ? (int)strlen(buf) : 0; }

CStr & CStr::operator=(const CStr& cs)
{
   int n = cs.Length();
   Resize(n);
   if (n) StringCbCopyA(buf, n, cs.buf);
   return *this;
}

CStr & CStr::operator=(const char *cs)
{
   int n = cs ? (int)strlen(cs) : 0;
   Resize(n);
   if (n) StringCbCopyA(buf, n, cs);
   return *this;
}

// Concatenation operators.
CStr CStr::operator+(const CStr& cs) const
{
   return CStr(*this).Append(cs);
}

CStr& CStr::operator+=(const CStr& cs)
{
   if (buf)
   {
      Resize(Length() + cs.Length());
      StringCbCopyA(buf+Length(), cs.Length(), cs.data());
   }
   else
   {
      Resize(cs.Length());
      StringCbCopyA(buf, cs.Length(), cs.data());
   }
   return *this;
}

CStr& CStr::remove(int pos)
{
   if (buf) buf[pos] = 0;
   return *this;
}

CStr& CStr::remove(int pos, int N)
{
   if (buf)
   {
      int len = Length();
      int n = min(pos+N, len) - pos;
      memmove(buf+pos, buf+pos+n, n+1);
   }
   return *this;
}

// Substring operator
CStr CStr::Substr(int start, int nchars) const
{
   CStr tmp;
   int len = Length();
   int n = min(start+nchars, len) - start;
   tmp.Resize(n);
   StringCbCopyA(tmp.buf, n, buf+start);
   return tmp;
}

char& CStr::operator[](int i)
{
   return buf[i];
}

const char& CStr::operator[](int i) const
{
   return buf[i];
}

// Char search:(return -1 if not found)
int CStr::first(char c) const
{
   if (!buf) return -1;
   char *v = strchr(buf, c);
   return (v ? (int)(v-buf) : -1);
}

int CStr::last(char c) const
{
   if (!buf) return -1;
   char *v = strrchr(buf, c);
   return (v ? (int)(v-buf) : -1);
}

// Comparison
int CStr::operator==(const CStr &cs) const
{
   if (buf == cs.buf) return true;
   if (!buf || !cs.buf) return false;
   return (strcmp(buf, cs.buf) == 0);
}
int CStr::operator!=(const CStr &cs) const
{
   return !(*this == cs);
}
int CStr::operator<(const CStr &cs) const
{
   if (buf == cs.buf) return false;
   if (!buf) return true;
   if (!cs.buf) return false;
   return (strcmp(buf, cs.buf) < 0);
}
int CStr::operator<=(const CStr &cs) const
{
   if (buf == cs.buf) return true;
   if (!buf) return true;
   if (!cs.buf) return false;
   return (strcmp(buf, cs.buf) <= 0);
}
int CStr::operator>(const CStr &cs) const
{
   return !(*this <= cs);
}

int CStr::operator>=(const CStr &cs) const
{
   return (*this < cs);
}

void CStr::toUpper()
{
   if (buf) _strupr(buf);
}

void CStr::toLower()
{
   if (buf) _strlwr(buf);
}

int CStr::printf(const char *format, ...)
{
   CHAR buffer[512];
   va_list args;
   va_start(args, format);
   size_t l;
   l = _countof(buffer);
   int nChars = _vsnprintf(buffer, _countof(buffer)-1, format, args);
   if (nChars != -1) {
	   buffer[_countof(buffer)-1] = _T('\0');
      *this = buffer;
   } else {
      size_t Size = _vscprintf(format, args);
	  if ((size_t)(int)(Size+1) < Size)
		  throw std::bad_alloc();
      Resize(int(Size+1));
      nChars = _vsnprintf(buf, Size, format, args);
	  buf[Size] = _T('\0');
   }
   va_end(args);
   return nChars;
}

int CStr::printfV(const char *format, va_list args)
{
   CHAR buffer[512];
   size_t l;
   l = _countof(buffer);
   int nChars = _vsnprintf(buffer, _countof(buffer)-1, format, args);
   if (nChars != -1) {
	   buffer[_countof(buffer)-1] = _T('\0');
      *this = buffer;
   } else {
      size_t Size = _vscprintf(format, args);
	  if ((size_t)(int)(Size+1) < Size)
		  throw std::bad_alloc();
      Resize(int(Size+1));
      nChars = _vsnprintf(buf, Size, format, args);
	  buf[Size] = _T('\0');
   }
   return nChars;
}

const Point2 Point2::Origin(0.0f, 0.0f);
const Point3 Point3::Origin(0.0f, 0.0f, 0.0f);
