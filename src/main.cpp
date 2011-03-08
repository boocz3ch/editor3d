#include "main.h"
#include "editor.h"
#include "exception.h"

IMPLEMENT_APP(CEditorApp)
/*
 * globalni pointer na editor, singleton	
 */
Editor::CEditor *editor;


/*
 * CPanel
 * panely na vykreslovani map vedle editacniho okna
 */
CPanel::CPanel(wxFrame *parent):
	wxPanel(parent), m_image(0), m_tex(false)
{
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
	
	int s = (w < h) ? w : h;
	dc.DrawBitmap(wxBitmap(m_image->Scale(s, s)), 0, 0);
	
	float factor_x = img_w / float(s);
	float factor_y = img_h / float(s);
	if (m_tex) {
		factor_x /= 8.0;
		factor_y /= 8.0;
	}
	
	wxRect r = editor->GetWorldView();
	
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(wxPen(wxColour(0, 0, 255), 1));
	dc.DrawRectangle(r.x / factor_x, r.y / factor_y,
					 r.width / factor_x, r.height / factor_y);
}
void CPanel::OnSize(wxSizeEvent &e)
{
	Refresh(false);
	e.Skip();
}
void CPanel::OnMouseLeftDown(wxMouseEvent &e)
{
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
	
	wxRect r = editor->GetWorldView();
	r.x = (mx/*  - r.width/2.0 */) * factor_x;
	r.y = (my/*  - r.height/2.0 */) * factor_y;
	editor->SetWorldView(r);
	
	// log
	wxLogVerbose(_T("World view set: [%d, %d]"), r.x, r.y);
	
	editor->CreateMapFromView();
	editor->SetSync();
	
	// poslat repaint event pro spravne vykresleni v celem gui
	wxCommandEvent evt(EVT_EDITOR_REPAINT_ALL);
	evt.SetEventObject(this);
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
		if (e.RightIsDown() && e.LeftIsDown()) {
			editor->SlideCamera(delta_x, -delta_y);
		}
		else if (e.RightIsDown()) {
			editor->MoveCameraFocus(delta_x, 0, -delta_y);
		}
	}
	else { // not dragging
		if (e.LeftIsDown()) {
			Vec3 p = editor->Pick(x, y);
			editor->ProcessPicked(p);
			
			wxTheApp->GetTopWindow()->Refresh();
		}
	}
	if ((wheel_rot = e.GetWheelRotation()) != 0) {
		editor->AdjustZoom((wheel_rot > 0) ? 0.9 : 1.1);
	}
	
	Vec3 p = editor->Pick(x, y);
	// TODO valgrind vypisuje naky chyby grafickej ovladacu s neinicializovanyma
	// hodnotama.. 
	editor->UpdateShader(p);
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

void CRootFrame::OnNew(wxCommandEvent &e)
{
	// TODO onnew nejspis nebude potreba
	// popr. to bude obnovovat data do stavu, v jakym byly pred editaci
	const int sizex = 256;
	const int sizey = 256;
	
	editor->GetMap()->Create(sizex, sizey, true);
	editor->SetSync();
	SetStatusHeightMap();
	
	Refresh(false);
}

void CRootFrame::OnSave(wxCommandEvent &e)
{

    /*
	 * wxFileDialog *save_file = new wxFileDialog(this, _T("Save file"), _T(""), _T(""), _T("*.png"),
	 *         wxSAVE, wxDefaultPosition);
	 * 
	 * if (save_file->ShowModal() == wxID_OK)
	 * {
	 *     // wxString fname = save_file->GetFilename();
	 *     // editor->SaveMap(fname);
	 *     editor->SaveMap(save_file->GetPath());
	 * }
     */
	
	editor->SaveWorld();
}

void CRootFrame::SetStatusHeightMap()
{
	int hm_w = editor->GetMap()->GetHeightMap()->GetWidth();
	int hm_h = editor->GetMap()->GetHeightMap()->GetHeight();
	wxString hm_text = editor->GetMap()->GetHeightMapName();
	hm_text << _T(": ") << hm_w << _T(" x ") << hm_h;
	SetStatusText(hm_text, 0);
}
void CRootFrame::SetStatusTexture()
{
	int hm_w = editor->GetMap()->GetTexture()->GetWidth();
	int hm_h = editor->GetMap()->GetTexture()->GetHeight();
	wxString hm_text = editor->GetMap()->GetTextureName();
	hm_text << _T(": ") << hm_w << _T(" x ") << hm_h;
	SetStatusText(hm_text, 1);
}

void CRootFrame::OnLoad(wxCommandEvent &e)
{
	// TODO onload bude nejspis jen vybirat cestu k datum
	wxFileDialog *open_file = new wxFileDialog(this, _T("Open file"), _T(""), _T(""), _T("*.png;*.jpg;*.bmp"),
			wxOPEN, wxDefaultPosition);
	if (open_file->ShowModal() == wxID_OK)
	{
		// wxString fname = open_file->GetFilename();
		editor->SetHeightMap(open_file->GetPath());
		SetStatusHeightMap();
		
		editor->InitCamera();
	}
}

