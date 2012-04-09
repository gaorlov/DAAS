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

#define  ADDDEV                          1
#define  ADDDEV_ADDRS                    2
#define  ADDDEV_CLOSE                    3       /* callback function: util_DiscardCallback */
#define  ADDDEV_SCAN                     4       /* callback function: AddDeviceControlCallback */
#define  ADDDEV_CONNECT                  5       /* callback function: AddDeviceControlCallback */
#define  ADDDEV_TEXT_1                   6
#define  ADDDEV_LIST                     7       /* callback function: AddDeviceControlCallback */
#define  ADDDEV_LABEL                    8

#define  DEV                             2
#define  DEV_LABEL                       2       /* callback function: DevControlCallback */
#define  DEV_STATUS                      3
#define  DEV_PADDR                       4
#define  DEV_SADDR                       5
#define  DEV_COMMAND                     6       /* callback function: DevControlCallback */
#define  DEV_LOGIO                       7       /* callback function: DevControlCallback */
#define  DEV_DISABLE                     8       /* callback function: DevControlCallback */
#define  DEV_CLEAR                       9       /* callback function: DevControlCallback */
#define  DEV_CLOSE                       10      /* callback function: DevControlCallback */
#define  DEV_RESPONSES                   11
#define  DEV_COMMANDS                    12
#define  DEV_TEXT1                       13
#define  DEV_DECORATION                  14

#define  IFC                             3
#define  IFC_TIMEOUT                     2       /* callback function: ifcControlCallback */
#define  IFC_STATUS                      3
#define  IFC_DISABLE                     4       /* callback function: ifcControlCallback */
#define  IFC_LOGIO                       5       /* callback function: ifcControlCallback */
#define  IFC_PADDR                       6
#define  IFC_SADDR                       7
#define  IFC_ENABLESADDR                 8       /* callback function: ifcControlCallback */
#define  IFC_COMMAND                     9       /* callback function: ifcControlCallback */
#define  IFC_CLOSE                       10      /* callback function: ifcControlCallback */
#define  IFC_RESPONSES                   11
#define  IFC_COMMANDS                    12
#define  IFC_TEXT1                       13
#define  IFC_DECORATION                  14

#define  SETUP                           4       /* callback function: SetupPanelCallback */
#define  SETUP_ADD                       2       /* callback function: AddDeviceCallback */
#define  SETUP_SAVE                      3       /* callback function: SaveDeviceSetupCallback */
#define  SETUP_LOAD                      4       /* callback function: LoadDeviceSetupCallback */
#define  SETUP_LIST                      5       /* callback function: DevListCallback */
#define  SETUP_TEXT                      6
#define  SETUP_TEXT_4                    7
#define  SETUP_TEXT_5                    8
#define  SETUP_TEXT_3                    9
#define  SETUP_TEXT_2                    10
#define  SETUP_TEXT4                     11
#define  SETUP_DECORATION                12


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK AddDeviceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AddDeviceControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DevControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DevListCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ifcControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LoadDeviceSetupCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SaveDeviceSetupCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SetupPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
