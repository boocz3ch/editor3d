#include "map.h"
#include "exception.h"
#include "globals.h" 
#include "jpgstream.h"
#include "pngstream.h"
#include "editor.h"

CMap::CMap():
	m_vertex_array(0), m_vertex_array_id(0),
	m_index_array(0), m_index_array_id(0),
	m_texcoord_array(0), m_texcoord_array_id(0),
	m_normal_array(0), m_normal_array_id(0),
	m_ntriangles(0), m_nindices(0), m_nverts(0),
	m_normalize(DEFAULT_NORMALIZE),

	m_heightmap(0), m_heightmap_name(_T("")),
	m_texture(0), m_texture_name(_T("")), m_texture_id(0),
	m_width(0), m_height(0),
	m_shader(0), m_compute_indices_and_texcoords(true)
{
}

CMap::~CMap()
{
	if (m_vertex_array)
		delete [] m_vertex_array;
	if (m_index_array)
		delete [] m_index_array;
	if (m_texcoord_array)
		delete [] m_texcoord_array;
	if (m_shader)
		delete m_shader;
}
void CMap::InitShaders(const Tools::CTool &tool)
{
	m_shader = new CShader();
	m_shader->Add("shader/shader.vert", GL_VERTEX_SHADER);
	m_shader->Add("shader/shader.frag", GL_FRAGMENT_SHADER);
	m_shader->Link();
	
	m_shader->Use();
	UpdateShader(0, true);
	UpdateShader(1, true);
	UpdateShaderScale(Vec3(1.0, 1.0, 1.0));
	UpdateShaderTool(tool);
	UpdateShaderLight(Vec3(LIGHT_POSITION[0], LIGHT_POSITION[1], LIGHT_POSITION[2]));
}

void CMap::Create(int w, int h, bool is_new = false)
{
	m_nverts = w * h;
	m_ntriangles = (w-1) * (h-1) * 2;
	m_nindices = m_ntriangles * 3;
	
	m_width = w;
	m_height = h;

	if (m_vertex_array != 0)
		delete [] m_vertex_array;
    /*
	 * if (m_index_array != 0)
	 *     delete [] m_index_array;
	 * if (m_texcoord_array != 0)
	 *     delete [] m_texcoord_array;
     */
	
	// if (m_heightmap == 0)
	
	// assert(m_nverts > 0);
	
	m_vertex_array = new GLfloat[m_nverts * 3];
	if (m_index_array == 0)
		m_index_array = new GLuint[m_nindices];
	if (m_texcoord_array == 0)
		m_texcoord_array = new GLfloat[m_nverts * 2];
	if (m_normal_array == 0)
		m_normal_array = new GLfloat[m_nverts * 3];

	GLfloat xyz[3];
	GLfloat uv[2];
	GLuint iii[3];
	GLfloat n_xyz[3];

	int ind = 0, tex_i = 0;
	float col;
	
	int iii_ind = 0;
	GLuint s1, s2;
	GLuint i_offset = 0;
	
	// heightmap -> vertex array
	unsigned char *data = m_heightmap->GetData();
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			// copy vertex
			xyz[0] = x / m_normalize;
			if (is_new) {
				col = 0.0;
			}
			else {
				//col = m_heightmap->GetRed(x, y) / (m_normalize*3);
				int low = data[y * h * 3 + x * 3 + 1];
				int high;
				high = data[y * 256 * 3 + x * 3 + 0];
				high <<= 8;
				high += low;
				col = high / (10.0 * m_normalize * 3.0);

			}
			xyz[1] = col;
			xyz[2] = y / m_normalize;
			memcpy(m_vertex_array + ind, xyz, sizeof(xyz));
			ind += 3;
		}
	}
	
	int n_ind = 0;
	if (m_compute_indices_and_texcoords) {
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				if (m_texture) {
					uv[0] = x / (float)w;
					uv[1] = y / (float)h;
					memcpy(m_texcoord_array + tex_i, uv, sizeof(uv));
					tex_i += 2;
				}
				
				int left = (x>0 ? x-1 : x);
				int right = (x<w-1 ? x+1 : x);
				int top = (y>0 ? -w : 0);
				int down = (y<h-1 ? +w : 0);
				int I[5];
				// prostredek
				I[0] = x + i_offset;
				// levo, pravo
				I[1] = right + i_offset;
				I[2] = left + i_offset;
				// nahore dole
				I[3] = x + i_offset + top;
				I[4] = x + i_offset + down;
				
				float sx = m_vertex_array[I[1]*3+1] - m_vertex_array[I[2]*3+1];
				if (x == 0 || x == w - 1) sx *= 2;
				float sy = m_vertex_array[I[4]*3+1] - m_vertex_array[I[3]*3+1];
				if (y == 0 || y == h - 1) sy *= 2;
				
				n_xyz[0] = -sx * 1.0/(m_normalize*3); 
				n_xyz[1] =   2 * 1.0/m_normalize; 
				n_xyz[2] =  sy * 1.0/(m_normalize*3); 
				
				memcpy(m_normal_array + n_ind, n_xyz, sizeof(n_xyz));
				n_ind += 3;
				
				// create indices to the vertex array
				if (x < w-1 && y < h-1) {
					iii[0] = x + i_offset;
					iii[1] = s1 = x+1 + i_offset;
					iii[2] = s2 = x + i_offset+w;
					memcpy(m_index_array + iii_ind, iii, sizeof(iii));
					iii_ind += 3;

					iii[0] = s2;
					iii[1] = s1;
					iii[2] = x+1 + i_offset+w;
					memcpy(m_index_array + iii_ind, iii, sizeof(iii));
					iii_ind += 3;
				}
			}
			if (y < h-1)
				i_offset += w;
		}
		m_compute_indices_and_texcoords = false;
	}
	// assert(m
}