void CRootFrame::OnQuit(wxCommandEvent &e)
{
	Close(true);
}

void CRootFrame::OnViewSolid(wxCommandEvent &e)
{
	editor->SetRenderState(GL_TRIANGLES);
}
void CRootFrame::OnViewWireframe(wxCommandEvent &e)
{
	editor->SetRenderState(GL_LINES);
}

void CRootFrame::OnEditorRepaint(wxCommandEvent &e)
{
	m_glcanvas->Refresh(false);
	m_world_hm_panel->Refresh(false);
	m_world_tex_panel->Refresh(false);
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
	
	// todle se provede jen jednou tady? vzhledem k tomu ze pointer je
	// globalni..
	editor = Editor::CEditor::GetInstance();
	try {
		editor->Init();
	}
	catch (CException &e) {
		wxLogError(wxString::FromAscii(e.what()));
		e.show();
		throw;
	}
	

	//// frame
	CRootFrame *frame = new CRootFrame;

	// menu
	wxMenuBar *menu_bar = new wxMenuBar;
	wxMenu *menu_file = new wxMenu;
	wxMenu *menu_view = new wxMenu;
	wxMenu *menu_window = new wxMenu;

	// menu file 
	menu_file->Append(ID_MF_New, _T("New"));
	menu_file->Append(ID_MF_Save, _T("Save"));
	menu_file->Append(ID_MF_Load, _T("Load"));
	menu_file->AppendSeparator();
	menu_file->Append(ID_MF_Quit, _T("E&xit"));

	// menu view
	menu_view->Append(ID_MV_Solid, _T("Solid"));
	menu_view->Append(ID_MV_Wireframe, _T("Wireframe"));
    /*
	 * menu_view->Append(ID_M_, _T(""));
	 * menu_view->Append(ID_M_, _T(""));
	 * menu_view->Append(ID_M_, _T(""));
	 * menu_view->Append(ID_M_, _T(""));
     */
	
	// menu window
	menu_window->AppendCheckItem(wxID_ANY + 99, _T("Show Toolbox"));
	menu_window->Check(wxID_ANY+99, true);


	// menubar
	menu_bar->Append(menu_file, _T("&File"));
	// menu_bar->Append(menu_edit, _T("&Edit"));
	menu_bar->Append(menu_view, _T("&View"));
	menu_bar->Append(menu_window, _T("&Window"));
	// menu_bar->Append(menu_help, _T("&Help"));
	frame->SetMenuBar(menu_bar);
	
	// status bar
	frame->CreateStatusBar(2);
	int hm_w = editor->GetMap()->GetHeightMap()->GetWidth();
	int hm_h = editor->GetMap()->GetHeightMap()->GetHeight();
	wxString hm_text = editor->GetMap()->GetHeightMapName();
	hm_text << _T(": ") << hm_w << _T(" x ") << hm_h;
	frame->SetStatusText(hm_text, 0);
	hm_text = editor->GetMap()->GetTextureName();
	hm_w = editor->GetMap()->GetTexture()->GetWidth();
	hm_h = editor->GetMap()->GetTexture()->GetHeight();
	hm_text << _T(": ") << hm_w << _T(" x ") << hm_h;
	frame->SetStatusText(hm_text, 1);

	// canvas
	int al[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER};
	CCanvas *canvas = new CCanvas(frame, al);
	frame->SetCanvas(canvas);

	// sizer hierarchy
	wxBoxSizer *top_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *toolpanel_top_sizer = new wxBoxSizer(wxVERTICAL);
	
	// sizer pro vykreslovani nactenyho sveta
	wxBoxSizer *world_sizer = new wxBoxSizer(wxVERTICAL);


    /*
	 * wxStaticBox *heightmap_sbox = new wxStaticBox(frame, -1, _T("Heightmap"));
	 * wxStaticBoxSizer *toolpanel_heightmap_sizer = new wxStaticBoxSizer(heightmap_sbox, wxVERTICAL);
	 * 
	 * wxStaticBox *texture_sbox = new wxStaticBox(frame, -1, _T("Texture"));
	 * wxStaticBoxSizer *toolpanel_texture_sizer = new wxStaticBoxSizer(texture_sbox, wxVERTICAL);
     */
	
	// wxButton *b1 = new wxButton(frame, wxID_OK, _T("test1"));

	 
	// bocni panel se svetem
	wxRect r = editor->GetWorldView(); 
	CPanel *tmp = new CPanel(frame);
	tmp->SetImg(editor->GetWorldHeightMap());
	world_sizer->Add(tmp, 1, wxEXPAND); 
	frame->SetHmPanel(tmp);
	
	tmp = new CPanel(frame);
	tmp->SetImg(editor->GetWorldTexture());
	tmp->SetTex(true);
	world_sizer->Add(tmp, 1, wxEXPAND); 
	frame->SetTexPanel(tmp);
	
	// naplnit hlavni sizer
	top_sizer->Add(toolpanel_top_sizer); 
	top_sizer->Add(canvas, 3, wxALL | wxEXPAND); 
	top_sizer->Add(world_sizer, 1, wxALL | wxEXPAND ); 

    /*
	 * toolpanel_top_sizer->Add(toolpanel_heightmap_sizer, 1, wxALL | wxEXPAND);
	 * toolpanel_top_sizer->Add(toolpanel_texture_sizer, 1, wxALL | wxEXPAND);
     */
	// toolpanel_top_sizer->Add(b1, wxALL | wxEXPAND);
	// toolpanel_top_sizer->Add(b2, wxALL | wxEXPAND);
	// 
    /*
	 * toolpanel_heightmap_sizer->Add(b1, 1, wxALL|wxEXPAND);
	 * toolpanel_heightmap_sizer->Add(b2, 1, wxALL|wxEXPAND);
	 * toolpanel_texture_sizer->Add(b3, 1, wxALL|wxEXPAND);
	 * toolpanel_texture_sizer->Add(b4, 1, wxALL|wxEXPAND);
     */
	
	// toolpanel_heightmap_sizer->Add(b3, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(frame->m_bmpbut_heightmap, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(b1, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(b2, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(b3, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(bmpb2, 1, wxALL|wxEXPAND);
	
	//// FRAME TOOLBOX
    /*
	 * wxFrame *frame_toolbox = new CToolboxFrame(frame);
	 * frame_toolbox->Show(TRUE);
     */
	
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


/*
 * CToolboxFrame
 * okno panelu nastroju
 */
CToolboxFrame::CToolboxFrame(wxWindow *parent)
	:wxFrame(parent, -1, TOOLBOX_NAME, TOOLBOX_POSITION, TOOLBOX_SIZE),
	m_notebook(new wxNotebook(this, wxID_ANY))
{
	wxPanel *page_toolbox = new wxPanel(m_notebook, wxID_ANY);
	wxPanel *page_maps = new wxPanel(m_notebook, wxID_ANY);
	wxPanel *page_layers = new wxPanel(m_notebook, wxID_ANY);
	m_notebook->AddPage(page_toolbox, _T("Toolbox"));
	m_notebook->AddPage(page_maps, _T("Maps"));
	m_notebook->AddPage(page_layers, _T("Layers"));
	
	wxPanel *p = page_toolbox;
    /*
	 * wxButton *b1 = new wxButton(p, wxID_ANY, _T("test1"));
	 * wxButton *b2 = new wxButton(p, wxID_ANY, _T("test2"));
     */
	wxPanel *panel_heightmap = new wxPanel(p, wxID_ANY);
	wxPanel *panel_texture = new wxPanel(p, wxID_ANY);
	wxPanel *p3 = new wxPanel(p, wxID_ANY);
	wxPanel *p4 = new wxPanel(p, wxID_ANY);
	// wxCheckBox *cb_heightmap = new wxCheckBox(p, wxID_ANY, _T(""));
	// wxCheckBox *cb_texture = new wxCheckBox(p, wxID_ANY, _T(""));
	// wxCheckBox *cb3 = new wxCheckBox(p, wxID_ANY, _T(""));
	// wxCheckBox *cb4 = new wxCheckBox(p, wxID_ANY, _T(""));
	
	panel_heightmap->SetMinSize(wxSize(200, 200));
	panel_texture->SetMinSize(wxSize(200, 200));
	p3->SetMinSize(wxSize(200, 200));
	p4->SetMinSize(wxSize(200, 200));
			
	//// toolbox
	//
	//
	//// maps
    /*
	 * wxFlexGridSizer *flex = new wxFlexGridSizer(2, 4, 1, 1);
	 * flex->Add(cb_heightmap, 1, wxEXPAND);
	 * flex->Add(panel_heightmap, 1, wxEXPAND);
	 * flex->Add(cb_texture, 1, wxEXPAND);
	 * flex->Add(panel_texture, 1, wxEXPAND);
	 * flex->Add(cb3, 1, wxEXPAND);
	 * flex->Add(p3, 1, wxEXPAND);
	 * flex->Add(cb4, 1, wxEXPAND);
	 * flex->Add(p4, 1, wxEXPAND);
	 * flex->AddGrowableCol(1);
	 * flex->AddGrowableCol(3);
	 * 
	 * flex->Layout();
	 * page_toolbox->SetSizerAndFit(flex);
	 * 
	 * wxSize s = flex->GetSize();
	 * s = s + wxSize(0, 30);
	 * SetMinSize(s);
	 * SetSize(s);
     */

	//// layers
	//
	//
}

