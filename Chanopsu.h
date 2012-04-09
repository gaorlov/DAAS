/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 1995. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/


     /* Panels and Controls: */

#define  CHANOPS_1                       1
#define  CHANOPS_1_CHANNELS              2
#define  CHANOPS_1_CANCEL                3       /* callback function: util_DiscardCallback */
#define  CHANOPS_1_GO                    4
#define  CHANOPS_1_TEXT_1                5

#define  CHANOPS_2                       2
#define  CHANOPS_2_CHAN_2                2
#define  CHANOPS_2_CANCEL                3       /* callback function: util_DiscardCallback */
#define  CHANOPS_2_GO                    4
#define  CHANOPS_2_CHAN_1                5
#define  CHANOPS_2_TEXT_1                6


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

