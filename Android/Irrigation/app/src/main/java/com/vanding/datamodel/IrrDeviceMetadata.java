package com.vanding.datamodel;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceMetadata {

    public String device;
    public String hw;
    public String loc;
    public String mac;
    public String sensorType;
    public String sw;

    public IrrDeviceMetadata() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }

}
