#ifndef _CMAP_H_
#define _CMAP_H_

#define GL_GLEXT_PROTOTYPES
#include <wx/wx.h>
#include <GL/gl.h>

class CMap {
	// data
	GLfloat *m_vertex_array;
	GLuint m_vertex_array_id;
	
	GLuint *m_index_array;
	GLuint m_index_array_id;
	
	GLfloat *m_color_array;
	GLuint m_color_array_id;
	
	GLfloat *m_texcoord_array;
	GLuint m_texcoord_array_id;
	
	GLuint m_ntriangles;
	GLuint m_nindices;
	GLuint m_nverts;
	
	GLfloat m_normalize;
	static const GLfloat DEFAULT_NORMALIZE = 5.0;
	
	GLuint m_width;
	GLuint m_height;
	wxImage *m_heightmap;
	wxImage *m_texture;
	GLuint m_texture_id;
	
	wxString m_heightmap_name;
	wxString m_texture_name;
	
public:
	CMap();
	~CMap();
	
	void Create(int, int, bool);
	void CreateNew(int, int);
	void Load(const wxString &, const wxString &);
	void Save(const wxString &);
	
	void SendToClient();
	void Render(int);
	
	// set n get
	GLuint GetVertexArrayID() { return m_vertex_array_id; }
	GLuint GetColorArrayID() { return m_color_array_id; }
	
	wxImage *GetHeightMap() { return m_heightmap; }
	
	GLfloat GetNormalize() { return m_normalize; }
	
	const wxString &GetHeightMapName() { return m_heightmap_name; }
	const wxString &GetTextureName() { return m_texture_name; }
	
	GLuint GetWidth() { return m_width; }
	GLuint GetHeight() { return m_height; }
};

#endif
