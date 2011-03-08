#include "map.h"
#include "exception.h"

CMap::CMap():
	m_vertex_array(0), m_vertex_array_id(0),
	m_index_array(0), m_index_array_id(0),
	m_texcoord_array(0), m_texcoord_array_id(0),
	m_ntriangles(0), m_nindices(0), m_nverts(0),
	m_normalize(DEFAULT_NORMALIZE), m_width(0), m_height(0),

	m_heightmap(0), m_texture(0), m_texture_id(0),
	m_heightmap_name(_T("")), m_texture_name(_T("")),
	m_shader(0), m_use_shaders(true)
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
void CMap::InitShaders()
{
	// TODO remove hardcode
	m_shader = new CShader();
	m_shader->Add("shader/shader.vert", GL_VERTEX_SHADER);
	m_shader->Add("shader/shader.frag", GL_FRAGMENT_SHADER);
	m_shader->Link();
}

void CMap::Create(int w, int h, bool is_new = false)
{
	m_nverts = w * h * 3;
	m_ntriangles = (w-1) * (h-1) * 2;
	m_nindices = m_ntriangles * 3;
	
	m_width = w;
	m_height = h;


	if (m_vertex_array != 0)
		delete [] m_vertex_array;
	if (m_index_array != 0)
		delete [] m_index_array;
	if (m_texcoord_array != 0)
		delete [] m_texcoord_array;
	
	// if (m_heightmap == 0)
	
	m_vertex_array = new GLfloat[m_nverts];
	m_index_array = new GLuint[m_nindices];
	m_texcoord_array = new GLfloat[m_nverts * 2];

	GLfloat xyz[3];
	GLfloat uv[2];
	GLuint iii[3];

	int ind = 0, tex_i = 0;
	float col;
	// heightmap -> vertex array
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			// copy vertex
			xyz[0] = x / m_normalize;
			if (is_new)
				col = 0.0;
			else
				col = m_heightmap->GetRed(x, y) / (m_normalize*4);
			xyz[1] = col;
			xyz[2] = y / m_normalize;
			memcpy(m_vertex_array + ind, xyz, sizeof(xyz));
			ind += 3;

			// copy texture coord
			if (m_texture) {
				uv[0] = x / (float)w;
				uv[1] = y / (float)h;
				memcpy(m_texcoord_array + tex_i, uv, sizeof(uv)); tex_i += 2;
			}
		}
	}

	ind = 0;
	GLuint s1, s2;
	// create indices to the vertex array
	GLuint i_offset = 0;
	GLuint siz = (w-1)*(h-1);
	while (i_offset < siz) {
		for (int x = 0; x < w-1; ++x) {
			iii[0] = x + i_offset;
			iii[1] = s1 = x+1 + i_offset;
			iii[2] = s2 = x + i_offset+w;
			memcpy(m_index_array + ind, iii, sizeof(iii));
			ind += 3;

			iii[0] = s2;
			iii[1] = s1;
			iii[2] = x+1 + i_offset+w;
			memcpy(m_index_array + ind, iii, sizeof(iii));
			ind += 3;
		}
		i_offset += w;
	}
	
}

void CMap::Load(const wxString &heightmap_name, const wxString &texture_name)
{
	if (m_heightmap != 0)
		delete m_heightmap;
	if (m_texture != 0)
		delete m_texture;
	
	m_heightmap = new wxImage(heightmap_name);
	// TODO
	if (!m_heightmap)
		throw;
	m_texture = new wxImage(texture_name);
	// TODO
	if (!m_texture)
		throw;
	m_heightmap_name = heightmap_name;
	m_texture_name = texture_name;
	

	int w = m_heightmap->GetWidth();
	int h = m_heightmap->GetHeight();
	
	
	this->Create(w, h);
}

/*
 * void CMap::Load(wxImage *heightmap, wxImage *texture,
 *         const wxString &heightmap_name, const wxString &texture_name)
 */
void CMap::Load(CTileGrid *tilegrid)
{
	
	// DEBUG
	std::cerr << "CMap::Load: Loading from tilegrid." << std::endl;
    /*
	 * 
	 * if (m_heightmap != 0)
	 *     delete m_heightmap;
	 * if (m_texture != 0)
	 *     delete m_texture;
	 * 
     */
}

void CMap::LoadFromWorldMap()
{
}

