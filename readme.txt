全志平台电池测试和校准工具

使用方法：

1. 先将机器充满电

2. 执行 batterytool.sh 收集电池放电数据
   adb remount
   adb push batterytool.sh /system/bin
   adb shell chmod 755 /system/bin/batterytool.sh
   adb shell
   batterytool.sh &
   exit

3. 将机器正常放电直至关机

4. 从机器取出 battery.log 电池放电数据
   adb pull /sdcard/battery.log

5. 使用 batterytool.exe 工具处理生成电池参数
   将 battery.log 与 batterytool.exe 放在同一目录双击执行 batterytool.exe 即可

6. 将生成的 pmu 参数配置到 sys_config.fex 中

7. 也可以使用 BatteryTest.apk 进行测试

