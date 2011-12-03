#ifndef _CMAP_H_
#define _CMAP_H_

#define GL_GLEXT_PROTOTYPES
#include <wx/wx.h>
#include <GL/gl.h>

#include <map>
#include <set>
#include <vector>

#include "vec3.h"
#include "shader.h"
#include "globals.h"
#include "tools.h"
#include "model.h"

/// implicitni velikost vyskove mapy
const wxSize DEFAULT_HM_SIZE(256, 256);
/// implicitni textury mapy
const wxSize DEFAULT_TEX_SIZE(2048, 2048);
/// implicitni velikost vyrezu sveta (pohledu do sveta)
const wxSize DEFAULT_VIEW_SIZE(256, 256);
/// pomer velikosti textury k velikosti vyskove mapy
const wxPoint HM_TEX_FACTOR(8, 8);

/**
 * TileInfo
 * informace o svetovem policku
 * mozna budouci rozsireni: udaje o dalsich mapach (displacement, ...)
 */
struct TileInfo {
	/// pozice
	wxPoint coord;
	/// cesta k souboru s vyskovou mapou
	wxString hm_path;
	/// cesta k souboru s texturou
	wxString tex_path
	
	TileInfo(): coord(wxPoint(0, 0)), hm_path(_T("")), tex_path(_T("")) {}
	TileInfo(const TileInfo &ti):
		coord(ti.coord),
		hm_path(ti.hm_path),
		tex_path(ti.tex_path) {}
	// TileInfo(const wxPoint &c): coord(c) {}
	~TileInfo() {};
	
	TileInfo &operator= (const TileInfo &ti)
	{
		coord = ti.coord;
		hm_path = ti.hm_path;
		tex_path = ti.tex_path;
		return (*this);
	}
	
	///util
	void dump() {
		std::cout << "coord: [" << coord.x << ", " << coord.y << "]" << std::endl;
		std::cout << "hm_path: " << hm_path.ToAscii() << std::endl;
		std::cout << "tex_path: " << tex_path.ToAscii() << std::endl;
	}
};

/**
 * pomocna struktura pro porovnavani souradnic typu wxPoint
 * nutne pro definovani usporadani v std::map/multimap
 */
struct PointCompare {
	bool operator()(const wxPoint &p1, const wxPoint &p2) {
		return (p1.x == p2.x) ? (p1.y < p2.y) : (p1.x < p2.x);
	}
};
struct VecCompare {
	bool operator()(const Vec3 &p1, const Vec3 &p2) {
		// == na floaty nic moc, ale vypada, ze to funguje
		return (p1.x == p2.x) ? ((p1.y == p2.y) ? (p1.z < p2.z) : (p1.y < p2.y)) : (p1.x < p2.x);
	}
};

struct Vec2i {
  int x;
  int y;
  Vec2i() {
  }
  Vec2i(int _x, int _y)  {
    x = _x;
    y = _y;
  }
};

struct s_model {
  float    scale;
  float    z_rotation;
  Vec3     position;
  Vec2i    tile;
  int      id_name;
  wxString modelname;
  Cmodel * model; 
  s_model() {
    scale = 1.0;;
    z_rotation = 0;
    position = Vec3(0, 0, 0);
    tile.x = 0;
    tile.y = 0;
    id_name = 0;
    modelname = (wxChar)"";
    model = NULL;
  }
};

/// reprezentace nacteneho sveta
typedef std::multimap<wxPoint, TileInfo, PointCompare> multimap_t;
typedef multimap_t::value_type pair_t;

class CTileGrid;
class CLoaderThread;
/**
 * CMap
 * reprezentace modelu sceny
 * data pro grafickou kartu
 */
class CMap {
	/// vyskova data
	GLfloat *m_vertex_array;
	/// VBO index
	GLuint m_vertex_array_id;
	
	/// indexy pro vyskova data
	GLuint *m_index_array;
	/// VBO index 
	GLuint m_index_array_id;
	
