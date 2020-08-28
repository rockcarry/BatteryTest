#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <cutils/properties.h>

static int g_exit = 0;
static void sig_handler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGTERM:
    case SIGKILL:
        g_exit = 1;
        break;
    }
}

static int read_battery_ocv(void)
{
    int   reg1, reg2;
    FILE *fp = NULL;
    char  buf[256] = {0};
    char *p;

    fp = fopen("/sys/class/axp/axp_reg", "wb");
    if (!fp) return -1;
    fprintf(fp, "0xbc");
    fclose(fp);

    fp = fopen("/sys/class/axp/axp_regs", "rb");
    if (!fp) return -1;
    fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    p    = strstr(buf, "=0x");
    reg1 = p ? strtol(p + 3, NULL, 16) : 0;
    p    = p ? p + 3 : buf;
    p    = strstr(p  , "=0x");
    reg2 = p ? strtol(p + 3, NULL, 16) : 0;
    return (((reg1 & 0xff) << 4) | (reg2 & 0x0f)) * 1100 / 1000;
}

static int read_battery_maxcap(void)
{
    int   reg1, reg2;
    FILE *fp = NULL;
    char  buf[256] = {0};
    char *p;

    fp = fopen("/sys/class/axp/axp_reg", "wb");
    if (!fp) return -1;
    fprintf(fp, "0xe0");
    fclose(fp);

    fp = fopen("/sys/class/axp/axp_regs", "rb");
    if (!fp) return -1;
    fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    p    = strstr(buf, "=0x");
    reg1 = p ? strtol(p + 3, NULL, 16) : 0;
    p    = p ? p + 3 : buf;
    p    = strstr(p  , "=0x");
    reg2 = p ? strtol(p + 3, NULL, 16) : 0;
    return (((reg1 & 0x7f) << 8) | (reg2 & 0xff)) * 1456 / 1000;
}

static int read_battery_colmeter(void)
{
    int   reg1, reg2;
    FILE *fp = NULL;
    char  buf[256] = {0};
    char *p;

    fp = fopen("/sys/class/axp/axp_reg", "wb");
    if (!fp) return -1;
    fprintf(fp, "0xe2");
    fclose(fp);

    fp = fopen("/sys/class/axp/axp_regs", "rb");
    if (!fp) return -1;
    fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    p    = strstr(buf, "=0x");
    reg1 = p ? strtol(p + 3, NULL, 16) : 0;
    p    = p ? p + 3 : buf;
    p    = strstr(p  , "=0x");
    reg2 = p ? strtol(p + 3, NULL, 16) : 0;
    return (((reg1 & 0x7f) << 8) | (reg2 & 0xff)) * 1456 / 1000;
}

static int read_battery_ocv_percent(void)
{
    int   reg1;
    FILE *fp = NULL;
    char  buf[256] = {0};
    char *p;

    fp = fopen("/sys/class/axp/axp_reg", "wb");
    if (!fp) return -1;
    fprintf(fp, "0xe4");
    fclose(fp);

    fp = fopen("/sys/class/axp/axp_reg", "rb");
    if (!fp) return -1;
    fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    p    = strstr(buf, "=0x");
    reg1 = p ? strtol(p + 3, NULL, 16) : 0;
    return (reg1 & 0x7f);
}

static int read_battery_col_percent(void)
{
    int   reg1;
    FILE *fp = NULL;
    char  buf[256] = {0};
    char *p;

    fp = fopen("/sys/class/axp/axp_reg", "wb");
    if (!fp) return -1;
    fprintf(fp, "0xe5");
    fclose(fp);

    fp = fopen("/sys/class/axp/axp_reg", "rb");
    if (!fp) return -1;
    fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    p    = strstr(buf, "=0x");
    reg1 = p ? strtol(p + 3, NULL, 16) : 0;
    return (reg1 & 0x7f);
}

static int read_battery_rdc(void)
{
    int   reg1, reg2;
    FILE *fp = NULL;
    char  buf[256] = {0};
    char *p;

    fp = fopen("/sys/class/axp/axp_reg", "wb");
    if (!fp) return -1;
    fprintf(fp, "0xba");
    fclose(fp);

    fp = fopen("/sys/class/axp/axp_regs", "rb");
    if (!fp) return -1;
    fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    p    = strstr(buf, "=0x");
    reg1 = p ? strtol(p + 3, NULL, 16) : 0;
    p    = p ? p + 3 : buf;
    p    = strstr(p  , "=0x");
    reg2 = p ? strtol(p + 3, NULL, 16) : 0;
    return ((reg1 << 8) | (reg2 << 0)) & 0x1fff;
}

static void read_file(char *file, char *buf, int len)
{
    FILE *fp = fopen(file, "rb");
    int   n  = 0;
    if (fp) {
        n = fread(buf, 1, len-1, fp);
        fclose(fp);
    }
    if (n < 0) n = 0;
    buf[n] = '\0';
}

