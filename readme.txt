ȫ־ƽ̨��ز��Ժ�У׼����

ʹ�÷�����

1. �Ƚ�����������

2. ִ�� batterytool.sh �ռ���طŵ�����
   adb remount
   adb push batterytool.sh /system/bin
   adb shell chmod 755 /system/bin/batterytool.sh
   adb shell
   batterytool.sh &
   exit

3. �����������ŵ�ֱ���ػ�

4. �ӻ���ȡ�� battery.log ��طŵ�����
   adb pull /sdcard/battery.log

5. ʹ�� batterytool.exe ���ߴ������ɵ�ز���
   �� battery.log �� batterytool.exe ����ͬһĿ¼˫��ִ�� batterytool.exe ����

6. �����ɵ� pmu �������õ� sys_config.fex ��

7. Ҳ����ʹ�� BatteryTest.apk ���в���

