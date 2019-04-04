package com.dycodex.vanding.model;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceMetadata {

    public String loc;
    public String mac;
    public String device;
    //NEW public String hw;
    //NEW public String sw;

    public IrrDeviceMetadata() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }

}
