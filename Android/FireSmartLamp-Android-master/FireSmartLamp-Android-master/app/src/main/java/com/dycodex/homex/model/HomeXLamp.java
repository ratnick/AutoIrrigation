package com.dycodex.homex.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import org.parceler.Parcel;

/**
 * Created by andri on 9/8/15.
 */
@Parcel
@JsonIgnoreProperties(ignoreUnknown=true)
public class HomeXLamp {
    public HomeXLampMetadata metadata;
    public HomeXLampParameter parameters;
}
