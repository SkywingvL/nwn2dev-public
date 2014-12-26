#ifndef _COLOR_H 
#define _COLOR_H

#include "point3.h"

namespace granny2 {

class Color {
public:
	union {
		struct {
	float r,g,b;
		};
		NWN::NWNRGB nrgb;
	};

	// Constructors
	Color()  {}
	Color(float R, float G, float B)  { r = R; g = G; b = B;  }
	Color(double R, double G, double B) { r = (float)R; g = (float)G; b = (float)B; }
	Color(int R, int G, int B) { r = (float)R; g = (float)G; b = (float)B; }
	Color(const Color& a) { r = a.r; g = a.g; b = a.b; } 
	explicit Color(DWORD rgb);  // from Windows RGB value
	Color(Point3 p) { r = p.x; g = p.y; b = p.z; }
	Color(float af[3]) { r = af[0]; g = af[1]; b = af[2]; }
	// Access operators
	float& operator[](int i) { return (&r)[i]; }     
	const float& operator[](int i) const { return (&r)[i]; }  

   // Convert to Point3
	operator Point3() { return Point3(r,g,b); }

	operator const NWN::NWNRGB&() const { return nrgb; };
	operator NWN::NWNRGB&() { return nrgb; };

};

}

#endif