void CMap::Load(const wxString &heightmap_name, const wxString &texture_name)
{
}

void CMap::Load(CTileGrid *tilegrid)
{
}


void CMap::CreateFromView(CTileGrid *tilegrid, bool shift)
{
	if (m_heightmap != 0)
		delete m_heightmap;
	if (m_texture != 0)
		delete m_texture;
	
	wxRect view = tilegrid->GetView();
	// log
	wxLogVerbose(_T("CMap::CreateFromView: Creating... (view=[%d, %d])"), view.x, view.y);
	
	wxImage *hm, *tex;
    /*
	 * if (!shift) {
     */
		hm = tilegrid->GetWorldHeightMap();
		tex = tilegrid->GetWorldTexture();
    /*
	 * }
	 * else {
	 *     hm = tilegrid->GetPreloadHeightMap();
	 *     tex = tilegrid->GetPreloadTexture();
	 *     view.x += 256;
	 *     view.y += 256;
	 * }
     */
		
	// uprava pohledu pri posunu po datasetu 
	if (view.x < 0) view.x += 256;
	if (view.y < 0) view.y += 256;
	if (view.x > 256) view.x -= 256;
	if (view.y > 256) view.y -= 256;
	
	int tex_factor_x = tex->GetWidth() / hm->GetWidth();
	int tex_factor_y = tex->GetHeight() / hm->GetHeight();
	
	m_heightmap = new wxImage(hm->GetSubImage(view));
	view.x *= tex_factor_x;
	view.width *= tex_factor_x;
	view.y *= tex_factor_y;
	view.height *= tex_factor_y;
	m_texture = new wxImage(tex->GetSubImage(view));

	this->Create(m_heightmap->GetWidth(), m_heightmap->GetHeight());
}

void CMap::SetOpenGLTexture()
{
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_texture->GetWidth(),
			m_texture->GetWidth(), 0, GL_RGB, GL_UNSIGNED_BYTE,
			m_texture->GetData());
}

void CMap::SetTexture(const wxString &fname)
{
	if (m_texture != 0)
		delete m_texture;
	
	m_texture = new wxImage(fname);
	if (!m_texture) {
		std::string m = "CMap::SetTexture: Texture not found" + std::string(fname.ToAscii());
		throw CException(m);
	}
	m_texture_name = fname;
	
	SetOpenGLTexture();
}

