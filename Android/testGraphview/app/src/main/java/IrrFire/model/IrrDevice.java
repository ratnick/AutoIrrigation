package IrrFire.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

import org.parceler.Parcel;

/**
 * Created by andri on 9/8/15.
 */
@Parcel
@JsonIgnoreProperties(ignoreUnknown=true)
public class IrrDevice {
    public IrrDeviceMetadata metadata;
    public IrrDeviceState state;
    public IrrDeviceTelemetry telemetry_current;

}
