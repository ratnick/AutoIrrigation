package IrrFire.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Created by andri on 10/16/15.
 */
@JsonIgnoreProperties(ignoreUnknown = true)
public class IrrDeviceTelemetry {
    @JsonProperty("Vcc")
    public float vcc;

    @JsonProperty("lastAnalogueReading")
    public int lastAnalogueReading;

    @JsonProperty("lastOpenTimestamp")
    public String lastOpenTimestamp;

    @JsonProperty("wifi")
    public int wifi;

    @JsonProperty("timestamp")
    public long timestampTelemetry;
}
