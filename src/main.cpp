#include "main.h"
#include "editor.h"

// TODO todle pride pryc az to nebude potreba
using namespace std;

IMPLEMENT_APP(CEditorApp)

Editor::CEditor *editor;

CCanvas::CCanvas(wxFrame *parent, int attrib_list[])
	:wxGLCanvas(parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas"), attrib_list)
{
}


void CCanvas::Render(/* wxPaintEvent &e */)
{
	// editor = Editor::CEditor::GetInstance();
	editor->Render();
}
void CCanvas::OnPaint(wxPaintEvent &e)
{
	int w, h;
	GetClientSize(&w, &h);
	static bool go = true;

	SetCurrent();
	wxPaintDC(this);

	// init gl only once, but after setcurrent
	if (go) {
		editor->InitGL(w, h);
		go = false;
	}

	Render();
	SwapBuffers();
}

void CCanvas::OnIdle(wxIdleEvent &e)
{
}

void CCanvas::OnResize(wxSizeEvent &e)
{
	int w, h;
	GetClientSize(&w, &h);
	editor->OnResize(w, h);
	// Refresh(false);
}


void CCanvas::OnKeyDown(wxKeyEvent &e)
{
	int key = e.GetKeyCode();
	if (key == WXK_ESCAPE) {
		wxTheApp->GetTopWindow()->Close(true);
	}
	else if (key == WXK_UP) {
		editor->MoveCamera(3.5);
	}
	else if (key == WXK_DOWN) {
		editor->MoveCamera(-3.5);
	}
	else if (key == WXK_LEFT) {
		editor->SlideCamera(-3.5, 0);
	}
	else if (key == WXK_RIGHT) {
		editor->SlideCamera(3.5, 0);
	}
	Refresh(false);
	e.Skip();
}
void CCanvas::OnMouseEvents(wxMouseEvent &e)
{
	static int last_x, last_y;
	int x = e.GetX();
	int y = e.GetY();
	float delta_x = (x - last_x);
	float delta_y = (y - last_y);
	int wheel_rot;

	if (e.Dragging()) { // dragging
		if (e.RightIsDown() && e.LeftIsDown()) {
			editor->SlideCamera(delta_x, delta_y);
			Refresh(false);
		}
		else if (e.RightIsDown()) {
			editor->MoveCameraFocus(delta_x, 0, -delta_y);
			Refresh(false);
		}
	}
	else { // not dragging
		if (e.LeftIsDown()) {
			Vec3 p = editor->Pick(x, y);
			editor->ProcessPicked(p);
			Refresh(false);
		}
	}
	if ((wheel_rot = e.GetWheelRotation()) != 0) {
		editor->AdjustZoom((wheel_rot > 0) ? 0.9 : 1.1);
		Refresh(false);
	}
	
	last_x = x;
	last_y = y;
	
	e.Skip();
}

CRootFrame::CRootFrame()
	:wxFrame(0, -1, APP_NAME, wxPoint(-1, -1), APP_SIZE)
{
}

void CRootFrame::OnNew(wxCommandEvent &e)
{
	// TODO otevrit dialog, ve kterym se bude dat nastavit pocatecni textura, mapa,
	// velikost, ...
	
	const int sizex = 128;
	const int sizey = 128;
	
	editor->GetMap()->Create(sizex, sizey, true);
#ifdef DEBUG
	// // std::cout << string(m_heightmap_name.ToAscii()) + " loaded" << endl;
	// std::cout << "triangles: " << m_ntriangles << std::endl;
	// std::cout << "indices: " << m_nindices << std::endl;
#endif
	editor->SetSync();
	
	Refresh();
}

void CRootFrame::OnSave(wxCommandEvent &e)
{

	wxFileDialog *save_file = new wxFileDialog(this, _T("Save file"), _T(""), _T(""), _T("*.png"),
			wxSAVE, wxDefaultPosition);
	
	if (save_file->ShowModal() == wxID_OK)
	{
		wxString fname = save_file->GetFilename();
		
		editor->SaveMap(save_file->GetPath());
	}
}

void CRootFrame::OnLoad(wxCommandEvent &e)
{

	wxFileDialog *open_file = new wxFileDialog(this, _T("Open file"), _T(""), _T(""), _T("*.png;*.jpg;*.bmp"),
			wxOPEN, wxDefaultPosition);
	if (open_file->ShowModal() == wxID_OK)
	{
		// wxString fname = open_file->GetFilename();
		// TODO
		editor->SetHeightMap(open_file->GetPath());

		// wxImage *tmp = new wxImage(*editor->GetHeightMap());
		// tmp->Rescale(100, 100);
		// m_bmpbut_heightmap->SetBitmapLabel(*tmp);
		// delete tmp;
		// SetStatusText(open_file->GetDirectory(), 1);
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

void CRootFrame::OnShadersEnable(wxCommandEvent &e)
{
	editor->EnableShaders();
	// DEBUG
	cout << "shaders enabled" << endl;
}
void CRootFrame::OnShadersDisable(wxCommandEvent &e)
{
	editor->DisableShaders();
	// DEBUG
	cout << "shaders disabled" << endl;
}

bool CEditorApp::OnInit()
{
	// todle se provede jen jednou tady? vzhledem k tomu ze pointer je
	// globalni..
	editor = Editor::CEditor::GetInstance();
	editor->Init();

	//// frame
	CRootFrame *frame = new CRootFrame;

	// menu
	wxMenuBar *menu_bar = new wxMenuBar;
	wxMenu *menu_file = new wxMenu;
	wxMenu *menu_view = new wxMenu;
	wxMenu *menu_shaders = new wxMenu;
	wxMenu *menu_mode = new wxMenu;

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
	
	// menu shaders
	menu_shaders->AppendRadioItem(ID_MS_Enable, _T("Enable"));
	menu_shaders->AppendRadioItem(ID_MS_Disable, _T("Disable"));
	menu_shaders->Check(ID_MS_Disable, true);
	
	// menu mode
	menu_mode->AppendRadioItem(wxID_ANY, _T("HM + texture"));
	menu_mode->AppendRadioItem(wxID_ANY, _T("Satellite set"));

	// menu bar
	menu_bar->Append(menu_file, _T("&File"));
	// menu_bar->Append(menu_edit, _T("&Edit"));
	menu_bar->Append(menu_view, _T("&View"));
	menu_bar->Append(menu_shaders, _T("&Shaders"));
	menu_bar->Append(menu_mode, _T("&Mode"));
	// menu_bar->Append(menu_help, _T("&Help"));
	frame->SetMenuBar(menu_bar);

	// heightmap bitmapbutton
    /*
	 * wxImage *tmp = new wxImage(*editor->GetHeightMap());
	 * tmp->Rescale(100, 100);
	 * wxBitmap *heightmap_bmp = new wxBitmap(*tmp);
	 * delete tmp;
	 * frame->m_bmpbut_heightmap = new wxBitmapButton(frame, ID_BMPBUT_Heightmap, *heightmap_bmp, wxDefaultPosition,
	 *         wxSize(100, 100));
     */

	// texture bitmapbutton
	// tmp = new wxImage(*editor->GetTexture());
	// tmp->Rescale(100, 100);
	// wxBitmap *texture_bmp = new wxBitmap(*tmp);
	// delete tmp;
	// wxBitmapButton *bmpb2 = new wxBitmapButton(frame, wxID_ANY, *texture_bmp, wxDefaultPosition,
			// wxSize(100, 100));


	// canvas
	int al[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER};
	CCanvas *canvas = new CCanvas(frame, al);
	frame->SetCanvas(canvas);

	// sizer hierarchy
	wxBoxSizer *top_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *toolpanel_top_sizer = new wxBoxSizer(wxVERTICAL);

	wxStaticBox *heightmap_sbox = new wxStaticBox(frame, -1, _T("Heightmap"));
	wxStaticBoxSizer *toolpanel_heightmap_sizer = new wxStaticBoxSizer(heightmap_sbox, wxVERTICAL);
	
	wxStaticBox *texture_sbox = new wxStaticBox(frame, -1, _T("Texture"));
	wxStaticBoxSizer *toolpanel_texture_sizer = new wxStaticBoxSizer(texture_sbox, wxVERTICAL);
	
	//test
	wxButton *b1 = new wxButton(frame, wxID_OK, _T("test1"));
	wxButton *b2 = new wxButton(frame, wxID_OK, _T("test2"));
	wxButton *b3 = new wxButton(frame, wxID_OK, _T("test3"));
	wxButton *b4 = new wxButton(frame, wxID_OK, _T("test3"));

	top_sizer->Add(toolpanel_top_sizer); 
	top_sizer->Add(canvas, 1, wxALL | wxEXPAND); 

	toolpanel_top_sizer->Add(toolpanel_heightmap_sizer, 1, wxALL | wxEXPAND);
	toolpanel_top_sizer->Add(toolpanel_texture_sizer, 1, wxALL | wxEXPAND);
	// toolpanel_top_sizer->Add(b1, wxALL | wxEXPAND);
	// toolpanel_top_sizer->Add(b2, wxALL | wxEXPAND);
	// 
	toolpanel_heightmap_sizer->Add(b1, 1, wxALL|wxEXPAND);
	toolpanel_heightmap_sizer->Add(b2, 1, wxALL|wxEXPAND);
	toolpanel_texture_sizer->Add(b3, 1, wxALL|wxEXPAND);
	toolpanel_texture_sizer->Add(b4, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(b3, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(frame->m_bmpbut_heightmap, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(b1, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(b2, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(b3, 1, wxALL|wxEXPAND);
	// toolpanel_heightmap_sizer->Add(bmpb2, 1, wxALL|wxEXPAND);

	frame->SetSizer(top_sizer);
	frame->Show(TRUE);
	return TRUE;
}

int CEditorApp::OnExit()
{
	// DEBUG
	cout << "APP EXITING" << endl;
	editor->CleanUp();
}