static int read_pmu_reg(int addr)
{
    int   reg;
    FILE *fp = NULL;
    char  buf[256] = {0};
    char *p;

    fp = fopen("/sys/class/axp/axp_reg", "wb");
    if (!fp) return -1;
    fprintf(fp, "0x%02x", addr);
    fclose(fp);

    fp = fopen("/sys/class/axp/axp_reg", "rb");
    if (!fp) return -1;
    fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    p   = strstr(buf, "=0x");
    reg = p ? strtol(p + 3, NULL, 16) : 0;
    return reg;
}

static int write_pmu_reg(int addr, int val)
{
    FILE *fp = NULL;
    int   ret;
    fp = fopen("/sys/class/axp/axp_reg", "wb");
    if (!fp) return -1;
    ret = fprintf(fp, "0x%02x%02x", addr, val);
    fclose(fp);
    return ret > 0 ? 0 : -1;
}

static int g_axp803_ocv_vol_tab[32] = {
    3133, 3273, 3344, 3414, 3485, 3555, 3590, 3608, 3626, 3643, 3661, 3696, 3731, 3766, 3802, 3819,
    3837, 3854, 3872, 3907, 3942, 3978, 4013, 4048, 4083, 4118, 4136, 4154, 4189, 4224, 4259, 4294
};

static void dump_batocv_percentage_table(void)
{
    int i;
    printf("axp803 ocv percentage table:\n");
    for (i=0; i<32; i++) {
        printf("pmu_bat_para%-2d = %-3d  ; %4d\n", i+1, read_pmu_reg(i+0xc0), g_axp803_ocv_vol_tab[i]);
    }
    printf("\n");
}

#define BAT_OCV_LOG_FILE "/sdcard/batocv.log"
static void start_record_ocv(void)
{
    signal(SIGINT , sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGKILL, sig_handler);
    unlink(BAT_OCV_LOG_FILE);

    FILE *fp = fopen(BAT_OCV_LOG_FILE, "ab");
    if (fp) {
        fprintf(fp, "bat_percent bat_ocv bat_voltage bat_current temp_bat temp_cpu temp_gpu max_cap col_meter ocv_percent col_percent bat_rdc\n");
        fclose(fp);
    }

    while (!g_exit) {
        char bat_percent[32], bat_voltage[32], bat_current[32], temp_bat[32], temp_cpu[32], temp_gpu[32];
        read_file("/sys/class/power_supply/battery/capacity"   , bat_percent, sizeof(bat_percent));
        read_file("/sys/class/power_supply/battery/voltage_now", bat_voltage, sizeof(bat_voltage));
        read_file("/sys/class/power_supply/battery/current_now", bat_current, sizeof(bat_current));
        read_file("/sys/class/power_supply/battery/temp" , temp_bat, sizeof(temp_bat));
        read_file("/sys/class/thermal/thermal_zone0/temp", temp_cpu, sizeof(temp_cpu));
        read_file("/sys/class/thermal/thermal_zone1/temp", temp_gpu, sizeof(temp_gpu));
        FILE *fp = fopen(BAT_OCV_LOG_FILE, "ab");
        if (fp) {
            fprintf(fp, "%-11d %-7d %-11d %-11d %-8d %-8d %-8d %-7d %-9d %-11d %-11d %-7d\n", atoi(bat_percent), read_battery_ocv(),
                atoi(bat_voltage)/1000, atoi(bat_current)/1000, atoi(temp_bat)/10, atoi(temp_cpu), atoi(temp_gpu),
                read_battery_maxcap(), read_battery_colmeter(), read_battery_ocv_percent(), read_battery_col_percent(), read_battery_rdc());
            fclose(fp);
        }
        sleep(10);
    }
}


#define BAT_LOW_VOL     3000
#define OUT_PARAMS_FMT  "pmu_bat_para%-2d  = %d\n"

static int cmp_int_item(const void *a, const void *b)
{
    return *(int*)a - *(int*)b;
}

static void generate_pmu_ocv_params(char *file)
{
    FILE *fpin = NULL;
//  FILE *fpout= NULL;
    int   lines= 0;
    int  *vlist= NULL;
    int   params[32] = { 0 };
    int   voltage, i, j;
    char  temp[256];

    fpin = fopen(file, "rb");
    if (!fpin) goto done;

    while (1) {
        int data = fgetc(fpin);
        if (data == EOF ) break;
        if (data == '\n') lines++;
    }
//  printf("total lines: %d\n", lines);

    vlist = (int*)malloc(sizeof(int) * lines);
    if (!vlist) goto done;

    //++ read voltage data
    fseek(fpin, 0, SEEK_SET);
    for (i=0; i<lines; i++) {
        fgets (temp, sizeof(temp), fpin);
        sscanf(temp, "%d %d", &j, &voltage);
        vlist[i] = voltage;
//      printf("%d\n", vlist[i]);
        if (feof(fpin)) break;
    }
    //-- read voltage data

    // sort vlist
    qsort(vlist, lines, sizeof(int), cmp_int_item);

    //++ calculate params for pmu
    for (i=0,j=0; i<32; i++) {
        if (g_axp803_ocv_vol_tab[i] < BAT_LOW_VOL) {
            params[i] = 0;
            continue;
        }

        for (; j<lines; j++) {
            if (g_axp803_ocv_vol_tab[i] < vlist[j]) {
                params[i] = 100 * j / lines;
                break;
            }
        }
    }
    //-- calculate params for pmu

//  fpout = fopen("pmu_params.fex", "wb");
    for (i=0; i<32; i++) {
        fprintf(stdout, OUT_PARAMS_FMT, i+1, params[i]);
//      fprintf(fpout , OUT_PARAMS_FMT, i+1, params[i]);
    }
//  fclose(fpout);

done:
    if (fpin ) fclose(fpin );
    if (vlist) free  (vlist);
}

