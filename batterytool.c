#include <stdlib.h>
#include <stdio.h>

#define BAT_LOW_VOL     3300
#define OUT_PARAMS_FMT  "pmu_bat_para%-2d  = %d\n"

static int cmp_int_item(const void *a, const void *b)
{
    return *(int*)a - *(int*)b;
}

int main(int argc, char *argv[])
{
    FILE *fpin = NULL;
    FILE *fpout= NULL;
    int   lines= 0;
    int  *vlist= NULL;
    char  buf[256];
    int   pmu   [32] = { 3133, 3273, 3344, 3414, 3485, 3555, 3590, 3608, 3626, 3643, 3661, 3696, 3731, 3766, 3802, 3819,
                         3837, 3854, 3872, 3907, 3942, 3978, 4013, 4048, 4083, 4118, 4136, 4154, 4189, 4224, 4259, 4294 };
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
        if (data == '\n') lines++;
    }
//  printf("total lines: %d\n", lines);
    lines -= 2; // skip first two lines

    vlist = (int*)malloc(sizeof(int) * lines);
    if (!vlist) goto done;

    //++ read voltage data
    fseek(fpin, 0, SEEK_SET);
    fgets(buf, 256, fpin); // skip first line
    fgets(buf, 256, fpin); // skip second line
    for (i=0; i<lines; i++) {
        fscanf(fpin, "%d %d %d %d", &min, &voltage, &percent, &current);
        vlist[i] = voltage + current * rdc / 1000;
//      printf("%d\n", vlist[i]);
        if (feof(fpin)) break;
    }
    //-- read voltage data

    // sort vlist
    qsort(vlist, lines, sizeof(int), cmp_int_item);

    //++ calculate params for pmu
    for (i=0,j=0; i<32; i++) {
        if (pmu[i] < BAT_LOW_VOL) {
            params[i] = 0;
            continue;
        }

        for (; j<lines; j++) {
            if (pmu[i] <= vlist[j]) {
                params[i] = 100 * j / lines;
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

