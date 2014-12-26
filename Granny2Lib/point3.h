#ifndef _POINT3_H 
#define _POINT3_H

namespace granny2 {

class Point3 
{
public:
	union {
		struct {
	float x,y,z;
		};
		NWN::Vector3 v;
	};
   static const Point3 Origin;

	Point3(){x=y=z=0.0f;}
	Point3(float X, float Y, float Z)  { x = X; y = Y; z = Z;  }
	Point3(double X, double Y, double Z) { x = (float)X; y = (float)Y; z = (float)Z; }
	Point3(int X, int Y, int Z) { x = (float)X; y = (float)Y; z = (float)Z; }
	Point3(const Point3& a) { x = a.x; y = a.y; z = a.z; } 
    float& operator[](int i) { return (&x)[i]; }     
    const float& operator[](int i) const { return (&x)[i]; }  

	void Set(float X, float Y, float Z)  { x = X; y = Y; z = Z;  }

   operator const NWN::Vector3&() const { return v; };
   operator NWN::Vector3&() { return v; };

};

}

#endif