#define ENABLE_OCVSOC_CALIBRATE     0
#define OUTPUT_CALIBRATE_LOG_STDOUT 1
#define OUTPUT_CALIBRATE_LOG_PROP   1
static void do_battery_calibration(void)
{
    char str[256];
    int calibrate_stage = 0;
    int ocv_voltage     = 0;
    int ocv_percent     = 0;
    int col_percent     = 0;
    int bat_current     = 0;
    int max_cap         = 0;
    int col_meter       = 0;
    int regval;

#if ENABLE_OCVSOC_CALIBRATE
    write_pmu_reg(0xB8, 0xE8);
#endif

    regval = read_pmu_reg(0xE0);
    regval&=~(1 << 7);
    write_pmu_reg(0xE0, regval);

    while (1) {
        read_file("/sys/class/power_supply/battery/current_now", str, sizeof(str));
        ocv_voltage = read_battery_ocv();
        ocv_percent = read_battery_ocv_percent();
        col_percent = read_battery_col_percent();
        bat_current = atoi(str) / 1000;
        max_cap     = read_battery_maxcap();
        col_meter   = read_battery_colmeter();
        snprintf(str, sizeof(str), "stage:%d ocvvol:%d ocvpct:%d colpct:%d batcur:%d maxcap:%d colmeter:%d",
            calibrate_stage, ocv_voltage, ocv_percent, col_percent, bat_current, max_cap, col_meter);
#if OUTPUT_CALIBRATE_LOG_STDOUT
        fprintf(stdout, "%s\n", str);
        fflush(stdout);
#endif
#if OUTPUT_CALIBRATE_LOG_PROP
        property_set("sys.bat_cal_log", str);
#endif
        switch (calibrate_stage) {
        case 0: // charge battery to 4.0V
            if (ocv_voltage >= 4000) {
                calibrate_stage = 1;
            }
            break;
        case 1: // discharge battery to OCV percent < 3%
            write_pmu_reg(0x35, 0x08);
            if (ocv_percent < 3) {
                calibrate_stage = 2;
            }
            break;
        case 2: // charge battery to full
            write_pmu_reg(0x35, 0x78);
            if (ocv_percent == 100 && col_percent == 100 && bat_current >= 0
               && (read_pmu_reg(0x01) & (1 << 6)) == 0
               && (read_pmu_reg(0xB8) & (1 << 4)) == 0
               && (read_pmu_reg(0xE0) & (1 << 7))) {
                calibrate_stage = 3;
            }
            break;
        case 3: // discharge battery to OCV percent < 3%
            write_pmu_reg(0x35, 0x08);
            if (ocv_percent < 3) {
                calibrate_stage = 4;
            }
            break;
        case 4: // charge battery to full
            write_pmu_reg(0x35, 0x78);
            if (ocv_percent == 100 && col_percent == 100 && bat_current >= 0
               && (read_pmu_reg(0x01) & (1 << 6)) == 0
               && (read_pmu_reg(0xB8) & (1 << 4)) == 0
               && (read_pmu_reg(0xE0) & (1 << 7))) {
                calibrate_stage = 5;
            }
            break;
        case 5: // battery calibration done
#if ENABLE_OCVSOC_CALIBRATE
            write_pmu_reg(0xB8, 0xE0);
#endif
#if OUTPUT_CALIBRATE_LOG_STDOUT
            fprintf(stdout, "battery calibration done !\n");
            fflush(stdout);
#endif
#if OUTPUT_CALIBRATE_LOG_PROP
            property_set("sys.bat_cal_log", "battery calibration done !");
#endif
            goto done;
        }
        sleep(10);
    }
done:
    return;
}

int main(int argc, char *argv[])
{
    if (argc <= 1 || strcmp(argv[1], "-r") == 0) { // record battery log
        start_record_ocv();
    } if (strcmp(argv[1], "-d") == 0) { // dump pmu ocv percent parameters table
        dump_batocv_percentage_table();
    } if (strcmp(argv[1], "-c") == 0) { // do battery calibration
        do_battery_calibration();
    } else { // generate ocv percent parameters table from battery log
        generate_pmu_ocv_params(argv[1]);
    }
    return 0;
}



