#include "editor.h"
#include <wx/image.h>
#include <wx/dir.h>
#include <GL/glu.h>

#include "globals.h"
#include "exception.h"
#include "model.h"

namespace Editor {

CEditor *CEditor::m_inst = 0;

void CEditor::InitCamera()
{
	m_camera.Set(Vec3(81, 186, 181), -1.8, -0.9);
}

CEditor::CEditor():
	m_map(0), m_tilegrid(0), m_render_state(GL_TRIANGLES),
	m_clickpointer(Vec3(0, 0, 0)),
	m_viewport_size(wxSize(0, 0)),
	m_sync(true),
	m_datapath_root(DEFAULT_DATA_ROOT),
	m_tool(Tools::BR_CIRCLE, Tools::TOOL_LOWER, 3),
        picked_point_x(0), picked_point_y(0), picking_enabled(false),
        left_mouse_down(false), mouse_dragging(false), mx(0), my(0),
		picked_name(-1) 
{
	InitCamera();
}
CEditor::~CEditor()
{
}

void CEditor::LoadWorldMap()
{
	wxString root = m_datapath_root;
	wxString terTilesStr = DEFAULT_TER_TILES_PATH;
	wxString texTilesStr = DEFAULT_TEX_TILES_PATH;
	
	wxString terRoot = root + terTilesStr;
	wxDir dir(terRoot);
	if (!dir.IsOpened()) {
		throw CException("CEditor::LoadWorldMap: Root dir not opened");
	}
	// log
	wxLogVerbose(_T("Opened ") + dir.GetName());
	
	wxString fname, fname_map, tex_path, tmp, val;
	long to_long;
	int x, y;
	TileInfo ins;
	
	wxArrayString all_files;
	wxDir::GetAllFiles(dir.GetName(), &all_files);
	
	// log
	wxLogVerbose(_T("Number of terrain tile file paths: %d"), all_files.GetCount());
	
	for (unsigned int i=0; i<all_files.GetCount(); ++i) {
		fname = all_files[i];
		fname_map = fname;
		
		tmp = fname.Remove(0, terRoot.Length());
		
		// neprenositelny kod!
		val = tmp.Mid(0, tmp.Find('/'));
		val.ToLong(&to_long);
		x = to_long * 1024;
		tmp = tmp.Remove(0, val.Length()+1);
		
		val = tmp.Mid(0, tmp.Find('/'));
		val.ToLong(&to_long);
		x += to_long;
		
		tmp = tmp.Remove(0, val.Length()+1);
		val = tmp.Mid(0, tmp.Find('/'));
		val.ToLong(&to_long);
		y = to_long * 1024;
		
		tmp = tmp.Remove(0, val.Length()+1);
		val = tmp.Mid(0, tmp.Find('.'));
		val.ToLong(&to_long);
		y += to_long;
		
		// cesta k texture
		tex_path = fname_map;
		tex_path.Replace(HEIGHTMAP_SUFFIX, TEXTURE_SUFFIX);
		tex_path.Replace(_T("terrain_tiles"), _T("texture_tiles"));
		
		ins.coord = wxPoint(x, y);
		ins.hm_path = fname_map;
		ins.tex_path = tex_path;
		m_world_map.insert( pair_t(ins.coord, ins) );
	}
}

void CEditor::CreateMapFromView(bool shift)
{
	// log
	wxLogVerbose(_T("CEditor::CreateMapFromView: Creating.."));
	m_map->CreateFromView(m_tilegrid, shift);
}

void CEditor::Init()
{
	// init image handlers
	wxInitAllImageHandlers();
	// nacist info o svete
	LoadWorldMap();
	
	multimap_t maps;
	
	// referencni policko, prvni nactene v datasetu
	TileInfo ref;
	// interni reprezentace souradnic, zacina [0,0]
	// odpovida offsetu od zacatku souradnic datasetu
	wxPoint internal_coord;
	// velikost pozadovane plochy
	wxSize load_size(2, 2);
	
	multimap_t::iterator it;
	it = m_world_map.begin();
	if (it == m_world_map.end())
		throw CException("CEditor::Init(): No maps found");
	ref = it->second;
	// internal_coord = wxPoint(0, 0);
	// maps.insert( pair_t(internal_coord, ref) );
	
	for (int y=0; y<load_size.GetHeight(); ++y) {
		for (int x=0; x<load_size.GetWidth(); ++x) {
			it = m_world_map.find(wxPoint(ref.coord.x + x, ref.coord.y + y));
			internal_coord = wxPoint(x, y);
			if (it == m_world_map.end())
				throw CException("CEditor::Init(): Could not find map");
			maps.insert( pair_t(internal_coord, it->second) );
		}
	}
	
	m_tilegrid = new CTileGrid(&m_world_map, maps, load_size, ref.coord);
	m_tilegrid->CreateWorldImage();
	
	m_map = new CMap();
	
	CreateMapFromView();
	SetSync();
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
	m_map->InitShaders(m_tool);
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
	GLfloat fmx = 0.0, fmy = 0.0, fmz = 0.0;
	GLdouble posx = 0.0, posy = 0.0, posz = 0.0;
	
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	
	fmx = mx;
	// obratit y kvuli opengl
	fmy = viewport[3] - my;
	glReadPixels(mx, static_cast<int>(fmy), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fmz);
	
	gluUnProject(fmx, fmy, fmz,
			modelview, projection, viewport,
			&posx, &posy, &posz);
	
	return Vec3(posx, posy, posz);
}

void CEditor::ProcessPicked(Vec3 &vpicked)
{
	if (m_tool.level) {
		m_map->AdjustTerrain(vpicked, m_tool, m_tool.strength, m_clickpointer);
	}
	else if (m_tool.use_model || m_tool.move || m_tool.rotate ||
			 m_tool.scale || m_tool.del || m_tool.clone)
	{
		m_map->SelectTerrain(vpicked, m_tool);
	}
	else {
		m_map->AdjustTerrain(vpicked, m_tool, m_tool.strength);
	}
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
}

void 
CEditor::Select(void) {

 if (picking_enabled) {
  GLuint buffer[512]; // pole pro ukladani picknutych hodnot
 
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  glSelectBuffer(512, buffer);
  glRenderMode(GL_SELECT);

  glInitNames();
  glPushName(0);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
    glLoadIdentity();
  
    gluPickMatrix(picked_point_x, (GLdouble) (viewport[3] - picked_point_y), 1.0f, 1.0f, viewport);

    gluPerspective(DEFAULT_FOV_Y, (GLfloat) (viewport[2] - viewport[0]) / (GLfloat) (viewport[3] - viewport[1]), NEAR_PLANE, FAR_PLANE);

    glMatrixMode(GL_MODELVIEW);

    glUseProgram(0);
        // TODO tmp
	std::vector<s_model>::iterator it = m_map->GetModels().begin();
	std::vector<s_model>::iterator ite = m_map->GetModels().end();

	for (; it != ite; it++) {
	  glPushMatrix();
            
	      GLfloat mod[4] = {1.0, 1.0, 1.0, 1.0};
            if (picked_name == it->id_name) {
		  // glColor3f(0.0, 1.0, 0.0);
		  	mod[0] = 0.0;
			mod[2] = 0.0;
            }
            else {
		  // glColor3f(1.0, 1.0, 1.0);
		  // mod = mod....
            }
	    glTranslatef(it->position.x, it->position.y, it->position.z);
	    glRotatef(it->z_rotation, 0.0, 1.0, 0.0);
            glScalef(it->scale, it->scale, it->scale);
            glLoadName(it->id_name);
		glEnable(GL_LIGHT0);
	    glLightfv(GL_LIGHT0, GL_POSITION, LIGHT_POSITION);
	    it->model->render(mod);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
	  glPopMatrix();
        }	
	m_map->GetShader()->Use();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);

  int hits = glRenderMode(GL_RENDER);

  int choose = buffer[3];
  int depth = buffer[1];

  for (int loop = 1; loop < hits; loop++) {
    if (buffer[loop*4 + 1] < GLuint(depth)) {
      choose = buffer[loop * 4 + 3];
      depth = buffer[loop * 4 + 1];
    }
  }

    if (hits != 0) {
      picked_name = choose; 
    }
    else {
      picked_name = 0; 
    }
    picking_enabled = false;
 }
}

