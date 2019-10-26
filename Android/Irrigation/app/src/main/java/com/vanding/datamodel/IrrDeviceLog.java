package com.vanding.datamodel;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceLog {

    public String txt;

    public IrrDeviceLog() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