void CMap::CreateFromView(CTileGrid *tilegrid)
{
	if (m_heightmap != 0)
		delete m_heightmap;
	if (m_texture != 0)
		delete m_texture;
	
	// DEBUG
	// std::cout << "CMap::CreateFromView: Creating.." << std::endl;
	
	// wxSize hm_size = DEFAULT_HM_SIZE;
	// wxSize tex_size = DEFAULT_TEX_SIZE;
	wxRect view = tilegrid->GetView();
	// DEBUG
	// std::cout << "CMap::CreateFromView: Using view: " << view.x<<","<<view.y<<" : "
	// <<view.width<<","<<view.height<< std::endl;
	
	
	wxImage *hm, *tex;
	hm = tilegrid->GetWorldHeightMap();
	tex = tilegrid->GetWorldTexture();
	
	wxBitmap tmp(*hm); 
	wxMemoryDC srcDC;
	srcDC.SelectObject(tmp);
	
	wxBitmap hm_bitmap(view.width, view.height);
	wxMemoryDC memDC;
	memDC.SelectObject(hm_bitmap);
	memDC.Blit(0, 0, view.width, view.height,
			&srcDC, view.x, view.y);
	m_heightmap = new wxImage(hm_bitmap.ConvertToImage());
	
	int tex_factor_x = tex->GetWidth() / hm->GetWidth();
	int tex_factor_y = tex->GetHeight() / hm->GetHeight();
	
	tmp = wxBitmap(*tex);
	srcDC.SelectObject(tmp);
	wxBitmap tex_bitmap(view.width*tex_factor_x, view.height*tex_factor_y);
	memDC.SelectObject(tex_bitmap);
	memDC.Blit(0, 0, view.width*tex_factor_x, view.height*tex_factor_y,
			&srcDC, view.x*tex_factor_x, view.y*tex_factor_y);
	
	m_texture = new wxImage(tex_bitmap.ConvertToImage());

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
	// TODO
	if (!m_texture)
		throw;
	m_texture_name = fname;
	
	SetOpenGLTexture();
}

void CMap::Save(const wxString &fname)
{
	// nejdriv vytvorit wximage
	glBindBuffer(GL_ARRAY_BUFFER, GetVertexArrayID());
	float *data = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	unsigned char *cdata = new unsigned char[m_nverts];
	
	unsigned char texel;
	int aux;
	for (unsigned int j = 0; j < m_nverts; j += 3) {
		aux = static_cast<int>(data[j+1] * m_normalize * 4);
		texel = (aux <= 255) ? aux : 255;
		cdata[j] = texel;
		cdata[j+1] = texel;
		cdata[j+2] = texel;
	}
	
	// ulozit
	std::cout << "saving" << std::endl;
	wxImage *bmp = new wxImage(m_width, m_height, cdata, true);
	bmp->SaveFile(fname, wxBITMAP_TYPE_PNG);
	std::cout << "saved" << std::endl;
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	delete [] cdata;
	delete bmp;
	
}

void CMap::SendToClient()
{
	GLuint nbufs_del = 0;
	GLuint ids_del[4] = {0,};
	// DEBUG var
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

	if (nbufs_del > 0) {
		glDeleteBuffers(nbufs_del, ids_del);
		// DEBUG
		std::cout << "deleting buffers from gpu ram: " << nbufs_del << std::endl;
	}
	
	/////////////////////////
	GLint prog = m_shader->GetProgram();
	m_shader->Use();
	
	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_array_id);
	glBufferData(GL_ARRAY_BUFFER, m_nverts*sizeof(GLfloat), m_vertex_array, GL_DYNAMIC_DRAW);
	
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
	assert(pos_vertex >= 0);
	assert(pos_texcoord >= 0);
	
	glVertexAttribPointer(pos_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(pos_vertex);
	// DEBUG
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &b);
	std::cout << "vertex array size: " << b << std::endl;
	
	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_array_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nindices*sizeof(GLuint),
			m_index_array, GL_STATIC_DRAW);
	// DEBUG
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &b);
	std::cout << "index array size: " << b << std::endl;
	
	// texture buffer
	if (m_texcoord_array) {
		glBindBuffer(GL_ARRAY_BUFFER, m_texcoord_array_id);
		glBufferData(GL_ARRAY_BUFFER, m_nverts*2*sizeof(float), m_texcoord_array, GL_STATIC_DRAW);
		glVertexAttribPointer(pos_texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(pos_texcoord);
	}
}

