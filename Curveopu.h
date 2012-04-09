/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 1995. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/


     /* Panels and Controls: */

#define  ACDC                            1
#define  ACDC_GO                         2       /* callback function: ACDCCallback */
#define  ACDC_DC                         3
#define  ACDC_AC                         4

#define  AMPPHASE                        2
#define  AMPPHASE_GRAPHS                 2       /* callback function: AmpPhaseControlCallback */
#define  AMPPHASE_WINDOW                 3       /* callback function: AmpPhaseControlCallback */
#define  AMPPHASE_SAMPFREQ               4
#define  AMPPHASE_BATCH_1                5       /* callback function: InitAmpSpecBatchCallback */
#define  AMPPHASE_BATCH_2                6       /* callback function: InitAmpSpecBatchCallback */
#define  AMPPHASE_KEEP_1                 7       /* callback function: KeepAmpCurveCallback */
#define  AMPPHASE_GRAPH_2                8
#define  AMPPHASE_KEEP_2                 9       /* callback function: KeepPhaseCurveCallback */
#define  AMPPHASE_GRAPH_1                10
#define  AMPPHASE_GO                     11      /* callback function: AmpPhaseCallback */

#define  BATCH                           3
#define  BATCH_CURVES                    2
#define  BATCH_ALL                       3       /* callback function: BatchControlCallback */
#define  BATCH_GRAPHS                    4       /* callback function: BatchControlCallback */
#define  BATCH_CANCEL                    5       /* callback function: util_DiscardCallback */
#define  BATCH_GO                        6

#define  EXPFIT                          4
#define  EXPFIT_GO                       2       /* callback function: ExpFitCallback */
#define  EXPFIT_KEEP                     3       /* callback function: CreateY1CurveCallback */
#define  EXPFIT_GRAPHS                   4       /* callback function: ExpFitControlCallback */
#define  EXPFIT_A                        5
#define  EXPFIT_B                        6
#define  EXPFIT_MSE                      7
#define  EXPFIT_TEXT_1                   8

#define  FILTER                          5
#define  FILTER_TYPE                     2       /* callback function: FilterControlCallback */
#define  FILTER_MODE                     3       /* callback function: FilterControlCallback */
#define  FILTER_ATTEN                    4       /* callback function: FilterControlCallback */
#define  FILTER_BATCH                    5       /* callback function: InitBatchCallback */
#define  FILTER_GO                       6       /* callback function: FilterCallback */
#define  FILTER_RIPPLE                   7       /* callback function: FilterControlCallback */
#define  FILTER_ORDER                    8       /* callback function: FilterControlCallback */
#define  FILTER_GRAPHS                   9       /* callback function: FilterControlCallback */
#define  FILTER_KEEP                     10      /* callback function: CreateY1CurveCallback */
#define  FILTER_SAMPFREQ                 11
#define  FILTER_HIGHFREQ                 12      /* callback function: FilterControlCallback */
#define  FILTER_LOWFREQ                  13      /* callback function: FilterControlCallback */

#define  INTEGDIFF                       6
#define  INTEGDIFF_GRAPHS                2       /* callback function: IntegDiffControlCallback */
#define  INTEGDIFF_GRAPH_RESULT          3
#define  INTEGDIFF_SAMPINTERVAL          4
#define  INTEGDIFF_BATCH                 5       /* callback function: InitBatchCallback */
#define  INTEGDIFF_GO                    6
#define  INTEGDIFF_FINAL                 7       /* callback function: IntegDiffControlCallback */
#define  INTEGDIFF_INIT                  8       /* callback function: IntegDiffControlCallback */
#define  INTEGDIFF_KEEP                  9       /* callback function: CreateY1CurveCallback */

#define  INTERP                          7
#define  INTERP_GO                       2
#define  INTERP_BATCH                    3       /* callback function: InitBatchCallback */
#define  INTERP_KEEP                     4       /* callback function: KeepInterpCurveCallback */
#define  INTERP_PTS                      5       /* callback function: InterpControlCallback */
#define  INTERP_GRAPHS                   6       /* callback function: InterpControlCallback */
#define  INTERP_TOGGLE                   7       /* callback function: InterpControlCallback */
#define  INTERP_ERR                      8
#define  INTERP_Y                        9
#define  INTERP_X                        10      /* callback function: InterpControlCallback */

#define  LINFIT                          8
#define  LINFIT_GO                       2       /* callback function: LinFitCallback */
#define  LINFIT_KEEP                     3       /* callback function: CreateY1CurveCallback */
#define  LINFIT_GRAPHS                   4       /* callback function: LinFitControlCallback */
#define  LINFIT_SLOPE                    5
#define  LINFIT_INVSLOPE                 6
#define  LINFIT_INTERCEPT                7
#define  LINFIT_MSE                      8

#define  OFFSET                          9
#define  OFFSET_GRAPHS                   2
#define  OFFSET_GRAPH                    3
#define  OFFSET_GO                       4
#define  OFFSET_DY                       5
#define  OFFSET_KEEP                     6

#define  POLYFIT                         10
#define  POLYFIT_GO                      2       /* callback function: PolyFitCallback */
#define  POLYFIT_KEEP                    3       /* callback function: CreateY1CurveCallback */
#define  POLYFIT_ORDER                   4       /* callback function: PolyFitControlCallback */
#define  POLYFIT_GRAPHS                  5       /* callback function: PolyFitControlCallback */
#define  POLYFIT_COEFFS                  6
#define  POLYFIT_MSE                     7

#define  POWERSPEC                       11
#define  POWERSPEC_GRAPHS                2       /* callback function: PowerSpecControlCallback */
#define  POWERSPEC_GRAPH                 3
#define  POWERSPEC_BATCH_2               4       /* callback function: InitBatchCallback */
#define  POWERSPEC_PEAK_GO               5       /* callback function: PowerFreqEstimateCallback */
#define  POWERSPEC_GO                    6       /* callback function: PowerSpecCallback */
#define  POWERSPEC_KEEP                  7       /* callback function: KeepAmpCurveCallback */
#define  POWERSPEC_WINDOW                8       /* callback function: PowerSpecControlCallback */
#define  POWERSPEC_PEAK_POWER            9
#define  POWERSPEC_PEAK_FREQ             10
#define  POWERSPEC_SRCHFREQ              11
#define  POWERSPEC_SPAN                  12
#define  POWERSPEC_TEXT_1                13
#define  POWERSPEC_BOX_1                 14

#define  SMOOTH                          12
#define  SMOOTH_KERNAL                   2       /* callback function: SmoothControlCallback */
#define  SMOOTH_GO                       3       /* callback function: SmoothCallback */
#define  SMOOTH_GRAPHS                   4       /* callback function: SmoothControlCallback */
#define  SMOOTH_KEEP                     5       /* callback function: CreateY1CurveCallback */
#define  SMOOTH_BATCH                    6       /* callback function: InitBatchCallback */
#define  SMOOTH_VARIANCE                 7       /* callback function: SmoothControlCallback */
#define  SMOOTH_ORDER                    8       /* callback function: SmoothControlCallback */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  ACDCCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  AmpPhaseCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  AmpPhaseControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  BatchControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CreateY1CurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ExpFitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ExpFitControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  FilterCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  FilterControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  InitAmpSpecBatchCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  InitBatchCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  IntegDiffControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  InterpControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  KeepAmpCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  KeepInterpCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  KeepPhaseCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  LinFitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  LinFitControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  PolyFitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  PolyFitControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  PowerFreqEstimateCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  PowerSpecCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  PowerSpecControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  SmoothCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  SmoothControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

