#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "vec3.h"

const float PI = 3.14159265;
const float EPSILON = 0.0000001;

class CCamera {
	Vec3 m_position;
	Vec3 m_target;
	Vec3 m_up;
	float m_radius;
	float m_h_radians;
	float m_v_radians;
	
	void ClampVRadians();
public:
	CCamera();
	CCamera(const Vec3 &, const Vec3 &, const Vec3 &);
	~CCamera();

	Vec3 &GetPosition() { return m_position; }
	Vec3 &GetTarget() { return m_target; }
	Vec3 &GetUp() { return m_up; }
	
	void Set(const Vec3 &, float, float);
	void Rotate(float, float, float scale = 1.0);
	void Move(float);
	void Slide(float, float);
	
};

#endif
