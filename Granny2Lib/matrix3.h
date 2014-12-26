#ifndef _MATRIX3_H 

#define _MATRIX3_H 

#include "point3.h"
#include "point4.h"

//Flags
#define POS_IDENT  1
#define ROT_IDENT  2
#define SCL_IDENT  4
#define MAT_IDENT (POS_IDENT|ROT_IDENT|SCL_IDENT)

namespace granny2 {

typedef float MRow[3];

class Quat;
class Matrix3 {
	friend class Quat;
	float m[4][3];
	// Access i-th row as Point3 for read or assignment:
	Point3& operator[](int i) { return((Point3&)(*m[i]));  }
	DWORD flags;

public:
	const Point3& operator[](int i) const { return((Point3&)(*m[i])); }
	// if change any components directly via GetAddr, must call this
	void SetNotIdent() { flags &= ~MAT_IDENT; }
	void SetIdentFlags(DWORD f) { flags &= ~MAT_IDENT; flags |= f; }
	DWORD GetIdentFlags() const { return flags; }
	void ClearIdentFlag(DWORD f) { flags &= ~f; }
	BOOL IsIdentity() const { return ((flags&MAT_IDENT)==MAT_IDENT); }

	// CAUTION: if you change the matrix via this pointer, you MUST clear the
	// proper IDENT flags !!!
	MRow* GetAddr() { return (MRow *)(m); }
	const MRow* GetAddr() const { return (MRow *)(m); }

	// Constructors
	Matrix3(){ flags = 0; }	 // NO INITIALIZATION done in this constructor!! 				 
	Matrix3(BOOL init) {flags=0; IdentityMatrix();} // RB: An option to initialize
    Matrix3(const Point3& U, const Point3& V, const Point3& N, const Point3& T) {
        flags = 0; SetRow(0, U); SetRow(1, V); SetRow(2, N); SetRow(3, T);
        ValidateFlags(); }

    Matrix3& Set(const Point3& U, const Point3& V, const Point3& N, const Point3& T) {
        flags = 0; SetRow(0, U); SetRow(1, V); SetRow(2, N); SetRow(3, T);
        ValidateFlags(); return *this; }

    // Data member
    //static const Matrix3 Identity;

	Point3 GetRow(int i) const { return (*this)[i]; }	

	// Access the translation row
	void SetTrans(const Point3 p) { (*this)[3] = p;  flags &= ~POS_IDENT; }
	void SetTrans(int i, float v) { (*this)[3][i] = v; flags &= ~POS_IDENT; }
	Point3 GetTrans() const { return (*this)[3]; }  

	void IdentityMatrix() {
		m[0][0] = m[1][1] = m[2][2] = 1.0f;
		m[1][0] = m[2][0] = m[0][1] = m[2][1] = m[0][2] = m[1][2] = 0.0f;
	}
	void SetRow(int i,	Point3 p) {
		m[i][0] = p.x; m[i][1] = p.y; m[i][2] = p.z;
	}
	void ValidateFlags() {}

	operator NWN::Matrix44() const
	{
		NWN::Matrix44 M = NWN::Matrix44::IDENTITY;

		M._00 = m[0][0];
		M._01 = m[0][1];
		M._02 = m[0][2];
		M._10 = m[1][0];
		M._11 = m[1][1];
		M._12 = m[1][2];
		M._20 = m[2][0];
		M._21 = m[2][1];
		M._22 = m[2][2];
		M._30 = m[3][0];
		M._31 = m[3][1];
		M._32 = m[3][2];
	}
};

}

#endif _MATRIX3_H 
