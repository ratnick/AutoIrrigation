package com.vanding.datamodel;

import com.google.firebase.database.IgnoreExtraProperties;

@IgnoreExtraProperties
public class IrrDeviceTelemetry {

    // generic
    public float Vcc;
    public int Wifi;
    public long timestamp;

    // irrigation device
    public int lastAnalog;
    public String lastOpen;
    public float Hum;
    public int vlvState;

    // DHT11 humidity and temperature
    public float Temp;

    // gas sensor
    public float cur_ppm;
    public float last_ppm;
    public float sens_val;
    public int phase;

    public IrrDeviceTelemetry() {}
    public IrrDeviceTelemetry(IrrDeviceTelemetry tm) {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
        Vcc = tm.Vcc;
        Wifi = tm.Wifi;
        timestamp = tm.timestamp;

        // irrigation device
        lastAnalog = tm.lastAnalog;
        lastOpen = tm.lastOpen;
        Hum = tm.Hum;
        vlvState = tm.vlvState;

        // DHT11 humidity and temperature
        Temp = tm.Temp;

        // gas sensor
        cur_ppm = tm.cur_ppm;
        last_ppm = tm.last_ppm;
        sens_val = tm.sens_val;
        phase = tm.phase;
    }
}
