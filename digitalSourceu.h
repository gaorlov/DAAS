/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2008. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  BITPANEL                         1       /* callback function: boards_DigitalSourcePanelCallback */
#define  BITPANEL_LED                     2
#define  BITPANEL_BIT                     3       /* callback function: boards_DigitalSourceControlCallback */

#define  DIGITALSRC                       2       /* callback function: boards_DigitalSourcePanelCallback */
#define  DIGITALSRC_RETPORT               2       /* callback function: boards_DigitalSourceControlCallback */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK boards_DigitalSourceControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK boards_DigitalSourcePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
