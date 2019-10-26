package com.vanding.datamodel;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceSettings {

    public boolean Updated;
    public int vlvOpen;
    public int totSlp;
    public int vlvSoak;
    public int humLim;
    public int db;
    public int loopSec;
    public String wakeTime0;
    public String wakeTime1;
    public String wakeTime2;
    public String wakeTime3;
    public String pauseTime;
    public boolean slpEnabl;
    public String runMode;

    public IrrDeviceSettings() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
