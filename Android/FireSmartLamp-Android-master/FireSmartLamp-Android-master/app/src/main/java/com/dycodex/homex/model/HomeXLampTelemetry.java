package com.dycodex.homex.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

/**
 * Created by andri on 10/16/15.
 */
@JsonIgnoreProperties(ignoreUnknown = true)
public class HomeXLampTelemetry {
    public float ldr;
    public float wattage;
}
