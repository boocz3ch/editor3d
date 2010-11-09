#include "editor.h"
#include <wx/image.h>
#include <GL/glu.h>

#include "globals.h"
// TODO
#include "quaternion.h"
// TODO

// debug
#include <iostream>
using namespace std;

namespace Editor {

CEditor *CEditor::m_inst = 0;

CEditor::CEditor():
	m_map(0), m_zoom(1.0), m_render_state(GL_TRIANGLES),
	m_clickpointer(Vec3(0, 0, 0)), m_viewport_size(wxSize(0, 0)),
	m_sync(true)
{
	memset(&m_camera, 0, sizeof(Camera));
	m_camera.pos[0] = 51;
	m_camera.pos[1] = 56.0;
	m_camera.pos[2] = 51.0;

	m_camera.eye[0] = m_camera.pos[0] ;
	m_camera.eye[1] = m_camera.pos[1]* sin(m_camera.v_angle/180 * 3.141592654f);
	m_camera.eye[2] = m_camera.pos[2]* sin(m_camera.v_angle/180 * 3.141592654f);

	m_camera.up[0] = 0.0;
	m_camera.up[1] = 1.0;
	m_camera.up[2] = 0.0;

	m_camera.h_angle = 0;
	m_camera.v_angle = 0;
}

void CEditor::Init()
{
	// init image handlers
	wxInitAllImageHandlers();
	
	m_map = new CMap();
	m_map->Load(_T("../data/test1.png"), _T("../data/rock_high.bmp"));
}

void CEditor::MoveCameraFocus(float x, float y, float z)
{
	m_camera.eye[0] += x;
	m_camera.eye[1] += y;
	m_camera.eye[2] += z;
	// m_camera.v_angle += x/10.0;
	// cout << m_camera.v_angle << endl;
	// // m_camera.eye[1] = m_camera.pos[1]* sin(m_camera.v_angle/180.0 * 3.141592654f);
	// m_camera.eye[0] = m_camera.pos[0]* sin(m_camera.v_angle/180.0 * 3.141592654f);
}

void CEditor::OnResize(int w, int h)
{
	m_viewport_size = wxSize(w, h);

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(DEFAULT_FOV_Y * m_zoom, w / static_cast<float>(h), NEAR_PLANE, FAR_PLANE);
	glMatrixMode(GL_MODELVIEW);
}

void CEditor::InitGL(int w, int h)
{
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glLoadIdentity();

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	gluPerspective(DEFAULT_FOV_Y, w / static_cast<float>(h), NEAR_PLANE, FAR_PLANE);
	glMatrixMode(GL_MODELVIEW);

	m_viewport_size = wxSize(w, h);
}

void CEditor::AdjustZoom(float offset) {
	float ratio = m_viewport_size.x / static_cast<float>(m_viewport_size.y);
	m_zoom *= offset;

	if (m_zoom > MAX_ZOOM) m_zoom = MAX_ZOOM;
	if (m_zoom < MIN_ZOOM) m_zoom = MIN_ZOOM;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(DEFAULT_FOV_Y * m_zoom, ratio, NEAR_PLANE, FAR_PLANE);
	glMatrixMode(GL_MODELVIEW);
}

Vec3 CEditor::Pick(int mx, int my)
{
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat fmx, fmy, fmz;
	GLdouble posx, posy, posz;
	
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	// DEBUG
	// std::cout << mx << "," << my << std::endl;
	
	fmx = mx;
	// obratit y kvuli opengl
	fmy = viewport[3] - my;
	glReadPixels(mx, (int)fmy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fmz);
	
	gluUnProject(fmx, fmy, fmz,
			modelview, projection, viewport,
			&posx, &posy, &posz);
	
	m_clickpointer = Vec3(posx, posy, posz);
	return m_clickpointer;
}

void CEditor::ProcessPicked(Vec3 &vpicked)
{
	Vec3 hm_coord = vpicked * m_map->GetNormalize();
	int sizex = m_map->GetWidth();
	int sizey = m_map->GetHeight();

	// DEBUG TODO NAPSAT DEBUG SYSTEM
	// std::cout << "souradnice v hmape: " << hm_coord.x << "," << hm_coord.z << std::endl;
	
	int hmx = (int)hm_coord.x; 
	int hmy = (int)hm_coord.z; 
	
	// vertexy
	glBindBuffer(GL_ARRAY_BUFFER, m_map->GetVertexArrayID());
	float *data = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	const int rad = 40;
	for (int offx = -rad/2; offx <= rad/2; ++offx) {
		for (int offy = -rad/2; offy <= rad/2; ++offy) {
			if (hmx+offx < 0  || hmx+offx > sizex-1) continue;
			if (hmy+offy < 0  || hmy+offy > sizey-1) continue;

			data[(hmx+offx + (hmy+offy) * sizex) * 3 + 1] += (rad-abs(offx))/10.0 * (rad-abs(offy))/10.0 / 4.0;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	// barva
	glBindBuffer(GL_ARRAY_BUFFER, m_map->GetColorArrayID());
	data = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	for (int offx = -rad/2; offx <= rad/2; ++offx) {
		for (int offy = -rad/2; offy <= rad/2; ++offy) {
			if (hmx+offx < 0  || hmx+offx > sizex-1) continue;
			if (hmy+offy < 0  || hmy+offy > sizey-1) continue;

			data[(hmx+offx + (hmy+offy) * sizex) * 3 ] += (rad-abs(offx))/10.0 * (rad-abs(offy))/10.0 / 68.0 + 0.15;
			data[(hmx+offx + (hmy+offy) * sizex) * 3 + 1] += (rad-abs(offx))/10.0 * (rad-abs(offy))/10.0 / 68.0 + 0.15;
			data[(hmx+offx + (hmy+offy) * sizex) * 3 + 2] += (rad-abs(offx))/10.0 * (rad-abs(offy))/10.0 / 68.0 + 0.15;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void CEditor::SetHeightMap(const wxString &fname)
{
	m_map->Load(fname, m_map->GetTextureName());
	m_sync = true;
}
void CEditor::SaveMap(const wxString &fname)
{
	m_map->Save(fname);
}

void CEditor::Render()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(m_camera.pos[0], m_camera.pos[1], m_camera.pos[2],
			m_camera.eye[0], m_camera.eye[1], m_camera.eye[2],
			m_camera.up[0], m_camera.up[1], m_camera.up[2]
			);
	
	if (m_sync) {
		m_map->SendToClient();
		m_sync = false;
	}
	// glDrawArrays(GL_TRIANGLES, 0, tricount);
	// glBindTexture(GL_TEXTURE_2D, tex);
	m_map->Render(m_render_state);

	glFlush();

	// glDrawPixels(512, 512, GL_RGB, GL_UNSIGNED_BYTE, m_texture->GetData());
}



} // namespace Editor
