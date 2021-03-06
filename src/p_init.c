
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"
#include "i_header.h"

#include "g_misc.h"

#include "e_slider.h"
#include "e_header.h"
#include "e_editor.h"
#include "e_slider.h"
#include "e_help.h"
#include "e_log.h"

#include "i_input.h"
#include "i_view.h"
#include "i_display.h"
#include "i_buttons.h"
#include "t_template.h"
#include "m_input.h"
#include "f_load.h"
#include "f_game.h"
#include "x_sound.h"

#include "s_mission.h"

#include "p_panels.h"

#include "d_draw.h"

#define STANDARD_PANEL_TOP 30
#define PANEL_RESIZE_W 10


//#include "s_setup.h"

extern struct fontstruct font [FONTS];
extern ALLEGRO_DISPLAY* display;

// these queues are declared in g_game.c. They're externed here so that they can be flushed when the editor does something slow.
extern ALLEGRO_EVENT_QUEUE* event_queue; // these queues are initialised in main.c
extern ALLEGRO_EVENT_QUEUE* fps_queue;

extern struct game_struct game; // in g_game.c
extern struct view_struct view;
extern struct log_struct mlog; // in e_log.c
extern struct slider_struct slider [SLIDERS];
extern struct editorstruct editor;
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct template_state_struct tstate;

char mstring [MENU_STRING_LENGTH];

void setup_log_panel(void);
void setup_sysmenu_panel(void);
void setup_design_panel(void);
void setup_template_panel(void);
void setup_editor_panel(void);

void add_resize_subpanel(int pan, int subpan, int el);

void set_subpanel_positions(int pan);
void set_element_positions(int pan, int subpan);
void init_element_button(int pan, int subpan, int el,
																									int style,
																									int x, int y,
																									int width, int h,
																									char* name);
void init_whole_subpanel(int pan, int subpan, int panel_resizable);
void init_basic_subpanel(int pan, int subpan, int type, int x1, int y1, int width, int h);

void attach_scrollbar_to_element(int pan,
																																	int subpan,
																																	int main_el,
																																	int slider_el,
																																	int dir,
																																	int value_max,
																																	int represents_size,
																																	int slider_index,
																																	int* ptr_value);

enum
{
	BUTTON_NAME_TAB_P0,
	BUTTON_NAME_TAB_P1,
	BUTTON_NAME_TAB_P2,
	BUTTON_NAME_TAB_P3,
	BUTTON_NAME_FILE_LOAD,
	BUTTON_NAME_FILE_SAVE,
	BUTTON_NAME_TEMPLATES,
	BUTTON_NAME_UNKNOWN,
	BUTTON_NAME_FILE,
	BUTTON_NAME_EDIT,
	BUTTON_NAME_SEARCH,
	BUTTON_NAME_BUILD,
	BUTTON_NAME_NEW,
	BUTTON_NAME_OPEN,
	BUTTON_NAME_SAVE,
	BUTTON_NAME_SAVE_AS,
	BUTTON_NAME_CLOSE,
	BUTTON_NAME_UNDO,
	BUTTON_NAME_REDO,
	BUTTON_NAME_CUT,
	BUTTON_NAME_COPY,
	BUTTON_NAME_PASTE,
	BUTTON_NAME_CLEAR,
	BUTTON_NAME_FIND,
	BUTTON_NAME_FIND_NEXT,
	BUTTON_NAME_TEST_BUILD,
	BUTTON_NAMES
};

char* button_name [] =
{
	"Player 0", // BUTTON_NAME_TAB_P0
	"Player 1", // BUTTON_NAME_TAB_P1
	"Player 2", // BUTTON_NAME_TAB_P2
	"Player 3", // BUTTON_NAME_TAB_P3
	"Load template file", // BUTTON_NAME_FILE_LOAD
	"Save template file", // BUTTON_NAME_FILE_SAVE
	"templates", // BUTTON_NAME_TEMPLATES
	"unknown", // BUTTON_NAME_UNKNOWN
	"File", // BUTTON_NAME_FILE
	"Edit", //	BUTTON_NAME_EDIT
	"Search", //	BUTTON_NAME_SEARCH
	"Compile", //	BUTTON_NAME_BUILD
	"New", // BUTTON_NAME_
	"Open", // BUTTON_NAME_
	"Save", // BUTTON_NAME_
	"Save as", // BUTTON_NAME_
	"Close", // BUTTON_NAME_
	"Undo", // BUTTON_NAME_
	"Redo", // BUTTON_NAME_
	"Cut", // BUTTON_NAME_
	"Copy", // BUTTON_NAME_
	"Paste", // BUTTON_NAME_
	"Clear", // BUTTON_NAME_
	"Find", // BUTTON_NAME_
	"Find next", // BUTTON_NAME_
	"Test build", // BUTTON_NAME_

};



void init_panels(void)
{

	int i, j;

	for (i = 0; i < PANELS; i ++)
	{
		panel[i].open = 0;
		panel[i].w = 400; // updated below for some panels
		panel[i].h = inter.display_h - LOG_WINDOW_H;
		panel[i].y1 = 0;
		panel[i].y2 = panel[i].h;
// don't need to set x,y values as these are set any time the panel is opened.
  for (j = 0; j < SUBPANELS; j ++)
		{
			panel[i].subpanel[j].type = SP_TYPE_NONE;
		}
  for (j = 0; j < ELEMENTS; j ++)
		{
			panel[i].element[j].type = PE_TYPE_NONE;
		}

	}

	panel[PANEL_TEMPLATE].w = 340;
	panel[PANEL_DESIGN].w = 540;

	panel[0].open = 1;
	panel[0].w = inter.display_w;
	panel[0].h = inter.display_h;
	panel[0].x1 = 0;
	panel[0].y1 = 0;
	panel[0].x2 = inter.display_w;
	panel[0].y2 = inter.display_h;

	panel[PANEL_LOG].h = LOG_WINDOW_H;
	panel[PANEL_LOG].y1 = inter.display_h - LOG_WINDOW_H;
	panel[PANEL_LOG].y2 = inter.display_h;
// the following are reset by reset_panel_positions
	panel[PANEL_LOG].w = 200;
	panel[PANEL_LOG].x1 = 0;
	panel[PANEL_LOG].x2 = inter.display_w;

// colours (can't use colours struct because it hasn't been initialised yet):
 panel[PANEL_SYSMENU].background_colour = al_map_rgb(25, 15, 60);
 panel[PANEL_EDITOR].background_colour = al_map_rgb(25, 15, 60);
 panel[PANEL_DESIGN].background_colour = al_map_rgb(25, 15, 60);
 panel[PANEL_TEMPLATE].background_colour = al_map_rgb(25, 15, 60);
 panel[PANEL_LOG].background_colour = al_map_rgb(20, 20, 60);
 panel[PANEL_MAIN].background_colour = colours.black; // not used

 reset_panel_positions();

 setup_log_panel();
 setup_sysmenu_panel();
 setup_design_panel();
 setup_template_panel();
 setup_editor_panel();


}


