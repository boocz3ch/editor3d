#ifndef _EDITOR_H_
#define _EDITOR_H_
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <wx/wx.h>

#include <map>

#include "map.h"
#include "vec3.h"
#include "camera.h"
#include "tools.h"
#include "model.h"

namespace Editor {

const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 20000.0f;
const float MAX_ZOOM = 3.0f;
const float MIN_ZOOM = 0.005f;
const float DEFAULT_FOV_Y = 45.0f;

/**
 * CEditor
 * reprezentace editoru jako celku
 * implementovan jako singleton
 */
class CEditor {
	/// instance editoru
	static CEditor *m_inst;
	/// model sceny, umoznuje editaci
	CMap *m_map;
	/// nacteny svet
	CTileGrid *m_tilegrid;
	/// zpusob renderovani
	GLuint m_render_state;
	/// souradnice ve svete po kliknuti mysi
	Vec3 m_clickpointer;
	
	/// kamera
	CCamera m_camera;
	/// viewport
	wxSize m_viewport_size;
	
	/// synchronizace pro odesilani dat na grafickou kartu
	bool m_sync;
	
	/// cesta ke korenovemu adresari datasetu
	wxString m_datapath_root;
	/// nacteny kompletni dataset z disku
	multimap_t m_world_map;
	
	/// nastroj
	Tools::CTool m_tool;

	int picked_point_x;
	int picked_point_y;
	bool picking_enabled;

	bool left_mouse_down;
	bool mouse_dragging;

protected:

	CEditor();
	~CEditor();
public:
	/// instance
	static CEditor *GetInstance() {
		if (!m_inst)
			m_inst = new CEditor;
		return m_inst;
	}

	int mx, my;
	int prev_mx, prev_my;
	int picked_name;

	void Init();
	void LoadWorldMap();
	void InitGL(int w, int h);
	void Render();
	void Select();
	void Update();
	void Finalize();

	void OnResize(int, int);

	void InitCamera();
	void MoveCameraFocus(float, float, float);
	void MoveCamera(float);
	void SlideCamera(float, float);
	void AdjustZoom(float);
	
	Vec3 Pick(int, int);
	void ProcessPicked(Vec3 &);

	void SaveMap(const wxString &);
	void SaveWorld();
	
	void CreateMapFromView(bool shift = false);
	void DeleteSelectedModel();

	// update shader
	void UpdateShader(const Vec3 &);
        void UpdatePickedPoint(int x, int y, bool);
        void SetChoosedModel(Cmodel *model);
	/// uklid
	void CleanUp();

	/// get
	CMap *GetMap() { return m_map; }
	wxImage *GetWorldHeightMap() { return m_tilegrid->GetWorldHeightMap(); }
	wxImage *GetWorldTexture() { return m_tilegrid->GetWorldTexture(); }
	wxImage *GetPreloadHeightMap() { return m_tilegrid->GetPreloadHeightMap(); } 
	wxRect &GetWorldView() { return m_tilegrid->GetView(); }
	const wxString &GetDataPath() { return m_datapath_root; }
	
	CTileGrid *GetTileGrid() { return m_tilegrid; }
	Tools::CTool GetTool() { return m_tool; }
	
	Vec3 &GetClickPointer() { return m_clickpointer; }
	
	/// set
	void SetRenderState(GLuint rs) { m_render_state = rs; }
	void SetHeightMap(const wxString &);
	void SetTexture(const wxString &);
	void SetSync() { m_sync = true; }
	void SetDataPath(const wxString &p) { m_datapath_root = p; }
	
	void SetTool(Tools::CTool &t) { m_tool = t; }
	void SetClickPointer(const Vec3 &p) { m_clickpointer = p;}
	void SetMouseState(bool left_down, bool dragging) { left_mouse_down = left_down; mouse_dragging = dragging;};
	void SetMousePos(int x, int y, int old_x, int old_y) { mx = x; my = y; prev_mx = old_x; prev_my = old_y;};

};

}; // namespace
#endif
