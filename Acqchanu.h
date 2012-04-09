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

#define  ACQDATA                         1       /* callback function: AcqDataPanelCallback */
#define  ACQDATA_READING                 2
#define  ACQDATA_COEFF                   3       /* callback function: AcqDataCallback */
#define  ACQDATA_CONV                    4       /* callback function: AcqDataCallback */
#define  ACQDATA_MEAS                    5
#define  ACQDATA_NOTE                    6       /* callback function: AcqDataNoteCallback */

#define  ACQLIST                         2       /* callback function: util_HidePanelCallback */
#define  ACQLIST_CHANNELS                2       /* callback function: SelectAcqChanCallback */
#define  ACQLIST_REMOVE                  3       /* callback function: RemoveAcqChanCallback */
#define  ACQLIST_TIME                    4       /* callback function: TimeAcqChannelsCallback */
#define  ACQLIST_INDEX                   5       /* callback function: IndexAcqChannelsCallback */
#define  ACQLIST_DOWN                    6       /* callback function: MoveDownAcqChanCallback */
#define  ACQLIST_UP                      7       /* callback function: MoveUpAcqChanCallback */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK AcqDataCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AcqDataNoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AcqDataPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK IndexAcqChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveDownAcqChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveUpAcqChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RemoveAcqChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SelectAcqChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TimeAcqChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