void setup_log_panel(void)
{

 init_whole_subpanel(PANEL_LOG, FSP_LOG_WHOLE, 0);
/*
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].exists = 1;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].open = 1;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].panel = PANEL_LOG;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].type = SP_TYPE_WHOLE;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].x1 = 1;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].y1 = 1;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].x2 = panel[PANEL_LOG].w;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].y2 = LOG_WINDOW_H;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].w = panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].x2;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].h = LOG_WINDOW_H;
//	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].first_element = FPE_LOG_WINDOW; // no elements
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].clip = 1;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].highlight = 0;
	panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].last_highlight = 0;*/

 panel[PANEL_LOG].element[FPE_LOG_WINDOW].exists = 1;
 panel[PANEL_LOG].element[FPE_LOG_WINDOW].open = 1;
 panel[PANEL_LOG].element[FPE_LOG_WINDOW].panel = PANEL_LOG;
 panel[PANEL_LOG].element[FPE_LOG_WINDOW].subpanel = FSP_LOG_WHOLE;
 panel[PANEL_LOG].element[FPE_LOG_WINDOW].type = PE_TYPE_WINDOW;
	panel[PANEL_LOG].element[FPE_LOG_WINDOW].location = ELEMENT_LOCATION_LEFT_TOP;
	panel[PANEL_LOG].element[FPE_LOG_WINDOW].offset_x = 0;
	panel[PANEL_LOG].element[FPE_LOG_WINDOW].offset_y = 0;
	panel[PANEL_LOG].element[FPE_LOG_WINDOW].fit = ELEMENT_FIT_FILL_WITH_V_SCROLLBAR;
 panel[PANEL_LOG].element[FPE_LOG_WINDOW].w = SLIDER_BUTTON_SIZE;
 panel[PANEL_LOG].element[FPE_LOG_WINDOW].h = LOG_WINDOW_H;
//	panel[PANEL_LOG].element[FPE_LOG_WINDOW].clip = 1;


	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].exists = 1;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].open = 1;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].panel = PANEL_LOG;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].subpanel = FSP_LOG_WHOLE;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].location = ELEMENT_LOCATION_RIGHT_TOP;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].offset_x = -SLIDER_BUTTON_SIZE;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].offset_y = 0;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].fit = ELEMENT_FIT_SUBPANEL_H;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].type = PE_TYPE_SCROLLBAR_EL_V_CHAR;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].w = SLIDER_BUTTON_SIZE;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].h = LOG_WINDOW_H;
//	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].first_element = -1; // no elements
//	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].clip = 0;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].highlight = 0;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].last_highlight = 0;
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].value [0] = FPE_LOG_WINDOW; // element affected by this scrollbar
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].value [1] = SLIDER_LOG_SCROLLBAR_V; // element affected by this scrollbar
	panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].ptr_value = &mlog.window_pos; // element affected by this scrollbar

 mlog.h_lines = panel[PANEL_LOG].h / LOG_LINE_HEIGHT;

 init_slider(panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].value [1],
													PANEL_LOG,
													FSP_LOG_WHOLE,
													FPE_LOG_SCROLLBAR,
													panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].ptr_value,
													SLIDEDIR_VERTICAL,
													mlog.h_lines,
													LOG_LINES, // - mlog.h_lines,
													LOG_WINDOW_H,
													1,
													mlog.h_lines - 1,
													mlog.h_lines,
//													0, //panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].x1,
//													0, //panel[PANEL_LOG].element[FPE_LOG_SCROLLBAR].y1,
													SLIDER_BUTTON_SIZE,
													COL_BLUE,
													0);

 set_subpanel_positions(PANEL_LOG);

 init_log();

}

char* sysmenu_button_name [4] =
{
"Pause",
//"Save",
"Quit",
"Really quit?"

};

void setup_sysmenu_panel(void)
{

	add_resize_subpanel(PANEL_SYSMENU, FSP_SYSMENU_PANEL_RESIZE, FPE_SYSMENU_PANEL_RESIZE);

 init_whole_subpanel(PANEL_SYSMENU, FSP_SYSMENU_WHOLE, 1);


#define SYSMENU_X 30
#define SYSMENU_Y 40
#define SYSMENU_H 50
#define SYSMENU_Y_GAP 5
#define SYSMENU_W 300

 int button_y = SYSMENU_Y;
/*
	init_element_button(PANEL_SYSMENU,
																					FSP_SYSMENU_WHOLE,
																					FPE_SYSMENU_PAUSE,
																					BUTTON_STYLE_MENU_BIG,
																					SYSMENU_X,
																					button_y,
																					SYSMENU_W,
																					SYSMENU_H,
																					sysmenu_button_name [0]);
	button_y += SYSMENU_H + SYSMENU_Y_GAP;
*/
/*
	init_element_button(PANEL_SYSMENU,
																					FSP_SYSMENU_WHOLE,
																					FPE_SYSMENU_SAVE,
																					BUTTON_STYLE_MENU_BIG,
																					SYSMENU_X,
																					button_y,
																					SYSMENU_W,
																					SYSMENU_H,
																					sysmenu_button_name [1]);

	button_y += SYSMENU_H + SYSMENU_Y_GAP;*/

	init_element_button(PANEL_SYSMENU,
																					FSP_SYSMENU_WHOLE,
																					FPE_SYSMENU_QUIT,
																					BUTTON_STYLE_MENU_BIG,
																					SYSMENU_X,
																					button_y,
																					SYSMENU_W,
																					SYSMENU_H,
																					sysmenu_button_name [1]);

	button_y += SYSMENU_H + SYSMENU_Y_GAP;

	init_element_button(PANEL_SYSMENU,
																					FSP_SYSMENU_WHOLE,
																					FPE_SYSMENU_CONFIRM_QUIT,
																					BUTTON_STYLE_MENU_BIG,
																					SYSMENU_X,
																					button_y,
																					SYSMENU_W,
																					SYSMENU_H,
																					sysmenu_button_name [2]);
 panel[PANEL_SYSMENU].element[FPE_SYSMENU_CONFIRM_QUIT].open = 0;

 set_subpanel_positions(PANEL_SYSMENU);


}


void add_resize_subpanel(int pan, int subpan, int el)
{


	panel[pan].subpanel[subpan].exists = 1;
	panel[pan].subpanel[subpan].open = 1;
	panel[pan].subpanel[subpan].panel = pan;
	panel[pan].subpanel[subpan].type = SP_TYPE_PANEL_RESIZE;
	panel[pan].subpanel[subpan].x1 = 1;
	panel[pan].subpanel[subpan].y1 = 1;
	panel[pan].subpanel[subpan].w = PANEL_RESIZE_W;
	panel[pan].subpanel[subpan].h = panel[pan].h;
	panel[pan].subpanel[subpan].x2 = panel[pan].subpanel[subpan].x1 + panel[pan].subpanel[subpan].w;
	panel[pan].subpanel[subpan].y2 = panel[pan].h;
//	panel[PANEL_LOG].subpanel[subpan].first_element = FPE_LOG_WINDOW; // no elements
	panel[pan].subpanel[subpan].clip = 1;
	panel[pan].subpanel[subpan].highlight = 0;
	panel[pan].subpanel[subpan].last_highlight = 0;

 panel[pan].element[el].exists = 1;
 panel[pan].element[el].open = 1;
 panel[pan].element[el].panel = pan;
 panel[pan].element[el].subpanel = subpan;
	panel[pan].element[el].highlight = 0;
	panel[pan].element[el].last_highlight = 0;
 panel[pan].element[el].type = PE_TYPE_PANEL_RESIZE;
 panel[pan].element[el].location = ELEMENT_LOCATION_LEFT_TOP;
 panel[pan].element[el].offset_x = 0;
	panel[pan].element[el].offset_y = 0;
	panel[pan].element[el].fit = ELEMENT_FIT_SUBPANEL_H;
 panel[pan].element[el].w = PANEL_RESIZE_W;
 panel[pan].element[el].h = panel[pan].subpanel[subpan].h;

}

