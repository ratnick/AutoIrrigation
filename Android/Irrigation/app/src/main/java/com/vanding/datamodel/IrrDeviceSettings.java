package com.vanding.datamodel;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceSettings {

    public boolean Updated;
    public int db;
    public int humLim;
    public int loopSec;
    public float offsetPrim1;
    public float offsetPrim2;
    public float offsetSec1;
    public String pauseTime;
    public String runMode;
    public boolean slpEnabl;
    public int totSlp;
    public float vccAdj;
    public float vccMin;
    public int vlvOpen;
    public int vlvSoak;
    public String wakeTime0;
    public String wakeTime1;
    public String wakeTime2;
    public String wakeTime3;

    public IrrDeviceSettings() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
