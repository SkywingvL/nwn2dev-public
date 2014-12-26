#ifndef __STRCLASS__H
#define __STRCLASS__H

namespace granny2 {

//-----------------------------------------------------------------------
// CStr: Simple char string class
//-----------------------------------------------------------------------
class CStr {
	char *buf;
	public:
		CStr(); 
		CStr(const char *cs);
		CStr(const wchar_t *wcstr);
		CStr(const CStr& ws);
		~CStr(); 
		char *data();
		const char *data() const;
		operator char *();

		// realloc to nchars (padding with blanks)
		void Resize(int nchars);

		int Length() const;
		int length() const { return Length(); }
		BOOL isNull() { return Length()==0?1:0; }

		CStr & operator=(const CStr& cs);
		CStr & operator=(const wchar_t *wcstr);
		CStr & operator=(const char *cs);

		// Concatenation operators.
		CStr operator+(const CStr& cs) const;
		CStr& operator+=(const CStr& cs); 
		CStr& Append(const CStr& cs)  { return ((*this) += cs); }
		CStr& append(const CStr& cs)  { return ((*this) += cs); }
		CStr& remove(int pos);	// remove all chars from pos to end
		CStr& remove(int pos, int N);	// remove N chars from pos to end

		// Substring operator
		CStr Substr(int start, int nchars) const;
		char& operator[](int i);
		const char& operator[](int i) const;

		// Char search:(return -1 if not found)
		int first(char c) const;
		int last(char c) const;

		// Comparison
		int operator==(const CStr &cs) const;
		int operator!=(const CStr &cs) const;
		int operator<(const CStr &cs) const;
		int operator<=(const CStr &ws) const;
		int operator>(const CStr &ws) const;
		int operator>=(const CStr &ws) const;

		void toUpper();
		void toLower();

		int printf(const char *format, ...);
		int printfV(const char *format, va_list args);
	};


//-----------------------------------------------------------------------
// WStr: Simple Wide char string class
//-----------------------------------------------------------------------
class WStr {
	wchar_t *buf;
	public:
		WStr();
		WStr(const char *cs);
		WStr(const wchar_t *wcstr);
		WStr(const WStr& ws);
		~WStr();
			wchar_t *data();
			const wchar_t *data() const;
		operator wchar_t *();

		// realloc to nchars (padding with blanks)
		void Resize(int nchars);
		int Length() const;
		int length() const { return Length(); }
		BOOL isNull() { return Length()==0?1:0; }

		WStr & operator=(const WStr& ws);
		WStr & operator=(const wchar_t *wcstr);
		WStr & operator=(const char *cstr);

		// Concatenation operators.
		WStr operator+(const WStr& ws) const; 
		WStr & operator+=(const WStr& ws); 
		WStr& Append(const WStr& ws) { return ((*this) += ws); }
		WStr& append(const WStr& ws)  { return ((*this) += ws); }
		WStr& remove(int pos);	// remove chars from pos to end
		WStr& remove(int pos, int N);	// remove N chars from pos to end

		// Substring operator
		WStr Substr(int start, int nchars) const;
		wchar_t& operator[](int i) {return buf[i];}
		const wchar_t& operator[](int i) const {return buf[i];}

		// Char search:(return -1 if not found)
		int first(wchar_t c) const;
		int last(wchar_t c) const;

		// Comparison
		int operator==(const WStr &ws) const;
		int operator!=(const WStr &ws) const;
		int operator<(const WStr &ws) const;
		int operator<=(const WStr &ws) const;
		int operator>(const WStr &ws) const;
		int operator>=(const WStr &ws) const;

		void toUpper();
		void toLower();
		int printf(const wchar_t *format, ...);
	};					



#ifdef _UNICODE
#define TSTR WStr
#else
#define TSTR CStr
#endif

}

#endif