//#define DESIGN_BUTTONS 32
char design_button_name [] [16] =
{
 	"New", // FPE_DESIGN_TOOLS_EMPTY_NEW,

 	//"Load", // FPE_DESIGN_TOOLS_MAIN_LOAD,
 	"Write header", // FPE_DESIGN_TOOLS_MAIN_AUTO,
 	"Autocode", //  	FPE_DESIGN_TOOLS_MAIN_AUTOCODE
 	"Symmetry", // FPE_DESIGN_TOOLS_MAIN_SYMM,
 	"Lock", // FPE_DESIGN_TOOLS_MAIN_LOCK,
 	"Unlock", // FPE_DESIGN_TOOLS_MAIN_UNLOCK,
 	"Delete", //  	FPE_DESIGN_TOOLS_MAIN_DELETE
 	"? help", //  	FPE_DESIGN_TOOLS_MAIN_HELP
 	"? more help", //  	FPE_DESIGN_TOOLS_MAIN_HELP_MORE



 	"Shape", // FPE_DESIGN_SUBTOOLS_MEMBER_SHAPE,
// 	"5-sided", // FPE_DESIGN_TOOLS_MEMBER_SHAPE5,
// 	"6-sided", // FPE_DESIGN_TOOLS_MEMBER_SHAPE6,
// 	"Flip", // FPE_DESIGN_TOOLS_MEMBER_FLIP,
 	"Delete", // FPE_DESIGN_TOOLS_MEMBER_DELETE,
 	"Cancel", // FPE_DESIGN_TOOLS_MEMBER_EXIT,

 	"Core shape", // FPE_DESIGN_SUBTOOLS_CORE_CORE_SHAPE,
 	"Cancel", // FPE_DESIGN_SUBTOOLS_CORE_EXIT,

 	"Link", // FPE_DESIGN_TOOLS_ADD_LINK,
 	"Data", // FPE_DESIGN_TOOLS_VERTEX_OBJ_STD,
 	"Move", // FPE_DESIGN_TOOLS_VERTEX_OBJ_MOVE,
 	"Attack", // FPE_DESIGN_TOOLS_VERTEX_OBJ_ATTACK,
 	"Defend", // FPE_DESIGN_TOOLS_VERTEX_OBJ_DEFEND,
 	"Misc", // FPE_DESIGN_TOOLS_VERTEX_OBJ_MISC,
 	"Clear", // FPE_DESIGN_TOOLS_VERTEX_OBJ_CLEAR,
 	"Cancel", // FPE_DESIGN_TOOLS_VERTEX_EXIT,

//  "Next link", // FPE_DESIGN_TOOLS_NEXT_LINK
 	"Confirm", //  	FPE_DESIGN_TOOLS_DELETE_CONFIRM
 	"Cancel", //  	FPE_DESIGN_TOOLS_DELETE_EXIT

 	"Cancel", // FPE_DESIGN_TOOLS_AUTOCODE_EXIT,


};



void setup_design_panel(void)
{

	add_resize_subpanel(PANEL_DESIGN, FSP_DESIGN_PANEL_RESIZE, FPE_DESIGN_PANEL_RESIZE);

	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].exists = 1;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].open = 1;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].panel = PANEL_DESIGN;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].type = SP_TYPE_WHOLE;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1 = PANEL_RESIZE_W;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y1 = 30;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].w = 250;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].h = 400;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].w;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].h;
//	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].w;
//	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y1 + panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].h;
//	panel[PANEL_LOG].subpanel[subpan].first_element = FPE_LOG_WINDOW; // no elements
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].clip = 1;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].highlight = 0;
	panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].last_highlight = 0;


 panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].exists = 1;
 panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].open = 1;
 panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].panel = PANEL_DESIGN;
 panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].subpanel = FSP_DESIGN_WINDOW;
 panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].type = PE_TYPE_DESIGN_WINDOW;
	panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].location = ELEMENT_LOCATION_LEFT_TOP;
	panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].offset_x = 0;
	panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].offset_y = 0;//STANDARD_PANEL_TOP;
	panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].fit = ELEMENT_FIT_FILL_WITH_SCROLLBARS;
 panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].w = 400;
 panel[PANEL_DESIGN].element[FPE_DESIGN_WINDOW].h = 400;

 attach_scrollbar_to_element(PANEL_DESIGN,
																													FSP_DESIGN_WINDOW,
																													FPE_DESIGN_WINDOW,
																													FPE_DESIGN_WINDOW_SCROLLBAR_V,
																													SLIDEDIR_VERTICAL,
																													DESIGN_WINDOW_H,
																													panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].h,
																													SLIDER_DESIGN_SCROLLBAR_V,
																													&dwindow.window_pos_y);

 attach_scrollbar_to_element(PANEL_DESIGN,
																													FSP_DESIGN_WINDOW,
																													FPE_DESIGN_WINDOW,
																													FPE_DESIGN_WINDOW_SCROLLBAR_H,
																													SLIDEDIR_HORIZONTAL,
																													DESIGN_WINDOW_W,
																													panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].w,
																													SLIDER_DESIGN_SCROLLBAR_H,
																													&dwindow.window_pos_x);

 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_DATA, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 5, // int y1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].w, // int width
																					20); // int h

 panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].exists = 1;
 panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].open = 1;
 panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].panel = PANEL_DESIGN;
 panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].subpanel = FSP_DESIGN_DATA;
 panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].type = PE_TYPE_DESIGN_DATA;
	panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].location = ELEMENT_LOCATION_LEFT_TOP;
	panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].offset_x = 0;
	panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].offset_y = 0;//STANDARD_PANEL_TOP;
	panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].fit = ELEMENT_FIT_FILL;
 panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].w = 400;
 panel[PANEL_DESIGN].element[FPE_DESIGN_DATA].h = 20;

#define DESIGN_TOOLS_WINDOW_W 100
#define DESIGN_TOOLS_WINDOW_H 200

#define DESIGN_SUBTOOLS_WINDOW_W 800


 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_TOOLS_EMPTY, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 30, // int y1
																					DESIGN_TOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h
 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_TOOLS_MAIN, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 30, // int y1
																					DESIGN_TOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h
 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_SUBTOOLS, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].x2 + 5, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1, // int y1
																					DESIGN_SUBTOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h

 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_TOOLS_MEMBER, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 30, // int y1
																					DESIGN_TOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h
 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_TOOLS_DELETE, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 30, // int y1
																					DESIGN_TOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h
 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_TOOLS_AUTOCODE, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 30, // int y1
																					DESIGN_TOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h
/* init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_SUBTOOLS_MEMBER_SHAPE, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].x2 + 5, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1, // int y1
																					DESIGN_SUBTOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h*/
// starts closed:
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MEMBER].open = 0;

 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_TOOLS_CORE, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 30, // int y1
																					DESIGN_TOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h
// starts closed:
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_CORE].open = 0;

// panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_MEMBER_SHAPE].open = 0;

 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_TOOLS_EMPTY_LINK, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 30, // int y1
																					DESIGN_TOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h
 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_TOOLS_ACTIVE_LINK, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 30, // int y1
																					DESIGN_TOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h
 init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_TOOLS_AUTOCODE, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x1, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 30, // int y1
																					DESIGN_TOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h
/* init_basic_subpanel(PANEL_DESIGN, // pan
																					FSP_DESIGN_SUBTOOLS_VERTEX_OBJECT, // subpan
																					SP_TYPE_WINDOW, // subpanel type
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].x2 + 5, // x1
																					panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1, // int y1
																					DESIGN_SUBTOOLS_WINDOW_W, // int width
																					DESIGN_TOOLS_WINDOW_H); // int h*/
// starts closed:
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY_LINK].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_ACTIVE_LINK].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY].open = 1;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_DELETE].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_AUTOCODE].open = 0;
 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS].open = 0;
// panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_VERTEX_OBJECT].open = 0;


// now init the elements:
 int x, y;
 x = 5;
#define DESIGN_BUTTON_Y 1
 y = DESIGN_BUTTON_Y;
 int dbut = 0;
#define DESIGN_BUTTON_W 80
#define DESIGN_BUTTON_H 20
#define DESIGN_BUTTON_Y_GAP 0
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY,
																					FPE_DESIGN_TOOLS_EMPTY_NEW, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
// y not increased because
 dbut++;
/* init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MAIN,
																					FPE_DESIGN_TOOLS_MAIN_LOAD, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;*/
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MAIN,
																					FPE_DESIGN_TOOLS_MAIN_AUTO, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MAIN,
																					FPE_DESIGN_TOOLS_MAIN_AUTOCODE, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MAIN,
																					FPE_DESIGN_TOOLS_MAIN_SYMM, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MAIN,
																					FPE_DESIGN_TOOLS_MAIN_LOCK, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MAIN,
																					FPE_DESIGN_TOOLS_MAIN_UNLOCK, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MAIN,
																					FPE_DESIGN_TOOLS_MAIN_DELETE, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MAIN,
																					FPE_DESIGN_TOOLS_MAIN_HELP, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MAIN,
																					FPE_DESIGN_TOOLS_MAIN_HELP_MORE, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
// member
 y = DESIGN_BUTTON_Y;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MEMBER,
																					FPE_DESIGN_TOOLS_MEMBER_SHAPE, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
