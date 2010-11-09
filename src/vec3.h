#ifndef _VEC3_H_
#define _VEC3_H_

#include <cmath>
#include <iostream>

class Vec3 {
public:
	double x;
	double y;
	double z;
	
	Vec3 () { x = y = z = 0.0; }
	Vec3 (double _x, double _y, double _z): x(_x), y(_y), z(_z) {}
	
	void Normalize ()
	{
		double l = 1.0/sqrt(x*x + y*y + z*z);
		x*=l; y*=l; z*=l;
	}
	
	double Length() const
	{ return sqrt(x*x + y*y + z*z); }

	const Vec3 operator- () const
	{ return Vec3 (-x, -y, -z); }
	
    void Flip ()
    { x=-x; y=-y; z=-z; }
    
    const Vec3 operator- (const Vec3 &vec) const
    { return Vec3 (x-vec.x, y-vec.y, z-vec.z); }

    const Vec3 operator+ (const Vec3 &vec) const
    { return Vec3 (x+vec.x, y+vec.y, z+vec.z); }
    
    const Vec3 operator* (double fact) const
    { return Vec3 (x*fact, y*fact, z*fact); }
    
    const Vec3 operator/ (double fact) const
    { double xfact = 1./fact; return Vec3 (x*xfact, y*xfact, z*xfact); }
    
    Vec3 &operator*= (double fact)
    { x*=fact; y*=fact; z*=fact; return *this; }

    double Dot(const Vec3 &v1, const Vec3 &v2)
    { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }

    Vec3 Cross(const Vec3 &a, const Vec3 &b)
    { return Vec3 (a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }

    /*
     * std::ostream &operator<< (std::ostream &os, const Vec3 &v)
     * {
	 *     os << v.x << ", " << v.y << ", " << v.z << std::endl;
	 *     return os;
     * }
     */
};

#endif
