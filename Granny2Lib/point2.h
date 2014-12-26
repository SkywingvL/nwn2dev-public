#ifndef __POINT2__ 
#define __POINT2__

namespace granny2 {

class Point2 
{
public:
	union {
		struct {
	float x,y;
		};
		NWN::Vector2 v;
	};
   static const Point2 Origin;

	Point2() { x=y=0.0f; }
	Point2(float X, float Y)  { x = X; y = Y;  }
	Point2(double X, double Y)  { x = (float)X; y = (float)Y;  }
	Point2(const Point2& a) { x = a.x; y = a.y; } 
	Point2(float af[2]) { x = af[0]; y = af[1]; }
   float& operator[](int i) { return (&x)[i]; }     
   const float& operator[](int i) const { return (&x)[i]; }  

   operator const NWN::Vector2&() const { return v; };
   operator NWN::Vector2&() { return v; };

};

}

#endif