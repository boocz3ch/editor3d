#ifndef _GLOBALS_H_
#define _GLOBALS_H_

const wxString APP_NAME = wxT("Interactive 3D Editor");
const wxSize APP_SIZE = wxSize(930, 728);
const wxPoint APP_POSITION = wxDefaultPosition;

const wxString TOOLBOX_NAME = wxT("Toolbox");
const wxSize TOOLBOX_SIZE = wxSize(-1, 800);
const wxPoint TOOLBOX_POSITION = wxPoint(0, 0);

const wxString HEIGHTMAP_SUFFIX = wxT(".png");
const wxString TEXTURE_SUFFIX = wxT(".jpg");

const wxString DEFAULT_DATA_ROOT = wxT("../data/");
const wxString DEFAULT_TER_TILES_PATH = wxT("terrain_tiles/");
const wxString DEFAULT_TEX_TILES_PATH = wxT("texture_tiles/");

const float LIGHT_POSITION[] = {79*1.5, 94.0, 20.0, 1.0};

DECLARE_EVENT_TYPE(EVT_EDITOR_PRELOAD_RDY, wxID_ANY)

#endif
