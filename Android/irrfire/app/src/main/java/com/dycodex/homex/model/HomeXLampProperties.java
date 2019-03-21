package com.dycodex.homex.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

/**
 * Created by andri on 10/17/15.
 */
@JsonIgnoreProperties(ignoreUnknown = true)
public class HomeXLampProperties {
    public String deviceID;
    public String deviceLocation;
    public float latitude;
    public float longitude;
}
