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

namespace Editor {

const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 200.0f;
const float MAX_ZOOM = 3.0f;
const float MIN_ZOOM = 0.005f;
const float DEFAULT_FOV_Y = 45.0f;

class CEditor {
	// instance editoru
	static CEditor *m_inst;
	// prave nactena cast sveta pro editovani
	CMap *m_map;
	// nacteny svet
	CTileGrid *m_tilegrid;
	
	GLuint m_render_state;
	
	// souradnice ve svete po kliknuti mysi
	Vec3 m_clickpointer;
	
	CCamera m_camera;
	wxSize m_viewport_size;
	
	// synchronizace pro odesilani dat na grafickou kartu
	bool m_sync;
	
	// pomocna struktura pro porovnavani souradnic
	struct PointCompare {
		bool operator()(const wxPoint &p1, const wxPoint &p2) {
			return (p1.x == p2.x) ? (p1.y < p2.y) : (p1.x < p2.x);
		}
	};
	// svet ve forme asociativniho pole
	std::multimap<wxPoint, wxString, PointCompare> m_world_map;

protected:
	CEditor();

public:
	static CEditor *GetInstance() {
		if (!m_inst)
			m_inst = new CEditor;
		return m_inst;
	}

	void Init();
	void LoadWorldMap();
	void InitGL(int w, int h);
	void Render();
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
	
	void CreateMapFromView();

	// update shader
	void UpdateShader(const Vec3 &);
	
	void MoveView(const wxPoint &pt) { m_tilegrid->MoveView(pt); }
	void CleanUp();


	// get
	CMap *GetMap() { return m_map; }
	wxImage *GetWorldHeightMap() { return m_tilegrid->GetWorldHeightMap(); }
	wxImage *GetWorldTexture() { return m_tilegrid->GetWorldTexture(); }
	wxRect &GetWorldView() { return m_tilegrid->GetView(); }
	
	// set
	void SetRenderState(GLuint rs) { m_render_state = rs; }
	void SetWorldView(const wxRect &r) { m_tilegrid->SetView(r); }
	void SetHeightMap(const wxString &);
	void SetTexture(const wxString &);
	void SetSync() { m_sync = true; }
};

}; // namespace
#endif
