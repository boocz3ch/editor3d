#ifndef EDITOR_H
#define EDITOR_H
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <wx/wx.h>

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
	CMap *m_map;
	float m_zoom;
	GLuint m_render_state;
	
	Vec3 m_clickpointer;
	
	CCamera m_camera;
	wxSize m_viewport_size;
	
	bool m_sync;
	
	bool m_enable_shaders;
	enum {
		MODE_HM,
		MODE_SAT,
	} m_mode;


	// CLog m_log;

	static CEditor *m_inst;
protected:
	CEditor();

public:
	static CEditor *GetInstance() {
		if (!m_inst)
			m_inst = new CEditor;
		return m_inst;
	}

	void Init();
	void InitGL(int w, int h);
	void Render();
	void Update();
	void Finalize();

	void OnResize(int, int);

	void MoveCameraFocus(float, float, float);
	void MoveCamera(float);
	void SlideCamera(float, float);
	void AdjustZoom(float);
	
	Vec3 Pick(int, int);
	void ProcessPicked(Vec3 &);

	void SaveMap(const wxString &);



	// set n get
	void SetRenderState(GLuint rs) { m_render_state = rs; }

	// wxImage *GetHeightMap() { return m_image; }
	// wxImage *GetTexture() { return m_texture; }

	// wxString &GetHeightMapFilename() { return m_heightmap_fname; }
	// wxString &GetTextureFilename() { return m_texture_fname; }

	void SetHeightMap(const wxString &);
	void SetTexture(const wxString &);
	
	CMap *GetMap() { return m_map; }
	
	void SetSync() { m_sync = true; }
	
	void EnableShaders() { m_enable_shaders = true; m_map->UseShaders(true); }
	void DisableShaders() { m_enable_shaders = false; m_map->UseShaders(false);}
	


	void CleanUp();
};

}; // namespace
#endif
