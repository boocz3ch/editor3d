#include "editor.h"
#include <wx/image.h>
#include <GL/glu.h>

#include "globals.h"

// debug
#include <iostream>
using namespace std;

namespace Editor {

CEditor *CEditor::m_inst = 0;

CEditor::CEditor():
	m_map(0), m_zoom(1.0), m_render_state(GL_TRIANGLES),
	m_clickpointer(Vec3(0, 0, 0)),
	// m_camera(Vec3(51,56,51), Vec3(10,0,10), Vec3(0,1,0)),
	m_viewport_size(wxSize(0, 0)),
	m_sync(true),
	m_enable_shaders(false),
	m_mode(MODE_HM)
{
	m_camera.Set(Vec3(51, 56, 51), -1.9, -0.9);
}

void CEditor::Init()
{
	// init image handlers
	wxInitAllImageHandlers();
	
	m_map = new CMap();
	m_map->Load(_T("../data/676.png"), _T("../data/676.jpg"));
}

void CEditor::InitGL(int w, int h)
{
	// init gl
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
	
	// init shaders
	m_map->InitShaders();
}

void CEditor::MoveCameraFocus(float x, float y, float z)
{
	m_camera.Rotate(x, z, 0.009);
}
void CEditor::MoveCamera(float d)
{
	m_camera.Move(d);
}
void CEditor::SlideCamera(float h, float v)
{
	m_camera.Slide(h, v);
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

void CEditor::AdjustZoom(float offset) {
/*
 *     float ratio = m_viewport_size.x / static_cast<float>(m_viewport_size.y);
 *     m_zoom *= offset;
 * 
 *     if (m_zoom > MAX_ZOOM) m_zoom = MAX_ZOOM;
 *     if (m_zoom < MIN_ZOOM) m_zoom = MIN_ZOOM;
 * 
 *     glMatrixMode(GL_PROJECTION);
 *     glLoadIdentity();
 *     gluPerspective(DEFAULT_FOV_Y * m_zoom, ratio, NEAR_PLANE, FAR_PLANE);
 *     glMatrixMode(GL_MODELVIEW);
 */
	if (offset < 1.0)
		m_camera.Move(offset * 3.0);
	else
		m_camera.Move(-offset * 3.0);
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	Vec3 pos = m_camera.GetPosition();
	Vec3 tar = m_camera.GetTarget();
	Vec3 up = m_camera.GetUp();
	
	gluLookAt(pos.x, pos.y, pos.z,
			tar.x, tar.y, tar.z,
			up.x, up.y, up.z);
	
	if (m_sync) {
		m_map->SendToClient();
		m_sync = false;
	}
	m_map->Render(m_render_state);

	// glFlush();
	// glDrawPixels(512, 512, GL_RGB, GL_UNSIGNED_BYTE, m_texture->GetData());
}

void CEditor::CleanUp()
{
	if (m_map)
		delete m_map;
	if (m_inst)
		delete m_inst;
}

} // namespace Editor
