/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 1995. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/


     /* Panels and Controls: */

#define  LIST                            1
#define  LIST_ITEMS                      2
#define  LIST_CANCEL                     3       /* callback function: util_DiscardCallback */
#define  LIST_CONT                       4
#define  LIST_ALL                        5       /* callback function: ListSelectAllCallback */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  ListSelectAllCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

