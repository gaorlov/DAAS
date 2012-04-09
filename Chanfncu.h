/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 1995. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/


     /* Panels and Controls: */

#define  CLIP                            1
#define  CLIP_CHANNELS                   2
#define  CLIP_MIN                        3
#define  CLIP_MAX                        4
#define  CLIP_GO                         5       /* callback function: DoClipCallback */
#define  CLIP_CLOSE                      6       /* callback function: util_DiscardCallback */

#define  DECIMATE                        2
#define  DECIMATE_CHANNELS               2
#define  DECIMATE_DFACT                  3
#define  DECIMATE_GO                     4       /* callback function: DoDecimateCallback */
#define  DECIMATE_AVE                    5
#define  DECIMATE_CLOSE                  6       /* callback function: util_DiscardCallback */

#define  EXPONENT                        3
#define  EXPONENT_CHANNELS               2
#define  EXPONENT_EXPON                  3
#define  EXPONENT_GO                     4       /* callback function: DoExponentCallback */
#define  EXPONENT_CLOSE                  5       /* callback function: util_DiscardCallback */
#define  EXPONENT_TEXT_1                 6

#define  LINEVAL                         4
#define  LINEVAL_CHANNELS                2
#define  LINEVAL_MULT                    3
#define  LINEVAL_ADD                     4
#define  LINEVAL_GO                      5       /* callback function: DoLinEvalCallback */
#define  LINEVAL_CLOSE                   6       /* callback function: util_DiscardCallback */
#define  LINEVAL_TEXT_1                  7
#define  LINEVAL_TEXT_2                  8

#define  MANIP                           5
#define  MANIP_INDEX                     2
#define  MANIP_READING                   3

#define  STATISTICS                      6
#define  STATISTICS_CHANNELS             2       /* callback function: DoStatisticsCallback */
#define  STATISTICS_MODE                 3
#define  STATISTICS_MEDIAN               4
#define  STATISTICS_MOMENT               5
#define  STATISTICS_CLOSE                6       /* callback function: util_DiscardCallback */
#define  STATISTICS_RMS                  7
#define  STATISTICS_VAR                  8
#define  STATISTICS_STDDEV               9
#define  STATISTICS_MEAN                 10
#define  STATISTICS_INTERVAL             11      /* callback function: DoStatisticsCallback */
#define  STATISTICS_ORDER                12      /* callback function: DoStatisticsCallback */
#define  STATISTICS_MAX                  13
#define  STATISTICS_MIN                  14
#define  STATISTICS_SAVE                 15      /* callback function: SaveStatisticsCallback */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  DoClipCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  DoDecimateCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  DoExponentCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  DoLinEvalCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  DoStatisticsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  SaveStatisticsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

