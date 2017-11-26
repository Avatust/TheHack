package com.example.thehack;

/**
 * Created by martin on 11/26/17.
 */

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.View;

//import com.google.gson.Gson;
import com.google.gson.Gson;
import com.movesense.mds.Mds;
import com.movesense.mds.MdsConnectionListener;
import com.movesense.mds.MdsException;
import com.movesense.mds.MdsNotificationListener;
import com.movesense.mds.MdsSubscription;/*
import com.polidea.rxandroidble.RxBleClient;
import com.polidea.rxandroidble.RxBleDevice;
import com.polidea.rxandroidble.scan.ScanSettings;

import rx.Subscription;*/

public class Connectivity {
    String LOG_TAG = "tg";
    // MDS
    private Mds mMds;
    public static final String URI_CONNECTEDDEVICES = "suunto://MDS/ConnectedDevices";
    public static final String URI_EVENTLISTENER = "suunto://MDS/EventListener";
    public static final String SCHEME_PREFIX = "suunto://";

    // Sensor subscription
    static private String SUBSCRIPTION_CONTRACT_URI = "/Sample/JumpCounter/LastJumpHeight";//"/Meas/Acc/13";//
    private MdsSubscription mdsSubscription;
    private String subscribedDeviceSerial;

    public void initMds(Context context) {
        mMds = Mds.builder().build(context);
    }


    public void connectTo(String macAddr) {
        mMds.connect(macAddr, new MdsConnectionListener() {

            @Override
            public void onConnect(String s) {

                Log.d(LOG_TAG, "onConnect:" + s);
            }

            @Override
            public void onConnectionComplete(String macAddress, String serial) {

            }

            @Override
            public void onError(MdsException e) {
                Log.e(LOG_TAG, "onError:" + e);

            }

            @Override
            public void onDisconnect(String bleAddress) {

            }
        });
    }

    public void subscribe(String serialNumber, Context context) {
        StringBuilder sb = new StringBuilder();
        String strContract = sb.append("{\"Uri\": \"").append(serialNumber).append(SUBSCRIPTION_CONTRACT_URI).append("\"}").toString();
        Log.d("qwerr", strContract);

        subscribedDeviceSerial = serialNumber;

        mdsSubscription = mMds.builder().build(context).subscribe(URI_EVENTLISTENER,
                strContract, new MdsNotificationListener() {
                    @Override
                    public void onNotification(String data) {
                        Log.d(LOG_TAG, "onNotification(): " + data);

                        int impactForce = Integer.parseInt(
                                data.substring(data.indexOf("\"Body\": ")+8, data.indexOf(", \"Uri\"")));

                    }

                    @Override
                    public void onError(MdsException error) {
                        Log.e(LOG_TAG, "subscription onError(): ", error);
                        unsubscribe();
                    }
                });

    }

    public void unsubscribe() {
        if (mdsSubscription != null) {
            mdsSubscription.unsubscribe();
            mdsSubscription = null;
        }

        subscribedDeviceSerial = null;

    }
}
