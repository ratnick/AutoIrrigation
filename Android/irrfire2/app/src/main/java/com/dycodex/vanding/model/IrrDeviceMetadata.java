package com.dycodex.vanding.model;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceMetadata {
    public String location;
    public String macAddr;
    public String deviceID;

    public IrrDeviceMetadata() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }

}
