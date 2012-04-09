#include "ruo_low.h"

#define RESISTANCE 0
#define TEMPERATURE 1
#define TABLEITEMS 65


static double   RuOLow_Table [TABLEITEMS][2];

void    RuOLow_Init (void);
double  RuOLow_Temperature (double resistance);

double RuOLow_Temperature (double resistance)
{
    int i;
    double b, m;

    i = 2;
    while (i < TABLEITEMS) {
        if ((resistance >= RuOLow_Table[i-1][RESISTANCE]) &&
            (resistance <= RuOLow_Table[i][RESISTANCE])) {
            m = (RuOLow_Table[i][TEMPERATURE] - RuOLow_Table[i-1][TEMPERATURE])/
                (RuOLow_Table[i][RESISTANCE] - RuOLow_Table[i-1][RESISTANCE]);
            b = (RuOLow_Table[i-1][TEMPERATURE]+RuOLow_Table[i][TEMPERATURE])/2 -
                (m * (RuOLow_Table[i-1][RESISTANCE]+RuOLow_Table[i][RESISTANCE]))/2;
            return (m * resistance) + b;
        }
        i++;
    }
    return resistance;
}

void RuOLow_Init (void)
{
    RuOLow_Table[1][TEMPERATURE] = 20.0; RuOLow_Table[1][RESISTANCE] = 1103.7;
    RuOLow_Table[2][TEMPERATURE] = 19.0; RuOLow_Table[2][RESISTANCE] = 1107.9;
    RuOLow_Table[3][TEMPERATURE] = 18.0; RuOLow_Table[3][RESISTANCE] = 1112.6;
    RuOLow_Table[4][TEMPERATURE] = 17.0; RuOLow_Table[4][RESISTANCE] = 1117.9;
    RuOLow_Table[5][TEMPERATURE] = 16.0; RuOLow_Table[5][RESISTANCE] = 1123.6;
    RuOLow_Table[6][TEMPERATURE] = 15.0; RuOLow_Table[6][RESISTANCE] = 1130.2;
    RuOLow_Table[7][TEMPERATURE] = 14.0; RuOLow_Table[7][RESISTANCE] = 1137.7;
    RuOLow_Table[8][TEMPERATURE] = 13.0; RuOLow_Table[8][RESISTANCE] = 1146.3;
    RuOLow_Table[9][TEMPERATURE] = 12.0; RuOLow_Table[9][RESISTANCE] = 1156.5;
    RuOLow_Table[10][TEMPERATURE] = 11.0; RuOLow_Table[10][RESISTANCE] = 1168.3;
    RuOLow_Table[11][TEMPERATURE] = 10.0; RuOLow_Table[11][RESISTANCE] = 1182.2;
    RuOLow_Table[12][TEMPERATURE] = 9.5; RuOLow_Table[12][RESISTANCE] = 1190.4;
    RuOLow_Table[13][TEMPERATURE] = 9.0; RuOLow_Table[13][RESISTANCE] = 1199.3;
    RuOLow_Table[14][TEMPERATURE] = 8.5; RuOLow_Table[14][RESISTANCE] = 1209.3;
    RuOLow_Table[15][TEMPERATURE] = 8.0; RuOLow_Table[15][RESISTANCE] = 1220.4;
    RuOLow_Table[16][TEMPERATURE] = 7.5; RuOLow_Table[16][RESISTANCE] = 1233.1;
    RuOLow_Table[17][TEMPERATURE] = 7.0; RuOLow_Table[17][RESISTANCE] = 1247.2;
    RuOLow_Table[18][TEMPERATURE] = 6.5; RuOLow_Table[18][RESISTANCE] = 1263.7;
    RuOLow_Table[19][TEMPERATURE] = 6.0; RuOLow_Table[19][RESISTANCE] = 1282.6;
    RuOLow_Table[20][TEMPERATURE] = 5.5; RuOLow_Table[20][RESISTANCE] = 1305.0;
    RuOLow_Table[21][TEMPERATURE] = 5.0; RuOLow_Table[21][RESISTANCE] = 1331.4;
    RuOLow_Table[22][TEMPERATURE] = 4.8; RuOLow_Table[22][RESISTANCE] = 1343.4;

    RuOLow_Table[23][TEMPERATURE] = 4.6; RuOLow_Table[23][RESISTANCE] = 1356.4;
    RuOLow_Table[24][TEMPERATURE] = 4.4; RuOLow_Table[24][RESISTANCE] = 1370.4;
    RuOLow_Table[25][TEMPERATURE] = 4.2; RuOLow_Table[25][RESISTANCE] = 1385.9;
    RuOLow_Table[26][TEMPERATURE] = 4.0; RuOLow_Table[26][RESISTANCE] = 1402.5;
    RuOLow_Table[27][TEMPERATURE] = 3.8; RuOLow_Table[27][RESISTANCE] = 1421.1;
    RuOLow_Table[28][TEMPERATURE] = 3.6; RuOLow_Table[28][RESISTANCE] = 1441.5;
    RuOLow_Table[29][TEMPERATURE] = 3.4; RuOLow_Table[29][RESISTANCE] = 1463.3;
    RuOLow_Table[30][TEMPERATURE] = 3.2; RuOLow_Table[30][RESISTANCE] = 1488.0;
    RuOLow_Table[31][TEMPERATURE] = 3.0; RuOLow_Table[31][RESISTANCE] = 1515.9;
    RuOLow_Table[32][TEMPERATURE] = 2.8; RuOLow_Table[32][RESISTANCE] = 1547.6;
    RuOLow_Table[33][TEMPERATURE] = 2.6; RuOLow_Table[33][RESISTANCE] = 1584.5;
    RuOLow_Table[34][TEMPERATURE] = 2.4; RuOLow_Table[34][RESISTANCE] = 1629.9;
    RuOLow_Table[35][TEMPERATURE] = 2.2; RuOLow_Table[35][RESISTANCE] = 1681.4;
    RuOLow_Table[36][TEMPERATURE] = 2.0; RuOLow_Table[36][RESISTANCE] = 1740.7;
    RuOLow_Table[37][TEMPERATURE] = 1.9; RuOLow_Table[37][RESISTANCE] = 1774.6;
    RuOLow_Table[38][TEMPERATURE] = 1.8; RuOLow_Table[38][RESISTANCE] = 1812.4;
    RuOLow_Table[39][TEMPERATURE] = 1.7; RuOLow_Table[39][RESISTANCE] = 1854.4;
    RuOLow_Table[40][TEMPERATURE] = 1.6; RuOLow_Table[40][RESISTANCE] = 1901.5;
    RuOLow_Table[41][TEMPERATURE] = 1.5; RuOLow_Table[41][RESISTANCE] = 1953.7;
    RuOLow_Table[42][TEMPERATURE] = 1.4; RuOLow_Table[42][RESISTANCE] = 2011.5;
    RuOLow_Table[43][TEMPERATURE] = 1.3; RuOLow_Table[43][RESISTANCE] = 2079.3;

    RuOLow_Table[44][TEMPERATURE] = 1.2; RuOLow_Table[44][RESISTANCE] = 2160.4;
    RuOLow_Table[45][TEMPERATURE] = 1.1; RuOLow_Table[45][RESISTANCE] = 2256.6;
    RuOLow_Table[46][TEMPERATURE] = 1.0; RuOLow_Table[46][RESISTANCE] = 2368.1;
    RuOLow_Table[47][TEMPERATURE] = 0.9; RuOLow_Table[47][RESISTANCE] = 2508.7;
    RuOLow_Table[48][TEMPERATURE] = 0.8; RuOLow_Table[48][RESISTANCE] = 2679.9;
    RuOLow_Table[49][TEMPERATURE] = 0.7; RuOLow_Table[49][RESISTANCE] = 2902.2;
    RuOLow_Table[50][TEMPERATURE] = 0.6; RuOLow_Table[50][RESISTANCE] = 3193.5;
    RuOLow_Table[51][TEMPERATURE] = 0.5; RuOLow_Table[51][RESISTANCE] = 3602.5;
    RuOLow_Table[52][TEMPERATURE] = 0.45; RuOLow_Table[52][RESISTANCE] = 3875.1;
    RuOLow_Table[53][TEMPERATURE] = 0.40; RuOLow_Table[53][RESISTANCE] = 4222.8;
    RuOLow_Table[54][TEMPERATURE] = 0.35; RuOLow_Table[54][RESISTANCE] = 4629.0;
    RuOLow_Table[55][TEMPERATURE] = 0.30; RuOLow_Table[55][RESISTANCE] = 5228.7;
    RuOLow_Table[56][TEMPERATURE] = 0.25; RuOLow_Table[56][RESISTANCE] = 6081.5;
    RuOLow_Table[57][TEMPERATURE] = 0.20; RuOLow_Table[57][RESISTANCE] = 7398.5;
    RuOLow_Table[58][TEMPERATURE] = 0.15; RuOLow_Table[58][RESISTANCE] = 9749.7;
    RuOLow_Table[59][TEMPERATURE] = 0.10; RuOLow_Table[59][RESISTANCE] = 15315.0;
    RuOLow_Table[60][TEMPERATURE] = 0.09; RuOLow_Table[60][RESISTANCE] = 17554;
    RuOLow_Table[61][TEMPERATURE] = 0.08; RuOLow_Table[61][RESISTANCE] = 20523;
    RuOLow_Table[62][TEMPERATURE] = 0.07; RuOLow_Table[62][RESISTANCE] = 23723;
    RuOLow_Table[63][TEMPERATURE] = 0.06; RuOLow_Table[63][RESISTANCE] = 29568;
    RuOLow_Table[64][TEMPERATURE] = 0.05; RuOLow_Table[64][RESISTANCE] = 37886;
}

