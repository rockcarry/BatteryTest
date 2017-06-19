package com.fanfan.batterytest;

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
            appendTextToFile(BATTERY_LOG_FILE, "time      voltage   percent   current   temp      \n");
            appendTextToFile(BATTERY_LOG_FILE, "--------------------------------------------------\n");

            mStarted = true;
            mCurTime = 0;
            mTimerHandler.post(mTimerRunnable);
        } catch (IOException e) { e.printStackTrace(); }
    }

    public void stopBatteryTest() {
        mTimerHandler.removeCallbacks(mTimerRunnable);
        mStarted = false;

        if (mWakeLock.isHeld()) {
            mWakeLock.release();
        }
    }

    private static final int TIMER_DELAY = 30*1000;
    private Handler  mTimerHandler  = new Handler();
    private Runnable mTimerRunnable = new Runnable() {
        @Override
        public void run() {
            mTimerHandler.postDelayed(this, TIMER_DELAY);

            int cap  = Integer.parseInt(execCmdRetOut("cat /sys/class/power_supply/battery/capacity")); 
            int vol  = Integer.parseInt(execCmdRetOut("cat /sys/class/power_supply/battery/voltage_now")) / 1000;
            int cur  = Integer.parseInt(execCmdRetOut("cat /sys/class/power_supply/battery/current_now")) / 1000; 
            int temp = Integer.parseInt(execCmdRetOut("cat /sys/class/power_supply/battery/temp")) / 10; 
            mCurTime+= 1;
            String str = String.format("%-10d%-10d%-10d%-10d%-10d\n", mCurTime, vol, cap, cur, temp);
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

    private static String execCmdRetOut(String cmd) {
        try {
            Process mProcess = Runtime.getRuntime().exec(cmd);
            try {
                mProcess.waitFor();
            } catch (Exception e) {
                e.printStackTrace();
                return e.getMessage();
            }
            InputStream inputStream = mProcess.getInputStream();
            DataInputStream dataInputStream = new DataInputStream(inputStream);
            String out = dataInputStream.readLine();
            inputStream.close();
            dataInputStream.close();
            return out;
        } catch (Exception e) {
            e.printStackTrace();
            return e.getMessage();
        }
    }
}


