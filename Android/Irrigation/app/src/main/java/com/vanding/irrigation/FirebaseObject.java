package com.vanding.irrigation;

import android.util.Log;

import com.google.firebase.database.ChildEventListener;
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

    private static boolean firebase_initialized = false;
    private static boolean firebase_listeners_initialized = false;

    private static IrrDeviceTelemetry[] tm;
    private static int nbrOfTmPoints;

    public FirebaseObject() {
    }

    public void LoadDeviceBasics() {
        if (!firebase_initialized) {
            firebase_initialized = true;

            dbAllDevicesReference.addListenerForSingleValueEvent(
                new ValueEventListener() {

                    @Override
                    public void onDataChange(DataSnapshot dataSnapshot) {
                        // Firebase guaratees that initial loading is done when onDataChange is called
                        dbLoadingState = dbFirebaseLoadingState.DEVICES_LOADING;
                        dbNbrOfDevices = (int) dataSnapshot.getChildrenCount();

                        int i = 0;
                        for (DataSnapshot postSnapshot : dataSnapshot.getChildren()) {
                            dbIrrDeviceStrings[i] = postSnapshot.getKey();
                            dbIrrDevice[i] = new IrrDevice();
                            dbIrrDevice[i].metadata = postSnapshot.child("metadata").getValue(IrrDeviceMetadata.class);
                            dbIrrDevice[i].state = postSnapshot.child("state").getValue(IrrDeviceState.class);
                            dbIrrDevice[i].settings = postSnapshot.child("settings").getValue(IrrDeviceSettings.class);
                            dbIrrDevice[i].telemetry_current = postSnapshot.child("telemetry_current").getValue(IrrDeviceTelemetry.class);
                            dbIrrDevice[i].nbrOfTelemetry = (int) postSnapshot.child("telemetry").getChildrenCount();
                            dbIrrDevice[i].nbrOfLogs = (int) postSnapshot.child("log").getChildrenCount();
                            // set the device run state:
//                            if (dbIrrDevice[i].state.deviceStatus == 0) {
//                                dbIrrDevice[i].overallStatus = IrrDevice.DeviceStatus.OK;
//                            } else if (dbIrrDevice[i].state.deviceStatus < 10) {
//                                dbIrrDevice[i].overallStatus = IrrDevice.DeviceStatus.WARNING;
//                            } else {
//                                dbIrrDevice[i].overallStatus = IrrDevice.DeviceStatus.FAULT;
//                            }
                            i++;
                        }

                        List<String> list = new ArrayList<String>();
                        for (i = 0; i < dbNbrOfDevices; i++) {
                            list.add(dbIrrDeviceStrings[i]);
                        }
                        dbLoadingState = dbFirebaseLoadingState.DEVICES_LOADED;
                        LoadEventListeners();
                    }

                    @Override
                    public void onCancelled(DatabaseError firebaseError) {
                    }
                }
            );
        }
    }

    public void LoadEventListeners() {

        if (firebase_initialized && !firebase_listeners_initialized) {
            firebase_listeners_initialized = true;


            for (int k=0; k< dbNbrOfDevices; k++) {
                dbDeviceReference[k] = FirebaseDatabase.getInstance().getReference().child("irrdevices").child(dbIrrDeviceStrings[k]);
                dbDeviceReference[k].addChildEventListener(
                    new ChildEventListener() {

                        @Override
                        public void onChildChanged(DataSnapshot dataSnapshot, String s) {
                            String dataKey = dataSnapshot.getKey();
                            String deviceKey = dataSnapshot.getRef().getParent().getKey();
                            int k = FindKeyInList(deviceKey);
                            Log.d("HomeX", "Key is " + dataKey);

                            if (dataKey.equals("metadata")) {
                                IrrDeviceMetadata meta = dataSnapshot.getValue(IrrDeviceMetadata.class);
                                if (meta != null) {
                                    dbIrrDevice[k].metadata = meta;
                                }
                            }

                            if (dataKey.equals("state")) {
                                IrrDeviceState devState = dataSnapshot.getValue(IrrDeviceState.class);
                                if (devState != null) {
                                    dbIrrDevice[k].state = devState;
                                }
                            }

                            if (dataKey.equals("settings")) {
                                IrrDeviceSettings devSettings = dataSnapshot.getValue(IrrDeviceSettings.class);
                                if (devSettings != null) {
                                    dbIrrDevice[k].settings = devSettings;
                                }
                            }

                            if (dataKey.equals("telemetry_current")) {
                                IrrDeviceTelemetry tele = dataSnapshot.getValue(IrrDeviceTelemetry.class);
                                if (tele != null) {
                                    dbIrrDevice[k].telemetry_current = tele;
                                }
                            }

                            // Firebase will fire two events: one for child creation, and one when timestamp is updated.
                            // https://stackoverflow.com/questions/37929003/firebase-ondatachanged-fire-twice-when-using-servervalue-timestamp-android?noredirect=1&lq=1
                            // This is dealt with through
                            if (dataKey.equals("telemetry")) {
                                String dKey = dataSnapshot.getRef().getParent().getKey();
                                if (dKey == dbIrrDeviceStrings[k]) {
                                    LoadDataSnapshop(dataSnapshot);
                                    Log.e("Count ", "" + dataSnapshot.getChildrenCount());
                                    if(LastTimestampIsOK(dataSnapshot)) {
                                        switch (dbIrrDevice[k].metadata.sensorType) {
                                            case DEVICE_TYPE_SOIL_STR:
                                                dbIrrDevice[k].xSeriesVcc.resetData(readAllData("Vcc"));
                                                dbIrrDevice[k].xSeriesPrimaryTm.resetData(readAllData( "Hum"));
                                                dbIrrDevice[k].xSeriesSecTm.resetData(readAllData( "vlvState"));
                                                //                            dbDeviceLoaded[k] = true;
                                                break;
                                            case DEVICE_TYPE_GAS_STR:
                                                dbIrrDevice[k].xSeriesVcc.resetData(readAllData( "Wifi"));
                                                dbIrrDevice[k].xSeriesPrimaryTm.resetData(readAllData( "cur_ppm"));
                                                dbIrrDevice[k].xSeriesSecTm.resetData(readAllData( "phase"));
                                                //                            dbDeviceLoaded[k] = true;
                                                break;
                                            case DEVICE_TYPE_HUMTEMP_STR:
                                                dbIrrDevice[k].xSeriesVcc.resetData(readAllData("Wifi"));
                                                dbIrrDevice[k].xSeriesPrimaryTm.resetData(readAllData( "Temp"));
                                                dbIrrDevice[k].xSeriesSecTm.resetData(readAllData( "Hum"));
                                                //                            dbDeviceLoaded[k] = true;
                                                break;

                                            default:
                                                break;
                                        }
                                    } else {
                                        Log.e("Event ignored. Timestamp is zero ", "");
                                    }
                                }
                            }
                        }

                        @Override
                        public void onChildAdded(DataSnapshot dataSnapshot, String s) {}

                        @Override
                        public void onChildRemoved(DataSnapshot dataSnapshot) { }

                        @Override
                        public void onChildMoved(DataSnapshot dataSnapshot, String s) { }

                        @Override
                        public void onCancelled(DatabaseError firebaseError) { }

                    }
                );
            }
        }
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
                    LoadDataSnapshop(dataSnapshot);
                    switch (dbIrrDevice[i].metadata.sensorType) {
                        case DEVICE_TYPE_SOIL_STR:
                            dbIrrDevice[i].xSeriesVcc.resetData(readAllData( "Vcc"));
                            dbIrrDevice[i].xSeriesPrimaryTm.resetData(readAllData( "Hum"));
                            dbIrrDevice[i].xSeriesSecTm.resetData(readAllData( "vlvState"));
                            dbDeviceLoaded[i] = true;
                            break;
                        case DEVICE_TYPE_GAS_STR:
                            dbIrrDevice[i].xSeriesVcc.resetData(readAllData( "Wifi"));
                            dbIrrDevice[i].xSeriesPrimaryTm.resetData(readAllData( "cur_ppm"));
                            dbIrrDevice[i].xSeriesSecTm.resetData(readAllData( "phase"));
                            dbDeviceLoaded[i] = true;
                            break;
                        case DEVICE_TYPE_HUMTEMP_STR:
                            dbIrrDevice[i].xSeriesVcc.resetData(readAllData("Wifi"));
                            dbIrrDevice[i].xSeriesPrimaryTm.resetData(readAllData( "Temp"));
                            dbIrrDevice[i].xSeriesSecTm.resetData(readAllData( "Hum"));
                            dbDeviceLoaded[i] = true;
                            break;
                        default:
                            break;
                    }
                }

                @Override
                public void onCancelled(DatabaseError firebaseError) {
                }
            }
        );
    }


    private void LoadDataSnapshop(DataSnapshot ds){
        // are we on telemetry level or on child level?
        DataSnapshot tmSnapshot;
        String parentKey = ds.getRef().getParent().getKey();
        if (parentKey.equals("irrdevices")) {
            tmSnapshot = ds.child("telemetry");
        }
        else if (ds.getKey().equals("telemetry")) {
            tmSnapshot = ds;
        } else {
            tmSnapshot = null;
            int except = 0/0;
        }
        nbrOfTmPoints = (int) tmSnapshot.getChildrenCount();
        tm=new IrrDeviceTelemetry[nbrOfTmPoints];
        int i=0;
        long last_timestamp = 0;
        IrrDeviceTelemetry cur_tm;
        for (DataSnapshot postSnapshot : tmSnapshot.getChildren()) {
            cur_tm = postSnapshot.getValue(IrrDeviceTelemetry.class);
            tm[i] = new IrrDeviceTelemetry(cur_tm);
            if (tm[i].timestamp == 0) {
                tm[i].timestamp = last_timestamp + 1;
            }
            last_timestamp = tm[i].timestamp;
            i++;
        }
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

    boolean LastTimestampIsOK(DataSnapshot dataSnapshot) {
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

        // Only way to find the last element is to iterate through all children.
        IrrDeviceTelemetry post = new IrrDeviceTelemetry();
        for (DataSnapshot postSnapshot : teledataSnapshot.getChildren()) {
            post = postSnapshot.getValue(IrrDeviceTelemetry.class);
        }
        if (post.timestamp != 0) {
            return true;
        } else {
            return false;
        }
    }


    private DataPoint[] readAllData(String parameterName)  {
        // are we on telemetry level or on child level?

        if (parameterName.equals("vlvState")) {
            DataPoint[] values = new DataPoint[nbrOfTmPoints*2];
            int i = 0;
            int j=0;  // counts the number of added points
            IrrDeviceTelemetry post;

            float prevValveState = 0;
            for (i=0;i<nbrOfTmPoints;i++) {
                float factor = 20;
                if (prevValveState != tm[i].vlvState) {  // insert extra point before real point to make a sharp edge on the graph
                    DataPoint x = new DataPoint(tm[i].timestamp -1, (float) factor * prevValveState);
                    values[i+j] = x;
                    j++;
                }
                DataPoint v = new DataPoint(tm[i].timestamp, (float) factor * tm[i].vlvState);
                values[i+j] = v;
                prevValveState = tm[i].vlvState;
            }
            //Log.d("data fetch", "i=" + Integer.toString(i) + "  nbrOfTmPoints=" + Integer.toString(nbrOfTmPoints*2));
            while (j < nbrOfTmPoints) {
                DataPoint x = new DataPoint(values[i+j-1].getX()+2, (float) 0.0);
                values[i+j] = x;
                j++;
            }
            //Log.d("data fetch", "i=" + Integer.toString(i) + "  nbrOfTmPoints=" + Integer.toString(nbrOfTmPoints*2));
            return values;

        } else {

            DataPoint[] values = new DataPoint[nbrOfTmPoints];
            int i = 0;
            IrrDeviceTelemetry post;

            switch (parameterName) {
                case "Vcc":
                    for (i=0;i<nbrOfTmPoints;i++) {
                        DataPoint v = new DataPoint(tm[i].timestamp, tm[i].Vcc);
                        values[i] = v;
                    }
                    break;
                case "Hum":
                    for (i=0;i<nbrOfTmPoints;i++) {
                        DataPoint v = new DataPoint(tm[i].timestamp, tm[i].Hum);
                        values[i] = v;
                    }
                    break;
                case "Temp":
                    for (i=0;i<nbrOfTmPoints;i++) {
                        DataPoint v = new DataPoint(tm[i].timestamp, tm[i].Temp);
                        values[i] = v;
                    }
                    break;
                case "cur_ppm":
                    for (i=0;i<nbrOfTmPoints;i++) {
                        DataPoint v = new DataPoint(tm[i].timestamp, tm[i].last_ppm);  // cur_ppm is the current measured CO value. last_ppm is the last CORRECT measurement
                        values[i] = v;
                    }
                    break;
                case "phase":
                    for (i=0;i<nbrOfTmPoints;i++) {
                        DataPoint v = new DataPoint(tm[i].timestamp, tm[i].phase);
                        values[i] = v;
                    }
                    break;
                case "Wifi":
                    for (i=0;i<nbrOfTmPoints;i++) {
                        DataPoint v = new DataPoint(tm[i].timestamp, tm[i].Wifi); // default: wifi
                        values[i] = v;
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
                        for (DataSnapshot postSnapshot : dataSnapshot.getChildren()) {
                            allLog.child(postSnapshot.getKey()).removeValue();
                        }
                    }
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
                        for (DataSnapshot postSnapshot : dataSnapshot.getChildren()) {
                            allTele.child(postSnapshot.getKey()).removeValue();
                        }
                    }
                }

                @Override
                public void onCancelled(DatabaseError databaseError) {
                    // ...
                }
            });
        };
    }


}
