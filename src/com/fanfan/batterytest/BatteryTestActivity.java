package com.fanfan.batterytest;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ArrayAdapter;
import java.io.*;
import java.util.*;

public class BatteryTestActivity extends Activity {
    private static final String BATTERY_LOG_FILE = "/sdcard/battery.log";

    private Button   mBtnStartStopTest;
    private ListView mLstBatteryInfo;
    private boolean  mStarted;
    private int      mCurTime;

    private ArrayAdapter      mListAdapter;
    private ArrayList<String> mBatInfoList;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        mBtnStartStopTest = (Button  )findViewById(R.id.btn_start_Stop);
        mLstBatteryInfo   = (ListView)findViewById(R.id.lst_bat_info  );
        mBtnStartStopTest.setOnClickListener(mOnClickListener);

        mBatInfoList = new ArrayList<String>();
        mListAdapter = new ArrayAdapter(this, R.layout.item, mBatInfoList);
        mLstBatteryInfo.setAdapter(mListAdapter);
    }

    private View.OnClickListener mOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
            case R.id.btn_start_Stop:
                if (!mStarted) {
                    mCurTime = 0;
                    mBatInfoList.clear();

                    try {
                        File file = new File(BATTERY_LOG_FILE);
                        file.delete();
                        file.createNewFile();
                        appendTextToFile(BATTERY_LOG_FILE, "time      voltage   percent   current   \n");
                        appendTextToFile(BATTERY_LOG_FILE, "----------------------------------------\n");
                    } catch (IOException e) { e.printStackTrace(); }

                    mBtnStartStopTest.setText(R.string.btn_stop_test);
                    mTimerHandler.post(mTimerRunnable);
                }
                else {
                    mBtnStartStopTest.setText(R.string.btn_start_test);
                    mTimerHandler.removeCallbacks(mTimerRunnable);
                }
                mStarted = !mStarted;
                break;
            }
        }
    };

    private static final int TIMER_DELAY = 60*1000;
    private Handler  mTimerHandler  = new Handler();
    private Runnable mTimerRunnable = new Runnable() {
        @Override
        public void run() {
            mTimerHandler.postDelayed(this, TIMER_DELAY);

            int cap  = Integer.parseInt(execCmdRetOut("cat /sys/class/power_supply/battery/capacity")); 
            int vol  = Integer.parseInt(execCmdRetOut("cat /sys/class/power_supply/battery/voltage_now")) / 1000;
            int cur  = Integer.parseInt(execCmdRetOut("cat /sys/class/power_supply/battery/current_now")) / 1000; 
            mCurTime+= 1;
            String str = String.format("%-10d%-10d%-10d%-10d\n", mCurTime, vol, cap, cur);
            mBatInfoList.add(str);
            mListAdapter.notifyDataSetChanged();
            mLstBatteryInfo.setSelection(mBatInfoList.size());
            appendTextToFile(BATTERY_LOG_FILE, str);
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

    public static String execCmdRetOut(String cmd) {
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



