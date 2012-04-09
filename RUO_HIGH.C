#include "ruo_high.h"

#define RESISTANCE 0
#define TEMPERATURE 1
#define TABLEITEMS 273


static double   RuOHigh_Table [TABLEITEMS][2];

void    RuOHigh_Init (void);
double  RuOHigh_Temperature (double resistance);

double RuOHigh_Temperature (double resistance)
{
    int i;
    double b, m;

    i = 2;
    while (i < TABLEITEMS) {
        if ((resistance >= RuOHigh_Table[i-1][RESISTANCE]) &&
            (resistance <= RuOHigh_Table[i][RESISTANCE])) {
            m = (RuOHigh_Table[i][TEMPERATURE] - RuOHigh_Table[i-1][TEMPERATURE])/
                (RuOHigh_Table[i][RESISTANCE] - RuOHigh_Table[i-1][RESISTANCE]);
            b = (RuOHigh_Table[i-1][TEMPERATURE]+RuOHigh_Table[i][TEMPERATURE])/2 -
                (m * (RuOHigh_Table[i-1][RESISTANCE]+RuOHigh_Table[i][RESISTANCE]))/2;
            return (m * resistance) + b;
        }
        i++;
    }
    return resistance;
}

void RuOHigh_Init (void)
{
    RuOHigh_Table[1][TEMPERATURE] = 273.0; RuOHigh_Table[1][RESISTANCE] = 100366.2;
    RuOHigh_Table[2][TEMPERATURE] = 272.0; RuOHigh_Table[2][RESISTANCE] = 100373.0;
    RuOHigh_Table[3][TEMPERATURE] = 271.0; RuOHigh_Table[3][RESISTANCE] = 100379.9;
    RuOHigh_Table[4][TEMPERATURE] = 270.0; RuOHigh_Table[4][RESISTANCE] = 100387.0;

    RuOHigh_Table[5][TEMPERATURE] = 269.0; RuOHigh_Table[5][RESISTANCE] = 100394.1;
    RuOHigh_Table[6][TEMPERATURE] = 268.0; RuOHigh_Table[6][RESISTANCE] = 100401.5;
    RuOHigh_Table[7][TEMPERATURE] = 267.0; RuOHigh_Table[7][RESISTANCE] = 100409.1;
    RuOHigh_Table[8][TEMPERATURE] = 266.0; RuOHigh_Table[8][RESISTANCE] = 100416.7;
    RuOHigh_Table[9][TEMPERATURE] = 265.0; RuOHigh_Table[9][RESISTANCE] = 100424.5;
    RuOHigh_Table[10][TEMPERATURE] = 264.0; RuOHigh_Table[10][RESISTANCE] = 100432.4;
    RuOHigh_Table[11][TEMPERATURE] = 263.0; RuOHigh_Table[11][RESISTANCE] = 100440.6;
    RuOHigh_Table[12][TEMPERATURE] = 262.0; RuOHigh_Table[12][RESISTANCE] = 100448.8;
    RuOHigh_Table[13][TEMPERATURE] = 261.0; RuOHigh_Table[13][RESISTANCE] = 100457.3;
    RuOHigh_Table[14][TEMPERATURE] = 260.0; RuOHigh_Table[14][RESISTANCE] = 100465.9;

    RuOHigh_Table[15][TEMPERATURE] = 259.0; RuOHigh_Table[15][RESISTANCE] = 100474.7;
    RuOHigh_Table[16][TEMPERATURE] = 258.0; RuOHigh_Table[16][RESISTANCE] = 100483.6;
    RuOHigh_Table[17][TEMPERATURE] = 257.0; RuOHigh_Table[17][RESISTANCE] = 100492.7;
    RuOHigh_Table[18][TEMPERATURE] = 256.0; RuOHigh_Table[18][RESISTANCE] = 100501.9;
    RuOHigh_Table[19][TEMPERATURE] = 255.0; RuOHigh_Table[19][RESISTANCE] = 100511.3;
    RuOHigh_Table[20][TEMPERATURE] = 254.0; RuOHigh_Table[20][RESISTANCE] = 100520.9;
    RuOHigh_Table[21][TEMPERATURE] = 253.0; RuOHigh_Table[21][RESISTANCE] = 100530.7;
    RuOHigh_Table[22][TEMPERATURE] = 252.0; RuOHigh_Table[22][RESISTANCE] = 100540.6;
    RuOHigh_Table[23][TEMPERATURE] = 251.0; RuOHigh_Table[23][RESISTANCE] = 100550.8;
    RuOHigh_Table[24][TEMPERATURE] = 250.0; RuOHigh_Table[24][RESISTANCE] = 100561.1;

    RuOHigh_Table[25][TEMPERATURE] = 249.0; RuOHigh_Table[25][RESISTANCE] = 100571.4;
    RuOHigh_Table[26][TEMPERATURE] = 248.0; RuOHigh_Table[26][RESISTANCE] = 100582.1;
    RuOHigh_Table[27][TEMPERATURE] = 247.0; RuOHigh_Table[27][RESISTANCE] = 100592.8;
    RuOHigh_Table[28][TEMPERATURE] = 246.0; RuOHigh_Table[28][RESISTANCE] = 100603.7;
    RuOHigh_Table[29][TEMPERATURE] = 245.0; RuOHigh_Table[29][RESISTANCE] = 100614.8;
    RuOHigh_Table[30][TEMPERATURE] = 244.0; RuOHigh_Table[30][RESISTANCE] = 100626.1;
    RuOHigh_Table[31][TEMPERATURE] = 243.0; RuOHigh_Table[31][RESISTANCE] = 100637.6;
    RuOHigh_Table[32][TEMPERATURE] = 242.0; RuOHigh_Table[32][RESISTANCE] = 100649.2;
    RuOHigh_Table[33][TEMPERATURE] = 241.0; RuOHigh_Table[33][RESISTANCE] = 100661.1;
    RuOHigh_Table[34][TEMPERATURE] = 240.0; RuOHigh_Table[34][RESISTANCE] = 100673.1;

    RuOHigh_Table[35][TEMPERATURE] = 239.0; RuOHigh_Table[35][RESISTANCE] = 100685.4;
    RuOHigh_Table[36][TEMPERATURE] = 238.0; RuOHigh_Table[36][RESISTANCE] = 100697.8;
    RuOHigh_Table[37][TEMPERATURE] = 237.0; RuOHigh_Table[37][RESISTANCE] = 100710.5;
    RuOHigh_Table[38][TEMPERATURE] = 236.0; RuOHigh_Table[38][RESISTANCE] = 100723.3;
    RuOHigh_Table[39][TEMPERATURE] = 235.0; RuOHigh_Table[39][RESISTANCE] = 100736.3;
    RuOHigh_Table[40][TEMPERATURE] = 234.0; RuOHigh_Table[40][RESISTANCE] = 100749.6;
    RuOHigh_Table[41][TEMPERATURE] = 233.0; RuOHigh_Table[41][RESISTANCE] = 100763.0;
    RuOHigh_Table[42][TEMPERATURE] = 232.0; RuOHigh_Table[42][RESISTANCE] = 100776.7;
    RuOHigh_Table[43][TEMPERATURE] = 231.0; RuOHigh_Table[43][RESISTANCE] = 100790.5;
    RuOHigh_Table[44][TEMPERATURE] = 230.0; RuOHigh_Table[44][RESISTANCE] = 100804.7;

    RuOHigh_Table[45][TEMPERATURE] = 229.0; RuOHigh_Table[45][RESISTANCE] = 100819.0;
    RuOHigh_Table[46][TEMPERATURE] = 228.0; RuOHigh_Table[46][RESISTANCE] = 100833.5;
    RuOHigh_Table[47][TEMPERATURE] = 227.0; RuOHigh_Table[47][RESISTANCE] = 100848.2;
    RuOHigh_Table[48][TEMPERATURE] = 226.0; RuOHigh_Table[48][RESISTANCE] = 100863.2;
    RuOHigh_Table[49][TEMPERATURE] = 225.0; RuOHigh_Table[49][RESISTANCE] = 100878.4;
    RuOHigh_Table[50][TEMPERATURE] = 224.0; RuOHigh_Table[50][RESISTANCE] = 100893.8;
    RuOHigh_Table[51][TEMPERATURE] = 223.0; RuOHigh_Table[51][RESISTANCE] = 100909.4;
    RuOHigh_Table[52][TEMPERATURE] = 222.0; RuOHigh_Table[52][RESISTANCE] = 100925.2;
    RuOHigh_Table[53][TEMPERATURE] = 221.0; RuOHigh_Table[53][RESISTANCE] = 100941.3;
    RuOHigh_Table[54][TEMPERATURE] = 220.0; RuOHigh_Table[54][RESISTANCE] = 100957.5;

    RuOHigh_Table[55][TEMPERATURE] = 219.0; RuOHigh_Table[55][RESISTANCE] = 100974.1;
    RuOHigh_Table[56][TEMPERATURE] = 218.0; RuOHigh_Table[56][RESISTANCE] = 100990.9;
    RuOHigh_Table[57][TEMPERATURE] = 217.0; RuOHigh_Table[57][RESISTANCE] = 101007.9;
    RuOHigh_Table[58][TEMPERATURE] = 216.0; RuOHigh_Table[58][RESISTANCE] = 101025.2;
    RuOHigh_Table[59][TEMPERATURE] = 215.0; RuOHigh_Table[59][RESISTANCE] = 101042.8;
    RuOHigh_Table[60][TEMPERATURE] = 214.0; RuOHigh_Table[60][RESISTANCE] = 101060.5;
    RuOHigh_Table[61][TEMPERATURE] = 213.0; RuOHigh_Table[61][RESISTANCE] = 101078.6;
    RuOHigh_Table[62][TEMPERATURE] = 212.0; RuOHigh_Table[62][RESISTANCE] = 101096.9;
    RuOHigh_Table[63][TEMPERATURE] = 211.0; RuOHigh_Table[63][RESISTANCE] = 101115.5;
    RuOHigh_Table[64][TEMPERATURE] = 210.0; RuOHigh_Table[64][RESISTANCE] = 101134.4;

    RuOHigh_Table[65][TEMPERATURE] = 209.0; RuOHigh_Table[65][RESISTANCE] = 101153.5;
    RuOHigh_Table[66][TEMPERATURE] = 208.0; RuOHigh_Table[66][RESISTANCE] = 101172.9;
    RuOHigh_Table[67][TEMPERATURE] = 207.0; RuOHigh_Table[67][RESISTANCE] = 101192.5;
    RuOHigh_Table[68][TEMPERATURE] = 206.0; RuOHigh_Table[68][RESISTANCE] = 101212.5;
    RuOHigh_Table[69][TEMPERATURE] = 205.0; RuOHigh_Table[69][RESISTANCE] = 101232.8;
    RuOHigh_Table[70][TEMPERATURE] = 204.0; RuOHigh_Table[70][RESISTANCE] = 101253.3;
    RuOHigh_Table[71][TEMPERATURE] = 203.0; RuOHigh_Table[71][RESISTANCE] = 101274.1;
    RuOHigh_Table[72][TEMPERATURE] = 202.0; RuOHigh_Table[72][RESISTANCE] = 101295.3;
    RuOHigh_Table[73][TEMPERATURE] = 201.0; RuOHigh_Table[73][RESISTANCE] = 101316.7;
    RuOHigh_Table[74][TEMPERATURE] = 200.0; RuOHigh_Table[74][RESISTANCE] = 101338.5;

    RuOHigh_Table[75][TEMPERATURE] = 199.0; RuOHigh_Table[75][RESISTANCE] = 101360.0;
    RuOHigh_Table[76][TEMPERATURE] = 198.0; RuOHigh_Table[76][RESISTANCE] = 101381.8;
    RuOHigh_Table[77][TEMPERATURE] = 197.0; RuOHigh_Table[77][RESISTANCE] = 101403.8;
    RuOHigh_Table[78][TEMPERATURE] = 196.0; RuOHigh_Table[78][RESISTANCE] = 101426.2;
    RuOHigh_Table[79][TEMPERATURE] = 195.0; RuOHigh_Table[79][RESISTANCE] = 101448.8;
    RuOHigh_Table[80][TEMPERATURE] = 194.0; RuOHigh_Table[80][RESISTANCE] = 101471.7;
    RuOHigh_Table[81][TEMPERATURE] = 193.0; RuOHigh_Table[81][RESISTANCE] = 101495.0;
    RuOHigh_Table[82][TEMPERATURE] = 192.0; RuOHigh_Table[82][RESISTANCE] = 101518.5;
    RuOHigh_Table[83][TEMPERATURE] = 191.0; RuOHigh_Table[83][RESISTANCE] = 101542.2;
    RuOHigh_Table[84][TEMPERATURE] = 190.0; RuOHigh_Table[84][RESISTANCE] = 101566.3;

    RuOHigh_Table[85][TEMPERATURE] = 189.0; RuOHigh_Table[85][RESISTANCE] = 101590.7;
    RuOHigh_Table[86][TEMPERATURE] = 188.0; RuOHigh_Table[86][RESISTANCE] = 101615.4;
    RuOHigh_Table[87][TEMPERATURE] = 187.0; RuOHigh_Table[87][RESISTANCE] = 101640.4;
    RuOHigh_Table[88][TEMPERATURE] = 186.0; RuOHigh_Table[88][RESISTANCE] = 101665.7;
    RuOHigh_Table[89][TEMPERATURE] = 185.0; RuOHigh_Table[89][RESISTANCE] = 101691.4;
    RuOHigh_Table[90][TEMPERATURE] = 184.0; RuOHigh_Table[90][RESISTANCE] = 101717.4;
    RuOHigh_Table[91][TEMPERATURE] = 183.0; RuOHigh_Table[91][RESISTANCE] = 101743.7;
    RuOHigh_Table[92][TEMPERATURE] = 182.0; RuOHigh_Table[92][RESISTANCE] = 101770.4;
    RuOHigh_Table[93][TEMPERATURE] = 181.0; RuOHigh_Table[93][RESISTANCE] = 101797.4;
    RuOHigh_Table[94][TEMPERATURE] = 180.0; RuOHigh_Table[94][RESISTANCE] = 101824.8;

    RuOHigh_Table[95][TEMPERATURE] = 179.0; RuOHigh_Table[95][RESISTANCE] = 101852.5;
    RuOHigh_Table[96][TEMPERATURE] = 178.0; RuOHigh_Table[96][RESISTANCE] = 101880.5;
    RuOHigh_Table[97][TEMPERATURE] = 177.0; RuOHigh_Table[97][RESISTANCE] = 101908.9;
    RuOHigh_Table[98][TEMPERATURE] = 176.0; RuOHigh_Table[98][RESISTANCE] = 101937.7;
    RuOHigh_Table[99][TEMPERATURE] = 175.0; RuOHigh_Table[99][RESISTANCE] = 101966.8;
    RuOHigh_Table[100][TEMPERATURE] = 174.0; RuOHigh_Table[100][RESISTANCE] = 101996.2;
    RuOHigh_Table[101][TEMPERATURE] = 173.0; RuOHigh_Table[101][RESISTANCE] = 102026.0;
    RuOHigh_Table[102][TEMPERATURE] = 172.0; RuOHigh_Table[102][RESISTANCE] = 102056.2;
    RuOHigh_Table[103][TEMPERATURE] = 171.0; RuOHigh_Table[103][RESISTANCE] = 102086.7;
    RuOHigh_Table[104][TEMPERATURE] = 170.0; RuOHigh_Table[104][RESISTANCE] = 102117.6;

    RuOHigh_Table[105][TEMPERATURE] = 169.0; RuOHigh_Table[105][RESISTANCE] = 102149.0;
    RuOHigh_Table[106][TEMPERATURE] = 168.0; RuOHigh_Table[106][RESISTANCE] = 102180.7;
    RuOHigh_Table[107][TEMPERATURE] = 167.0; RuOHigh_Table[107][RESISTANCE] = 102212.8;
    RuOHigh_Table[108][TEMPERATURE] = 166.0; RuOHigh_Table[108][RESISTANCE] = 102245.3;
    RuOHigh_Table[109][TEMPERATURE] = 165.0; RuOHigh_Table[109][RESISTANCE] = 102278.3;
    RuOHigh_Table[110][TEMPERATURE] = 164.0; RuOHigh_Table[110][RESISTANCE] = 102311.7;
    RuOHigh_Table[111][TEMPERATURE] = 163.0; RuOHigh_Table[111][RESISTANCE] = 102345.5;
    RuOHigh_Table[112][TEMPERATURE] = 162.0; RuOHigh_Table[112][RESISTANCE] = 102379.8;
    RuOHigh_Table[113][TEMPERATURE] = 161.0; RuOHigh_Table[113][RESISTANCE] = 102414.6;
    RuOHigh_Table[114][TEMPERATURE] = 160.0; RuOHigh_Table[114][RESISTANCE] = 102449.7;

    RuOHigh_Table[115][TEMPERATURE] = 159.0; RuOHigh_Table[115][RESISTANCE] = 102485.4;
    RuOHigh_Table[116][TEMPERATURE] = 158.0; RuOHigh_Table[116][RESISTANCE] = 102521.5;
    RuOHigh_Table[117][TEMPERATURE] = 157.0; RuOHigh_Table[117][RESISTANCE] = 102558.0;
    RuOHigh_Table[118][TEMPERATURE] = 156.0; RuOHigh_Table[118][RESISTANCE] = 102595.1;
    RuOHigh_Table[119][TEMPERATURE] = 155.0; RuOHigh_Table[119][RESISTANCE] = 102632.7;
    RuOHigh_Table[120][TEMPERATURE] = 154.0; RuOHigh_Table[120][RESISTANCE] = 102670.8;
    RuOHigh_Table[121][TEMPERATURE] = 153.0; RuOHigh_Table[121][RESISTANCE] = 102709.4;
    RuOHigh_Table[122][TEMPERATURE] = 152.0; RuOHigh_Table[122][RESISTANCE] = 102748.5;
    RuOHigh_Table[123][TEMPERATURE] = 151.0; RuOHigh_Table[123][RESISTANCE] = 102788.1;
    RuOHigh_Table[124][TEMPERATURE] = 150.0; RuOHigh_Table[124][RESISTANCE] = 102828.3;

    RuOHigh_Table[125][TEMPERATURE] = 149.0; RuOHigh_Table[125][RESISTANCE] = 102868.4;
    RuOHigh_Table[126][TEMPERATURE] = 148.0; RuOHigh_Table[126][RESISTANCE] = 102908.8;
    RuOHigh_Table[127][TEMPERATURE] = 147.0; RuOHigh_Table[127][RESISTANCE] = 102949.8;
    RuOHigh_Table[128][TEMPERATURE] = 146.0; RuOHigh_Table[128][RESISTANCE] = 102991.3;
    RuOHigh_Table[129][TEMPERATURE] = 145.0; RuOHigh_Table[129][RESISTANCE] = 103033.3;
    RuOHigh_Table[130][TEMPERATURE] = 144.0; RuOHigh_Table[130][RESISTANCE] = 103075.8;
    RuOHigh_Table[131][TEMPERATURE] = 143.0; RuOHigh_Table[131][RESISTANCE] = 103118.8;
    RuOHigh_Table[132][TEMPERATURE] = 142.0; RuOHigh_Table[132][RESISTANCE] = 103162.5;
    RuOHigh_Table[133][TEMPERATURE] = 141.0; RuOHigh_Table[133][RESISTANCE] = 103206.6;
    RuOHigh_Table[134][TEMPERATURE] = 140.0; RuOHigh_Table[134][RESISTANCE] = 103251.4;

    RuOHigh_Table[135][TEMPERATURE] = 139.0; RuOHigh_Table[135][RESISTANCE] = 103296.7;
    RuOHigh_Table[136][TEMPERATURE] = 138.0; RuOHigh_Table[136][RESISTANCE] = 103342.5;
    RuOHigh_Table[137][TEMPERATURE] = 137.0; RuOHigh_Table[137][RESISTANCE] = 103389.0;
    RuOHigh_Table[138][TEMPERATURE] = 136.0; RuOHigh_Table[138][RESISTANCE] = 103436.1;
    RuOHigh_Table[139][TEMPERATURE] = 135.0; RuOHigh_Table[139][RESISTANCE] = 103483.8;
    RuOHigh_Table[140][TEMPERATURE] = 134.0; RuOHigh_Table[140][RESISTANCE] = 103532.1;
    RuOHigh_Table[141][TEMPERATURE] = 133.0; RuOHigh_Table[141][RESISTANCE] = 103581.1;
    RuOHigh_Table[142][TEMPERATURE] = 132.0; RuOHigh_Table[142][RESISTANCE] = 103630.7;
    RuOHigh_Table[143][TEMPERATURE] = 131.0; RuOHigh_Table[143][RESISTANCE] = 103681.0;
    RuOHigh_Table[144][TEMPERATURE] = 130.0; RuOHigh_Table[144][RESISTANCE] = 103731.9;

    RuOHigh_Table[145][TEMPERATURE] = 129.0; RuOHigh_Table[145][RESISTANCE] = 103783.6;
    RuOHigh_Table[146][TEMPERATURE] = 128.0; RuOHigh_Table[146][RESISTANCE] = 103836.0;
    RuOHigh_Table[147][TEMPERATURE] = 127.0; RuOHigh_Table[147][RESISTANCE] = 103889.1;
    RuOHigh_Table[148][TEMPERATURE] = 126.0; RuOHigh_Table[148][RESISTANCE] = 103942.9;
    RuOHigh_Table[149][TEMPERATURE] = 125.0; RuOHigh_Table[149][RESISTANCE] = 103997.4;
    RuOHigh_Table[150][TEMPERATURE] = 124.0; RuOHigh_Table[150][RESISTANCE] = 104052.6;
    RuOHigh_Table[151][TEMPERATURE] = 123.0; RuOHigh_Table[151][RESISTANCE] = 104108.5;
    RuOHigh_Table[152][TEMPERATURE] = 122.0; RuOHigh_Table[152][RESISTANCE] = 104165.3;
    RuOHigh_Table[153][TEMPERATURE] = 121.0; RuOHigh_Table[153][RESISTANCE] = 104222.8;
    RuOHigh_Table[154][TEMPERATURE] = 120.0; RuOHigh_Table[154][RESISTANCE] = 104281.1;

    RuOHigh_Table[155][TEMPERATURE] = 119.0; RuOHigh_Table[155][RESISTANCE] = 104340.3;
    RuOHigh_Table[156][TEMPERATURE] = 118.0; RuOHigh_Table[156][RESISTANCE] = 104400.4;
    RuOHigh_Table[157][TEMPERATURE] = 117.0; RuOHigh_Table[157][RESISTANCE] = 104461.2;
    RuOHigh_Table[158][TEMPERATURE] = 116.0; RuOHigh_Table[158][RESISTANCE] = 104523.0;
    RuOHigh_Table[159][TEMPERATURE] = 115.0; RuOHigh_Table[159][RESISTANCE] = 104585.6;
    RuOHigh_Table[160][TEMPERATURE] = 114.0; RuOHigh_Table[160][RESISTANCE] = 104649.3;
    RuOHigh_Table[161][TEMPERATURE] = 113.0; RuOHigh_Table[161][RESISTANCE] = 104713.8;
    RuOHigh_Table[162][TEMPERATURE] = 112.0; RuOHigh_Table[162][RESISTANCE] = 104779.3;
    RuOHigh_Table[163][TEMPERATURE] = 111.0; RuOHigh_Table[163][RESISTANCE] = 104845.8;
    RuOHigh_Table[164][TEMPERATURE] = 110.0; RuOHigh_Table[164][RESISTANCE] = 104913.3;

    RuOHigh_Table[165][TEMPERATURE] = 109.0; RuOHigh_Table[165][RESISTANCE] = 104981.7;
    RuOHigh_Table[166][TEMPERATURE] = 108.0; RuOHigh_Table[166][RESISTANCE] = 105051.3;
    RuOHigh_Table[167][TEMPERATURE] = 107.0; RuOHigh_Table[167][RESISTANCE] = 105122.0;
    RuOHigh_Table[168][TEMPERATURE] = 106.0; RuOHigh_Table[168][RESISTANCE] = 105193.8;
    RuOHigh_Table[169][TEMPERATURE] = 105.0; RuOHigh_Table[169][RESISTANCE] = 105266.6;
    RuOHigh_Table[170][TEMPERATURE] = 104.0; RuOHigh_Table[170][RESISTANCE] = 105340.7;
    RuOHigh_Table[171][TEMPERATURE] = 103.0; RuOHigh_Table[171][RESISTANCE] = 105415.9;
    RuOHigh_Table[172][TEMPERATURE] = 102.0; RuOHigh_Table[172][RESISTANCE] = 105492.4;
    RuOHigh_Table[173][TEMPERATURE] = 101.0; RuOHigh_Table[173][RESISTANCE] = 105570.1;
    RuOHigh_Table[174][TEMPERATURE] = 100.0; RuOHigh_Table[174][RESISTANCE] = 105649.0;

    RuOHigh_Table[175][TEMPERATURE] = 99.0; RuOHigh_Table[175][RESISTANCE] = 105729.8;
    RuOHigh_Table[176][TEMPERATURE] = 98.0; RuOHigh_Table[176][RESISTANCE] = 105812.0;
    RuOHigh_Table[177][TEMPERATURE] = 97.0; RuOHigh_Table[177][RESISTANCE] = 105895.6;
    RuOHigh_Table[178][TEMPERATURE] = 96.0; RuOHigh_Table[178][RESISTANCE] = 105980.6;
    RuOHigh_Table[179][TEMPERATURE] = 95.0; RuOHigh_Table[179][RESISTANCE] = 106067.2;
    RuOHigh_Table[180][TEMPERATURE] = 94.0; RuOHigh_Table[180][RESISTANCE] = 106155.3;
    RuOHigh_Table[181][TEMPERATURE] = 93.0; RuOHigh_Table[181][RESISTANCE] = 106244.9;
    RuOHigh_Table[182][TEMPERATURE] = 92.0; RuOHigh_Table[182][RESISTANCE] = 106336.3;
    RuOHigh_Table[183][TEMPERATURE] = 91.0; RuOHigh_Table[183][RESISTANCE] = 106429.3;
    RuOHigh_Table[184][TEMPERATURE] = 90.0; RuOHigh_Table[184][RESISTANCE] = 106524.0;

    RuOHigh_Table[185][TEMPERATURE] = 89.0; RuOHigh_Table[185][RESISTANCE] = 106621.3;
    RuOHigh_Table[186][TEMPERATURE] = 88.0; RuOHigh_Table[186][RESISTANCE] = 106720.8;
    RuOHigh_Table[187][TEMPERATURE] = 87.0; RuOHigh_Table[187][RESISTANCE] = 106822.2;
    RuOHigh_Table[188][TEMPERATURE] = 86.0; RuOHigh_Table[188][RESISTANCE] = 106925.9;
    RuOHigh_Table[189][TEMPERATURE] = 85.0; RuOHigh_Table[189][RESISTANCE] = 107031.7;
    RuOHigh_Table[190][TEMPERATURE] = 84.0; RuOHigh_Table[190][RESISTANCE] = 107139.7;
    RuOHigh_Table[191][TEMPERATURE] = 83.0; RuOHigh_Table[191][RESISTANCE] = 107250.1;
    RuOHigh_Table[192][TEMPERATURE] = 82.0; RuOHigh_Table[192][RESISTANCE] = 107363.0;
    RuOHigh_Table[193][TEMPERATURE] = 81.0; RuOHigh_Table[193][RESISTANCE] = 107478.3;
    RuOHigh_Table[194][TEMPERATURE] = 80.0; RuOHigh_Table[194][RESISTANCE] = 107596.1;

    RuOHigh_Table[195][TEMPERATURE] = 79.0; RuOHigh_Table[195][RESISTANCE] = 107716.4;
    RuOHigh_Table[196][TEMPERATURE] = 78.0; RuOHigh_Table[196][RESISTANCE] = 107839.4;
    RuOHigh_Table[197][TEMPERATURE] = 77.0; RuOHigh_Table[197][RESISTANCE] = 107965.1;
    RuOHigh_Table[198][TEMPERATURE] = 76.0; RuOHigh_Table[198][RESISTANCE] = 108092.9;
    RuOHigh_Table[199][TEMPERATURE] = 75.0; RuOHigh_Table[199][RESISTANCE] = 108223.0;
    RuOHigh_Table[200][TEMPERATURE] = 74.0; RuOHigh_Table[200][RESISTANCE] = 108355.8;
    RuOHigh_Table[201][TEMPERATURE] = 73.0; RuOHigh_Table[201][RESISTANCE] = 108491.2;
    RuOHigh_Table[202][TEMPERATURE] = 72.0; RuOHigh_Table[202][RESISTANCE] = 108629.4;
    RuOHigh_Table[203][TEMPERATURE] = 71.0; RuOHigh_Table[203][RESISTANCE] = 108770.4;
    RuOHigh_Table[204][TEMPERATURE] = 70.0; RuOHigh_Table[204][RESISTANCE] = 108914.3;

    RuOHigh_Table[205][TEMPERATURE] = 69.0; RuOHigh_Table[205][RESISTANCE] = 109063.4;
    RuOHigh_Table[206][TEMPERATURE] = 68.0; RuOHigh_Table[206][RESISTANCE] = 109216.4;
    RuOHigh_Table[207][TEMPERATURE] = 67.0; RuOHigh_Table[207][RESISTANCE] = 109373.3;
    RuOHigh_Table[208][TEMPERATURE] = 66.0; RuOHigh_Table[208][RESISTANCE] = 109534.3;
    RuOHigh_Table[209][TEMPERATURE] = 65.0; RuOHigh_Table[209][RESISTANCE] = 109699.5;
    RuOHigh_Table[210][TEMPERATURE] = 64.0; RuOHigh_Table[210][RESISTANCE] = 109869.1;
    RuOHigh_Table[211][TEMPERATURE] = 63.0; RuOHigh_Table[211][RESISTANCE] = 110043.3;
    RuOHigh_Table[212][TEMPERATURE] = 62.0; RuOHigh_Table[212][RESISTANCE] = 110222.2;
    RuOHigh_Table[213][TEMPERATURE] = 61.0; RuOHigh_Table[213][RESISTANCE] = 110406.1;
    RuOHigh_Table[214][TEMPERATURE] = 60.0; RuOHigh_Table[214][RESISTANCE] = 110595.1;

    RuOHigh_Table[215][TEMPERATURE] = 59.0; RuOHigh_Table[215][RESISTANCE] = 110791.4;
    RuOHigh_Table[216][TEMPERATURE] = 58.0; RuOHigh_Table[216][RESISTANCE] = 110993.7;
    RuOHigh_Table[217][TEMPERATURE] = 57.0; RuOHigh_Table[217][RESISTANCE] = 111202.2;
    RuOHigh_Table[218][TEMPERATURE] = 56.0; RuOHigh_Table[218][RESISTANCE] = 111417.4;
    RuOHigh_Table[219][TEMPERATURE] = 55.0; RuOHigh_Table[229][RESISTANCE] = 111639.3;
    RuOHigh_Table[220][TEMPERATURE] = 54.0; RuOHigh_Table[220][RESISTANCE] = 111868.6;
    RuOHigh_Table[221][TEMPERATURE] = 53.0; RuOHigh_Table[221][RESISTANCE] = 112105.3;
    RuOHigh_Table[222][TEMPERATURE] = 52.0; RuOHigh_Table[222][RESISTANCE] = 112350.0;
    RuOHigh_Table[223][TEMPERATURE] = 51.0; RuOHigh_Table[223][RESISTANCE] = 112603.0;
    RuOHigh_Table[224][TEMPERATURE] = 50.0; RuOHigh_Table[224][RESISTANCE] = 112864.8;

    RuOHigh_Table[225][TEMPERATURE] = 49.0; RuOHigh_Table[225][RESISTANCE] = 113137.3;
    RuOHigh_Table[226][TEMPERATURE] = 48.0; RuOHigh_Table[226][RESISTANCE] = 113419.9;
    RuOHigh_Table[227][TEMPERATURE] = 47.0; RuOHigh_Table[227][RESISTANCE] = 113713.1;
    RuOHigh_Table[228][TEMPERATURE] = 46.0; RuOHigh_Table[228][RESISTANCE] = 114017.4;
    RuOHigh_Table[229][TEMPERATURE] = 45.0; RuOHigh_Table[229][RESISTANCE] = 114333.6;
    RuOHigh_Table[230][TEMPERATURE] = 44.0; RuOHigh_Table[230][RESISTANCE] = 114662.3;
    RuOHigh_Table[231][TEMPERATURE] = 43.0; RuOHigh_Table[231][RESISTANCE] = 115004.2;
    RuOHigh_Table[232][TEMPERATURE] = 42.0; RuOHigh_Table[232][RESISTANCE] = 115360.3;
    RuOHigh_Table[233][TEMPERATURE] = 41.0; RuOHigh_Table[233][RESISTANCE] = 115731.3;
    RuOHigh_Table[234][TEMPERATURE] = 40.0; RuOHigh_Table[234][RESISTANCE] = 116118.1;

    RuOHigh_Table[235][TEMPERATURE] = 39.0; RuOHigh_Table[235][RESISTANCE] = 116523.6;
    RuOHigh_Table[236][TEMPERATURE] = 38.0; RuOHigh_Table[236][RESISTANCE] = 116947.6;
    RuOHigh_Table[237][TEMPERATURE] = 37.0; RuOHigh_Table[237][RESISTANCE] = 117391.9;
    RuOHigh_Table[238][TEMPERATURE] = 36.0; RuOHigh_Table[238][RESISTANCE] = 117857.8;
    RuOHigh_Table[239][TEMPERATURE] = 35.0; RuOHigh_Table[239][RESISTANCE] = 118347.3;
    RuOHigh_Table[240][TEMPERATURE] = 34.0; RuOHigh_Table[240][RESISTANCE] = 118861.6;
    RuOHigh_Table[241][TEMPERATURE] = 33.0; RuOHigh_Table[241][RESISTANCE] = 119404.0;
    RuOHigh_Table[242][TEMPERATURE] = 32.0; RuOHigh_Table[242][RESISTANCE] = 119976.2;
    RuOHigh_Table[243][TEMPERATURE] = 31.0; RuOHigh_Table[243][RESISTANCE] = 120580.7;
    RuOHigh_Table[244][TEMPERATURE] = 30.0; RuOHigh_Table[244][RESISTANCE] = 121220.3;

    RuOHigh_Table[245][TEMPERATURE] = 29.0; RuOHigh_Table[245][RESISTANCE] = 121898.4;
    RuOHigh_Table[246][TEMPERATURE] = 28.0; RuOHigh_Table[246][RESISTANCE] = 122618.5;
    RuOHigh_Table[247][TEMPERATURE] = 27.0; RuOHigh_Table[247][RESISTANCE] = 123386.0;
    RuOHigh_Table[248][TEMPERATURE] = 26.0; RuOHigh_Table[248][RESISTANCE] = 124205.1;
    RuOHigh_Table[249][TEMPERATURE] = 25.0; RuOHigh_Table[249][RESISTANCE] = 125082.5;
    RuOHigh_Table[250][TEMPERATURE] = 24.0; RuOHigh_Table[250][RESISTANCE] = 126024.0;
    RuOHigh_Table[251][TEMPERATURE] = 23.0; RuOHigh_Table[251][RESISTANCE] = 127038.1;
    RuOHigh_Table[252][TEMPERATURE] = 22.0; RuOHigh_Table[252][RESISTANCE] = 128132.9;
    RuOHigh_Table[253][TEMPERATURE] = 21.0; RuOHigh_Table[253][RESISTANCE] = 129317.7;
    RuOHigh_Table[254][TEMPERATURE] = 20.0; RuOHigh_Table[254][RESISTANCE] = 130604.6;

    RuOHigh_Table[255][TEMPERATURE] = 19.0; RuOHigh_Table[255][RESISTANCE] = 132008.6;
    RuOHigh_Table[256][TEMPERATURE] = 18.0; RuOHigh_Table[256][RESISTANCE] = 133549.6;
    RuOHigh_Table[257][TEMPERATURE] = 17.0; RuOHigh_Table[257][RESISTANCE] = 135245.9;
    RuOHigh_Table[258][TEMPERATURE] = 16.0; RuOHigh_Table[258][RESISTANCE] = 137122.0;
    RuOHigh_Table[259][TEMPERATURE] = 15.0; RuOHigh_Table[259][RESISTANCE] = 139213.9;
    RuOHigh_Table[260][TEMPERATURE] = 14.0; RuOHigh_Table[260][RESISTANCE] = 141556.3;
    RuOHigh_Table[261][TEMPERATURE] = 13.0; RuOHigh_Table[261][RESISTANCE] = 144196.7;
    RuOHigh_Table[262][TEMPERATURE] = 12.0; RuOHigh_Table[262][RESISTANCE] = 147197.2;
    RuOHigh_Table[263][TEMPERATURE] = 11.0; RuOHigh_Table[263][RESISTANCE] = 150624.2;
    RuOHigh_Table[264][TEMPERATURE] = 10.0; RuOHigh_Table[264][RESISTANCE] = 154569.1;

    RuOHigh_Table[265][TEMPERATURE] = 9.0; RuOHigh_Table[265][RESISTANCE] = 159156.5;
    RuOHigh_Table[266][TEMPERATURE] = 8.0; RuOHigh_Table[266][RESISTANCE] = 164590.2;
    RuOHigh_Table[267][TEMPERATURE] = 7.0; RuOHigh_Table[267][RESISTANCE] = 171089.3;
    RuOHigh_Table[268][TEMPERATURE] = 6.0; RuOHigh_Table[268][RESISTANCE] = 178961.5;
    RuOHigh_Table[269][TEMPERATURE] = 5.0; RuOHigh_Table[269][RESISTANCE] = 188603.8;
    RuOHigh_Table[270][TEMPERATURE] = 4.0; RuOHigh_Table[270][RESISTANCE] = 200519.3;
    RuOHigh_Table[271][TEMPERATURE] = 3.0; RuOHigh_Table[271][RESISTANCE] = 214778.0;
    RuOHigh_Table[272][TEMPERATURE] = 2.0; RuOHigh_Table[272][RESISTANCE] = 230349.3;
}
