#include "main.h"
#include "editor.h"
#include "exception.h"

#include <string>

IMPLEMENT_APP(CEditorApp)
/*
 * globalni pointer na editor, singleton	
 */
Editor::CEditor *editor;

/*
 * CPanel
 * panely na vykreslovani map vedle editacniho okna
 */
CPanel::CPanel(wxFrame *parent, bool clickable):
	wxPanel(parent), m_image(0), m_tex(false), m_clickable(clickable),
	m_shift(false)
{
	SetMinSize(wxSize(100, 100));
	SetMaxSize(wxSize(100, 100));
}
CPanel::~CPanel()
{
	if (m_image)
		delete m_image;
}
void CPanel::OnPaint(wxPaintEvent &e)
{
	if (!m_image) {
		e.Skip();
		return;
	}
	wxPaintDC dc(this);
	int w, h;
	GetSize(&w, &h);
	
	int img_w = m_image->GetWidth();
	int img_h = m_image->GetHeight();
	
	// int s = (w < h) ? w : h;
	dc.DrawBitmap(wxBitmap(m_image->Scale(w, h)), 0, 0);
	
	float factor_x = img_w / float(w);
	float factor_y = img_h / float(h);
	if (m_tex) {
		factor_x /= 8.0;
		factor_y /= 8.0;
	}
	
	wxRect r = editor->GetWorldView();
	
	if (m_clickable) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(wxPen(wxColour(0, 0, 255), 1));
		dc.DrawRectangle(r.x / factor_x, r.y / factor_y,
						 r.width / factor_x, r.height / factor_y);
		if (m_shift) {
			wxRect r2 = r;
			
			// posun okynka na minimape pri posunu po datasetu
			// tohle musi bejt totozny s map.cpp: CMap::CreateFromView ~250
			if (r.x< 0) r2.x += 256;
			if (r.x> 256) r2.x -= 256;
			if (r.y< 0) r2.y += 256;
			if (r.y> 256) r2.y -= 256;
			
			m_shift = false;
			
			dc.SetPen(wxPen(wxColour(250, 0, 0), 1));
			dc.DrawRectangle(r2.x / factor_x, r2.y / factor_y,
							 r2.width / factor_x, r2.height / factor_y);
		}
	}
}
void CPanel::OnSize(wxSizeEvent &e)
{
	Refresh(false);
	e.Skip();
}
void CPanel::OnMouseLeftDown(wxMouseEvent &e)
{
	if (!m_clickable)
		return;
	
	long mx, my;
	e.GetPosition(&mx, &my);
	// std::cout << mx << ", " << my << std::endl;
	
	int w, h;
	GetSize(&w, &h);
	
	int img_w = m_image->GetWidth();
	int img_h = m_image->GetHeight();
	int s = (w < h) ? w : h;
	
	float factor_x = img_w / float(s);
	float factor_y = img_h / float(s);
	if (m_tex) {
		factor_x /= 8.0;
		factor_y /= 8.0;
	}
	
	CTileGrid *tg = editor->GetTileGrid();
	
	wxRect r = editor->GetWorldView();
	r.x = mx * factor_x - r.width/2.0;
	r.y = my * factor_y - r.width/2.0;
	tg->SetView(r);
	
	bool should_move;
	// pokud uz vlakno skoncilo, kontrolujem hranice
	if (tg->GetPreloadReady()) {
		wxPoint dir, coord;
		TileInfo cur_tile;
		
		if (tg->CheckViewBounds(&dir)) {
			should_move = tg->ShiftWorld(dir);
			if (should_move) {
				tg->CreateWorldImage();
				tg->SetAndRunLoader();
			}
			m_shift = true;
		}
	}
	
	// log
	wxLogVerbose(_T("World view set: [%d, %d]"), r.x, r.y);
	
	// vytvorit novou mapu z pohledu a poslat na grafiku
	editor->CreateMapFromView(m_shift);
	editor->SetSync();
	
	// poslat repaint event pro spravne vykresleni v celem gui
	wxCommandEvent evt(EVT_EDITOR_REPAINT_ALL);
	wxPostEvent(GetParent(), evt);
	
	e.Skip();
}

/*
 * CCanvas
 * hlavni zobrazovaci a editacni canvas
 */
