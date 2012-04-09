/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 1995. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/


     /* Panels and Controls: */

#define  CHANEDIT                        1
#define  CHANEDIT_LABEL                  2       /* callback function: ChannelLabelCallback */
#define  CHANEDIT_NOTE                   3       /* callback function: util_NoteCallback */

#define  CHANNELS                        2       /* callback function: util_HidePanelCallback */
#define  CHANNELS_LIST                   2       /* callback function: ChannelSelectionCallback */
#define  CHANNELS_INDEX                  3       /* callback function: ChannelIndexCallback */
#define  CHANNELS_TOTAL                  4
#define  CHANNELS_LOCATION               5
#define  CHANNELS_SAVE                   6       /* callback function: SaveChannelsCallback */
#define  CHANNELS_LOAD                   7       /* callback function: LoadChannelsCallback */
#define  CHANNELS_REMOVE                 8       /* callback function: RemoveChannelsCallback */

#define  CHANVIEW                        3
#define  CHANVIEW_POINTS_2               2       /* callback function: ChannelViewWindowCallback */
#define  CHANVIEW_POINTS                 3
#define  CHANVIEW_GRAPH                  4
#define  CHANVIEW_CHANNELS               5       /* callback function: ChannelViewSelectCallback */
#define  CHANVIEW_OFFSET                 6       /* callback function: ChannelViewWindowCallback */
#define  CHANVIEW_SCATTER                7       /* callback function: ChannelViewScatterCallback */
#define  CHANVIEW_GRID                   8       /* callback function: ChannelViewGraphCallback */
#define  CHANVIEW_LOG                    9       /* callback function: ChannelViewGraphCallback */
#define  CHANVIEW_DONE                   10      /* callback function: ChannelViewDoneCallback */
#define  CHANVIEW_OFFSETVAL              11


     /* Menu Bars, Menus, and Menu Items: */

#define  CHANMENUS                       1
#define  CHANMENUS_GEN                   2       /* callback function: LoadDataFileCallback */
#define  CHANMENUS_GEN_LOADDATA          3       /* callback function: LoadDataFileCallback */
#define  CHANMENUS_GEN_SEP_1             4
#define  CHANMENUS_GEN_CONSTANT          5
#define  CHANMENUS_GEN_IMPULSE           6
#define  CHANMENUS_GEN_PULSE             7
#define  CHANMENUS_GEN_RAMP              8
#define  CHANMENUS_GEN_TRIANGLE          9
#define  CHANMENUS_GEN_SINEPAT           10
#define  CHANMENUS_GEN_WAVES             11
#define  CHANMENUS_GEN_WAVES_SUBMENU     12
#define  CHANMENUS_GEN_WAVES_CHIRP       13
#define  CHANMENUS_GEN_WAVES_SAWTOOTH    14
#define  CHANMENUS_GEN_WAVES_SINE        15
#define  CHANMENUS_GEN_WAVES_SINC        16
#define  CHANMENUS_GEN_WAVES_SQUARE      17
#define  CHANMENUS_GEN_WAVES_TRIANGLE    18
#define  CHANMENUS_GEN_NOISE             19
#define  CHANMENUS_GEN_NOISE_SUBMENU     20
#define  CHANMENUS_GEN_NOISE_UNIFORM     21
#define  CHANMENUS_GEN_NOISE_WHITE       22
#define  CHANMENUS_GEN_NOISE_GAUSS       23
#define  CHANMENUS_FUNC                  24
#define  CHANMENUS_FUNC_MATH             25
#define  CHANMENUS_FUNC_MATH_SUBMENU     26
#define  CHANMENUS_FUNC_MATH_NEG         27
#define  CHANMENUS_FUNC_MATH_INVERSE     28
#define  CHANMENUS_FUNC_MATH_ABSOLUTE    29
#define  CHANMENUS_FUNC_MATH_LN          30
#define  CHANMENUS_FUNC_MATH_LOG         31
#define  CHANMENUS_FUNC_MATH_EXPONENT    32
#define  CHANMENUS_FUNC_SCALE            33
#define  CHANMENUS_FUNC_SCALE_SUBMENU    34
#define  CHANMENUS_FUNC_SCALE_NORMAL     35
#define  CHANMENUS_FUNC_SCALE_QUICK      36
#define  CHANMENUS_FUNC_MANIP            37
#define  CHANMENUS_FUNC_LINEVAL          38
#define  CHANMENUS_FUNC_STATISTICS       39
#define  CHANMENUS_FUNC_MISC             40
#define  CHANMENUS_FUNC_MISC_SUBMENU     41
#define  CHANMENUS_FUNC_MISC_COPY        42
#define  CHANMENUS_FUNC_MISC_SUBSET      43
#define  CHANMENUS_FUNC_MISC_SORT        44
#define  CHANMENUS_FUNC_MISC_REVERSE     45
#define  CHANMENUS_FUNC_MISC_CLIP        46
#define  CHANMENUS_FUNC_MISC_DECIMATE    47
#define  CHANMENUS_OP                    48
#define  CHANMENUS_OP_ADD                49
#define  CHANMENUS_OP_SUBTRACT           50
#define  CHANMENUS_OP_MULTIPLY           51
#define  CHANMENUS_OP_DIVISION           52


     /* Callback Prototypes: */

int  ChannelIndexCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ChannelLabelCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ChannelSelectionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ChannelViewDoneCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ChannelViewGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ChannelViewScatterCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ChannelViewSelectCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ChannelViewWindowCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  LoadChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void LoadDataFileCallback(int menubar, int menuItem, void *callbackData, int panel);
int  RemoveChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  SaveChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  util_NoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