/* init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MEMBER,
																					FPE_DESIGN_TOOLS_MEMBER_SHAPE5, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MEMBER,
																					FPE_DESIGN_TOOLS_MEMBER_SHAPE6, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;*/
/* init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MEMBER,
																					FPE_DESIGN_TOOLS_MEMBER_FLIP, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;*/
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MEMBER,
																					FPE_DESIGN_TOOLS_MEMBER_DELETE, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_MEMBER,
																					FPE_DESIGN_TOOLS_MEMBER_EXIT, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
// core
 y = DESIGN_BUTTON_Y;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_CORE,
																					FPE_DESIGN_TOOLS_CORE_CORE_SHAPE, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_CORE,
																					FPE_DESIGN_TOOLS_CORE_EXIT, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
// empty link
 y = DESIGN_BUTTON_Y;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY_LINK,
																					FPE_DESIGN_TOOLS_ADD_LINK, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
// used link
// y = DESIGN_BUTTON_Y;
/* init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_ACTIVE_LINK,
																					FPE_DESIGN_TOOLS_NEXT_LINK, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;*/
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY_LINK,
																					FPE_DESIGN_TOOLS_VERTEX_OBJ_STD, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY_LINK,
																					FPE_DESIGN_TOOLS_VERTEX_OBJ_MOVE, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY_LINK,
																					FPE_DESIGN_TOOLS_VERTEX_OBJ_ATTACK, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY_LINK,
																					FPE_DESIGN_TOOLS_VERTEX_OBJ_DEFEND, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY_LINK,
																					FPE_DESIGN_TOOLS_VERTEX_OBJ_MISC, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY_LINK,
																					FPE_DESIGN_TOOLS_VERTEX_OBJ_CLEAR, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY_LINK,
																					FPE_DESIGN_TOOLS_VERTEX_EXIT, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
// Delete
 y = DESIGN_BUTTON_Y;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_DELETE,
																					FPE_DESIGN_TOOLS_DELETE_CONFIRM, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_DELETE,
																					FPE_DESIGN_TOOLS_DELETE_EXIT, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;
 y = DESIGN_BUTTON_Y;
 init_element_button(PANEL_DESIGN, FSP_DESIGN_TOOLS_AUTOCODE,
																					FPE_DESIGN_TOOLS_AUTOCODE_EXIT, BUTTON_STYLE_DESIGN,
																					x, y, DESIGN_BUTTON_W, DESIGN_BUTTON_H,
																					design_button_name [dbut]);
	y += DESIGN_BUTTON_H + DESIGN_BUTTON_Y_GAP;
	dbut++;

// Now the sub-buttons:
 int i;
 int sub_but;
#define DESIGN_SUB_BUTTON_W 120
#define DESIGN_SUB_BUTTON_H 20
#define DESIGN_SUB_BUTTON_Y_GAP 0
// gap not currently used

 for (i = 0; i < DESIGN_SUB_BUTTONS; i ++)
	{
		if (i < 8)
		{
   x = 5;
   y = DESIGN_BUTTON_Y + i * (DESIGN_SUB_BUTTON_H + DESIGN_SUB_BUTTON_Y_GAP);
		}
		 else
			{
    x = 145;
    y = DESIGN_BUTTON_Y + (i-8) * (DESIGN_SUB_BUTTON_H + DESIGN_SUB_BUTTON_Y_GAP);
			}
  sub_but = FPE_DESIGN_SUB_BUTTON_0 + i;
  init_element_button(PANEL_DESIGN, FSP_DESIGN_SUBTOOLS,
																					 sub_but, BUTTON_STYLE_DESIGN_SUB,
																					 x, y, DESIGN_SUB_BUTTON_W, DESIGN_SUB_BUTTON_H,
																					 NULL);
	}


/* for (i = 0; i < 8; i ++)
	{
  x = DESIGN_SUB_BUTTON_X_GAP + i * (DESIGN_SUB_BUTTON_W + DESIGN_SUB_BUTTON_X_GAP);
  y = DESIGN_SUB_BUTTON_Y_GAP;
  sub_but = FPE_DESIGN_SUB_BUTTON_0 + i;
  init_element_button(PANEL_DESIGN, FSP_DESIGN_SUBTOOLS_MEMBER_SHAPE,
																					 sub_but, BUTTON_STYLE_DESIGN_SUB,
																					 x, y, DESIGN_SUB_BUTTON_W, DESIGN_SUB_BUTTON_H,
																					 NULL);
  y = DESIGN_SUB_BUTTON_H + (DESIGN_SUB_BUTTON_Y_GAP * 2);
  sub_but = FPE_DESIGN_SUB_BUTTON_8 + i;
  init_element_button(PANEL_DESIGN, FSP_DESIGN_SUBTOOLS_MEMBER_SHAPE,
																					 sub_but, BUTTON_STYLE_DESIGN_SUB,
																					 x, y, DESIGN_SUB_BUTTON_W, DESIGN_SUB_BUTTON_H,
																					 NULL);
	}
*/
/*
// subtools
FPE_DESIGN_SUBTOOLS_MEMBER_SHAPE,
FPE_DESIGN_SUBTOOLS_MEMBER_SCROLLBAR_H,

FPE_DESIGN_SUBTOOLS_VERTEX_OBJ,
FPE_DESIGN_SUBTOOLS_VERTEX_OBJ_SCROLLBAR_H,

*/

 set_subpanel_positions(PANEL_DESIGN);

	slider_moved_to_value(&slider[SLIDER_DESIGN_SCROLLBAR_V], panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].h / 2);
	slider_moved_to_value(&slider[SLIDER_DESIGN_SCROLLBAR_H], panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].w / 2);

 init_design_window();

}






