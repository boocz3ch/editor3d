#include "camera.h"

CCamera::CCamera():
	m_position(Vec3(0.0, 0.0, 0.0)),
	m_target(Vec3(0.0, 0.0, 0.0)),
	m_up(Vec3(0.0, 0.0, 0.0)),
	m_radius(1.0), m_h_radians(0.0), m_v_radians(0.0)
{
	Rotate(0.0, 0.0);
}

CCamera::CCamera(const Vec3 &p, const Vec3 &t, const Vec3 &u):
	m_position(p),
	m_target(t),
	m_up(u),
	m_radius(1.0), m_h_radians(0.0), m_v_radians(0.0)
{
	// Rotate(0,0);
	// m_h_radians = asin((t.z - p.z) / (t.x - p.x));
	Rotate(0.0, 0.0);
}

CCamera::~CCamera()
{
}

void CCamera::ClampVRadians()
{
	if (m_v_radians < -PI/2 + EPSILON) m_v_radians = -PI/2 + EPSILON;
	if (m_v_radians > PI/2 - EPSILON) m_v_radians = PI/2 - EPSILON;
}

void CCamera::Set(const Vec3 &pos, float h, float v)
{
	m_position = pos;
	m_target = Vec3(0,0,0);
	m_up = Vec3(0,1,0);
	m_h_radians = h;
	m_v_radians = v;

	Rotate(0.0, 0.0);
}

void CCamera::Rotate(float h, float v, float scale)
{
	m_h_radians += h * scale;
	m_v_radians += v * scale;

	ClampVRadians();

	m_target.x = m_position.x + m_radius*cos(m_v_radians)*cos(m_h_radians);
	m_target.y = m_position.y + m_radius*sin(m_v_radians);
	m_target.z = m_position.z + m_radius*cos(m_v_radians)*sin(m_h_radians);

	m_up.x = m_position.x - m_target.x;
	m_up.y = fabs(m_position.y + m_radius*sin(m_v_radians+PI/2.0)) ;
	m_up.z = m_position.z - m_target.z;
}

void CCamera::Slide(float h, float v)
{
	m_position.x += h * cos(m_h_radians+PI/2.0);
	m_position.y += v;
	m_position.z += h * sin(m_h_radians+PI/2.0);
	Rotate(0.0, 0.0);
}

void CCamera::Move(float d){
	m_position.x += d * cos(m_v_radians)*cos(m_h_radians);
	m_position.y += d * sin(m_v_radians);
	m_position.z += d * cos(m_v_radians)*sin(m_h_radians);
	Rotate(0.0, 0.0);
}

