package com.vanding.datamodel;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceTelemetry {

    // generic
    public float Vcc;
    public int W;  // Wifi
    public long timestamp;

    // irrigation device
    public int lastAnalog;
    public String lastOpen;
    public float H;  // Humidity

    // Pump controller
    public int state;

    // DHT11 humidity and temperature
    public float T;  // Temperature

    // Distance sensor
    public float Dist;

    // gas sensor
    public float cur_ppm;
    public float last_ppm;
    public float sens_val;
    public int phase;

    public IrrDeviceTelemetry() {}
    public IrrDeviceTelemetry(IrrDeviceTelemetry tm) {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
        Vcc = tm.Vcc;
        W = tm.W;
        timestamp = tm.timestamp;

        // irrigation device
        lastAnalog = tm.lastAnalog;
        lastOpen = tm.lastOpen;
        H = tm.H;
        state = tm.state;

        // DHT11 humidity and temperature
        T = tm.T;

        // Distance sensor
        Dist = tm.Dist;

        // gas sensor
        cur_ppm = tm.cur_ppm;
        last_ppm = tm.last_ppm;
        sens_val = tm.sens_val;
        phase = tm.phase;

    }
}
