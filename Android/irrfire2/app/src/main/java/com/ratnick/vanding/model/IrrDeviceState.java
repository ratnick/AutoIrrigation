package com.ratnick.vanding.model;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceState {

    public String SSID;
    public int slpCurCyc;
    public int slpMxCyc;
    public String mode;
    public boolean runOnce;
    public long ts;
    public int slpDura;

    public IrrDeviceState() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