void setup_template_panel(void)
{

//	add_resize_subpanel(PANEL_TEMPLATE, FSP_TEMPLATE_PANEL_RESIZE, FPE_TEMPLATE_PANEL_RESIZE);

// init_whole_subpanel(PANEL_SYSMENU, FSP_SYSMENU_WHOLE);

/*

Template buttons needed:

New

Load
Save
Save as
 ? how to do saving when design is inconsistent with code?
  - I think that actually loading/saving will need to be done in the code editor.
  - Loading a file will probably cause the interface to be loaded into the designer.
   - and probably also fix the code into the template if possible.
    - Maybe need a separate button/menu thing for load+fix?
  - Saving when the design is inconsistent with the code will save but given a warning as well
  -
Duplicate
Clear


*/

#define TEMPL_PANEL_TAB_X 1
#define TEMPL_PANEL_TAB_Y 15
#define TEMPL_PANEL_TAB_W 70
#define TEMPL_PANEL_TAB_H 15

 init_basic_subpanel(PANEL_TEMPLATE, // pan
																					FSP_TEMPLATES_TABS, // subpan
																					SP_TYPE_TEMPLATES_TABS, // subpanel type
																					TEMPL_PANEL_TAB_X, // x1
																					TEMPL_PANEL_TAB_Y, // int y1
																					TEMPL_PANEL_TAB_X + (TEMPL_PANEL_TAB_W * PLAYERS), // int width
																					TEMPL_PANEL_TAB_H); // int h

 int i;

 for (i = 0; i < PLAYERS; i ++)
	{

	 init_element_button(PANEL_TEMPLATE,
																					 FSP_TEMPLATES_TABS,
																					 FPE_TEMPLATES_TAB_P0 + i,
																					 BUTTON_STYLE_TAB,
																					 ((TEMPL_PANEL_TAB_W + 4) * i),
																					 0,
																					 TEMPL_PANEL_TAB_W,
																					 TEMPL_PANEL_TAB_H,
																					 button_name [BUTTON_NAME_TAB_P0 + i]);
	}

#define TEMPL_PANEL_FILE_Y 35
#define TEMPL_PANEL_FILE_H 15
#define TEMPL_PANEL_FILE_W 144

 init_basic_subpanel(PANEL_TEMPLATE, // pan
																					FSP_TEMPLATES_FILE, // subpan
																					SP_TYPE_TEMPLATES_FILE, // subpanel type
																					TEMPL_PANEL_TAB_X, // x1
																					TEMPL_PANEL_FILE_Y, // int y1
																					TEMPL_PANEL_TAB_X + (TEMPL_PANEL_FILE_W * 2), // int width
																					TEMPL_PANEL_FILE_H); // int h

 for (i = 0; i < 2; i ++)
	{

	 init_element_button(PANEL_TEMPLATE,
																					 FSP_TEMPLATES_FILE,
																					 FPE_TEMPLATES_FILE_LOAD + i,
																					 BUTTON_STYLE_TAB,
																					 ((TEMPL_PANEL_FILE_W + 4) * i),
																					 0,
																					 TEMPL_PANEL_FILE_W,
																					 TEMPL_PANEL_FILE_H,
																					 button_name [BUTTON_NAME_FILE_LOAD + i]);
	}


 init_basic_subpanel(PANEL_TEMPLATE, // pan
																					FSP_TEMPLATES_MAIN, // subpan
																					SP_TYPE_TEMPLATES_MAIN, // subpanel type
																					0, // x1
																					TEMPL_PANEL_TAB_Y + TEMPL_PANEL_FILE_H + TEMPL_PANEL_TAB_H + 15, // int y1
																					panel[PANEL_TEMPLATE].w, // int width
																					panel[PANEL_TEMPLATE].h - (TEMPL_PANEL_TAB_Y + TEMPL_PANEL_TAB_H + TEMPL_PANEL_FILE_H)); // int h

#define TEMPL_PANEL_TEMPL_W 320
//#define TEMPL_PANEL_TEMPL_H 70
#define TEMPL_PANEL_TEMPL_H 50

 for (i = 0; i < TEMPLATES_PER_PLAYER; i ++)
	{

	 init_element_button(PANEL_TEMPLATE,
																					 FSP_TEMPLATES_MAIN,
																					 FPE_TEMPLATES_TEMPL_0 + (i * ELEMENTS_PER_TEMPLATE),
																					 BUTTON_STYLE_TEMPLATE,
																					 10,
																					 ((TEMPL_PANEL_TEMPL_H + 5) * i),
																					 TEMPL_PANEL_TEMPL_W,
																					 TEMPL_PANEL_TEMPL_H,
																					 NULL);
//																					 button_name [BUTTON_NAME_TEMPLATES]);
		panel[PANEL_TEMPLATE].element[FPE_TEMPLATES_TEMPL_0 + (i * ELEMENTS_PER_TEMPLATE)].value [0] = tstate.template_player_tab;
		panel[PANEL_TEMPLATE].element[FPE_TEMPLATES_TEMPL_0 + (i * ELEMENTS_PER_TEMPLATE)].value [1] = i;
	}


 set_subpanel_positions(PANEL_TEMPLATE);

}


#define EDIT_WINDOW_X 25
#define EDIT_WINDOW_Y 50
#define EDIT_LINE_H 12
#define EDIT_LINE_OFFSET 2


void setup_editor_panel(void)
{

// int i;

	add_resize_subpanel(PANEL_EDITOR, FSP_EDITOR_PANEL_RESIZE, FPE_EDITOR_PANEL_RESIZE);

	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].exists = 1;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].open = 1;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].panel = PANEL_EDITOR;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].type = SP_TYPE_WHOLE;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].x1 = PANEL_RESIZE_W;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].y1 = 1;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].w = 250;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].h = 400;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].x2 = panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].x1 + panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].w;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].y2 = panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].y1 + panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].h;
//	panel[PANEL_LOG].subpanel[subpan].first_element = FPE_LOG_WINDOW; // no elements
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].clip = 1;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].highlight = 0;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].last_highlight = 0;

// scrollbars
 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].type = PE_TYPE_SCROLLBAR_EL_V_CHAR;
 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].value [1] = SLIDER_EDITOR_SCROLLBAR_V;
 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].type = PE_TYPE_SCROLLBAR_EL_H_CHAR;
 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].value [1] = SLIDER_EDITOR_SCROLLBAR_H;

return;

/*
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].exists = 1;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].open = 1;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].panel = PANEL_EDITOR;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].type = SP_TYPE_WHOLE;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].x1 = PANEL_RESIZE_W;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].y1 = EDIT_WINDOW_Y;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].w = 250;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].h = 400;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].x2 = panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].x1 + panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].w;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].y2 = panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].y1 + panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].h;
//	panel[PANEL_LOG].subpanel[subpan].first_element = FPE_LOG_WINDOW; // no elements
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].clip = 1;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].highlight = 0;
	panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].last_highlight = 0;


 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].exists = 1;
 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].open = 1;
 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].panel = PANEL_EDITOR;
 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].subpanel = FSP_EDITOR_WINDOW;
 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].type = PE_TYPE_EDITOR_WINDOW;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].location = ELEMENT_LOCATION_LEFT_TOP;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].offset_x = 0;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].offset_y = 0;//STANDARD_PANEL_TOP;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].fit = ELEMENT_FIT_FILL_WITH_SCROLLBARS;
 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].w = 400;
 panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW].h = 400;

 attach_scrollbar_to_element(PANEL_EDITOR,
																													FSP_EDITOR_WINDOW,
																													FPE_EDITOR_WINDOW,
																													FPE_EDITOR_WINDOW_SCROLLBAR_V,
																													SLIDEDIR_VERTICAL,
																													editor.edit_window_h,
																													panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].h,
																													SLIDER_EDITOR_SCROLLBAR_V,
																													NULL); // NULL is set to correct source_edit[].window_x_pos when code opened in editor

 attach_scrollbar_to_element(PANEL_EDITOR,
																													FSP_EDITOR_WINDOW,
																													FPE_EDITOR_WINDOW,
																													FPE_EDITOR_WINDOW_SCROLLBAR_H,
																													SLIDEDIR_HORIZONTAL,
																													editor.edit_window_w,
																													panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].w,
																													SLIDER_DESIGN_SCROLLBAR_H,
																													NULL); // need to set to &source_edit[0].window_x_pos or whatever


 init_basic_subpanel(PANEL_EDITOR, // pan
																					FSP_EDITOR_TABS, // subpan
																					SP_TYPE_EDITOR_TABS, // subpanel type
																					panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].x1, // x1
																					panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].y1 - 30, // int y1
																					panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].w, // int width
																					15); // int h

	for (i = 0; i < ESOURCES; i ++)
	{

	 init_element_button(PANEL_EDITOR,
																					 FSP_EDITOR_TABS,
																					 FPE_EDITOR_TAB_0 + i,
																					 BUTTON_STYLE_TAB,
																					 10 + (SOURCE_TAB_W * i),
																					 1,
																					 SOURCE_TAB_W,
																					 SOURCE_TAB_H,
																					 button_name [BUTTON_NAME_UNKNOWN]);
	}

#define SUBMENU_BAR_W 65
#define SUBMENU_BAR_H 15

#define SUBMENU_W 100
#define SUBMENU_H 200
#define SUBMENU_LINE_H 15

 init_basic_subpanel(PANEL_EDITOR, // pan
																					FSP_EDITOR_SUBMENUS, // subpan
																					SP_TYPE_EDITOR_SUBMENU, // subpanel type
																					panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].x1, // x1
																					panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].y1 - 15, // int y1
																					panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].w, // int width
																					SUBMENU_BAR_H); // int h

	for (i = 0; i < 4; i ++)
	{
	 init_element_button(PANEL_EDITOR,
																					 FSP_EDITOR_SUBMENUS,
																					 FPE_EDITOR_SMB_FILE + i,
																					 BUTTON_STYLE_SUBMENU,
																					 10 + (SUBMENU_BAR_W * i),
																					 1,
																					 SUBMENU_BAR_W,
																					 SUBMENU_BAR_H,
																					 button_name [BUTTON_NAME_FILE + i]);
  init_basic_subpanel(PANEL_EDITOR, // pan
																				 	FSP_EDITOR_SUBMENU_FILE + i, // subpan
																				 	SP_TYPE_EDITOR_SUBMENU, // subpanel type
																			  		10 + (SUBMENU_BAR_W * i), // x1
																			 		35, // int y1
																		 			SUBMENU_W, // int width
																		 			SUBMENU_H); // int h
  panel[PANEL_EDITOR].subpanel[FSP_EDITOR_SUBMENU_FILE + i].open = 0;
	}

 int bname = BUTTON_NAME_OPEN;
 int x = 2;//x = panel[PANEL_EDITOR].subpanel[FSP_EDITOR_SUBMENU_FILE].x1 + 2;
 int y = 0;//y = panel[PANEL_EDITOR].subpanel[FSP_EDITOR_SUBMENU_FILE].y1;

 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_FILE,
																					FPE_EDITOR_FILE_NEW,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 y += SUBMENU_LINE_H;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_FILE,
																					FPE_EDITOR_FILE_OPEN,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 y += SUBMENU_LINE_H;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_FILE,
																					FPE_EDITOR_FILE_SAVE,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 y += SUBMENU_LINE_H;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_FILE,
																					FPE_EDITOR_FILE_SAVE_AS,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 y += SUBMENU_LINE_H;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_FILE,
																					FPE_EDITOR_FILE_CLOSE,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);

 x = 2;//panel[PANEL_EDITOR].subpanel[FSP_EDITOR_SUBMENU_EDIT].x1 + 2;
 y = 0;//panel[PANEL_EDITOR].subpanel[FSP_EDITOR_SUBMENU_EDIT].y1;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_EDIT,
																					FPE_EDITOR_EDIT_UNDO,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 y += SUBMENU_LINE_H;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_EDIT,
																					FPE_EDITOR_EDIT_REDO,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 y += SUBMENU_LINE_H;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_EDIT,
																					FPE_EDITOR_EDIT_CUT,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 y += SUBMENU_LINE_H;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_EDIT,
																					FPE_EDITOR_EDIT_COPY,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 y += SUBMENU_LINE_H;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_EDIT,
																					FPE_EDITOR_EDIT_PASTE,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 y += SUBMENU_LINE_H;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_EDIT,
																					FPE_EDITOR_EDIT_CLEAR,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);

 x = 2;//panel[PANEL_EDITOR].subpanel[FSP_EDITOR_SUBMENU_SEARCH].x1 + 2;
 y = 0;//panel[PANEL_EDITOR].subpanel[FSP_EDITOR_SUBMENU_SEARCH].y1;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_SEARCH,
																					FPE_EDITOR_SEARCH_FIND,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 y += SUBMENU_LINE_H;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_SEARCH,
																					FPE_EDITOR_SEARCH_NEXT,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
 x = 2;//panel[PANEL_EDITOR].subpanel[FSP_EDITOR_SUBMENU_SEARCH].x1 + 2;
 y = 0;//panel[PANEL_EDITOR].subpanel[FSP_EDITOR_SUBMENU_SEARCH].y1;
 init_element_button(PANEL_EDITOR, FSP_EDITOR_SUBMENU_BUILD,
																					FPE_EDITOR_BUILD_TEST,
																					BUTTON_STYLE_SUBMENU_LINE,
																					x, y, SUBMENU_W, SUBMENU_LINE_H,
																					button_name [bname++]);
// y += SUBMENU_LINE_H;



 set_subpanel_positions(PANEL_EDITOR);

//	slider_moved_to_value(&slider[SLIDER_DESIGN_SCROLLBAR_V], panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].h / 2);
//	slider_moved_to_value(&slider[SLIDER_DESIGN_SCROLLBAR_H], panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].w / 2);

// init_editor_window();
*/
}





