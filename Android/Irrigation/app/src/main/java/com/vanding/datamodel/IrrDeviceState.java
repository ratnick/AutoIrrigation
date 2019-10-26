package com.vanding.datamodel;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceState {

    public String SSID;
    public int slpCurCyc;
    public int slpMxCyc;
    public boolean runOnce;
    public long ts;
    public int slpDura;

    public IrrDeviceState() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