void CMap::SendToServer()
{
	GLuint nbufs_del = 0;
	GLuint ids_del[5] = {0,};
	int b;
	
	// textures
	if (m_texture) {
		if (m_texture_id == 0)
			glGenTextures(1, &m_texture_id);
		SetOpenGLTexture();
		glEnable(GL_TEXTURE_2D);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}

	//// VBOs
	if (m_vertex_array_id == 0)
		glGenBuffers(1, &m_vertex_array_id);
	else
		ids_del[nbufs_del++] = m_vertex_array_id;
	if (m_index_array_id == 0)
		glGenBuffers(1, &m_index_array_id);
	else
		ids_del[nbufs_del++] = m_index_array_id;
	if (m_texcoord_array_id == 0)
		glGenBuffers(1, &m_texcoord_array_id);
	else
		ids_del[nbufs_del++] = m_texcoord_array_id;
	if (m_normal_array_id == 0)
		glGenBuffers(1, &m_normal_array_id);
	else
		ids_del[nbufs_del++] = m_normal_array_id;

	if (nbufs_del > 0) {
		glDeleteBuffers(nbufs_del, ids_del);
	}
	
	/////////////////////////
	GLint prog = m_shader->GetProgram();
	m_shader->Use();
	
	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_array_id);
	glBufferData(GL_ARRAY_BUFFER, m_nverts*3*sizeof(GLfloat), m_vertex_array, GL_DYNAMIC_DRAW);
	
	// fixed pipeline data
	// --------------
	// glVertexPointer(3, GL_FLOAT, 0, 0);
	// glEnableClientState(GL_VERTEX_ARRAY);
	// --------------
	// 
	// shaders data
	// --------------
	// glVertexAttribPointer(pos_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// glEnableVertexAttribArray(pos_vertex);
	// --------------

	GLint pos_vertex = glGetAttribLocation(prog, "vertex");
	GLint pos_texcoord = glGetAttribLocation(prog, "in_texcoord");
	GLint pos_normal = glGetAttribLocation(prog, "in_normal");
	assert(pos_vertex >= 0);
	// assert(pos_texcoord >= 0);
	assert(pos_normal >= 0);
	
	glVertexAttribPointer(pos_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(pos_vertex);
	// log
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &b);
	wxLogVerbose(_T("GPU: Vertex array size: %d B"), b);
	
	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_array_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nindices*sizeof(GLuint),
			m_index_array, GL_STATIC_DRAW);
	// log
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &b);
	wxLogVerbose(_T("GPU: Index array size: %d B"), b);
	
	// texture buffer
	if (m_texcoord_array) {
		glBindBuffer(GL_ARRAY_BUFFER, m_texcoord_array_id);
		glBufferData(GL_ARRAY_BUFFER, m_nverts*2*sizeof(float), m_texcoord_array, GL_STATIC_DRAW);
		glVertexAttribPointer(pos_texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(pos_texcoord);
	}
	// log
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &b);
	wxLogVerbose(_T("GPU: Texture array size: %d B"), b);
	
	// normal
	glBindBuffer(GL_ARRAY_BUFFER, m_normal_array_id);
	glBufferData(GL_ARRAY_BUFFER, m_nverts*3*sizeof(float), m_normal_array, GL_STATIC_DRAW);
	glVertexAttribPointer(pos_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(pos_normal);
	// log
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &b);
	wxLogVerbose(_T("GPU: Normal array size: %d B"), b);
}

void CMap::Render(int render_state)
{
	GLuint prog = m_shader->GetProgram();
	
	GLint pos_modelview = glGetUniformLocation(prog, "modelview");
	assert(pos_modelview >= 0);
	
	GLint pos_projection = glGetUniformLocation(prog, "projection");
	assert(pos_projection >= 0);
	
	GLfloat projection[16], modelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
	glGetFloatv(GL_PROJECTION_MATRIX, projection);
	
	// poslat matice do grafiky
	glUniformMatrix4fv(pos_modelview, 1, GL_FALSE, modelview);
	glUniformMatrix4fv(pos_projection, 1, GL_FALSE, projection);
	
	glDrawElements(render_state, m_nindices, GL_UNSIGNED_INT, 0);
}

void CMap::SetChoosedModel(Cmodel *model) {
  choosed_model = model;
}

void CMap::UpdateShader(const Vec3 &p)
{
	GLint pos_in_picked = glGetUniformLocation(m_shader->GetProgram(), "in_picked");
	assert(pos_in_picked >= 0);
	glUniform3f(pos_in_picked, p.x, p.y, p.z);
        
}

