package com.vanding.irrigation;

import android.util.Log;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;
import com.jjoe64.graphview.series.DataPoint;
import com.vanding.datamodel.IrrDevice;
import com.vanding.datamodel.IrrDeviceLog;
import com.vanding.datamodel.IrrDeviceMetadata;
import com.vanding.datamodel.IrrDeviceSettings;
import com.vanding.datamodel.IrrDeviceState;
import com.vanding.datamodel.IrrDeviceTelemetry;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import static com.vanding.irrigation.db.*;

public class FirebaseObject implements Serializable {

    // Firebase setup
    private static final String FB_PROJECT_ID = "irrfire";
    private static final String FB_URL = "https://" + FB_PROJECT_ID + ".firebaseio.com";
    private static final String FB_PATH = "irrdevices";
    private static final String INITIAL_DEVICE_ID = "84:F3:EB: C:38:7A";

    public FirebaseObject() {
    }

    public void LoadDeviceBasics() {
        dbAllDevicesReference.addListenerForSingleValueEvent(
            new ValueEventListener() {

                @Override
                public void onDataChange(DataSnapshot dataSnapshot) {
                    dbLoadingState = dbFirebaseLoadingState.DEVICES_LOADING;
                    dbNbrOfDevices = (int) dataSnapshot.getChildrenCount();

                    int i=0;
                    for (DataSnapshot postSnapshot : dataSnapshot.getChildren()) {
                        dbIrrDeviceStrings[i] = postSnapshot.getKey();
                        dbIrrDevice[i] = new IrrDevice();
                        dbIrrDevice[i].metadata = postSnapshot.child("metadata").getValue(IrrDeviceMetadata.class);
                        dbIrrDevice[i].state = postSnapshot.child("state").getValue(IrrDeviceState.class);
                        dbIrrDevice[i].settings = postSnapshot.child("settings").getValue(IrrDeviceSettings.class);
                        dbIrrDevice[i].telemetry_current = postSnapshot.child("telemetry_current").getValue(IrrDeviceTelemetry.class);
                        dbIrrDevice[i].nbrOfTelemetry = 0;  // nnr postSnapshot.child("telemetry").getChildrenCount();
                        dbIrrDevice[i].nbrOfLogs = 0;       // nnr postSnapshot.child("log").getChildrenCount();
                        // set the device run state:
                        if (dbIrrDevice[i].telemetry_current.Vcc < 3.6) {
                            dbIrrDevice[i].overallStatus = IrrDevice.DeviceStatus.FAULT;
                        } else if (dbIrrDevice[i].telemetry_current.Vcc < 3.8) {
                            dbIrrDevice[i].overallStatus = IrrDevice.DeviceStatus.WARNING;
                        } else {
                            dbIrrDevice[i].overallStatus = IrrDevice.DeviceStatus.OK;
                        }
                        i++;
                    }

                    List<String> list = new ArrayList<String>();
                    for(i=0; i< dbNbrOfDevices; i++) {
                        list.add(dbIrrDeviceStrings[i]);
                    }
                    dbLoadingState = dbFirebaseLoadingState.DEVICES_LOADED;
                }

                @Override
                public void onCancelled(DatabaseError firebaseError) {
                }
            }
        );

    }

    public void LoadDeviceTelemetry(int _k) {

        final int k = _k;
        dbDeviceReference[k] = FirebaseDatabase.getInstance().getReference().child("irrdevices").child(dbIrrDeviceStrings[k]);
        dbDeviceReference[k].addListenerForSingleValueEvent(
            new ValueEventListener() {

                @Override
                // Called once on creation + every time a change (incl children) happen (https://www.learnhowtoprogram.com/android/data-persistence/firebase-reading-data-and-event-listeners)
                public void onDataChange(DataSnapshot dataSnapshot) {
                    String dsKey = dataSnapshot.getKey();
                    int i = FindKeyInList(dsKey);
                    if (i == -1) {
                        i = dbIrrDeviceStrings.length;
                        //                    dbIrrDevice[i] = new IrrDevice();
                    }
                    dbIrrDevice[i].xSeriesVcc.resetData(readAllData(dataSnapshot, "Vcc"));
                    dbIrrDevice[i].xSeriesHumidity.resetData(readAllData(dataSnapshot, "Hum"));
                    dbIrrDevice[i].xSeriesValveState.resetData(readAllData(dataSnapshot, "vlvState"));
                    dbDeviceLoaded[i] = true;
                }

                @Override
                public void onCancelled(DatabaseError firebaseError) {
                }
            }
        );
    }

    private int FindKeyInList(String key) {  //return -1 if not found
        int i = 0;
        for(String str: dbIrrDeviceStrings) {
            if(str.trim().contains(key))
                return i;
            i++;
        }
        return (int) -1;
    }

