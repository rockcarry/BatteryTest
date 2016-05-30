package com.fanfan.batterytest;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.view.View;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ArrayAdapter;
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
}