void CMap::UpdateShaderScale(const Vec3 &s)
{
        // GLint pos_in_scale = glGetUniformLocation(m_shader->GetProgram(), "in_scaled");
	// assert(pos_in_scale >= 0);
	// glUniform3f(pos_in_scale, s.x, s.y, s.z);
        // std::cout << "scale" << std::endl;
}

void CMap::UpdateShaderLight(const Vec3 &l) {
	GLint pos_light = glGetUniformLocation(m_shader->GetProgram(), "lightpos");
	assert(pos_light >= 0);
	glUniform3f(pos_light, l.x, l.y, l.z);
}

void CMap::UpdateShader(int id, const bool val)
{
	GLint pos = -1;
	switch(id) {
	case 0:
		pos = glGetUniformLocation(m_shader->GetProgram(), "show_texture");
		assert(pos >= 0);
		glUniform1f(pos, (val ? 1.0 : 0.0));
		break;
	case 1:
		pos = glGetUniformLocation(m_shader->GetProgram(), "show_light");
		assert(pos >= 0);
		glUniform1f(pos, (val ? 1.0 : 0.0));
		break;
	default:
		assert(0);
	}
}

void CMap::UpdateShaderTool(const Tools::CTool &tool)
{
	GLint pos_brush_c = glGetUniformLocation(m_shader->GetProgram(), "brush.circle");
	GLint pos_brush_r = glGetUniformLocation(m_shader->GetProgram(), "brush.rect");
	GLint pos_brush_s = glGetUniformLocation(m_shader->GetProgram(), "brush.size");
	assert(pos_brush_c >= 0);
	assert(pos_brush_r >= 0);
	assert(pos_brush_s >= 0);
	
	glUniform1i(pos_brush_s, tool.brush_size);
	
	switch(tool.brush) {
	case Tools::BR_CIRCLE:
		glUniform1i(pos_brush_c, 1);
		glUniform1i(pos_brush_r, 0);
		break;
	case Tools::BR_RECT:
		glUniform1i(pos_brush_c, 0);
		glUniform1i(pos_brush_r, 1);
		break;
	default:
		break;
	}
}

