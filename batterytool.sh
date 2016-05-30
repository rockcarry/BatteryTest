#!/system/bin/busybox sh

CHECK_SLEEP=60
CUR_TIME=0
CUR_VOLTAGE=0
CUR_PERCENT=0
CUR_CURRENT=0

echo "time    voltage percent current " > /sdcard/battery.log
echo "--------------------------------" >>/sdcard/battery.log

while true
do
    CUR_VOLTAGE=`cat /sys/class/power_supply/battery/voltage_now`
    CUR_PERCENT=`cat /sys/class/power_supply/battery/capacity`
    CUR_CURRENT=`cat /sys/class/power_supply/battery/current_now`
    let CUR_VOLTAGE=$CUR_VOLTAGE/1000
    let CUR_CURRENT=$CUR_CURRENT/1000

    OUTPUT=`busybox printf "%-8s%-8s%-8s%-8s" $CUR_TIME $CUR_VOLTAGE $CUR_PERCENT $CUR_CURRENT`
    echo "$OUTPUT" >> /sdcard/battery.log && sync

    let CUR_TIME=$CUR_TIME+1    
    sleep $CHECK_SLEEP
done

