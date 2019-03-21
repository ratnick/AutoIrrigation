package com.dycodex.homex.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Created by andri on 10/15/15.
 */
@JsonIgnoreProperties(ignoreUnknown=true)
public class IrrDeviceState {
    @JsonProperty("currentSleepCycle")
    public int currentSleepCycle;

    @JsonProperty("maxSlpCycles")
    public int maxSlpCycles;

    @JsonProperty("measureMode")
    public String measureMode;

    @JsonProperty("secsToSleep")
    public int secsToSleep;

    @JsonProperty("useDeepSleep")
    public boolean useDeepSleep;

    @JsonProperty("wifiSSID")
    public String wifiSSID;

    @JsonProperty("timestamp")
    public long timestampState;

}