    private DataPoint[] readAllData(DataSnapshot dataSnapshot, String parameterName)  {
        // are we on telemetry level or on child level?
        DataSnapshot teledataSnapshot;
        String parentKey = dataSnapshot.getRef().getParent().getKey();
        if (parentKey.equals("irrdevices")) {
            teledataSnapshot = dataSnapshot.child("telemetry");
        }
        else if (dataSnapshot.getKey().equals("telemetry")) {
            teledataSnapshot = dataSnapshot;
        } else {
            teledataSnapshot = null;
            int except = 0/0;
        }
        int count = (int) teledataSnapshot.getChildrenCount();

        if (parameterName.equals("vlvState")) {
            DataPoint[] values = new DataPoint[count*2];
            int i = 0;
            IrrDeviceTelemetry post;

            float prevValveState = 0;
            for (DataSnapshot postSnapshot : teledataSnapshot.getChildren()) {
                float factor = 20;
                post = postSnapshot.getValue(IrrDeviceTelemetry.class);
                if (prevValveState != post.vlvState) {  // insert extra point before real point to make a sharp edge on the graph
                    DataPoint x = new DataPoint(post.timestamp -1, (float) factor * prevValveState);
                    values[i++] = x;
                }
                DataPoint v = new DataPoint(post.timestamp, (float) factor * post.vlvState);
                values[i++] = v;
                prevValveState = post.vlvState;
            }
            Log.d("data fetch", "i=" + Integer.toString(i) + "  count=" + Integer.toString(count*2));
            while (i < count*2) {
                DataPoint x = new DataPoint(values[i-1].getX()+2, (float) 0.0);
                values[i++] = x;
            }
            Log.d("data fetch", "i=" + Integer.toString(i) + "  count=" + Integer.toString(count*2));
            return values;

        } else {

            DataPoint[] values = new DataPoint[count];
            int i = 0;
            IrrDeviceTelemetry post;

            switch (parameterName) {
                case "Vcc":
                    for (DataSnapshot postSnapshot : teledataSnapshot.getChildren()) {
                        post = postSnapshot.getValue(IrrDeviceTelemetry.class);
                        DataPoint v = new DataPoint(post.timestamp, post.Vcc);
                        values[i++] = v;
                    }
                    break;
                case "Hum":
                    for (DataSnapshot postSnapshot : teledataSnapshot.getChildren()) {
                        post = postSnapshot.getValue(IrrDeviceTelemetry.class);
                        DataPoint v = new DataPoint(post.timestamp, (float) post.Hum);
                        values[i++] = v;
                    }
                    break;
                default:
                    int except = 0/0;
            }
            return values;
        }

    }

    public void PurgeLogAndTeleData(int deviceNbr, int keepTeleDays, int keepLogDays) {
        if (keepLogDays >= 0) {
            Date logDateFilter = new Date(System.currentTimeMillis() - (keepLogDays * 1000 * 60 * 60 * 24));
            final DatabaseReference allLog = dbDeviceReference[deviceNbr].child("log");
            allLog.orderByChild("timestamp").endAt(logDateFilter.getTime()).addListenerForSingleValueEvent(new ValueEventListener() {
                //allLog.addListenerForSingleValueEvent(new ValueEventListener() {

                @Override
                public void onDataChange(DataSnapshot dataSnapshot) {
                    int count = (int) dataSnapshot.getChildrenCount();
                    if (count > 0) {
                        IrrDeviceLog post;
                        for (DataSnapshot postSnapshot : dataSnapshot.getChildren()) {
                            //post = postSnapshot.getValue(IrrDeviceLog.class);
                            //System.out.println(" timestamp " + post.timestamp + "  Vcc=" + post.Vcc);
                            //allLog.child(postSnapshot.getKey()).removeValue();
                        }
                    }
                    //showToast("Deleted " + count + " log posts");
                }

                @Override
                public void onCancelled(DatabaseError databaseError) {
                    // ...
                }
            });
        };

        if (keepTeleDays >= 0) {
            Date teleDateFilter = new Date(System.currentTimeMillis() - (keepTeleDays * 1000 * 60 * 60 * 24));
            final DatabaseReference allTele = dbDeviceReference[deviceNbr].child("telemetry");
            allTele.orderByChild("timestamp").endAt(teleDateFilter.getTime()).addListenerForSingleValueEvent(new ValueEventListener() {

                @Override
                public void onDataChange(DataSnapshot dataSnapshot) {
                    int count = (int) dataSnapshot.getChildrenCount();
                    if (count > 0) {
                        IrrDeviceLog post;
                        for (DataSnapshot postSnapshot : dataSnapshot.getChildren()) {
                            //allTele.child(postSnapshot.getKey()).removeValue();
                        }
                    }
                    //showToast("Deleted " + count + " telemetry posts");
                }

                @Override
                public void onCancelled(DatabaseError databaseError) {
                    // ...
                }
            });
        };
    }


}