	/// texturovaci souradnice
	GLfloat *m_texcoord_array;
	/// VBO index 
	GLuint m_texcoord_array_id;
	
	/// normaly
	GLfloat *m_normal_array;
	/// VBO index 
	GLuint m_normal_array_id;
	
	/// pocet trojuhelniku pro grafickou kartu
	GLuint m_ntriangles;
	/// pocet indexu pro grafickou kartu
	GLuint m_nindices;
	/// pocet vrcholu pro grafickou kartu
	GLuint m_nverts;
	
	/// hodnota pro normalizaci vyskovych dat pri vykreslovani
	GLfloat m_normalize;
	
	/// data vyskove mapy
	wxImage *m_heightmap;
	/// cesta ve filesystemu
	wxString m_heightmap_name;
	
	/// data textury
	wxImage *m_texture;
	/// cesta ve filesystemu
	wxString m_texture_name;
	/// id textury
	GLuint m_texture_id;
	
	/// sirka modelu
	GLuint m_width;
	/// vyska modelu
	GLuint m_height;

	/// shader objekt
	CShader *m_shader;
	
	///
	bool m_compute_indices_and_texcoords;
	
	std::set<Vec3, VecCompare> m_selected;
	std::vector<s_model> m_models;
	std::map<std::string, Cmodel *> m_loaded_models;
        Cmodel *choosed_model;
	
	/// implicitni hodnota pro normalizaci vyskovych dat pri vykreslovani
	static const GLfloat DEFAULT_NORMALIZE = 2.0;
public:
	CMap();
	~CMap();
	
	/// vytvoreni modelu z vyskove mapy
	void Create(int, int, bool);
	
	/// vytvoreni modelu z pohledu na svet
	void CreateFromView(CTileGrid *, bool shift = false);
	
	void Load(const wxString &, const wxString &);
	// void Load(wxImage *, wxImage *, const wxString &heightmap_name,
			// const wxString &texture_name);
	void Load(CTileGrid *);
	void Save(CTileGrid *);
	
	/// odeslani dat graficke karte
	void SendToServer();
	/// vykresleni sceny
	void Render(int);
	
	void InitShaders(const Tools::CTool &);
        void SetChoosedModel(Cmodel *model);
	void UpdateShader(const Vec3 &);
	void UpdateShaderScale(const Vec3 &);
	void UpdateShader(int, const bool);
	void UpdateShaderTool(const Tools::CTool &);
	void UpdateShaderSelected();
	void UpdateShaderLight(const Vec3&);
	
	// nastroje
	/// snizeni/zvyseni/zarovnani terenu
	void AdjustTerrain(const Vec3 &, const Tools::CTool &, float, const Vec3 & = Vec3(0,0,0));
	void SelectTerrain(const Vec3 &, Tools::CTool &);
	
	/// set n get
	GLuint GetVertexArrayID() { return m_vertex_array_id; }
	GLfloat *GetVertexArray() { return m_vertex_array; }
	GLuint GetTexCoordArrayID() { return m_texcoord_array_id; }
	GLfloat GetNormalize() { return m_normalize; }
	void SetOpenGLTexture();
	void SetTexture(const wxString &);
	
	wxImage *GetHeightMap() { return m_heightmap; }
	const wxString &GetHeightMapName() { return m_heightmap_name; }
	wxImage *GetTexture() { return m_texture; }
	const wxString &GetTextureName() { return m_texture_name; }
	GLuint GetWidth() { return m_width; }
	GLuint GetHeight() { return m_height; }
	
	CShader *GetShader() { return m_shader; }
	std::set<Vec3, VecCompare> &GetSelected() { return m_selected; }
	std::vector<s_model> &GetModels() { return m_models; }
	std::map<std::string, Cmodel *> &GetLoadedModels() { return m_loaded_models; }
	
};

