#ifndef _CMAP_H_
#define _CMAP_H_

#define GL_GLEXT_PROTOTYPES
#include <wx/wx.h>
#include <GL/gl.h>
#include "vec3.h"
#include "shader.h"
#include "globals.h"

const wxSize DEFAULT_HM_SIZE(256, 256);
const wxSize DEFAULT_TEX_SIZE(2048, 2048);
const wxSize DEFAULT_VIEW_SIZE(256, 256);
/*
 * const wxSize DEFAULT_TILEGRID_HM_SIZE(512, 512);
 * const wxSize DEFAULT_TILEGRID_TEX_SIZE(4096, 4096);
 */

struct TileInfo {
	wxPoint coord;
	wxString path;
	TileInfo(): coord(wxPoint(0, 0)), path(_T("")) {}
	TileInfo(const wxPoint &c, const wxString &p): coord(c), path(p) {}
	~TileInfo() {};
};

class CTileGrid;
class CMap {
	// data
	GLfloat *m_vertex_array;
	GLuint m_vertex_array_id;
	
	GLuint *m_index_array;
	GLuint m_index_array_id;
	
	GLfloat *m_texcoord_array;
	GLuint m_texcoord_array_id;
	
	GLuint m_ntriangles;
	GLuint m_nindices;
	GLuint m_nverts;
	
	GLfloat m_normalize;
	static const GLfloat DEFAULT_NORMALIZE = 5.0;
	
	wxImage *m_world;
	
	GLuint m_width;
	GLuint m_height;
	wxImage *m_heightmap;
	wxImage *m_texture;
	GLuint m_texture_id;
	
	
	wxString m_heightmap_name;
	wxString m_texture_name;
	
	CShader *m_shader;
	bool m_use_shaders;
public:
	CMap();
	~CMap();
	
	void Create(int, int, bool);
	void CreateNew(int, int);
	void CreateFromView(CTileGrid *);
	void Load(const wxString &, const wxString &);
	// void Load(wxImage *, wxImage *, const wxString &heightmap_name,
			// const wxString &texture_name);
	void Load(CTileGrid *);
	void Save(const wxString &);
	
	void SendToServer();
	void Render(int);
	
	// set n get
	GLuint GetVertexArrayID() { return m_vertex_array_id; }
	GLuint GetTexCoordArrayID() { return m_texcoord_array_id; }
	
	wxImage *GetHeightMap() { return m_heightmap; }
	wxImage *GetTexture() { return m_texture; }
	void SetOpenGLTexture();
	void SetTexture(const wxString &);
	
	GLfloat GetNormalize() { return m_normalize; }
	
	const wxString &GetHeightMapName() { return m_heightmap_name; }
	const wxString &GetTextureName() { return m_texture_name; }
	
	GLuint GetWidth() { return m_width; }
	GLuint GetHeight() { return m_height; }
	
	void InitShaders();
	void UseShaders(bool b) { m_use_shaders = b; }
	void UpdateShader(const Vec3 &);
};


class CTileGrid {
	/// informace o polickach
	std::vector<TileInfo> m_tiles;
	/// velikost gridu
	wxSize m_size;
	/// [x, y] pohledu
	wxRect m_view;
	/// data
	// loaded world tiles
	wxImage *m_world_hm;
	wxImage *m_world_tex;
	
	void NextHMOffset(int *, int *, int, int);
	void NextTexOffset(int *, int *, int, int);
protected:
public:
	CTileGrid(std::vector<TileInfo> &, const wxSize &);
	~CTileGrid();
	
	void MoveView(const wxPoint &);
	
	void Save();
	
	// get
	std::vector<TileInfo> &GetTiles() { return m_tiles; }
	wxSize &GetSize() { return m_size; }
	wxRect &GetView() { return m_view; }
	wxImage *GetWorldHeightMap() { return m_world_hm; }
	wxImage *GetWorldTexture() { return m_world_tex; }
	
	// set
	void SetView(const wxRect &r) { m_view = r; }
};

#endif