void CEditor::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
        glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	Vec3 pos = m_camera.GetPosition();
	Vec3 tar = m_camera.GetTarget();
	Vec3 up = m_camera.GetUp();
	
	gluLookAt(pos.x, pos.y, pos.z,
			tar.x, tar.y, tar.z,
			up.x, up.y, up.z);
	
	if (m_sync) {
		m_map->SendToServer();
		m_sync = false;
	}
	m_map->Render(m_render_state);
 
	// solve mouse before rendering the objects
	if (mouse_dragging && left_mouse_down) {
		if (GetTool().level || GetTool().use_model ||
				GetTool().move || GetTool().rotate || GetTool().scale ||
				GetTool().del || GetTool().tool == Tools::TOOL_SMOOTH ||
				GetTool().tool == Tools::TOOL_CLEARBACK)
		{
			Vec3 p = Pick(mx, my);
			ProcessPicked(p);
		}
	}
	else if (left_mouse_down && !mouse_dragging) {
		Vec3 p = Pick(mx, my);
		// SetClickPointer(p);
		  ProcessPicked(p);
        }
        else if (GetTool().move && GetTool().clone) {
         Tools::CTool t = GetTool();
         t.move = false;
         SetTool(t);
        }

        Vec3 p = Pick(mx, my);
        UpdateShader(p);

        std::vector<s_model> models = m_map->GetModels();
       
	std::vector<s_model>::iterator it = m_map->GetModels().begin();
	std::vector<s_model>::iterator ite = m_map->GetModels().end();
	glUseProgram(0);
	glDisable(GL_TEXTURE_2D);

        int name = 1;
	for (; it != ite; it++) {
	  glPushMatrix();
            
	      GLfloat mod[4] = {1.0, 1.0, 1.0, 1.0};
            if (picked_name == it->id_name) {
		  // glColor3f(0.0, 1.0, 0.0);
		  	mod[0] = 0.0;
			mod[2] = 0.0;
            }
            else {
	      glColor3f(1.0, 1.0, 1.0);
            }
	    glTranslatef(it->position.x, it->position.y, it->position.z);
	    glRotatef(it->z_rotation, 0.0, 1.0, 0.0);
            glScalef(it->scale, it->scale, it->scale);
            
		glEnable(GL_LIGHT0);
	    glLightfv(GL_LIGHT0, GL_POSITION, LIGHT_POSITION);
	    it->model->render(mod);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
	  glPopMatrix();
          name++;
	}
	glEnable(GL_TEXTURE_2D);
        
	m_map->GetShader()->Use();
}

void CEditor::UpdateShader(const Vec3 &p)
{
	m_map->UpdateShader(p);
}

void CEditor::UpdatePickedPoint(int x, int y, bool _pick_enabled)
{
	picked_point_x = x;
	picked_point_y = y;
        if (!_pick_enabled) picked_name = 0;
        picking_enabled = _pick_enabled;
}

void CEditor::SetChoosedModel(Cmodel *model) {
  m_map->SetChoosedModel(model);
}

void CEditor::DeleteSelectedModel() {
  if (picked_name > 0) {
    std::vector<s_model> *m_models = &m_map->GetModels();
    std::vector<s_model>::iterator iter = m_models->begin();
    for (int i = 0; iter != m_models->end(); iter++, i++) {
      if (i == (picked_name - 1)) {
        m_models->erase(iter);
        
        // renumber all models
        for (int j = 0; j != m_models->size(); j++) {
          (*m_models)[j].id_name = j + 1;
        }
        break;
      }
    }
    picked_name = 0;
  }
}


void CEditor::CleanUp()
{
	if (m_map)
		delete m_map;
	if (m_inst)
		delete m_inst;
}

} // namespace Editor
