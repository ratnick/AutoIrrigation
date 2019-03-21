package com.dycodex.homex.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Created by andri on 10/15/15.
 */
@JsonIgnoreProperties(ignoreUnknown=true)
public class HomeXLampParameter {
    @JsonProperty("state")
    public boolean state;

    @JsonProperty("lightSensorEnabled")
    public boolean lightSensorEnabled;

    @JsonProperty("dimLevel")
    public int dimLevel;
}
