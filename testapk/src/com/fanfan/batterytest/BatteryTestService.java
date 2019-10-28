package com.fanfan.batterytest;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Binder;
import android.os.IBinder;
import android.os.PowerManager;
import android.util.Log;
import java.io.*;

public class BatteryTestService extends Service
{
    private static final String TAG = "BatteryTestService";
    private static final String BATTERY_LOG_FILE = "/sdcard/battery.log";

    private BatteryTestBinder        mBinder   = null;
    private BatteryTestEventListener mListener = null;
    private PowerManager.WakeLock    mWakeLock = null;
    private boolean  mStarted;
    private int      mCurTime;

    public interface BatteryTestEventListener {
        public void onBatteryTestEvent(String str);
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");

        // binder
        mBinder = new BatteryTestBinder();

        // wake lock
        PowerManager pm = (PowerManager)getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
        mWakeLock.setReferenceCounted(false);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        stopBatteryTest();
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind");
        return mBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand");
//      return super.onStartCommand(intent, flags, startId);
        return START_STICKY;
    }

    public class BatteryTestBinder extends Binder {
        public BatteryTestService getService(BatteryTestEventListener l) {
            mListener = l;
            return BatteryTestService.this;
        }
    }

    public boolean isTestStarted() {
        return mStarted;
    }

    public void startBatteryTest() {
        if (mStarted) return;
        try {
            mWakeLock.acquire();

            File file = new File(BATTERY_LOG_FILE);
            file.delete();
            file.createNewFile();
            appendTextToFile(BATTERY_LOG_FILE, " time    percent    voltage     current  bat_temp  cpu_temp  gpu_temp \n");
            appendTextToFile(BATTERY_LOG_FILE, "----------------------------------------------------------------------\n");

            mStarted = true;
            mCurTime = 0;
            mTimerHandler.post(mTimerRunnable);
        } catch (IOException e) { e.printStackTrace(); }
        showNotification(this, true, getString(R.string.battery_log_is_recording));
    }

    public void stopBatteryTest() {
        showNotification(this, false, null);
        mTimerHandler.removeCallbacks(mTimerRunnable);
        mStarted = false;

        if (mWakeLock.isHeld()) {
            mWakeLock.release();
        }
    }

    private static final int TIMER_DELAY = 10*1000;
    private Handler  mTimerHandler  = new Handler();
    private Runnable mTimerRunnable = new Runnable() {
        @Override
        public void run() {
            mTimerHandler.postDelayed(this, TIMER_DELAY);

            float vol = 0, cur = 0, tempb = 0, tempc = 0, tempg = 0;
            int   cap = 0;
            BufferedReader br = null;

            try {
                br = new BufferedReader(new InputStreamReader(new FileInputStream("/sys/class/power_supply/battery/capacity")));
                cap  = Integer.parseInt(br.readLine()) ;
                br.close();

                br = new BufferedReader(new InputStreamReader(new FileInputStream("/sys/class/power_supply/battery/voltage_now")));
                vol  = Float.parseFloat(br.readLine()) / 1000000;
                br.close();

                br = new BufferedReader(new InputStreamReader(new FileInputStream("/sys/class/power_supply/battery/current_now")));
                cur  = Float.parseFloat(br.readLine()) / 1000;
                br.close();

                br = new BufferedReader(new InputStreamReader(new FileInputStream("/sys/class/power_supply/battery/temp")));
                tempb = Float.parseFloat(br.readLine()) / 10;
                br.close();

                br = new BufferedReader(new InputStreamReader(new FileInputStream("/sys/class/thermal/thermal_zone0/temp")));
                tempc = Float.parseFloat(br.readLine()) / 1;
                br.close();

                br = new BufferedReader(new InputStreamReader(new FileInputStream("/sys/class/thermal/thermal_zone1/temp")));
                tempg = Float.parseFloat(br.readLine()) / 1;
                br.close();
            } catch (Exception e) { e.printStackTrace(); }

            mCurTime+= 1;
            String str = String.format("%5d  %8d%%  %8.3fV  %8dmA  %8.1f  %8.1f  %8.1f\n", mCurTime, cap, vol, (int)cur, tempb, tempc, tempg);
            appendTextToFile(BATTERY_LOG_FILE, str);

            if (mListener != null) {
                mListener.onBatteryTestEvent(str);
            }
        }
    };

    private static void appendTextToFile(String file, String text) {
        FileWriter writer = null;
        try {
            writer = new FileWriter(file, true);
            writer.write(text);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (writer != null) {
                    writer.close();
                }
            } catch (IOException e) { e.printStackTrace(); }
        }
    }

    private static final int NOTIFICATION_ID = 1;
    private static Notification        mNotification = new Notification();
    private static NotificationManager mNotifyManager= null;
    private static void showNotification(Context context, boolean show, String msg) {
        if (mNotifyManager == null) mNotifyManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        if (show) {
            PendingIntent pi    = PendingIntent.getActivity(context, 0, new Intent(context, BatteryTestActivity.class), 0);
            mNotification.flags = Notification.FLAG_ONGOING_EVENT;
            mNotification.icon  = R.drawable.ic_launcher;
            mNotification.setLatestEventInfo(context, context.getResources().getString(R.string.app_name), msg, pi);
            mNotifyManager.notify(NOTIFICATION_ID, mNotification);
        } else {
            mNotifyManager.cancel(NOTIFICATION_ID);
        }
    }
}


