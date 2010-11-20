#include "camera.h"

CCamera::CCamera():
	m_position(Vec3(0.0, 0.0, 0.0)),
	m_target(Vec3(0.0, 0.0, 0.0)),
	m_up(Vec3(0.0, 0.0, 0.0)),
	m_radius(1.0), m_movedistance(1.0),
	m_h_radians(0.0), m_v_radians(0.0)
{
		Rotate(0,0);
}

CCamera::CCamera(const Vec3 &p, const Vec3 &t, const Vec3 &u):
	m_position(p),
	m_target(t),
	m_up(u),
	m_radius(1.0), m_movedistance(1.0),
	m_h_radians(0.0), m_v_radians(0.0)
{
		// Rotate(0,0);
		// m_h_radians = asin((t.z - p.z) / (t.x - p.x));
		Rotate(0,0);
}

CCamera::~CCamera()
{
}