void attach_scrollbar_to_element(int pan,
																																	int subpan,
																																	int main_el,
																																	int slider_el,
																																	int dir,
																																	int value_max,
																																	int represents_size,
																																	int slider_index,
																																	int* ptr_value)
{


 panel[pan].element[slider_el].exists = 1;
 panel[pan].element[slider_el].open = 1;
 panel[pan].element[slider_el].panel = pan;
 panel[pan].element[slider_el].subpanel = subpan;

	panel[pan].element[slider_el].highlight = 0;
	panel[pan].element[slider_el].last_highlight = 0;
	panel[pan].element[slider_el].value [0] = main_el; // element affected by this scrollbar
	panel[pan].element[slider_el].value [1] = slider_index; // element affected by this scrollbar
	panel[pan].element[slider_el].ptr_value = ptr_value; // element affected by this scrollbar

 if (dir == SLIDEDIR_VERTICAL)
	{
  panel[pan].element[slider_el].type = PE_TYPE_SCROLLBAR_EL_V_PIXEL;
	 panel[pan].element[slider_el].location = ELEMENT_LOCATION_RIGHT_TOP;
	 panel[pan].element[slider_el].offset_x = -SLIDER_BUTTON_SIZE;
 	panel[pan].element[slider_el].offset_y = 0;
 	panel[pan].element[slider_el].fit = ELEMENT_FIT_SUBPANEL_H_WITH_SCROLLBAR;
  panel[pan].element[slider_el].w = SLIDER_BUTTON_SIZE;
  panel[pan].element[slider_el].h = panel[pan].subpanel[subpan].h;


 init_slider(panel[pan].element[slider_el].value [1],
													pan, // pan
													subpan, // subpan
													slider_el, // element
													ptr_value, // value_pointer
													dir, // dir
													0, // value_min
													value_max, // value_max
													panel[pan].element[main_el].h, // total_length
													32, // button_increment
													represents_size, // track_increment // panel[pan].element[FPE_DESIGN_WINDOW].h
													represents_size, // slider_represents_size
													SLIDER_BUTTON_SIZE, // thickness
													COL_BLUE, // colour
													0); // hidden if unused
	}
	 else
		{
   panel[pan].element[slider_el].type = PE_TYPE_SCROLLBAR_EL_H_PIXEL;
	  panel[pan].element[slider_el].location = ELEMENT_LOCATION_LEFT_BOTTOM;
	  panel[pan].element[slider_el].offset_x = 0;
 	 panel[pan].element[slider_el].offset_y = -SLIDER_BUTTON_SIZE;
 	 panel[pan].element[slider_el].fit = ELEMENT_FIT_SUBPANEL_W_WITH_SCROLLBAR;
   panel[pan].element[slider_el].w = panel[pan].subpanel[subpan].w;
   panel[pan].element[slider_el].h = SLIDER_BUTTON_SIZE;

 init_slider(panel[pan].element[slider_el].value [1],
													pan, // pan
													subpan, // subpan
													slider_el, // element
													ptr_value, // value_pointer
													dir, // dir
													0, // value_min
													value_max, // value_max
													panel[pan].element[main_el].w, // total_length
													32, // button_increment
													represents_size, // track_increment // panel[pan].element[FPE_DESIGN_WINDOW].h
													represents_size, // slider_represents_size
													SLIDER_BUTTON_SIZE, // thickness
													COL_BLUE, // colour
													0); // hidden if unused
		}

}



void init_element_button(int pan, int subpan, int el,
																									int style,
																									int x, int y,
																									int width, int h,
																									char* name)
{

	panel[pan].element[el].exists = 1;
	panel[pan].element[el].open = 1;
	panel[pan].element[el].panel = pan;
	panel[pan].element[el].subpanel = subpan;
	panel[pan].element[el].type = PE_TYPE_BUTTON;
	panel[pan].element[el].style = style;
	panel[pan].element[el].location = ELEMENT_LOCATION_LEFT_TOP;
	panel[pan].element[el].offset_x = x;
	panel[pan].element[el].offset_y = y;
	panel[pan].element[el].fit = ELEMENT_FIT_FIXED;
	panel[pan].element[el].w = width;
	panel[pan].element[el].h = h;

	panel[pan].element[el].highlight = 0;
	panel[pan].element[el].last_highlight = 0;

	panel[pan].element[el].name = name;



}


