#ifndef _QUAT_H 
#define _QUAT_H 

namespace granny2 {

class Quat 
{
public:
	union {
		struct {
	float x,y,z,w;
		};
		NWN::Quaternion Q;
	};

	// Constructors
	Quat(): x(0.0f),y(0.0f),z(0.0f),w(1.0f) {}
	Quat(float X, float Y, float Z, float W)  { x = X; y = Y; z = Z; w = W; }
	Quat(double X, double Y, double Z, double W)  { 
		x = (float)X; y = (float)Y; z = (float)Z; w = (float)W; 
		}
	Quat(const Quat& a) { x = a.x; y = a.y; z = a.z; w = a.w; } 
	Quat(float af[4]) { x = af[0]; y = af[1]; z = af[2]; w = af[3]; }

	// Access operators
	float& operator[](int i) { return (&x)[i]; }     
	const float& operator[](int i) const { return (&x)[i]; }
    
	float Scalar() { return w; }
	Point3 Vector() { return Point3(x, y, z); }

	// Conversion function
	operator float*() { return(&x); }

	// Unary operators
	Quat operator-() const { return(Quat(-x,-y,-z,-w)); } 
	Quat operator+() const { return *this; }
    
	Quat& Set(float X, float Y, float Z, float W)
		{ x = X; y = Y; z = Z; w = W; return *this; }
	Quat& Set(double X, double Y, double Z, double W)
		{ x = (float)X; y = (float)Y; z = (float)Z; w = (float)W;
		return *this; }
	Quat& Set(const Point3& V, float W)
		{ x = V.x; y = V.y; z = V.z; w = W; return *this; } 

	void Identity() { x = y = z = (float)0.0; w = (float) 1.0; }

	operator const NWN::Quaternion&() const { return Q; };
	operator NWN::Quaternion&() { return Q; };

};

}

#endif _QUAT_H 
