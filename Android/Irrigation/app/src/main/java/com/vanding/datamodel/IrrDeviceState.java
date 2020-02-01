package com.vanding.datamodel;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceState {

    public String SSID;
    public boolean runOnce;
    public int slpCurCyc;
    public int slpDura;
    public int slpMxCyc;
    public long ts;
    public int deviceStatus;

    public IrrDeviceState() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