void CMap::Render(int render_state)
{
	GLuint prog = m_shader->GetProgram();
    /*
	 * if (m_use_shaders) {
	 *     m_shader->Use();
	 * }
	 * else {
	 *     glUseProgram(0);
	 * }
     */
	
	// TODO optim: getUniformy se muzou volat jen jednou a ne tady
	GLint pos_modelview = glGetUniformLocation(prog, "modelview");
	assert(pos_modelview >= 0);
	
	GLint pos_projection = glGetUniformLocation(prog, "projection");
	assert(pos_projection >= 0);
	
	GLfloat projection[16], modelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
	glGetFloatv(GL_PROJECTION_MATRIX, projection);
	
	glUniformMatrix4fv(pos_modelview, 1, GL_FALSE, modelview);
	glUniformMatrix4fv(pos_projection, 1, GL_FALSE, projection);
	
	// glEnable(GL_CULL_FACE);

	glDrawElements(render_state, m_nindices, GL_UNSIGNED_INT, 0);
}

void CMap::UpdateShader(const Vec3 &p)
{
	GLint pos_in_picked = glGetUniformLocation(m_shader->GetProgram(), "in_picked");
	assert(pos_in_picked >= 0);
	glUniform3f(pos_in_picked, p.x, p.y, p.z);
}

////////////////////////////////////////////////////////////////////////////////
// CTileGrid
////////////////////////////////////////////////////////////////////////////////
/*
 * CTileGrid
 * nacteny svet
 */
CTileGrid::CTileGrid(std::vector<TileInfo> &maps, const wxSize &size):
	m_tiles(maps), m_size(size), m_view(DEFAULT_VIEW_SIZE)
{
	// DEBUG
    /*
	 * std::vector<TileInfo>::iterator it = maps.begin();
	 * for (; it != maps.end(); it++) {
	 *     std::cout << it->coord.x << ", " << it->coord.y << ": " << it->path.ToAscii() << std::endl;
	 * }
     */
	
	wxSize hm_size = DEFAULT_HM_SIZE;
	hm_size.Scale(m_size.GetWidth(), m_size.GetHeight());
	wxSize tex_size = DEFAULT_TEX_SIZE;
	tex_size.Scale(m_size.GetWidth(), m_size.GetHeight());
	
	// DEBUG
	std::cout << "CTileGrid::CTileGrid: loading world" << std::endl;
	std::cout << "\tgrid size: \t" << m_size.GetWidth() << "x" << m_size.GetHeight() << std::endl;	
	std::cout << "\thm grid: \t" << hm_size.GetWidth() << "x" << hm_size.GetHeight() << std::endl;	
	std::cout << "\ttex grid: \t" << tex_size.GetWidth() << "x" << tex_size.GetHeight() << std::endl;	
	
	// wxSize hm_size_x = DEFAULT_HM_SIZE.GetWidth() * tilegrid.GetSize().GetWidth();

	m_world_hm = new wxImage(
		hm_size.GetWidth(),
		hm_size.GetHeight()
	);
	m_world_tex = new wxImage(
		tex_size.GetWidth(),
		tex_size.GetHeight()
	);
	
	wxString tex_path;
	wxImage *tile_hm, *tile_tex;
	int hm_xoff = 0, hm_yoff = 0, tex_xoff = 0, tex_yoff = 0;
	int hm_offlimit = hm_size.GetWidth() - DEFAULT_HM_SIZE.GetWidth();
	int tex_offlimit = tex_size.GetWidth() - DEFAULT_TEX_SIZE.GetWidth();

	std::vector<TileInfo>::iterator it = m_tiles.begin();
	for (; it != m_tiles.end(); it++) {
		tex_path = it->path;
        /*
		 * TODO Replace bere celou cestu a nahradi suffix vyskovy mapy za suffix textury.
		 * Pokud bude nekde v ceste podretezec stejnej jako suffix vyskovy mapy, nahradi ho taky
		 * mozna FIXME
         */
		tex_path.Replace(HEIGHTMAP_SUFFIX, TEXTURE_SUFFIX);
		tex_path.Replace(_T("terrain_tiles"), _T("texture_tiles"));
		
		// DEBUG
		std::cout << "tex_path: " << tex_path.ToAscii() << std::endl;
		
		tile_hm = new wxImage(it->path);
		if (!tile_hm)
			throw CException("CTileGrid::CTileGrid: Can't create wxImage");
		tile_tex = new wxImage(tex_path);
		if (!tile_tex)
			throw CException("CTileGrid::CTileGrid: Can't create wxImage");
		 
		// DEBUG
		std::cout << "hm_xoff " << hm_xoff << ", hm_yoff " << hm_yoff << std::endl;
		
		// TODO nahradit za memDC jestli to nebude rychlejsi?
		m_world_hm->Paste(*tile_hm, hm_xoff, hm_yoff);
		m_world_tex->Paste(*tile_tex, tex_xoff, tex_yoff);
		
		NextHMOffset(&hm_xoff, &hm_yoff, hm_offlimit, hm_offlimit);
		NextTexOffset(&tex_xoff, &tex_yoff, tex_offlimit, tex_offlimit);
		
		// DEBUG
		std::cout << it->path.ToAscii() << ": " << it->coord.x << ", " << it->coord.y << std::endl;
		
		delete tile_hm;
		delete tile_tex;
	}
	
}
CTileGrid::~CTileGrid()
{
}

