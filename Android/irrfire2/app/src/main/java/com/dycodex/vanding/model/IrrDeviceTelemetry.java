package com.dycodex.vanding.model;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceTelemetry {
    public float Vcc;
    public float humidity;
    public int lastAnalogueReading;
    public String lastOpenTimestamp;
    public long timestamp;
    public int valveState;
    public int wifi;

    public IrrDeviceTelemetry() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }
}
