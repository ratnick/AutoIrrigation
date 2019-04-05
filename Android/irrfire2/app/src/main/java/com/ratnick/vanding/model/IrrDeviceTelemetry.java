package com.ratnick.vanding.model;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceTelemetry {

    public float Vcc;
    public float Hum;
    public int lastAnalog;
    public String lastOpen;
    public long timestamp;
    public int vlvState;
    public int Wifi;

    public IrrDeviceTelemetry() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
