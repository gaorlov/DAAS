/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2007. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  ADDPANEL                        1       /* callback function: util_HidePanelCallback */
#define  ADDPANEL_STOP                   2       /* callback function: AddDevCallback */
#define  ADDPANEL_DATA                   3       /* callback function: AddDevCallback */
#define  ADDPANEL_PARITY                 4       /* callback function: AddDevCallback */
#define  ADDPANEL_BAUD                   5       /* callback function: AddDevCallback */
#define  ADDPANEL_COMPORT                6       /* callback function: AddDevCallback */
#define  ADDPANEL_INSTRUMENT             7       /* callback function: AddDevCallback */
#define  ADDPANEL_LABEL                  8
#define  ADDPANEL_ADDINSTR               9       /* callback function: AddDevCallback */
#define  ADDPANEL_OUTQ                   10      /* callback function: AddDevCallback */
#define  ADDPANEL_INQ                    11      /* callback function: AddDevCallback */

#define  GEN_INST                        2       /* callback function: util_HidePanelCallback */
#define  GEN_INST_READBOX                2
#define  GEN_INST_SENDBOX                3
#define  GEN_INST_CLEAR                  4       /* callback function: GenInstControlCallback */
#define  GEN_INST_READ                   5       /* callback function: GenInstControlCallback */
#define  GEN_INST_POLL                   6       /* callback function: GenInstControlCallback */
#define  GEN_INST_SEND                   7       /* callback function: GenInstControlCallback */
#define  GEN_INST_TERM                   8

#define  SETUP                           3       /* callback function: util_HidePanelCallback */
#define  SETUP_ADDCOMDEV                 2       /* callback function: rs232SetupControlCallback */
#define  SETUP_SAVESETUP                 3       /* callback function: rs232SetupControlCallback */
#define  SETUP_LOADSETUP                 4       /* callback function: rs232SetupControlCallback */
#define  SETUP_LIST                      5       /* callback function: rs232SetupControlCallback */
#define  SETUP_DECORATION                6
#define  SETUP_TEXTMSG                   7
#define  SETUP_TEXTMSG_2                 8
#define  SETUP_TEXTMSG_3                 9


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK AddDevCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GenInstControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK rs232SetupControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