void init_basic_subpanel(int pan, int subpan, int type, int x1, int y1, int width, int h)
{

	panel[pan].subpanel[subpan].exists = 1;
	panel[pan].subpanel[subpan].open = 1;
	panel[pan].subpanel[subpan].panel = pan;
	panel[pan].subpanel[subpan].type = type;
	panel[pan].subpanel[subpan].x1 = x1;
	panel[pan].subpanel[subpan].y1 = y1;
	panel[pan].subpanel[subpan].w = width;
	panel[pan].subpanel[subpan].h = h;
	panel[pan].subpanel[subpan].x2 = x1 + width;
	panel[pan].subpanel[subpan].y2 = y1 + h;
	panel[pan].subpanel[subpan].clip = 1;
	panel[pan].subpanel[subpan].highlight = 0;
	panel[pan].subpanel[subpan].last_highlight = 0;

}

// This initialises a subpanel that takes up a whole panel, like a couple of them do.
void init_whole_subpanel(int pan, int subpan, int panel_resizable)
{

	panel[pan].subpanel[subpan].exists = 1;
	panel[pan].subpanel[subpan].open = 1;
	panel[pan].subpanel[subpan].panel = pan;
	panel[pan].subpanel[subpan].type = SP_TYPE_WHOLE;
	panel[pan].subpanel[subpan].x1 = 0;
	if (panel_resizable)
	 panel[pan].subpanel[subpan].x1 += PANEL_RESIZE_W;
	panel[pan].subpanel[subpan].y1 = 1;
	panel[pan].subpanel[subpan].x2 = panel[pan].w;
	panel[pan].subpanel[subpan].y2 = panel[pan].h;
	panel[pan].subpanel[subpan].w = panel[pan].w - panel[pan].subpanel[subpan].x1;
	panel[pan].subpanel[subpan].h = panel[pan].h - panel[pan].subpanel[subpan].y1;
//	panel[PANEL_LOG].subpanel[subpan].first_element = FPE_LOG_WINDOW; // no elements
	panel[pan].subpanel[subpan].clip = 1;
	panel[pan].subpanel[subpan].highlight = 0;
	panel[pan].subpanel[subpan].last_highlight = 0;

}


// Sets positions of subpanels and elements of the log panel
// Can be called at init and also when panel resized (but not needed when panel is just closed)
void set_subpanel_positions(int pan)
{

 switch(pan)
 {
	case PANEL_LOG:
	 panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].x2 = panel[PANEL_LOG].w - panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].x1;
//	 panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].y2 = LOG_WINDOW_H;
	 panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].w = panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].x2 - panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].x1;
//	 panel[PANEL_LOG].subpanel[FSP_LOG_WHOLE].h = LOG_WINDOW_H;
  set_element_positions(PANEL_LOG, FSP_LOG_WHOLE);
	 break;
	case PANEL_SYSMENU:
	 panel[PANEL_SYSMENU].subpanel[FSP_SYSMENU_WHOLE].x2 = panel[PANEL_SYSMENU].w - PANEL_RESIZE_W;
	 panel[PANEL_SYSMENU].subpanel[FSP_SYSMENU_WHOLE].w = panel[PANEL_SYSMENU].subpanel[FSP_SYSMENU_WHOLE].x2;
	 panel[PANEL_SYSMENU].subpanel[FSP_SYSMENU_PANEL_RESIZE].y2 = panel[PANEL_SYSMENU].h;
  set_element_positions(PANEL_SYSMENU, FSP_SYSMENU_WHOLE);
  set_element_positions(PANEL_SYSMENU, FSP_SYSMENU_PANEL_RESIZE);
	 break;
	case PANEL_DESIGN:
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x2 = panel[PANEL_DESIGN].w;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].w = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].x2 - PANEL_RESIZE_W;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_PANEL_RESIZE].y2 = panel[PANEL_DESIGN].h;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_DATA].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_WINDOW].y2 + 5;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_DATA].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_DATA].y1 + 20;
// Empty
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_DATA].y2 + 5;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1 + DESIGN_TOOLS_WINDOW_H;
// main tools
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_DATA].y2 + 5;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1 + DESIGN_TOOLS_WINDOW_H;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y2;
// member tools
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MEMBER].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MEMBER].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y2;
//	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_MEMBER_SHAPE].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_MAIN].y1;
//	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_MEMBER_SHAPE].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_MAIN].y2;
// member tools
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY_LINK].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_EMPTY_LINK].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y2;

	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_ACTIVE_LINK].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_ACTIVE_LINK].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y2;
//	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_VERTEX_OBJECT].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_MAIN].y1;
//	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_VERTEX_OBJECT].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_SUBTOOLS_MAIN].y2;
// delete confirm
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_DELETE].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_DELETE].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y2;

	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_AUTOCODE].y1 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y1;
	 panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_AUTOCODE].y2 = panel[PANEL_DESIGN].subpanel[FSP_DESIGN_TOOLS_MAIN].y2;

// elements
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_WINDOW);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_PANEL_RESIZE);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_DATA);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_TOOLS_MAIN);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_SUBTOOLS);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_TOOLS_MEMBER);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_TOOLS_CORE);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_TOOLS_EMPTY_LINK);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_TOOLS_ACTIVE_LINK);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_TOOLS_DELETE);
  set_element_positions(PANEL_DESIGN, FSP_DESIGN_TOOLS_AUTOCODE);
//  set_element_positions(PANEL_DESIGN, FSP_DESIGN_SUBTOOLS_VERTEX_OBJECT);
		break;
	case PANEL_TEMPLATE:
//		panel[PANEL_TEMPLATE].subpanel[FSP_TEMPLATES_TABS].
  set_element_positions(PANEL_TEMPLATE, FSP_TEMPLATES_TABS);
  set_element_positions(PANEL_TEMPLATE, FSP_TEMPLATES_FILE);
  set_element_positions(PANEL_TEMPLATE, FSP_TEMPLATES_MAIN);
  break;
 case PANEL_EDITOR:
		panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].open = 1;
		panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].x2 = panel[PANEL_EDITOR].x2 - 5;
		panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].y2 = panel[PANEL_EDITOR].y2 - 5;
		panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WHOLE].w = panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].x2 - panel[PANEL_EDITOR].subpanel[FSP_EDITOR_WINDOW].x1;
  set_element_positions(PANEL_EDITOR, FSP_EDITOR_PANEL_RESIZE);
  panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].open = 1;
  panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].open = 1;

//		panel[PANEL_EDITOR].subpanel[FSP_EDITOR_TABS
//  set_element_positions(PANEL_EDITOR, FSP_EDITOR_WHOLE);
//  set_element_positions(PANEL_EDITOR, FSP_EDITOR_PANEL_RESIZE);
/*  set_element_positions(PANEL_EDITOR, FSP_EDITOR_WINDOW);
  set_element_positions(PANEL_EDITOR, FSP_EDITOR_PANEL_RESIZE);
  set_element_positions(PANEL_EDITOR, FSP_EDITOR_SUBMENUS);
  set_element_positions(PANEL_EDITOR, FSP_EDITOR_SUBMENU_BUILD);
  set_element_positions(PANEL_EDITOR, FSP_EDITOR_SUBMENU_EDIT);
  set_element_positions(PANEL_EDITOR, FSP_EDITOR_SUBMENU_FILE);
  set_element_positions(PANEL_EDITOR, FSP_EDITOR_SUBMENU_SEARCH);
  set_element_positions(PANEL_EDITOR, FSP_EDITOR_TABS);*/
		break;

 }

}

