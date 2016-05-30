#include <stdlib.h>
#include <stdio.h>

#define BAT_LOW_VOL     3500
#define OUT_PARAMS_FMT  "pmu_bat_para%-2d  = %d\n"

static int cmp_int_item(const void *a, const void *b)
{
    return *(int*)a - *(int*)b;
}

int main(int argc, char *argv[])
{
    FILE *fpin = NULL;
    FILE *fpout= NULL;
    int   line = 0;
    int  *vlist= NULL;
    char  buf[256];
    int   pmu   [32] = { 3130, 3270, 3340, 3410, 3480, 3520, 3550, 3570, 3590, 3610, 3630, 3640, 3660, 3700, 3730, 3770,
                         3778, 3800, 3820, 3840, 3850, 3870, 3910, 3940, 3980, 4010, 4050, 4080, 4100, 4120, 4140, 4150 };
    int   params[32] = { 0 };
    int   min, voltage, percent, current;
    int   rdc = 100;
    int   i, j;

    if (argc >= 2) {
        rdc = atoi(argv[1]);
    }

    fpin = fopen("battery.log", "rb");
    if (!fpin) goto done;

    while (1) {
        int data = fgetc(fpin);
        if (data == EOF ) break;
        if (data == '\n') line++;
    }
//  printf("total line: %d\n", line);
    line -= 2; // skip first two lines

    vlist = (int*)malloc(sizeof(int) * line);
    if (!vlist) goto done;

    //++ read voltage data
    fseek(fpin, 0, SEEK_SET);
    fgets(buf, 256, fpin); // skip first line
    fgets(buf, 256, fpin); // skip second line
    for (i=0; i<line; i++) {
        fscanf(fpin, "%d %d %d %d", &min, &voltage, &percent, &current);
        vlist[i] = voltage - current * rdc / 1000;
//      printf("%d\n", vlist[i]);
        if (feof(fpin)) break;
    }
    //-- read voltage data

    // sort vlist
    qsort(vlist, line, sizeof(int), cmp_int_item);

    //++ calculate params for pmu
    for (i=0,j=0; i<32; i++) {
        if (pmu[i] < BAT_LOW_VOL) {
            params[i] = 0;
            continue;
        }

        for (; j<line; j++) {
            if (pmu[i] <= vlist[j]) {
                params[i] = 100 * j / line;
                break;
            }
        }
    }
    //-- calculate params for pmu

    fpout = fopen("pmu_params.fex", "wb");
    for (i=0; i<32; i++) {
        fprintf(stdout, OUT_PARAMS_FMT, i+1, params[i]);
        fprintf(fpout , OUT_PARAMS_FMT, i+1, params[i]);
    }
    fclose(fpout);

done:
    if (fpin ) fclose(fpin );
    if (vlist) free  (vlist);
}