void CTileGrid::MoveView(const wxPoint &offset)
{
	m_view.Offset(offset);
	// DEBUG
	std::cout << "CTileGrid::MoveView: " << m_view.GetX() << ", " << m_view.GetY() << std::endl;
}
// inflateview, deflateview => wxRect::Inflate, deflate...

void CTileGrid::Save()
{
	// DEBUG
	std::cout << "CTileGrid::Save: Saving.." << std::endl;
	
	int hm_xoff = 0, hm_yoff = 0, tex_xoff = 0, tex_yoff = 0;
	int hm_offlimit = m_world_hm->GetWidth() - DEFAULT_HM_SIZE.GetWidth();
	int tex_offlimit = m_world_tex->GetWidth() - DEFAULT_TEX_SIZE.GetWidth();
	
	wxBitmap world_bmp(*m_world_hm);
	wxMemoryDC srcDC;
	srcDC.SelectObject(world_bmp);
	
	wxBitmap dest_bmp(DEFAULT_HM_SIZE.GetWidth(), DEFAULT_HM_SIZE.GetHeight());
	wxMemoryDC destDC;
	destDC.SelectObject(dest_bmp);
	
	std::vector<TileInfo>::iterator it = m_tiles.begin();
	for (; it != m_tiles.end(); it++) {
		// DEBUG
		std::cout << it->coord.x << ", " << it->coord.y << ": " << it->path.ToAscii() << std::endl;
		std::cout << "hm_xoff " << hm_xoff << ", hm_yoff " << hm_yoff << std::endl;

		destDC.Blit(0, 0, dest_bmp.GetWidth(), dest_bmp.GetHeight(),
					&srcDC, hm_xoff, hm_yoff);
		wxImage *img = new wxImage(dest_bmp.ConvertToImage());
		wxString p = it->path;
		// TODO tmp
		// p.Replace(_T("../"), _T("./"));
		p.Replace(_T(".png"), _T(".png.png"));
		// TODO tmp
		bool ok = img->SaveFile(p);
		assert(ok);

		NextHMOffset(&hm_xoff, &hm_yoff, hm_offlimit, hm_offlimit);
		NextTexOffset(&tex_xoff, &tex_yoff, tex_offlimit, tex_offlimit);
	}
}

void CTileGrid::NextHMOffset(int *xoff, int *yoff, int xofflimit, int yofflimit)
{
		if (*xoff >= xofflimit) {
			*xoff = 0;
			*yoff += DEFAULT_HM_SIZE.GetHeight();
		}
		else {
			*xoff += DEFAULT_HM_SIZE.GetWidth();
		}
        /*
		 * if (yoff > yofflimit)
		 *     yoff = 0;
         */
}
void CTileGrid::NextTexOffset(int *xoff, int *yoff, int xofflimit, int yofflimit)
{
		if (*xoff >= xofflimit) {
			*xoff = 0;
			*yoff += DEFAULT_TEX_SIZE.GetHeight();
		}
		else {
			*xoff += DEFAULT_TEX_SIZE.GetWidth();
		}
        /*
		 * if (yoff > yofflimit)
		 *     yoff = 0;
         */
}