CCanvas::CCanvas(wxFrame *parent, int attrib_list[])
	:wxGLCanvas(parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas"), attrib_list)
{
}

void CCanvas::Render()
{
	editor->Select();
	editor->Render();
}

void CCanvas::OnPaint(wxPaintEvent &e)
{
	int w, h;
	GetClientSize(&w, &h);
	static bool go = true;

	SetCurrent();
	wxPaintDC(this);

	// inicializovat GL jednou, ale az po SetCurrent
	if (go) {
		editor->InitGL(w, h);
		go = false;
	}

	Render();
	SwapBuffers();
}
void CCanvas::OnResize(wxSizeEvent &e)
{
	int w, h;
	GetClientSize(&w, &h);
	editor->OnResize(w, h);
}
void CCanvas::OnKeyDown(wxKeyEvent &e)
{
	int key = e.GetKeyCode();
	CRootFrame *rootframe = reinterpret_cast<CRootFrame *>(wxTheApp->GetTopWindow());
	switch(key) {
		case WXK_ESCAPE:
			rootframe->Close(true);
			break;
		case WXK_UP:
			editor->MoveCamera(3.5);
			break;
		case WXK_DOWN:
			editor->MoveCamera(-3.5);
			break;
		case WXK_LEFT:
			editor->SlideCamera(-3.5, 0);
			break;
		case WXK_RIGHT:
			editor->SlideCamera(3.5, 0);
			break;
		default:
			break;
	}
	
	Refresh(false);
	e.Skip();
}

void CCanvas::OnMouseEvents(wxMouseEvent &e)
{
	static int last_x = 0, last_y = 0;
	int x = e.GetX();
	int y = e.GetY();
	float delta_x = (x - last_x);
	float delta_y = (y - last_y);
	int wheel_rot = 0;

	if (e.Dragging()) { // dragging
		if (e.MiddleIsDown()) {
			editor->SlideCamera(delta_x, delta_y);
		        editor->SetMouseState(false, false);
		}
		else if (e.RightIsDown()) {
			editor->MoveCameraFocus(delta_x, 0, -delta_y);
		        editor->SetMouseState(false, false);
		}
		else if (e.LeftIsDown()) {
			editor->SetMouseState(true, false);
		}
		else {
			editor->SetMouseState(false, false);
		}
	}
	else { // not dragging
		if (e.LeftIsDown()) {
			Vec3 p = editor->Pick(x, y);
			editor->SetClickPointer(p);
			editor->SetMouseState(true, false);
			editor->UpdatePickedPoint(x, y, true);
			wxTheApp->GetTopWindow()->Refresh();
		}
		else {
			editor->SetMouseState(false, false);
			editor->UpdatePickedPoint(x, y, false);
		}
	}
	if ((wheel_rot = e.GetWheelRotation()) != 0) {
		editor->AdjustZoom((wheel_rot > 0) ? 0.9 : 1.1);
	}

	editor->SetMousePos(x, y, last_x, last_y);
	Refresh(false);
	
	last_x = x;
	last_y = y;
	
	e.Skip();
}

/*
 * CRootFrame
 * hlavni wxwidgets okno aplikace
 */
CRootFrame::CRootFrame()
	:wxFrame(0, -1, APP_NAME, APP_POSITION, APP_SIZE),
	m_glcanvas(0), m_world_hm_panel(0), m_world_tex_panel(0)
{
}
CRootFrame::~CRootFrame()
{
}

void CRootFrame::OnSave(wxCommandEvent &e)
{
	wxMessageBox( wxT("To be implemented :("), wxT("Saving.."),
			wxICON_INFORMATION);
}

void CRootFrame::OnQuit(wxCommandEvent &e)
{
	Close(true);
}

void CRootFrame::OnBrushCircle(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.brush = Tools::BR_CIRCLE;
	t.use_model = false;
	t.move = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
	editor->GetMap()->UpdateShaderTool(t);
}
void CRootFrame::OnBrushRect(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.brush = Tools::BR_RECT;
	t.use_model = false;
	t.move = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
	editor->GetMap()->UpdateShaderTool(t);
}
void CRootFrame::OnToolLower(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.tool = Tools::TOOL_LOWER;
	t.use_model = false;
	t.move = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
}
void CRootFrame::OnToolRaise(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.tool = Tools::TOOL_RAISE;
	t.use_model = false;
	t.move = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
}
void CRootFrame::OnToolLevel(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	// t.tool = Tools::TOOL_LEVEL;
	t.level = !t.level;
	t.use_model = false;
	t.move = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
}

void CRootFrame::OnToolSelect(wxCommandEvent &e)
{
/*	Tools::CTool t = editor->GetTool();
	t.select = !t.select;
	editor->SetTool(t);
*/
}

void CRootFrame::OnToolSmooth(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.tool = Tools::TOOL_SMOOTH;
	t.use_model = false;
	t.move = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
}
void CRootFrame::OnToolClearback(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.tool = Tools::TOOL_CLEARBACK;
	t.use_model = false;
	t.move = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
}
void CRootFrame::OnSpinBrushSize(wxSpinEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.brush_size = e.GetPosition();
	t.use_model = false;
	t.move = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
	editor->GetMap()->UpdateShaderTool(t);
}
void CRootFrame::OnSpinToolStrength(wxSpinEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.strength = e.GetPosition() / 20.0;
	t.use_model = false;
	t.move = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
}

void CRootFrame::OnViewSolid(wxCommandEvent &e)
{
	editor->SetRenderState(GL_TRIANGLES);
}
void CRootFrame::OnViewWireframe(wxCommandEvent &e)
{
	editor->SetRenderState(GL_LINES);
}
void CRootFrame::OnViewTexture(wxCommandEvent &e) 
{
	editor->GetMap()->UpdateShader(0, e.IsChecked());
}
void CRootFrame::OnViewLight(wxCommandEvent &e)
{
	editor->GetMap()->UpdateShader(1, e.IsChecked());
}

void CRootFrame::OnEditorRepaint(wxCommandEvent &e)
{
	m_glcanvas->Refresh(false);
	m_world_hm_panel->SetImg(editor->GetWorldHeightMap());
	m_world_hm_panel->Refresh(false);
	m_world_tex_panel->SetImg(editor->GetWorldTexture());
	m_world_tex_panel->Refresh(false);
	
	m_preload_hm_panel->Refresh(false);
	m_preload_tex_panel->Refresh(false);
}

void CRootFrame::OnPreloadReady(wxCommandEvent &e)
{
	std::vector<wxImage *> *data = reinterpret_cast<std::vector<wxImage *> *>(e.GetClientData());
	m_preload_hm_panel->SetImg(data->at(0));
	m_preload_hm_panel->Refresh(false);
	m_preload_tex_panel->SetImg(data->at(1));
	m_preload_tex_panel->Refresh(false);
	delete data;
}

void CRootFrame::OnUseModel(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.use_model = true;
	t.move = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
}

void CRootFrame::OnMoveModel(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.move = true;
        t.use_model = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
}

void CRootFrame::OnRotateModel(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.move = false;
        t.use_model = false;
        t.rotate = true;
        t.scale = false;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
}

void CRootFrame::OnScaleModel(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.move = false;
        t.use_model = false;
        t.rotate = false;
        t.scale = true;
        t.del = false;
        t.clone = false;
	editor->SetTool(t);
}

void CRootFrame::OnDeleteModel(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.move = false;
        t.use_model = false;
        t.rotate = false;
        t.scale = false;
        t.del = true;
        t.clone = false;
	editor->SetTool(t);
}

void CRootFrame::OnCloneModel(wxCommandEvent &e)
{
	Tools::CTool t = editor->GetTool();
	t.move = false;
        t.use_model = false;
        t.rotate = false;
        t.scale = false;
        t.del = false;
        t.clone = true;
	editor->SetTool(t);
}

void CRootFrame::OnChooseModel(wxCommandEvent &e)
{
	wxFileDialog *dlg = new wxFileDialog(this, _T("Model path"), _T(""), _T(""), _T("*.*"));

	if (dlg->ShowModal() == wxID_OK)
	{
		// wxString fname = dlg->GetFilename();
		wxString path = dlg->GetPath();
		m_tctrl_model_path->SetValue(path);

		//wxString path = m_tctrl_model_path->GetValue();

		CMap *m_map = editor->GetMap();
		std::map<std::string, Cmodel *> *loaded_models = &m_map->GetLoadedModels();
		std::map<std::string, Cmodel *>::iterator iter;

		std::string search_string(path.ToAscii()); 
		iter = loaded_models->find(search_string);              

		if (iter != loaded_models->end()) {
			editor->SetChoosedModel(iter->second);
			wxLogVerbose(_T("Model reused"));
		}
		else {
			Cmodel *model = new Cmodel();
			bool succeed = model->loadModel(search_string);
			wxString m;
			if (!succeed) {
				m = _T("Model '") + path + _T("' has not been loaded");
				wxLogError(m);
			}
			else {
				m = _T("New model loaded: ") + path;
				wxLogVerbose(m);
				loaded_models->insert(std::pair<std::string, Cmodel *>(search_string, model));
				wxLogVerbose(_T("Models count: %d"), loaded_models->size());
				editor->SetChoosedModel(model);
			}
		} 
		// nastavit tool
		Tools::CTool t = editor->GetTool();
		t.use_model = true;
		t.move = false;
		t.rotate = false;
		t.scale = false;
		t.del = false;
		t.clone = false;
		editor->SetTool(t);
	}
}

/*
 * CEditorApp
 * trida aplikace
 */
bool CEditorApp::OnInit()
{
	// nastavit loggera
	wxLog *logger = new wxLogStream(&std::cout);
	wxLog::SetActiveTarget(logger);
#ifdef DEBUG
	logger->SetVerbose(true);
#endif
	
	//// frame
	CRootFrame *frame = new CRootFrame;
	
	// inicializace GLOBALNIHO SINGLETONU
	editor = Editor::CEditor::GetInstance();
	try {
		editor->Init();
	}
	catch (CException &e) {
		wxLogError(wxString::FromAscii(e.what()));
		e.show();
		throw;
	}

	// menu
	wxMenuBar *menu_bar = new wxMenuBar;
	wxMenu *menu_file = new wxMenu;
	wxMenu *menu_view = new wxMenu;

	// menu file 
	// menu_file->Append(ID_MF_New, _T("New"));
	menu_file->Append(ID_MF_Save, _T("Save"));
	// menu_file->Append(ID_MF_Load, _T("Load"));
	menu_file->AppendSeparator();
	menu_file->Append(ID_MF_Quit, _T("E&xit"));
	
	// menu view
	menu_view->Append(ID_MV_Solid, _T("Solid"));
	menu_view->Append(ID_MV_Wireframe, _T("Wireframe"));
	menu_view->AppendCheckItem(ID_MV_Texture, _T("Texture"));
	menu_view->Check(ID_MV_Texture, true);
	menu_view->AppendCheckItem(ID_MV_Light, _T("Light"));
	menu_view->Check(ID_MV_Light, true);
	
	// menubar
	menu_bar->Append(menu_file, _T("&File"));
	menu_bar->Append(menu_view, _T("&View"));
	frame->SetMenuBar(menu_bar);

	// canvas
	int al[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER};
	CCanvas *canvas = new CCanvas(frame, al);
	frame->SetCanvas(canvas);

	// sizerova hierarchie
	wxBoxSizer *top_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *minimap_sizer = new wxBoxSizer(wxVERTICAL);
	wxGridSizer *worlds_sizer = new wxGridSizer(2,2,0,0);

	// bocni panel se svetem
	wxRect r = editor->GetWorldView(); 
	CPanel *tmp = new CPanel(frame, true);
	tmp->SetImg(editor->GetWorldHeightMap());
	worlds_sizer->Add(tmp, 0); 
	frame->SetHmPanel(tmp);
	
	tmp = new CPanel(frame, false);
	tmp->SetImg(editor->GetWorldTexture());
	tmp->SetTex(true);
	worlds_sizer->Add(tmp, 0); 
	frame->SetTexPanel(tmp);
	
	// preload
	tmp = new CPanel(frame, false);
	tmp->SetImg(0);
	worlds_sizer->Add(tmp, 0); 
	frame->SetPreloadHMPanel(tmp);
	tmp = new CPanel(frame, false);
	tmp->SetImg(0);
	worlds_sizer->Add(tmp, 0); 
	frame->SetPreloadTexPanel(tmp);
	
	minimap_sizer->Add(worlds_sizer, 7, wxEXPAND );
	
	//// notebook
	wxNotebook *m_notebook = new wxNotebook(frame, wxID_ANY);
	wxPanel *page_toolbox = new wxPanel(m_notebook, wxID_ANY);
	wxPanel *page_models = new wxPanel(m_notebook, wxID_ANY);
	
	m_notebook->AddPage(page_toolbox, _T("Toolbox"));
	m_notebook->AddPage(page_models, _T("Models"));
	
	wxFlexGridSizer *toolbox_sizer = new wxFlexGridSizer(6, 2, 0, 0);
	toolbox_sizer->SetFlexibleDirection(wxHORIZONTAL);
	toolbox_sizer->AddGrowableCol(0, 0);
	toolbox_sizer->AddGrowableCol(1, 0);
	wxGridBagSizer *models_sizer = new wxGridBagSizer();
	
	// butonky s nastojema
	wxButton *btn_brush_circle = new wxButton(page_toolbox, ID_Brush_Circle, _T("brush:circle"));
	wxButton *btn_brush_rect = new wxButton(page_toolbox, ID_Brush_Rect, _T("brush:rect"));
	wxButton *btn_tool_lower = new wxButton(page_toolbox, ID_Tool_Lower, _T("tool:lower"));
	wxButton *btn_tool_raise = new wxButton(page_toolbox, ID_Tool_Raise, _T("tool:raise"));
	wxToggleButton *btn_level = new wxToggleButton(page_toolbox, ID_Tool_Level, _T("tool:level"));
	btn_level->SetValue(false);
        wxButton *btn_tool_smooth = new wxButton(page_toolbox, ID_Tool_Smooth, _T("tool:smooth"));
	wxButton *btn_tool_clearback = new wxButton(page_toolbox, ID_Tool_Clearback, _T("tool:clear"));
	
	wxSpinCtrl *spin_brush_size = new wxSpinCtrl(page_toolbox, ID_Spin_Brush_Size);
	spin_brush_size->SetValue(3);
	spin_brush_size->SetRange(1, 20);
	
	wxSpinCtrl *spin_tool_strength = new wxSpinCtrl(page_toolbox, ID_Spin_Tool_Strength);
	spin_tool_strength->SetValue(2);
	spin_tool_strength->SetRange(1, 100);
	
	// ovladani modelu
	frame->m_tctrl_model_path = new wxTextCtrl(page_models, wxID_ANY, _T("path.."));
	wxButton *btn_choose = new wxButton(page_models, ID_Choose_Model, _T("choose model.."));
	wxButton *btn_use = new wxButton(page_models, ID_Use_Model, _T("Place model"));
	wxButton *btn_move = new wxButton(page_models, ID_Move_Model, _T("Move model"));
	wxButton *btn_rotate = new wxButton(page_models, ID_Rotate_Model, _T("Rotate model"));
	wxButton *btn_scale = new wxButton(page_models, ID_Scale_Model, _T("Scale model"));
	wxButton *btn_delete = new wxButton(page_models, ID_Delete_Model, _T("Delete model"));
	wxButton *btn_clone = new wxButton(page_models, ID_Clone_Model, _T("Clone model"));
	
	// naplnit toolbox
	toolbox_sizer->Add(btn_brush_circle, 1, wxEXPAND);
	toolbox_sizer->Add(btn_brush_rect, 1, wxEXPAND);
	toolbox_sizer->Add(btn_tool_lower, 1, wxEXPAND);
	toolbox_sizer->Add(btn_tool_raise, 1, wxEXPAND);
	toolbox_sizer->Add(btn_level, 1, wxEXPAND);
	toolbox_sizer->Add(btn_tool_smooth, 1, wxEXPAND);
	toolbox_sizer->Add(btn_tool_clearback, 1, wxEXPAND);
	// vypln, aby byl sudy pocet prvku a bylo zarovnano
	toolbox_sizer->Add(new wxStaticText(page_toolbox, -1, _T("")), 1);
	
	// naplnit toolbox
	toolbox_sizer->Add(new wxStaticText(page_toolbox, -1, _T("Brush size: ")), 1);
	toolbox_sizer->Add(spin_brush_size, 1, wxEXPAND);
	toolbox_sizer->Add(new wxStaticText(page_toolbox, -1, _T("Brush strength: ")), 1);
	toolbox_sizer->Add(spin_tool_strength, 1, wxEXPAND);
	
	// naplnit modely
	models_sizer->Add(frame->m_tctrl_model_path, wxGBPosition(0,0), wxGBSpan(1, 2), wxEXPAND);
	models_sizer->Add(btn_choose, wxGBPosition(1,0), wxGBSpan(1,1), wxEXPAND);
	models_sizer->Add(btn_use, wxGBPosition(1,1), wxGBSpan(1,1), wxEXPAND);
	models_sizer->Add(btn_move, wxGBPosition(2,0), wxGBSpan(1,1), wxEXPAND);
	models_sizer->Add(btn_rotate, wxGBPosition(2,1), wxGBSpan(1,1), wxEXPAND);
	models_sizer->Add(btn_scale, wxGBPosition(3,0), wxGBSpan(1,1), wxEXPAND);
	models_sizer->Add(btn_delete, wxGBPosition(3,1), wxGBSpan(1,1), wxEXPAND);
	models_sizer->Add(btn_clone, wxGBPosition(4,0), wxGBSpan(1,1), wxEXPAND);
	
	page_toolbox->SetSizer(toolbox_sizer);
	page_models->SetSizer(models_sizer);
	minimap_sizer->Add(m_notebook, 15, wxALL | wxEXPAND);
	
	// naplnit hlavni sizer
	top_sizer->Add(canvas, 3, wxALL | wxEXPAND); 
	top_sizer->Add(minimap_sizer, 1, wxALL | wxEXPAND ); 
	
	SetTopWindow(frame);
	frame->SetSizer(top_sizer);
	frame->Show(TRUE);
	
	return TRUE;
}

int CEditorApp::OnExit()
{
	wxLogVerbose(_T("Editor exiting.."));
	editor->CleanUp();
	
	return GL_TRUE;
}
