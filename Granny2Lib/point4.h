/**********************************************************************
 *<
	FILE: point4.h

	DESCRIPTION: Class definitions for Point4

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef _POINT4_H 
#define _POINT4_H

namespace granny2 {

class Point4 {
public:
	float x,y,z,w;
   static const Point4 Origin;

	// Constructors
	Point4(){ x=y=z=w=0.0f; }
	Point4(float X, float Y, float Z, float W)  { x = X; y = Y; z = Z; w = W; }
	Point4(double X, double Y, double Z, double W) { x = (float)X; y = (float)Y; z = (float)Z; w = (float)W; }
	Point4(int X, int Y, int Z, int W) { x = (float)X; y = (float)Y; z = (float)Z; w = (float)W; }
	Point4(const Point3& a, float W=0.0f) { x = a.x; y = a.y; z = a.z; w = W; } 
	Point4(const Point4& a) { x = a.x; y = a.y; z = a.z; w = a.w; } 
	Point4(float af[4]) { x = af[0]; y = af[1]; z = af[2]; w = af[3]; }
   float& operator[](int i) { return (&x)[i]; }     
   const float& operator[](int i) const { return (&x)[i]; }  
};

}

#endif