void CMap::AdjustTerrain(
		const Vec3 &p,
		const Tools::CTool &tool,
		float offset,
		const Vec3 &limit)
{
	GLfloat brush_size = tool.brush_size * DEFAULT_NORMALIZE;
	
	Vec3 hm_coord = p * this->GetNormalize();
	int sizex = this->GetWidth();
	int sizey = this->GetHeight();
	
	int hmx = static_cast<int>(hm_coord.x); 
	int hmy = static_cast<int>(hm_coord.z); 
	
	// vertexy
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_array_id);
	float *data = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	// normaly
	glBindBuffer(GL_ARRAY_BUFFER, m_normal_array_id);
	float *normal_data = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

        const GLfloat *initial_vertex_array = m_vertex_array;
	
	int ind;
	int v[4];
	int left, right, top, down;
	if (tool.brush == Tools::BR_CIRCLE || tool.brush == Tools::BR_RECT) {
		for (int offx = -brush_size; offx <= brush_size; ++offx) {
			for (int offy = -brush_size; offy <= brush_size; ++offy) {
				if (hmx+offx < 0  || hmx+offx > sizex-1) continue;
				if (hmy+offy < 0  || hmy+offy > sizey-1) continue;
				// zaruci kruh
				if (tool.brush == Tools::BR_CIRCLE) {
					if (sqrt( pow(offx, 2) + pow(offy, 2) ) > brush_size) continue;
				}

				// float set = offset * (BRUSH_SIZE-abs(offx))/10.0 * (BRUSH_SIZE-abs(offy))/10.0;
				ind = (hmx+offx + (hmy+offy) * sizex) * 3 + 1;
				//// normaly
				// levy
				left = (hmx+offx > 0) ? (hmx+offx-1) : hmx+offx;
				v[0] = (left + (hmy+offy) * sizex) * 3 + 1;
				// pravy
				right = (hmx+offx <= sizex-1) ? (hmx+offx+1) : hmx+offx;
				v[1] = (right + (hmy+offy) * sizex) * 3 + 1;
				// horni
				top = (hmy+offy > 0) ? (hmy+offy-1) : hmy+offy;
				v[2] = (hmx+offx + top * sizex) * 3 + 1;
				// spodni
				down = (hmy+offy <= sizey-1) ? (hmy+offy+1) : hmy+offy;
				v[3] = (hmx+offx + down * sizex) * 3 + 1;
				
				float sx = data[v[1]] - data[v[0]];
				// if (x == 0 || x == w - 1)
					// sx *= 2;
				float sy = data[v[2]] - data[v[3]];
				// if (y == 0 || y == h - 1)
					// sy *= 2;
				
				normal_data[ind-1] = -sx * 1.0/(m_normalize*3); 
				normal_data[ind  ] =   2 * 1.0/m_normalize; 
				normal_data[ind+1] =  sy * 1.0/(m_normalize*3); 
				//// konec normaly

				if (!tool.level) {
					if (tool.tool == Tools::TOOL_LOWER)
						data[ind] -= offset;
					else if (tool.tool == Tools::TOOL_RAISE)
						data[ind] += offset;
					else if (tool.tool == Tools::TOOL_SMOOTH) {
						// Box filter 1/9 1/9 1/9
						//            1/9 1/9 1/9
						//            1/9 1/9 1/9
						const GLfloat *_data = data;                                          
						if (ind-sizex*3-3 >= 0 && ind+sizex*3+3 <= (sizex*sizey)*3){
							float box[9];
							box[0] = _data[ind-sizex*3-3] / 9.0;
							box[1] = _data[ind-sizex*3] / 9.0;
							box[2] = _data[ind-sizex*3+3] / 9.0;
							box[3] = _data[ind-3] / 9.0;
							box[4] = _data[ind] / 9.0;
							box[5] = _data[ind+3] / 9.0;
							box[6] = _data[ind+sizex*3-3] / 9.0;
							box[7] = _data[ind+sizex*3] / 9.0;
							box[8] = _data[ind+sizex*3+3] / 9.0;

							float sum = 0.0;
							for (int i = 0; i != 9; i++) {
								sum += box[i];
							}
							data[ind] = sum; // correct round 
						}
					}
					else if (tool.tool == Tools::TOOL_CLEARBACK) {
						const GLfloat *_data = initial_vertex_array;                                          

						data[ind] = _data[ind];
					}
				}
				else if (tool.level) {
					if (tool.tool == Tools::TOOL_RAISE && data[ind] < limit.y)
						data[ind] += offset;
					if (tool.tool == Tools::TOOL_LOWER && data[ind] > limit.y)
						data[ind] -= offset;
				}
			}
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_normal_array_id);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_array_id);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void CMap::SelectTerrain(const Vec3 &p, Tools::CTool &tool)
{
	Editor::CEditor *editor = Editor::CEditor::GetInstance();
	
	if (m_loaded_models.empty())
		return;

	if (tool.use_model) {
		s_model new_model;
		new_model.id_name = m_models.size() + 1;
		new_model.model = choosed_model; 
		new_model.position = p;
		m_models.push_back(new_model);
	}

	if (tool.move) {
		if (editor->picked_name > 0) {
			m_models[editor->picked_name - 1].position = p; 
		}
	}

	if (tool.rotate) {
		if (editor->picked_name > 0) {
			int diff = (((editor->mx - editor->prev_mx) + (editor->my - editor->prev_my)) / 2) * 4;
			//rotation in degrees;
			if (diff > 0) diff = diff % 360;
			else diff = -abs(diff % 360);
			m_models[editor->picked_name - 1].z_rotation += diff; 
		}
	}

	if (tool.scale) {
		if (editor->picked_name > 0) {
			int diff = (((editor->mx - editor->prev_mx) + (editor->my - editor->prev_my)) / 2) * 4;
			//rotation in degrees;
			if (diff > 0) diff = diff % 360;
			else diff = -abs(diff % 360);
			m_models[editor->picked_name - 1].scale += (diff / 360.0);
		}
	}

	if (tool.del) {
		if (editor->picked_name > 0) {
			editor->DeleteSelectedModel();
		}
	}

	if (tool.clone && !tool.move) {
		s_model new_model;
		if (editor->picked_name > 0) {
			new_model = m_models[editor->picked_name - 1];
			new_model.id_name = m_models.size() + 1;
			m_models.push_back(new_model);
			editor->picked_name = new_model.id_name;
			Tools::CTool *t = &tool;
			t->move = true;
			t->use_model = false;
			t->rotate = false;
			t->scale = false;
			t->del = false;
			t->clone = true;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// CTileGrid
////////////////////////////////////////////////////////////////////////////////
DEFINE_EVENT_TYPE(EVT_EDITOR_PRELOAD_RDY)
/*
 * CTileGrid
 * nacteny svet
 */
CTileGrid::CTileGrid(multimap_t *w, multimap_t &maps, const wxSize &size, const wxPoint &ref):
	m_world_map(w), m_tiles(maps), m_ref(ref), m_size(size),
	m_view(DEFAULT_VIEW_SIZE),
	m_world_offset(wxPoint(0, 0)), m_dir(wxPoint(0,0)),
	m_world_hm(0), m_world_tex(0), m_world_old_hm(0), m_world_old_tex(0),
	m_preload_hm(0), m_preload_tex(0), m_preload_rdy(false),
	m_loader(0)
{ 
	wxSize hm_size = DEFAULT_HM_SIZE;
	hm_size.Scale(m_size.GetWidth(), m_size.GetHeight());
	wxSize tex_size = DEFAULT_TEX_SIZE;
	tex_size.Scale(m_size.GetWidth(), m_size.GetHeight());
	
	// log
	wxLogVerbose(_T("CTileGrid::CTileGrid: Loading world..\n Grid size:\t%dx%d\n HM Grid:\t%dx%d\n Tex Grid:\t%dx%d"),
		m_size.GetWidth(), m_size.GetHeight(),	
		hm_size.GetWidth(), hm_size.GetHeight(),	
		tex_size.GetWidth(), tex_size.GetHeight()
	);
	
	
	// nacist okoli poprve
	SetAndRunLoader();
}

CTileGrid::~CTileGrid()
{
    /*
	 * if (m_world_hm)
	 *     delete m_world_hm;
	 * if (m_world_tex)
	 *     delete m_world_tex;
     */
}

bool CTileGrid::CheckViewBounds(wxPoint *dir)
{
	wxPoint view_br = m_view.GetBottomRight();
	wxPoint world_br = wxPoint(m_size.GetWidth() * DEFAULT_HM_SIZE.GetWidth(),
							   m_size.GetHeight() * DEFAULT_HM_SIZE.GetHeight());
	
	if (m_view.x < 0)
		dir->x = -1;
	if (view_br.x > world_br.x)
		dir->x = 1;
	if (m_view.y < 0)
		dir->y = -1;
	if (view_br.y > world_br.y)
		dir->y = 1;
	
	m_dir = *dir;
	
	if (dir->x == 0 && dir->y == 0)
		return false;
	
	return true;
}

bool CTileGrid::ShiftWorld(wxPoint &dir)
{
	wxPoint pkey(0, 0);
	pkey = dir + m_world_offset;

	multimap_t::iterator it;
	multimap_t newtiles;
	multimap_t::iterator it2;
	for (int y=pkey.y; y<pkey.y+m_size.y; ++y) {
		for (int x=pkey.x; x<pkey.x+m_size.x; ++x) {
			wxPoint p = m_ref + wxPoint(x, y);
			it2 = m_world_map->find(p);
			if (it2 != m_world_map->end()) {
				newtiles.insert(pair_t(wxPoint(x, y), it2->second));
			}
		}
	}

	bool should_preload = false;
	if (newtiles.size() > 0) {
		m_tiles = newtiles;
		m_world_offset += dir;
		should_preload = true;
	}
	
	return should_preload;
}	

void CTileGrid::CreateWorldImage()
{
	wxSize hm_size = DEFAULT_HM_SIZE;
	hm_size.Scale(m_size.GetWidth(), m_size.GetHeight());
	wxSize tex_size = DEFAULT_TEX_SIZE;
	tex_size.Scale(m_size.GetWidth(), m_size.GetHeight());
	
	int tex_factor_x = tex_size.GetWidth() / hm_size.GetWidth();
	int tex_factor_y = tex_size.GetHeight() / hm_size.GetHeight();
	
	// busy wait..
	while (!m_preload_rdy) {}
	assert(m_preload_hm);
	assert(m_preload_tex);
	
	wxRect view;
	view.x = 256 + m_dir.x * 256;
	view.y = 256 + m_dir.y * 256;
	view.width = m_size.GetWidth() * 256;
	view.height = m_size.GetHeight() * 256;
	
	if (m_world_hm)
		delete m_world_hm;
	
	m_world_hm = new wxImage(m_preload_hm->GetSubImage(view));
	view.x *= tex_factor_x;
	view.width *= tex_factor_x;
	view.y *= tex_factor_y;
	view.height *= tex_factor_y;
	
	if (m_world_tex) {
		delete m_world_tex;
	}
	
	m_world_tex = new wxImage(m_preload_tex->GetSubImage(view));
}

void CTileGrid::SetAndRunLoader()
{
	wxLogVerbose(_T("CTileGrid::SetAndRunLoader: Creating loader thread"));
	
	m_preload_rdy = false;
	m_loader = new CLoaderThread(this, m_world_map, m_tiles);
		
	if (m_loader->Create() != wxTHREAD_NO_ERROR)
		throw CException("CTileGrid::SetAndRunLoader: Fatal: Can't create thread");
	
	if (m_loader->Run() != wxTHREAD_NO_ERROR)
		throw CException("CTileGrid::SetAndRunLoader: Fatal: Can't run thread");
}

////////////////////////////////////////////////////////////////////////////////
// CLoaderThread
////////////////////////////////////////////////////////////////////////////////
CLoaderThread::CLoaderThread(CTileGrid *c, multimap_t *w, multimap_t tiles)
	:wxThread(), m_caller(c), m_world_map(w), m_tiles(tiles),
	m_event(EVT_EDITOR_PRELOAD_RDY)
{
	wxEvtHandler();
}
CLoaderThread::~CLoaderThread()
{
}

void *CLoaderThread::Entry()
{
	wxLogVerbose(_T("CLoaderThread: Entry... "));
	wxLogVerbose(_T("CLoaderThread: Tiles: %d"), m_tiles.size());
	
	LoadSurroundingTiles();

	m_caller->SetPreloadHeightMap(m_world_hm);
	m_caller->SetPreloadTexture(m_world_tex);
	m_caller->SetPreloadReady(true);
	
	// zaslat event hlavnimu oknu pro vykresleni prednactenyho obsahu
	wxTheApp->GetTopWindow()->GetEventHandler()->AddPendingEvent(m_event);
	
	return 0;
}

void CLoaderThread::LoadSurroundingTiles()
{
	wxPoint tcoord, wcoord;
	TileInfo tinfo;
	
	multimap_t::iterator it = m_tiles.begin();
	multimap_t::iterator it2;
	
	wxLogVerbose(_T("CLoaderThread: Loading surroundings... "));
	
	multimap_t newtiles;
	// nacist okoli ve forme pozic a cest
	for (; it != m_tiles.end(); ++it) {
		for (int j=-1; j<=1; ++j) {
			for (int i=-1; i<=1; ++i) {
				tcoord = it->first + wxPoint(i, j);
				wcoord = it->second.coord + wxPoint(i, j);
				if (newtiles.find(tcoord) == newtiles.end()) {
					it2 = m_world_map->find(wcoord);
					if (it2 == m_world_map->end()) {
						tinfo.hm_path = _T("");
						tinfo.tex_path = _T("");
					}
					else {
						tinfo = it2->second;
					}
					newtiles.insert(pair_t(tcoord, tinfo));
				}
			}
		}
	}
	
	// novej prednactenej kus sveta bude na kazdy strane o jeden dilek vetsi
	int hm_w = DEFAULT_HM_SIZE.GetWidth() * (2+m_caller->GetSize().GetWidth());
	int hm_h = DEFAULT_HM_SIZE.GetHeight() * (2+m_caller->GetSize().GetHeight());
	m_world_hm = new wxImage(hm_w, hm_h);
	int tex_w = DEFAULT_TEX_SIZE.GetWidth() * ((2+m_caller->GetSize().GetWidth()));
	int tex_h = DEFAULT_TEX_SIZE.GetHeight() * ((2+m_caller->GetSize().GetHeight()));
	m_world_tex = new wxImage(tex_w, tex_h);
	
		
	wxImage *tile_hm, *tile_tex;
	const wxPoint ref = m_caller->GetRef();
	int hm_xoff = 0, hm_yoff = 0, tex_xoff = 0, tex_yoff = 0;

	it = newtiles.begin();
	for (; it != newtiles.end(); ++it)
	{
		tinfo = it->second;
		
		if (tinfo.hm_path == _T("")) {
			tile_hm = new wxImage(256, 256);
		}
		else { 
			// wxImage load
            /*
			 * tile_hm = new wxImage(tinfo.hm_path);
             */
			unsigned short **hm_rows = new unsigned short *[256];
			FILE *input = fopen(tinfo.hm_path.ToAscii(), "rb");
			if (input) {
				pngstream pngs; 
				pngs.initializeInput(input, false); 
				pngs.readImage(256, hm_rows); 
				pngs.finishInput();
				fclose(input);	

				// convert 2B(SHORT HEIGHT VALUE) to 3B (RGB)
				unsigned char *rows = new unsigned char [256 * 256 * 3];
				for (int row = 0; row != 256; row++) {
					for (int i = 0; i != 256; i++) {
						unsigned short val = hm_rows[row][i];
						rows[row * 256 * 3 + i * 3 + 0] = val >> 8;
						rows[row * 256 * 3 + i * 3 + 1] = val & 255;
						rows[row * 256 * 3 + i * 3 + 2] = 0;
					}
				}


				tile_hm = new wxImage(pngs.pngheader.width, pngs.pngheader.height, false);
				if (rows) {
					tile_hm->SetData(rows);
				}
			}
			else {
				// teren v datasetu neni, nahradit
				tile_hm = new wxImage(_T("../gfx/terrain_missing.png"));
			}
		}
		assert(tile_hm->GetWidth() > 0);

		if (tinfo.tex_path == _T("")) {
			tile_tex = new wxImage(2048, 2048);
		}
		else {
			// wxImage load
            /*
			 * tile_tex = new wxImage(tinfo.tex_path);
             */
			
			// create jpgreader
			jpgstream jpgs;

			// open input file
			FILE *input = fopen(tinfo.tex_path.ToAscii(), "rb");
			if (input) {
				// init jpg reader
				jpgs.initializeInput(input);

				// allocate memory for whole image == width * height * number of components
				unsigned char *rows = (unsigned char *) malloc(sizeof(unsigned char) * jpgs.jpgheader.width * jpgs.jpgheader.nc * jpgs.jpgheader.height);
				if (rows) {
					// decompress jpg
					jpgs.readLines(rows);
				}
				// finish decompression
				jpgs.finishInput();
				fclose(input);                

				// don't clear data because of slow filling
				tile_tex = new wxImage(jpgs.jpgheader.width, jpgs.jpgheader.height, false);
				if (rows) {
					tile_tex->SetData(rows);
				}
			}
			else {
				// textura v datasetu neni, nahradit
				tile_tex = new wxImage(_T("../gfx/texture_missing.png"));
			}
			
			wxLogVerbose(_T("Loading texture tile: ") + tinfo.tex_path);
		}
		assert(tile_tex->GetWidth() > 0);
		
		if (!tile_hm)
			throw CException("CLoaderThread::LoadSurroundingTiles: Can't create wxImage");
		if (!tile_tex)
			throw CException("CLoaderThread::LoadSurroundingTiles: Can't create wxImage");
		 
		wxPoint woff = m_caller->GetWorldOffset();
		hm_xoff = (it->first.x-woff.x+1) * 256;
		hm_yoff = (it->first.y-woff.y+1) * 256;
		tex_xoff = (it->first.x-woff.x+1) * 2048;
		tex_yoff = (it->first.y-woff.y+1) * 2048;
		m_world_hm->Paste(*tile_hm, hm_xoff, hm_yoff);
		m_world_tex->Paste(*tile_tex, tex_xoff, tex_yoff);
		
		delete tile_hm;
		delete tile_tex;
	}
	
	// data pro hlavni okno, aby mohlo prekreslit mapky
	std::vector<wxImage *> *evt_data = new std::vector<wxImage *>();
	evt_data->push_back(m_world_hm);
	evt_data->push_back(m_world_tex);
	m_event.SetClientData(evt_data);
	
	wxLogVerbose(_T("CLoaderThread: Done loading surroundings."));
}