class CTileGrid: public wxEvtHandler {
	/// info o svete, ziskava od CEditor
	multimap_t *m_world_map;
	/// informace o polickach
	multimap_t m_tiles;
	/// vychozi souradnice datasetu
	wxPoint m_ref;
	/// velikost gridu
	wxSize m_size;
	/// [x, y] pohledu na nacteny svet
	wxRect m_view;
	wxRect m_old_view;
	/// svetovy offset vzhledem k zacatku datasetu
	wxPoint m_world_offset;
	/// smer, kam se pohled pohnul
	wxPoint m_dir;
	/// data
	/// vyskova mapa nacteneho sveta
	wxImage *m_world_hm;
	/// textura nacteneho sveta
	wxImage *m_world_tex;
	/// vyskova mapa nacteneho sveta *pred posunem*
	wxImage *m_world_old_hm;
	/// textura nacteneho sveta *pred posunem*
	wxImage *m_world_old_tex;
	
	/// vyskova mapa prednacteneho sveta
	wxImage *m_preload_hm;
	/// textura prednacteneho sveta
	wxImage *m_preload_tex;
	///
	bool m_preload_rdy;
	/// nacitaci vlakno
	CLoaderThread *m_loader;
	
public:
	CTileGrid(multimap_t *, multimap_t &, const wxSize &, const wxPoint &);
	~CTileGrid();
	
	bool CheckViewBounds(wxPoint *);
	bool CheckWorldBounds(wxPoint *dir, wxPoint *c, TileInfo *current_tile);
	bool ShiftWorld(wxPoint &);
	void CreateWorldImage();
	
	void SetAndRunLoader();
	
	/// get
	wxSize &GetSize() { return m_size; }
	wxRect &GetView() { return m_view; }
	wxRect &GetOldView() { return m_old_view; }
	wxImage *GetWorldHeightMap() { return m_world_hm; }
	wxImage *GetWorldTexture() { return m_world_tex; }
	wxPoint &GetWorldOffset() { return m_world_offset; }
	
	wxPoint &GetDir() { return m_dir; }
	
	multimap_t &GetTiles() { return m_tiles; }
	const wxPoint &GetRef() const { return m_ref; }
	
	bool GetPreloadReady() { return m_preload_rdy; }
	
	/// set
	void SetView(const wxRect &r) {
		m_old_view = m_view;
		m_view = r;
	}
	void SetWorld(multimap_t *world) { m_world_map = world; }
	
	void SetWorldHeightMap(wxImage *hm) { m_world_hm = hm; }
	void SetWorldTexture(wxImage *tex) { m_world_tex = tex; }
	
	wxImage *GetPreloadHeightMap() { return m_preload_hm; }
	wxImage *GetPreloadTexture() { return m_preload_tex; }
	void SetPreloadHeightMap(wxImage *hm) {
		if (m_preload_hm)
			delete m_preload_hm;
		m_preload_hm = hm;
	}
	void SetPreloadTexture(wxImage *tex) {
		if (m_preload_tex)
			delete m_preload_tex;
		m_preload_tex = tex;
	}
	void SetPreloadReady(bool r) { m_preload_rdy = r; }
	
	void OnPreloadRdy(wxCommandEvent &e);
};

/**
 * CLoaderThread
 * vlakno pro prednacteni kousku mapy
 */
class CLoaderThread : public wxThread {
	/// volajici tilegrid
	CTileGrid *m_caller;
	/// info o svete
	multimap_t *m_world_map;
	/// informace o polickach
	multimap_t m_tiles;
	/// data
	/// vyskova mapa nacteneho sveta
	wxImage *m_world_hm;
	/// textura nacteneho sveta
	wxImage *m_world_tex;
	
	wxCommandEvent m_event;
	
public:
	CLoaderThread(CTileGrid *, multimap_t *, multimap_t);
	~CLoaderThread();
	void *Entry();
	void LoadSurroundingTiles();
	
	wxImage *GetHM() { return m_world_hm; }
	wxImage *GetTex() { return m_world_tex; }
	
	void SetTiles(multimap_t ts) { m_tiles = ts; }
	void SetWorldMap(multimap_t *wm) {m_world_map = wm; }
};


#endif
