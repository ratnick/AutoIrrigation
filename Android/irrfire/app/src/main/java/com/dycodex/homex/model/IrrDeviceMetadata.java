package com.dycodex.homex.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Created by andri on 10/17/15.
 */
@JsonIgnoreProperties(ignoreUnknown = true)
public class IrrDeviceMetadata {
    @JsonProperty("location")
    public String location;
    @JsonProperty("macAddr")
    public String macAddr;
    @JsonProperty("deviceID")
    public String deviceID;
}
