package com.dycodex.vanding.model;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceState {
    public String SSID;
    public boolean UserUpdate;
    public int curSleepCycle;
    public int maxSlpCycles;
    public String mode;
    public int openDur;
    public boolean runOnce;
    public int secsToSleep;
    public int soakTime;
    public int mainLoopDelay;
    public long timestamp;
    public boolean deepSleepEnabled;

    public IrrDeviceState() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
