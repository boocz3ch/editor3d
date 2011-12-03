#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/gbsizer.h>
#include "globals.h"
#include "tools.h"

enum {
	// identifikace polozek v menu
	ID_MF_New,
	ID_MF_Save,
	ID_MF_Load,
	ID_MF_Quit,
	
	ID_MD_Select,

	ID_MV_Solid,
	ID_MV_Wireframe,
	ID_MV_Texture,
	ID_MV_Light,
	
	// tlacitka pro modely
	ID_Choose_Model,
	ID_Use_Model,
	ID_Move_Model,
	ID_Rotate_Model,
	ID_Scale_Model,
	ID_Delete_Model,
	ID_Clone_Model,
	
	// identifikace klikacich nastroju
	ID_Brush_Circle,
	ID_Brush_Rect,
	ID_Tool_Lower,
	ID_Tool_Raise,
	ID_Tool_Level,
	ID_Tool_Select,
	ID_Tool_Smooth,
	ID_Tool_Clearback,
	
	ID_Spin_Brush_Size,
	ID_Spin_Tool_Strength,
};


// definice vlastniho nazvu eventu
DEFINE_EVENT_TYPE(EVT_EDITOR_REPAINT_ALL)

/**
 * CPanel
 * vykresluje pouze jeden obrazek
 */
class CPanel: public wxPanel {
	/// obrazek, ktery se na panelu vykresli
	wxImage *m_image;
	// vykresluje texturu?
	bool m_tex;
	// muze se na nej klikat?
	bool m_clickable;
	// doslo k posunu po datasetu?
	bool m_shift;
public:
	CPanel(wxFrame *, bool);
	virtual ~CPanel();
	void OnPaint(wxPaintEvent &);
	void OnSize(wxSizeEvent &);
	void OnMouseLeftDown(wxMouseEvent &);
	// void OnEditorRepaint(wxCommandEvent &);
	
	/// set
	void SetImg(wxImage *i) { m_image = i; }
	void SetTex(bool t) { m_tex = t; } 
protected:
	DECLARE_EVENT_TABLE()
};
BEGIN_EVENT_TABLE(CPanel, wxPanel)
	EVT_PAINT(CPanel::OnPaint)
	EVT_SIZE(CPanel::OnSize)
	EVT_LEFT_DOWN(CPanel::OnMouseLeftDown)
END_EVENT_TABLE()


/**
 * CCanvas
 * hlavni vykreslovaci prvek 3D sceny
 */
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

/**
 * CRootFrame
 * hlavni okno aplikace
 */
class CRootFrame: public wxFrame
{
	/// canvas pro 3D scenu
	CCanvas *m_glcanvas;
	/// panel pro vyskovou mapu nacteneho sveta
	CPanel *m_world_hm_panel;
	/// panel pro texturu nacteneho sveta
	CPanel *m_world_tex_panel;
	
	/// panel pro preload - tmp
	CPanel *m_preload_hm_panel;
	CPanel *m_preload_tex_panel;

public:
	/// cesta k modelu - tmp
	wxTextCtrl *m_tctrl_model_path;

	CRootFrame();
	virtual ~CRootFrame();
	void OnSave(wxCommandEvent &);
	void OnQuit(wxCommandEvent &);
	
	void OnViewSolid(wxCommandEvent &);
	void OnViewWireframe(wxCommandEvent &);
	void OnViewTexture(wxCommandEvent &);
	void OnViewLight(wxCommandEvent &);
	
	void OnEditorRepaint(wxCommandEvent &);
	void OnPreloadReady(wxCommandEvent &);
	
	void OnChooseModel(wxCommandEvent &);
	void OnUseModel(wxCommandEvent &);
	void OnMoveModel(wxCommandEvent &);
	void OnRotateModel(wxCommandEvent &);
	void OnScaleModel(wxCommandEvent &);
	void OnDeleteModel(wxCommandEvent &);
	void OnCloneModel(wxCommandEvent &);
	
