package com.ratnick.vanding.model;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceSettings {

    public boolean Updated;
    public int vlvOpen;
    public int totSlp;
    public int vlvSoak;
    public int humLim;
    public int loopSec;
    public String wakeTime;
    public boolean slpEnabl;
    public String runMode;

    public IrrDeviceSettings() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