void set_element_positions(int pan, int subpan)
{

	int el;

	for (el = 0; el < ELEMENTS; el ++)
	{
		if (panel[pan].element[el].exists
			&&	panel[pan].element[el].subpanel == subpan)
		{
//				 	fpr("\n resize pan %i subpan %i el %i location %i fit %i", pan, subpan, el, panel[pan].element[el].location, panel[pan].element[el].fit);

			switch(panel[pan].element[el].location)
			{
			 case ELEMENT_LOCATION_LEFT_TOP:
			 	panel[pan].element[el].x1 = panel[pan].element[el].offset_x;
			 	panel[pan].element[el].y1 = panel[pan].element[el].offset_y;
			 	break;
			 case ELEMENT_LOCATION_LEFT_BOTTOM:
			 	panel[pan].element[el].x1 = panel[pan].element[el].offset_x;
			 	panel[pan].element[el].y1 = panel[pan].subpanel[subpan].h + panel[pan].element[el].offset_y;
			 	break;
			 case ELEMENT_LOCATION_RIGHT_TOP:
			 	panel[pan].element[el].x1 = panel[pan].subpanel[subpan].w + panel[pan].element[el].offset_x;
			 	panel[pan].element[el].y1 = panel[pan].element[el].offset_y;
			 	break;
			 case ELEMENT_LOCATION_RIGHT_BOTTOM:
			 	panel[pan].element[el].x1 = panel[pan].subpanel[subpan].w + panel[pan].element[el].offset_x;
			 	panel[pan].element[el].y1 = panel[pan].subpanel[subpan].h + panel[pan].element[el].offset_y;
			 	break;
			}
			switch(panel[pan].element[el].fit)
			{
			 case ELEMENT_FIT_FIXED:
			 	panel[pan].element[el].x2 = panel[pan].element[el].x1 + panel[pan].element[el].w;
			 	panel[pan].element[el].y2 = panel[pan].element[el].y1 + panel[pan].element[el].h;
			 	break;
 			case ELEMENT_FIT_FILL:
			 	panel[pan].element[el].x2 = panel[pan].subpanel[subpan].x2;
			 	panel[pan].element[el].y2 = panel[pan].subpanel[subpan].h;
			 	break;
 			case ELEMENT_FIT_SUBPANEL_W:
			 	panel[pan].element[el].x2 = panel[pan].subpanel[subpan].x2;
			 	panel[pan].element[el].y2 = panel[pan].element[el].y1 + panel[pan].element[el].h;
			 	break;
 			case ELEMENT_FIT_SUBPANEL_H:
			 	panel[pan].element[el].x2 = panel[pan].element[el].x1 + panel[pan].element[el].w;
			 	panel[pan].element[el].y2 = panel[pan].subpanel[subpan].h;
			 	break;
 			case ELEMENT_FIT_SUBPANEL_W_WITH_SCROLLBAR:
			 	panel[pan].element[el].x2 = panel[pan].subpanel[subpan].x2 - SLIDER_BUTTON_SIZE;
			 	panel[pan].element[el].y2 = panel[pan].element[el].y1 + panel[pan].element[el].h;
			 	break;
 			case ELEMENT_FIT_SUBPANEL_H_WITH_SCROLLBAR:
			 	panel[pan].element[el].x2 = panel[pan].element[el].x1 + panel[pan].element[el].w;
			 	panel[pan].element[el].y2 = panel[pan].subpanel[subpan].h - SLIDER_BUTTON_SIZE;
			 	break;
			 case ELEMENT_FIT_FILL_WITH_V_SCROLLBAR: // fills whole subpanel but leaves room for vertical scrollbar
			 	panel[pan].element[el].x2 = panel[pan].subpanel[subpan].x2 - SLIDER_BUTTON_SIZE;
			 	panel[pan].element[el].y2 = panel[pan].subpanel[subpan].h;
			 	break;
			 case ELEMENT_FIT_FILL_WITH_H_SCROLLBAR: // fills whole subpanel but leaves room for horizontal scrollbar
			 	panel[pan].element[el].x2 = panel[pan].subpanel[subpan].x2;
			 	panel[pan].element[el].y2 = panel[pan].subpanel[subpan].h - SLIDER_BUTTON_SIZE;
			 	break;
			 case ELEMENT_FIT_FILL_WITH_SCROLLBARS: // fills whole subpanel but leaves room for h + v scrollbars
			 	panel[pan].element[el].x2 = panel[pan].subpanel[subpan].x2 - SLIDER_BUTTON_SIZE;
			 	panel[pan].element[el].y2 = panel[pan].subpanel[subpan].h - SLIDER_BUTTON_SIZE;
			 	break;

			}
// may need to work out w/h (although in some cases it will not have been changed by this function)
			panel[pan].element[el].w = panel[pan].element[el].x2 - panel[pan].element[el].x1;
			panel[pan].element[el].h = panel[pan].element[el].y2 - panel[pan].element[el].y1;
			switch(panel[pan].element[el].type)
			{
			 case PE_TYPE_SCROLLBAR_EL_H_PIXEL:
// NOTE: currently this relies on DESIGN_WINDOW_W
			 	slider[panel[pan].element[el].value [1]].value_max = DESIGN_WINDOW_W - panel[pan].element[panel[pan].element[el].value [0]].w;
				 reset_slider_length(panel[pan].element[el].value [1],
																									panel[pan].element[panel[pan].element[el].value [0]].w,
																									panel[pan].element[panel[pan].element[el].value [0]].w);
				 break;
			 case PE_TYPE_SCROLLBAR_EL_V_PIXEL:
			 	slider[panel[pan].element[el].value [1]].value_max = DESIGN_WINDOW_H - panel[pan].element[panel[pan].element[el].value [0]].h;
				 reset_slider_length(panel[pan].element[el].value [1],
																									panel[pan].element[panel[pan].element[el].value [0]].h,
																									panel[pan].element[panel[pan].element[el].value [0]].h);
				 break;
			}
//		 fpr(" at %i,%i %i,%i (wh %i,%i)", panel[pan].element[el].x1,panel[pan].element[el].y1,panel[pan].element[el].x2,panel[pan].element[el].y2, panel[pan].element[el].w, panel[pan].element[el].h);
		}
	}

}


// call this at start of a game
// currently this just resets highlight timestamps for button elements
void init_panels_for_new_game(void)
{

	int pan, el;

	for (pan = 0; pan < PANELS; pan ++)
	{
			for (el = 0; el < ELEMENTS; el ++)
			{
				panel[pan].element[el].last_highlight = 0;
				panel[pan].element[el].highlight = 0;
			}
	}

}


/*

How to handle panels, subpanels and elements:

Each panel has an array of subpanels. This should be fine because there aren't many of them.

Elements?
 - probably give each panel an array of elements? Not sure about this...
  - just do this for now. Can change later.
	- each subpanel has index of first element belonging to it. This element should be followed by
			a contiguous list of all elements belonging to the subpanel.

- How to move editor.x_split to


Also: overwindow.
 - probably have a separate array of elements for the overwindow.
  - although - the current approach to panels may mean that the overwindow can be a panel itself.
   - actually I think this would work.


How are panels drawn?


Checked one by one
if closed, ignore

Each element adds polygons/text
Can also have a stop instruction as an element or something
 - forces all polygons to be drawn, then text.
 - use if polygons need to overlap text.
Should be able to assume that a later element always overwrites an earlier element.

message log is always drawn last
 - think about:


Mode buttons?
 - mode button for open panel sits in the top left of the panel as a >> right (close) button
 - mode buttons for leftmost closed panels sit to the left of the leftmost open panel (or right edge of screen)
 - mode buttons for other closed panels sit... hm. Tricky. I think they'll go on the right side of the next left panel?
   (unless this confuses people who think that's where close buttons go)

Tabs?
 - tabs might be generalisable - could have a special tab structure specific to a panel.
  - this would allow the whole of the panel below the tab to be scrollable without needing to be its own subpanel


*/


/*

new data for input struct:

enum
{
MOUSE_STATUS_OUTSIDE, // unavailable to the game or panels
MOUSE_STATUS_GAME, // in the game window and available to the game. Not dragging anything.
MOUSE_STATUS_PANEL, // on a panel
MOUSE_STATUS_DRAG, // is dragging something
//MOUSE_STATUS_DRAG_SHIFT, // is dragging something and shift is being pressed (not sure if needed)
};


enum
{
MOUSE_DRAG_SLIDER, // mouse is dragging slider handle
MOUSE_DRAG_TEXT, // mouse is dragging selected text from editor
MOUSE_DRAG_PANEL_RESIZE, // mouse is dragging panel edge
};

 int mouse_status;

 int mouse_drag_type; // MOUSE_DRAG_* enum
 int mouse_drag_panel; // source of drag - panel
 int mouse_drag_subpanel; // source of drag - subpanel
 int mouse_drag_element; // source of drag - element


*/


