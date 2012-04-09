/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 1995. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/


     /* Panels and Controls: */

#define  CHIRP                           1
#define  CHIRP_PTS                       2
#define  CHIRP_AMP                       3
#define  CHIRP_START                     4
#define  CHIRP_STOP                      5
#define  CHIRP_GO                        6       /* callback function: GenChirpCallback */
#define  CHIRP_CLOSE                     7       /* callback function: util_DiscardCallback */
#define  CHIRP_TEXT_1                    8

#define  CONSTANT                        2
#define  CONSTANT_PTS                    2
#define  CONSTANT_AMP                    3
#define  CONSTANT_GO                     4       /* callback function: GenConstantCallback */
#define  CONSTANT_CLOSE                  5       /* callback function: util_DiscardCallback */

#define  GAUSS                           3
#define  GAUSS_PTS                       2
#define  GAUSS_STDDEV                    3
#define  GAUSS_SEED                      4
#define  GAUSS_GO                        5       /* callback function: GenGaussCallback */
#define  GAUSS_CLOSE                     6       /* callback function: util_DiscardCallback */

#define  IMPULSE                         4
#define  IMPULSE_PTS                     2
#define  IMPULSE_AMP                     3
#define  IMPULSE_INDEX                   4
#define  IMPULSE_GO                      5       /* callback function: GenImpulseCallback */
#define  IMPULSE_CLOSE                   6       /* callback function: util_DiscardCallback */

#define  PULSE                           5
#define  PULSE_PTS                       2
#define  PULSE_AMP                       3
#define  PULSE_DELAY                     4
#define  PULSE_WIDTH                     5
#define  PULSE_GO                        6       /* callback function: GenPulseCallback */
#define  PULSE_CLOSE                     7       /* callback function: util_DiscardCallback */

#define  RAMP                            6
#define  RAMP_PTS                        2
#define  RAMP_FIRST                      3
#define  RAMP_LAST                       4
#define  RAMP_GO                         5       /* callback function: GenRampCallback */
#define  RAMP_CLOSE                      6       /* callback function: util_DiscardCallback */

#define  SAWTOOTH                        7
#define  SAWTOOTH_PTS                    2
#define  SAWTOOTH_AMP                    3
#define  SAWTOOTH_FREQ                   4
#define  SAWTOOTH_PHASE                  5
#define  SAWTOOTH_GO                     6       /* callback function: GenSawtoothCallback */
#define  SAWTOOTH_CLOSE                  7       /* callback function: util_DiscardCallback */
#define  SAWTOOTH_TEXT_1                 8
#define  SAWTOOTH_TEXT_2                 9

#define  SINC                            8
#define  SINC_PTS                        2
#define  SINC_AMP                        3
#define  SINC_DELAY                      4
#define  SINC_DT                         5
#define  SINC_GO                         6       /* callback function: GenSincCallback */
#define  SINC_CLOSE                      7       /* callback function: util_DiscardCallback */
#define  SINC_TEXT_1                     8

#define  SINE                            9
#define  SINE_PTS                        2
#define  SINE_AMP                        3
#define  SINE_FREQ                       4
#define  SINE_PHASE                      5
#define  SINE_GO                         6       /* callback function: GenSineCallback */
#define  SINE_CLOSE                      7       /* callback function: util_DiscardCallback */
#define  SINE_TEXT_1                     8
#define  SINE_TEXT_2                     9

#define  SINEPAT                         10
#define  SINEPAT_PTS                     2
#define  SINEPAT_AMP                     3
#define  SINEPAT_PHASE                   4
#define  SINEPAT_CYCLES                  5
#define  SINEPAT_GO                      6       /* callback function: GenSinePatternCallback */
#define  SINEPAT_CLOSE                   7       /* callback function: util_DiscardCallback */

#define  SQUARE                          11
#define  SQUARE_PTS                      2
#define  SQUARE_AMP                      3
#define  SQUARE_FREQ                     4
#define  SQUARE_PHASE                    5
#define  SQUARE_DUTY                     6
#define  SQUARE_GO                       7       /* callback function: GenSquareCallback */
#define  SQUARE_CLOSE                    8       /* callback function: util_DiscardCallback */
#define  SQUARE_TEXT_1                   9
#define  SQUARE_TEXT_2                   10
#define  SQUARE_TEXT_3                   11

#define  TRIANGLE                        12
#define  TRIANGLE_PTS                    2
#define  TRIANGLE_AMP                    3
#define  TRIANGLE_GO                     4       /* callback function: GenTriangleCallback */
#define  TRIANGLE_CLOSE                  5       /* callback function: util_DiscardCallback */

#define  TRIWAVE                         13
#define  TRIWAVE_PTS                     2
#define  TRIWAVE_AMP                     3
#define  TRIWAVE_FREQ                    4
#define  TRIWAVE_PHASE                   5
#define  TRIWAVE_GO                      6       /* callback function: GenTriangleWvfmCallback */
#define  TRIWAVE_CLOSE                   7       /* callback function: util_DiscardCallback */
#define  TRIWAVE_TEXT_1                  8
#define  TRIWAVE_TEXT_2                  9

#define  UNIFORM                         14
#define  UNIFORM_PTS                     2
#define  UNIFORM_SEED                    3
#define  UNIFORM_GO                      4       /* callback function: GenUniformCallback */
#define  UNIFORM_CLOSE                   5       /* callback function: util_DiscardCallback */

#define  WHITE                           15
#define  WHITE_PTS                       2
#define  WHITE_AMP                       3
#define  WHITE_SEED                      4
#define  WHITE_GO                        5       /* callback function: GenWhiteCallback */
#define  WHITE_CLOSE                     6       /* callback function: util_DiscardCallback */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  GenChirpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenConstantCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenGaussCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenImpulseCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenPulseCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenRampCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenSawtoothCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenSincCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenSineCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenSinePatternCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenSquareCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenTriangleCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenTriangleWvfmCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenUniformCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  GenWhiteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

