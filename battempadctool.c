#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static float g_resistance_tab[16] = {
    // -25  -15     -10     -5      0       5       10      20      30      40      45      50      55      60      70      80
    91.083, 55.183, 43.499, 34.569, 27.680, 22.316, 18.104, 12.122, 8.2937, 5.7954, 4.8820, 4.1331, 3.5158, 3.0043, 2.2212, 1.6669,
};

static int g_temp_map_tab_20uA[16] = {
    1821, 1103, 869, 691, 553, 446, 362, 242, 165, 115, 97, 82, 70, 60, 44, 33,
};

static int g_temp_map_tab_40uA[16] = {
    3643, 2207, 1739, 1382, 1107, 892, 724, 484, 331, 231, 195, 165, 140, 120, 88, 66,
};

static int g_temp_map_tab_60uA[16] = {
    5465, 3311, 2610, 2074, 1661, 1339, 1086, 727, 498, 348, 293, 248, 211, 180, 133, 100,
};

static int g_temp_map_tab_80uA[16] = {
    7286, 4414, 3479, 2765, 2214, 1785, 1448, 969, 663, 463, 390, 330, 281, 240, 177, 133,
};

static int adc_to_temp(int adc, int params[16])
{
    int temp;
    if (adc < params[15]) {
        temp = 80;
    } else if (adc < params[14]) {
        temp = 70 + (params[14] - adc) * 10 / (params[14] - params[15]);
    } else if (adc < params[13]) {
        temp = 60 + (params[13] - adc) * 10 / (params[13] - params[14]);
    } else if (adc < params[12]) {
        temp = 55 + (params[12] - adc) * 5  / (params[12] - params[13]);
    } else if (adc < params[11]) {
        temp = 50 + (params[11] - adc) * 5  / (params[11] - params[12]);
    } else if (adc < params[10]) {
        temp = 45 + (params[10] - adc) * 5  / (params[10] - params[11]);
    } else if (adc < params[9 ]) {
        temp = 40 + (params[9 ] - adc) * 5  / (params[9 ] - params[10]);
    } else if (adc < params[8 ]) {
        temp = 30 + (params[8 ] - adc) * 10 / (params[8 ] - params[9 ]);
    } else if (adc < params[7 ]) {
        temp = 20 + (params[7 ] - adc) * 10 / (params[7 ] - params[8 ]);
    } else if (adc < params[6 ]) {
        temp = 10 + (params[6 ] - adc) * 10 / (params[6 ] - params[7 ]);
    } else if (adc < params[5 ]) {
        temp =  5 + (params[5 ] - adc) * 5  / (params[5 ] - params[6 ]);
    } else if (adc < params[4 ]) {
        temp =  0 + (params[4 ] - adc) * 5  / (params[4 ] - params[5 ]);
    } else if (adc < params[3 ]) {
        temp = -5 + (params[3 ] - adc) * 5  / (params[3 ] - params[4 ]);
    } else if (adc < params[2 ]) {
        temp =-10 + (params[2 ] - adc) * 5  / (params[2 ] - params[3 ]);
    } else if (adc < params[1 ]) {
        temp =-15 + (params[1 ] - adc) * 5  / (params[1 ] - params[2 ]);
    } else if (adc < params[0 ]) {
        temp =-25 + (params[0 ] - adc) * 10 / (params[0 ] - params[1 ]);
    } else {
        temp =-25;
    }
    return temp;
}

int main(int argc, char *argv[])
{
    int tsout, adcval, *maptab, lhtf, i;
    if (argc < 3) return 0;
    if (strcmp(argv[1], "-a") == 0) { // generate temp params
        tsout = atoi(argv[2]);
        printf("tsout current: %d\n", tsout);
        for (i=0; i<16; i++) {
            printf("pmu_bat_temp_para%d = %d\n", i+1, (int)(g_resistance_tab[i] * tsout));
        }
        printf("\n");
    } else if (strcmp(argv[1], "-c") == 0) { // input tsout adc, output temperature value
        tsout  = atoi(argv[2]);
        adcval = argc < 4 ? 0 : atoi(argv[3]);
        switch (tsout) {
        case 20: maptab = g_temp_map_tab_20uA; break;
        case 40: maptab = g_temp_map_tab_40uA; break;
        case 60: maptab = g_temp_map_tab_60uA; break;
        case 80: maptab = g_temp_map_tab_80uA; break;
        default: printf("invalid tsout !\n"); return 0;
        }
        printf("tsout current: %d\n", tsout );
        printf("adc value    : %d\n", adcval);
        printf("temp value   : %d\n", adc_to_temp(adcval, maptab));
    } else if (strcmp(argv[1], "-t") == 0) { // input tsout lhtf, output temperature value
        tsout  = atoi(argv[2]);
        lhtf   = argc < 4 ? 0 : atoi(argv[3]);
        adcval = lhtf * 10 / 128;
        adcval = adcval * 12.8;
        switch (tsout) {
        case 20: maptab = g_temp_map_tab_20uA; break;
        case 40: maptab = g_temp_map_tab_40uA; break;
        case 60: maptab = g_temp_map_tab_60uA; break;
        case 80: maptab = g_temp_map_tab_80uA; break;
        default: printf("invalid tsout !\n"); return 0;
        }
        printf("tsout current: %d\n", tsout );
        printf("lhtf value   : %d\n", lhtf  );
        printf("adc value    : %d\n", adcval);
        printf("temp value   : %d\n", adc_to_temp(adcval, maptab));
    }
    return 0;
}


