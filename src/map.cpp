#include "map.h"

CMap::CMap():
	m_vertex_array(0), m_vertex_array_id(0),
	m_index_array(0), m_index_array_id(0),
	m_color_array(0), m_color_array_id(0),
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
	if (m_color_array)
		delete [] m_color_array;
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
	if (m_color_array != 0)
		delete [] m_color_array;
	if (m_texcoord_array != 0)
		delete [] m_texcoord_array;
	
	// if (m_heightmap == 0)
	
	m_vertex_array = new GLfloat[m_nverts];
	m_index_array = new GLuint[m_nindices];
	m_color_array = new GLfloat[m_nverts];
	m_texcoord_array = new GLfloat[m_nverts * 2];

	GLfloat xyz[3];
	GLfloat uv[2];
	GLuint iii[3];

	int ind = 0, col_i = 0, tex_i = 0;
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

			// copy color
			xyz[0] = xyz[1] = xyz[2] = col / 17.0 + 0.15;
			memcpy(m_color_array + col_i, xyz, sizeof(xyz)); col_i += 3;
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
	if (!m_heightmap)
		throw;
	m_heightmap_name = heightmap_name;
	m_texture_name = texture_name;

	int w = m_heightmap->GetWidth();
	int h = m_heightmap->GetHeight();
	
	this->Create(w, h);
}

void CMap::Save(const wxString &fname)
{
	// vzit displacement mapu, pricist k originalu, ulozit pres wxImage
}

// TODO TODO TODO
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
		glBindTexture(GL_TEXTURE_2D, m_texture_id);

		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_texture->GetWidth(),
				m_texture->GetWidth(), 0, GL_RGB, GL_UNSIGNED_BYTE,
				m_texture->GetData());
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
	if (m_color_array_id == 0)
		glGenBuffers(1, &m_color_array_id);
	else
		ids_del[nbufs_del++] = m_color_array_id;
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
	// shaders data
	// --------------
	// glVertexAttribPointer(pos_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// glEnableVertexAttribArray(pos_vertex);
	// --------------

	GLint pos_vertex = glGetAttribLocation(prog, "vertex");
	GLint pos_color = glGetAttribLocation(prog, "in_color");
	GLint pos_texcoord = glGetAttribLocation(prog, "in_texcoord");
	assert(pos_vertex >= 0);
	assert(pos_color >= 0);
	assert(pos_texcoord >= 0);
	
	glVertexAttribPointer(pos_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(pos_vertex);
	// DEBUG
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &b);
	cout << "vertex array size: " << b << endl;
	
	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_array_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nindices*sizeof(GLuint),
			m_index_array, GL_STATIC_DRAW);
	// DEBUG
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &b);
	cout << "index array size: " << b << endl;
	
	// color buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_color_array_id);
	glBufferData(GL_ARRAY_BUFFER, m_nverts*sizeof(GLfloat), m_color_array, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(pos_color, 3, GL_FLOAT, GL_TRUE, 0, 0);
	glEnableVertexAttribArray(pos_color);
	
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
	
	GLint pos_modelview = glGetUniformLocation(prog, "modelview");
	assert(pos_modelview >= 0);
	
	GLint pos_projection = glGetUniformLocation(prog, "projection");
	assert(pos_projection >= 0);
	
	GLfloat projection[16], modelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
	glGetFloatv(GL_PROJECTION_MATRIX, projection);
	
	glUniformMatrix4fv(pos_modelview, 1, GL_FALSE, modelview);
	glUniformMatrix4fv(pos_projection, 1, GL_FALSE, projection);
	
    /*
	 * if (m_use_shaders) {
	 *     m_vert_shader->Use();
	 *     m_frag_shader->Use();
	 * }
	 * else {
	 *     glUseProgram(0);
	 * }
     */
	// glEnable(GL_CULL_FACE);

	glDrawElements(render_state, m_nindices, GL_UNSIGNED_INT, 0);
}
