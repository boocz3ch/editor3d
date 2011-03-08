#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/notebook.h>
#include "globals.h"

enum {
	// identifikace polozek v menu
	ID_MF_New,
	ID_MF_Save,
	ID_MF_Load,
	ID_MF_Quit,

	ID_MV_Solid,
	ID_MV_Wireframe,
	//
};

// definice vlastniho nazvu eventu
DEFINE_EVENT_TYPE(EVT_EDITOR_REPAINT_ALL)

class CPanel: public wxPanel {
	wxImage *m_image;
	// vykresluje texturu?
	bool m_tex;
public:
	CPanel(wxFrame *);
	virtual ~CPanel();
	void OnPaint(wxPaintEvent &);
	void OnSize(wxSizeEvent &);
	void OnMouseLeftDown(wxMouseEvent &);
	// void OnEditorRepaint(wxCommandEvent &);
	
	void SetImg(wxImage *i) { m_image = i; }
	void SetTex(bool t) { m_tex = t; } 
protected:
	DECLARE_EVENT_TABLE()
};
BEGIN_EVENT_TABLE(CPanel, wxPanel)
	EVT_PAINT(CPanel::OnPaint)
	EVT_SIZE(CPanel::OnSize)
	EVT_LEFT_DOWN(CPanel::OnMouseLeftDown)
	// EVT_COMMAND(wxID_ANY, EVT_EDITOR_REPAINT_ALL, CPanel::OnEditorRepaint)
END_EVENT_TABLE()


class CCanvas: public wxGLCanvas {
public:
	CCanvas(wxFrame* parent, int []);
	void Render();
	void OnPaint(wxPaintEvent &);
	void OnResize(wxSizeEvent &);
	void OnKeyDown(wxKeyEvent &);
	void OnMouseEvents(wxMouseEvent &);
protected:
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CCanvas, wxGLCanvas)
	EVT_PAINT(CCanvas::OnPaint)
	EVT_KEY_DOWN(CCanvas::OnKeyDown)
	EVT_SIZE(CCanvas::OnResize)
	EVT_MOUSE_EVENTS(CCanvas::OnMouseEvents)
END_EVENT_TABLE()

class CRootFrame: public wxFrame
{
	CCanvas *m_glcanvas;
	CPanel *m_world_hm_panel;
	CPanel *m_world_tex_panel;
public:

	CRootFrame();
	virtual ~CRootFrame();
	void OnNew(wxCommandEvent &);
	void OnSave(wxCommandEvent &);
	void OnLoad(wxCommandEvent &);
	void OnQuit(wxCommandEvent &);
	void OnViewSolid(wxCommandEvent &);
	void OnViewWireframe(wxCommandEvent &);
	
	 void OnEditorRepaint(wxCommandEvent &);
	
	// OnKey v GTK buildu NECHYTA eventy na framu!
	// void OnKeyDown(wxKeyEvent &);
	
	CCanvas *GetCanvas() { return m_glcanvas; }
	void SetCanvas(CCanvas *c) { m_glcanvas = c; }
	
	void SetHmPanel(CPanel *p) { m_world_hm_panel = p; }
	void SetTexPanel(CPanel *p) { m_world_tex_panel = p; }
	
	void SetStatusHeightMap();
	void SetStatusTexture();
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
	//EVT_CLOSE()
	
	// vlastni event; prekresleni prvku po kliknuti na panel s mapou
	EVT_COMMAND(wxID_ANY, EVT_EDITOR_REPAINT_ALL, CRootFrame::OnEditorRepaint)
END_EVENT_TABLE()
	
class CToolboxFrame: public wxFrame
{
	wxNotebook *m_notebook;
public:
	CToolboxFrame(wxWindow *);
};

class CEditorApp: public wxApp
{
	virtual bool OnInit();
	virtual int OnExit();
};

