package com.fanfan.batterytest;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Typeface;
import android.os.BatteryManager;
import android.os.Bundle;
import android.os.IBinder;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.util.Log;
import java.util.*;

public class BatteryTestActivity extends Activity {
    private static final String TAG = "BatteryTestActivity";

    private BatteryTestService mBatServ = null;
    private ServiceConnection mBatServiceConn = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder serv) {
            mBatServ = ((BatteryTestService.BatteryTestBinder)serv).getService(mBatteryEventListener);
            mBtnStartStopTest.setText(mBatServ.isTestStarted() ? R.string.btn_stop_test : R.string.btn_start_test);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mBatServ = null;
        }
    };

    private Button   mBtnStartStopTest;
    private ListView mLstBatteryInfo;
    private TextView mTxtBatteryInfo;

    private ArrayAdapter      mListAdapter;
    private ArrayList<String> mBatInfoList;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        mBtnStartStopTest = (Button  )findViewById(R.id.btn_start_Stop);
        mLstBatteryInfo   = (ListView)findViewById(R.id.lst_bat_info  );
        mTxtBatteryInfo   = (TextView)findViewById(R.id.txt_bat_info  );
        mTxtBatteryInfo  .setTypeface(Typeface.MONOSPACE, Typeface.NORMAL);
        mBtnStartStopTest.setOnClickListener(mOnClickListener);

        mBatInfoList = new ArrayList<String>();
        mListAdapter = new ArrayAdapter(this, R.layout.item, mBatInfoList);
        mLstBatteryInfo.setAdapter(mListAdapter);

        // start record service
        Intent i = new Intent(BatteryTestActivity.this, BatteryTestService.class);
        startService(i);

        // bind record service
        bindService(i, mBatServiceConn, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");

        // unbind record service
        unbindService(mBatServiceConn);

        // stop record service
        Intent i = new Intent(BatteryTestActivity.this, BatteryTestService.class);
        stopService(i);

        super.onDestroy();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mBatServ != null) {
            mBtnStartStopTest.setText(mBatServ.isTestStarted() ?
                R.string.btn_stop_test : R.string.btn_start_test);
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_BATTERY_CHANGED);
        registerReceiver(mBroadcastReceiver, filter);
    }

    @Override
    public void onPause() {
        super.onPause();
        unregisterReceiver(mBroadcastReceiver);
    }

    private View.OnClickListener mOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
            case R.id.btn_start_Stop:
                if (mBatServ.isTestStarted()) {
                    mBtnStartStopTest.setText(R.string.btn_start_test);
                    mBatServ.stopBatteryTest();
                }
                else {
                    mBatInfoList.clear();
                    mBtnStartStopTest.setText(R.string.btn_stop_test);
                    mBatServ.startBatteryTest();
                }
                break;
            }
        }
    };

    private BatteryTestService.BatteryTestEventListener mBatteryEventListener = new BatteryTestService.BatteryTestEventListener() {
        @Override
        public void onBatteryTestEvent(String str) {
            mBatInfoList.add(str);
            mListAdapter.notifyDataSetChanged();
            mLstBatteryInfo.setSelection(mBatInfoList.size());
        }
    };

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_BATTERY_CHANGED)) {
                int     status      = intent.getIntExtra("status", 0);
                int     health      = intent.getIntExtra("health", 0);
                boolean present     = intent.getBooleanExtra("present", false);
                int     level       = intent.getIntExtra("level", 0);
                int     scale       = intent.getIntExtra("scale", 0);
                int     icon_small  = intent.getIntExtra("icon-small", 0);
                int     plugged     = intent.getIntExtra("plugged", 0);
                int     voltage     = intent.getIntExtra("voltage", 0);
                int     temperature = intent.getIntExtra("temperature", 0);
                String  technology  = intent.getStringExtra("technology");

                String statusString = "";
                switch (status) {
                case BatteryManager.BATTERY_STATUS_UNKNOWN:
                    statusString = "unknown";
                    break;
                case BatteryManager.BATTERY_STATUS_CHARGING:
                    statusString = "charging";
                    break;
                case BatteryManager.BATTERY_STATUS_DISCHARGING:
                    statusString = "discharging";
                    break;
                case BatteryManager.BATTERY_STATUS_NOT_CHARGING:
                    statusString = "not charging";
                    break;
                case BatteryManager.BATTERY_STATUS_FULL:
                    statusString = "full";
                    break;
                }

                String healthString = "";
                switch (health) {
                case BatteryManager.BATTERY_HEALTH_UNKNOWN:
                    healthString = "unknown";
                    break;
                case BatteryManager.BATTERY_HEALTH_GOOD:
                    healthString = "good";
                    break;
                case BatteryManager.BATTERY_HEALTH_OVERHEAT:
                    healthString = "overheat";
                    break;
                case BatteryManager.BATTERY_HEALTH_DEAD:
                    healthString = "dead";
                    break;
                case BatteryManager.BATTERY_HEALTH_OVER_VOLTAGE:
                    healthString = "voltage";
                    break;
                case BatteryManager.BATTERY_HEALTH_UNSPECIFIED_FAILURE:
                    healthString = "unspecified failure";
                    break;
                }

                String acString = "none";
                switch (plugged) {
                case BatteryManager.BATTERY_PLUGGED_AC:
                    acString = "ac";
                    break;
                case BatteryManager.BATTERY_PLUGGED_USB:
                    acString = "usb";
                    break;
                case 3:
                    acString = "usb-charger";
                    break;
                }

                String s = "BatteryTest tool v1.0.1\n\n"
                         + "battery info:\n"
                         + "status      - " + statusString + "\n"
                         + "health      - " + healthString + "\n"
                         + "present     - " + String.valueOf(present) + "\n"
                         + "level       - " + String.valueOf(level) + "\n"
                         + "scale       - " + String.valueOf(scale) + "\n"
                         + "plugged     - " + acString + "\n"
                         + "voltage     - " + String.valueOf(voltage) + "\n"
                         + "temperature - " + String.valueOf(temperature) + "\n"
                         + "technology  - " + technology + "\n\n"
                         + "Click START TEST button to record battery log.\n";
                mTxtBatteryInfo.setText(s);
            }
        }
    };
}



