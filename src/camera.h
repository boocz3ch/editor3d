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
	float m_movedistance;
	float m_h_radians;
	float m_v_radians;
	
	void ClampVRadians()
	{
		if (m_v_radians < -PI/2 + EPSILON) m_v_radians = -PI/2 + EPSILON;
		if (m_v_radians > PI/2 - EPSILON) m_v_radians = PI/2 - EPSILON;
	}
public:
	CCamera();
	CCamera(const Vec3 &, const Vec3 &, const Vec3 &);
	~CCamera();

	Vec3 &GetPosition() { return m_position; }
	Vec3 &GetTarget() { return m_target; }
	Vec3 &GetUp() { return m_up; }
	
	void Set(const Vec3 &pos, float h, float v)
	{
		m_position = pos;
		m_target = Vec3(0,0,0);
		m_up = Vec3(0,1,0);
		m_h_radians = h;
		m_v_radians = v;

		Rotate(0,0);
	}

	void Rotate(float h, float v, float scale = 1.0)
	{
		m_h_radians += h * scale;
		m_v_radians += v * scale;

		ClampVRadians();
		
		m_target.y = m_position.y+(float)(m_radius*sin(m_v_radians));
		m_target.x = m_position.x+(float)(m_radius*cos(m_v_radians)*cos(m_h_radians));
		m_target.z = m_position.z+(float)(m_radius*cos(m_v_radians)*sin(m_h_radians));

		m_up.x = m_position.x-m_target.x;
		m_up.y = fabs(m_position.y+(float)(m_radius*sin(m_v_radians+PI/2.0))) ;
		m_up.z = m_position.z-m_target.z;
	}

/*
 *   public static void SlideCamera(float h, float v){
 *     m_position.Y += v*moveDist;
 *     m_position.X += h*moveDist*(float)Math.Cos(m_h_radians+Math.PI/2);
 *     m_position.Z += h*moveDist*(float)Math.Sin(m_h_radians+Math.PI/2);
 *     RotateCamera(0,0);
 *   }
 * 
 */
	void Move(float d){
		m_position.y += d*m_movedistance*(float)sin(m_v_radians);
		m_position.x += d*m_movedistance*(float)(cos(m_v_radians)*cos(m_h_radians));
		m_position.z += d*m_movedistance*(float)(cos(m_v_radians)*sin(m_h_radians));
		Rotate(0,0);
	}
	
};

#endif
