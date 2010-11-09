#include <wx/wx.h>
#include <wx/glcanvas.h>
#include "globals.h"

enum {
	ID_MF_New,
	ID_MF_Save,
	ID_MF_Load,
	ID_MF_Quit,

	ID_MV_Solid,
	ID_MV_Wireframe,

	ID_BMPBUT_Heightmap,
	ID_BMPBUT_Texture,
};


class CCanvas: public wxGLCanvas {
public:
	CCanvas(wxFrame* parent, int []);
	void Render();
	void OnIdle(wxIdleEvent &);
	void OnPaint(wxPaintEvent &);
	void OnResize(wxSizeEvent &);
	void OnKeyDown(wxKeyEvent &);
	void OnMouseEvents(wxMouseEvent &);
protected:
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CCanvas, wxGLCanvas)
	EVT_PAINT(CCanvas::OnPaint)
	EVT_IDLE(CCanvas::OnIdle)
	EVT_KEY_DOWN(CCanvas::OnKeyDown)
	EVT_SIZE(CCanvas::OnResize)
	EVT_MOUSE_EVENTS(CCanvas::OnMouseEvents)
END_EVENT_TABLE()

class CRootFrame: public wxFrame
{
	wxBitmapButton *m_bmpbut_texture;
	CCanvas *m_glcanvas;
public:
	// TODO
	wxBitmapButton *m_bmpbut_heightmap;

	CRootFrame();
	void OnNew(wxCommandEvent &);
	void OnSave(wxCommandEvent &);
	void OnLoad(wxCommandEvent &);
	void OnQuit(wxCommandEvent &);
	void OnViewSolid(wxCommandEvent &);
	void OnViewWireframe(wxCommandEvent &);
	
	CCanvas *GetCanvas() { return m_glcanvas; }
	void SetCanvas(CCanvas *c) { m_glcanvas = c; }
protected:
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CRootFrame, wxFrame)
	EVT_MENU(ID_MF_New, CRootFrame::OnNew)
	EVT_MENU(ID_MF_Save, CRootFrame::OnSave)
	EVT_MENU(ID_MF_Load, CRootFrame::OnLoad)
	EVT_MENU(ID_MF_Quit, CRootFrame::OnQuit)
	EVT_MENU(ID_MV_Solid, CRootFrame::OnViewSolid)
	EVT_MENU(ID_MV_Wireframe, CRootFrame::OnViewWireframe)
	// EVT_BUTTON(ID_BMPBUT_Heightmap, CRootFrame::OnLoad)
	// EVT_CLOSE()
END_EVENT_TABLE()

class CEditorApp: public wxApp
{
	virtual bool OnInit();
};