	// nastroje
	void OnBrushCircle(wxCommandEvent &);
	void OnBrushRect(wxCommandEvent &);
	void OnToolLower(wxCommandEvent &);
	void OnToolRaise(wxCommandEvent &);
	void OnToolLevel(wxCommandEvent &);
	void OnToolSelect(wxCommandEvent &);
	void OnToolSmooth(wxCommandEvent &);
	void OnToolClearback(wxCommandEvent &);
	void OnSpinBrushSize(wxSpinEvent &);
	void OnSpinToolStrength(wxSpinEvent &);
	
	// OnKey v GTK buildu NECHYTA eventy na framu!
	// void OnKeyDown(wxKeyEvent &);
	
	CCanvas *GetCanvas() { return m_glcanvas; }
	void SetCanvas(CCanvas *c) { m_glcanvas = c; }
	
	void SetHmPanel(CPanel *p) { m_world_hm_panel = p; }
	void SetTexPanel(CPanel *p) { m_world_tex_panel = p; }
	void SetPreloadHMPanel(CPanel *p) { m_preload_hm_panel = p; }
	void SetPreloadTexPanel(CPanel *p) { m_preload_tex_panel = p; }
	
protected:
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CRootFrame, wxFrame)
	EVT_MENU(ID_MF_Save, CRootFrame::OnSave)
	EVT_MENU(ID_MF_Quit, CRootFrame::OnQuit)
	EVT_MENU(ID_MV_Solid, CRootFrame::OnViewSolid)
	EVT_MENU(ID_MV_Wireframe, CRootFrame::OnViewWireframe)
	EVT_MENU(ID_MV_Texture, CRootFrame::OnViewTexture)
	EVT_MENU(ID_MV_Light, CRootFrame::OnViewLight)
	
	EVT_BUTTON(ID_Choose_Model, CRootFrame::OnChooseModel)
	EVT_BUTTON(ID_Use_Model, CRootFrame::OnUseModel)
	EVT_BUTTON(ID_Move_Model, CRootFrame::OnMoveModel)
	EVT_BUTTON(ID_Rotate_Model, CRootFrame::OnRotateModel)
	EVT_BUTTON(ID_Scale_Model, CRootFrame::OnScaleModel)
	EVT_BUTTON(ID_Delete_Model, CRootFrame::OnDeleteModel)
	EVT_BUTTON(ID_Clone_Model, CRootFrame::OnCloneModel)
	// EVT_BUTTON(ID_BMPBUT_Heightmap, CRootFrame::OnLoad)
	//EVT_CLOSE()
	EVT_BUTTON(ID_Brush_Circle, CRootFrame::OnBrushCircle)
	EVT_BUTTON(ID_Brush_Rect, CRootFrame::OnBrushRect)
	EVT_BUTTON(ID_Tool_Lower, CRootFrame::OnToolLower)
	EVT_BUTTON(ID_Tool_Raise, CRootFrame::OnToolRaise)
	EVT_BUTTON(ID_Tool_Select, CRootFrame::OnToolSelect)
	EVT_BUTTON(ID_Tool_Smooth, CRootFrame::OnToolSmooth)
	EVT_BUTTON(ID_Tool_Clearback, CRootFrame::OnToolClearback)
	EVT_TOGGLEBUTTON(ID_Tool_Level, CRootFrame::OnToolLevel)
	
	EVT_SPINCTRL(ID_Spin_Brush_Size, CRootFrame::OnSpinBrushSize)
	EVT_SPINCTRL(ID_Spin_Tool_Strength, CRootFrame::OnSpinToolStrength)
	
	// vlastni event; prekresleni prvku po kliknuti na panel s mapou
	EVT_COMMAND(wxID_ANY, EVT_EDITOR_REPAINT_ALL, CRootFrame::OnEditorRepaint)
	EVT_COMMAND(wxID_ANY, EVT_EDITOR_PRELOAD_RDY, CRootFrame::OnPreloadReady)
END_EVENT_TABLE()
	
class CEditorApp: public wxApp
{
	virtual bool OnInit();
	virtual int OnExit();
};

