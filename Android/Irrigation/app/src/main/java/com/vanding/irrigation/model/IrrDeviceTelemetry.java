package com.vanding.irrigation.model;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceTelemetry {
//    @JsonProperty("Vcc")
    public float vcc;

//    @JsonProperty("lastAnalogueReading")
    public int lastAnalogueReading;

//    @JsonProperty("lastOpenTimestamp")
    public String lastOpenTimestamp;

//    @JsonProperty("wifi")
    public int wifi;

//    @JsonProperty("timestamp")
    public long timestampTelemetry;


    public IrrDeviceTelemetry() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
    }

}
