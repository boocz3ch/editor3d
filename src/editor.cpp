#include "editor.h"
#include <wx/image.h>
#include <wx/dir.h>
#include <GL/glu.h>

#include "globals.h"
#include "exception.h"

namespace Editor {

CEditor *CEditor::m_inst = 0;

void CEditor::InitCamera()
{
	m_camera.Set(Vec3(51, 56, 51), -1.9, -0.9);
}

CEditor::CEditor():
	m_map(0), m_render_state(GL_TRIANGLES),
	m_clickpointer(Vec3(0, 0, 0)),
	// m_camera(Vec3(51,56,51), Vec3(10,0,10), Vec3(0,1,0)),
	m_viewport_size(wxSize(0, 0)),
	m_sync(true)
{
	InitCamera();
}

void CEditor::LoadWorldMap()
{
	wxString root = _T("../data/");
	wxString terTilesStr = _T("terrain_tiles/");
	wxString texTilesStr = _T("texture_tiles/");
	
	wxString terRoot = root + terTilesStr;
	wxDir dir(terRoot);
	if (!dir.IsOpened()) {
		throw CException("CEditor::LoadWorldMap: Root dir not opened");
	}
	else {
		// DEBUG
		std::cout << "opened " << dir.GetName().ToAscii() << std::endl;
	}
	
    /*
	 * bool cont = dir.GetFirst(&fname);
	 * while (cont) {
	 *     // DEBUG
	 *     std::cout << fname.ToAscii() << std::endl;
	 *     cont = dir.GetNext(&fname);
	 * }
     */
	
	
	wxString fname, tmp, val;
	long to_long;
	int x, y;
	
	wxArrayString all_files;
	wxDir::GetAllFiles(dir.GetName(), &all_files);
	// DEBUG
	std::cerr << "number of terrain tile file paths: " << all_files.GetCount() << std::endl;
	for (unsigned int i=0; i<all_files.GetCount(); ++i) {
		fname = all_files[i];
		// DEBUG
		// std::cout << fname.ToAscii() << std::endl;
		
		tmp = fname.Remove(0, terRoot.Length());
		// DEBUG
		// std::cout << tmp.ToAscii() << std::endl;
		
		// TODO: neprenositelnej kod!
		val = tmp.Mid(0, tmp.Find('/'));
		// DEBUG
		// std::cout << "x1:" << val.ToAscii() << std::endl;
		val.ToLong(&to_long);
		x = to_long * 1024;
		tmp = tmp.Remove(0, val.Length()+1);
		
		val = tmp.Mid(0, tmp.Find('/'));
		// DEBUG
		// std::cout << "x2:" << val.ToAscii() << std::endl;
		val.ToLong(&to_long);
		x += to_long;
		// DEBUG
		// std::cout << "x:" << x << " " [> <<std::endl <];
		
		tmp = tmp.Remove(0, val.Length()+1);
		val = tmp.Mid(0, tmp.Find('/'));
		// DEBUG
		// std::cout << "y1: " << val.ToAscii() << std::endl;
		val.ToLong(&to_long);
		y = to_long * 1024;
		
		tmp = tmp.Remove(0, val.Length()+1);
		val = tmp.Mid(0, tmp.Find('.'));
		// DEBUG
		// std::cout << "y2:" << val.ToAscii() << std::endl;
		val.ToLong(&to_long);
		y += to_long;
		// std::cout << "y:" << y << std::endl;
		
		m_world_map.insert( std::pair<wxPoint, wxString>(wxPoint(x, y), all_files[i]) );
	}
	
	// DEBUG
    /*
	 * std::multimap<wxPoint,wxString,PointCompare>::iterator it;
	 * it = m_world_map.begin();
	 * for (; it != m_world_map.end(); it++) {
	 *     std::std::cout << it->second.ToAscii() << std::std::endl;
	 * }
     */
}

void CEditor::CreateMapFromView()
{
	// DEBUG
	std::cout << "CEditor::CreateMapFromView: Creating.." << std::endl;
	m_map->CreateFromView(m_tilegrid);
}
	

void CEditor::Init()
{
	// init image handlers
	wxInitAllImageHandlers();
	
	LoadWorldMap();
	
	// TODO prozatim napevno nacitat 4 policka
	std::vector<TileInfo> maps;
	TileInfo ref;
	
	std::multimap<wxPoint, wxString, PointCompare>::iterator it;
	it = m_world_map.begin();
	if (it == m_world_map.end())
		throw CException("CEditor::Init(): No maps found");
	ref = TileInfo(it->first, it->second);
	maps.push_back(ref);
	
	// DEBUG
	// std::std::cout << ref.coord.x+1 << std::std::endl;

	it = m_world_map.find(wxPoint(ref.coord.x+1, ref.coord.y));
	// TODO dopsat zbytek vyjimek
	if (it == m_world_map.end())
		throw CException("CEditor::Init(): Could not find x+1 map");
	maps.push_back(TileInfo(it->first, it->second));
	
    /*
	 * it = m_world_map.find(wxPoint(ref.coord.x+2, ref.coord.y));
	 * // TODO dopsat zbytek vyjimek
	 * if (it == m_world_map.end())
	 *     throw CException("CEditor::Init(): Could not find x+2 map");
	 * maps.push_back(TileInfo(it->first, it->second));
     */
	
	it = m_world_map.find(wxPoint(ref.coord.x, ref.coord.y+1));
	if (it == m_world_map.end())
		throw CException("CEditor::Init(): Could not find y+1 map");
	maps.push_back(TileInfo(it->first, it->second));
	
	it = m_world_map.find(wxPoint(ref.coord.x+1, ref.coord.y+1));
	if (it == m_world_map.end())
		throw;
	maps.push_back(TileInfo(it->first, it->second));
	
    /*
	 * it = m_world_map.find(wxPoint(ref.coord.x+2, ref.coord.y+1));
	 * if (it == m_world_map.end())
	 *     throw;
	 * maps.push_back(TileInfo(it->first, it->second));
     */
	


    /*
	 * it = m_world_map.find(wxPoint(ref.coord.x, ref.coord.y+2));
	 * if (it == m_world_map.end())
	 *     throw CException("CEditor::Init(): Could not find y+1 map");
	 * maps.push_back(TileInfo(it->first, it->second));
	 * 
	 * it = m_world_map.find(wxPoint(ref.coord.x+1, ref.coord.y+2));
	 * if (it == m_world_map.end())
	 *     throw;
	 * maps.push_back(TileInfo(it->first, it->second));
	 * 
	 * it = m_world_map.find(wxPoint(ref.coord.x+2, ref.coord.y+2));
	 * if (it == m_world_map.end())
	 *     throw;
	 * maps.push_back(TileInfo(it->first, it->second));
     */


	// FIXME ?? 3x2 spatne vykresluje texturu.. protoze neni ctvercova?
	m_tilegrid = new CTileGrid(maps, wxSize(2, 2));
	// throw CException("CEditor::Init(): stopper throw");
	
	m_map = new CMap();
	// m_map->Load(m_tilegrid);
	// throw CException("CEditor::Init(): stopper throw");
	m_map->Load(_T("../data/676.png"), _T("../data/676.jpg"));
	
	// throw;
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
	gluPerspective(DEFAULT_FOV_Y, w / static_cast<float>(h), NEAR_PLANE, FAR_PLANE);
	glMatrixMode(GL_MODELVIEW);
}

void CEditor::AdjustZoom(float offset) {
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
	// std::std::cout << mx << "," << my << std::std::endl;
	
	fmx = mx;
	// obratit y kvuli opengl
	fmy = viewport[3] - my;
	glReadPixels(mx, static_cast<int>(fmy), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fmz);
	
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
	std::cout << "souradnice v hmape: " << hm_coord.x << "," << hm_coord.z << std::endl;
	
	int hmx = (int)hm_coord.x; 
	int hmy = (int)hm_coord.z; 
	
	// TODO tohle prepsat
	// vertexy
	glBindBuffer(GL_ARRAY_BUFFER, m_map->GetVertexArrayID());
	float *data = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	const int rad = 40;
	int ind;
	for (int offx = -rad/2; offx <= rad/2; ++offx) {
		for (int offy = -rad/2; offy <= rad/2; ++offy) {
			if (hmx+offx < 0  || hmx+offx > sizex-1) continue;
			if (hmy+offy < 0  || hmy+offy > sizey-1) continue;

			float set = (rad-abs(offx))/10.0 * (rad-abs(offy))/10.0 / 4.0;
			ind = (hmx+offx + (hmy+offy) * sizex) * 3 + 1;
			data[ind] += set;
			if (data[ind] * m_map->GetNormalize()*4 > 255.0) data[ind] = 255.0/(m_map->GetNormalize()*4);
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void CEditor::SetHeightMap(const wxString &fname)
{
	m_map->Load(fname, m_map->GetTextureName());
	m_sync = true;
}
void CEditor::SetTexture(const wxString &fname)
{
	m_map->SetTexture(fname);
}
void CEditor::SaveMap(const wxString &fname)
{
	m_map->Save(fname);
}
void CEditor::SaveWorld()
{
	m_tilegrid->Save();
}

void CEditor::Render()
{
	// TODO TODO TODO
	// return;
	// TODO TODO TODO
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

void CEditor::UpdateShader(const Vec3 &p)
{
	m_map->UpdateShader(p);
}

void CEditor::CleanUp()
{
	if (m_map)
		delete m_map;
	if (m_inst)
		delete m_inst;
}

} // namespace Editor
