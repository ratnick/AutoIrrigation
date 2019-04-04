package com.dycodex.vanding.model;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceLog {

    public long ts;
    public String txt;

    public IrrDeviceLog() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
