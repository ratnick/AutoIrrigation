package com.vanding.datamodel;

import com.google.firebase.database.IgnoreExtraProperties;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

@IgnoreExtraProperties
public class IrrDevice {
    public enum DeviceStatus {OK, WARNING, FAULT};
    public DeviceStatus overallStatus;
    public IrrDeviceMetadata metadata;
    public IrrDeviceState state;
    public IrrDeviceSettings settings;
    public IrrDeviceTelemetry telemetry_current;
    public IrrDeviceTelemetry telemetry;
    public LineGraphSeries<DataPoint> xSeriesVcc;
    public LineGraphSeries<DataPoint> xSeriesValveState;
    public LineGraphSeries<DataPoint> xSeriesHumidity;
    public int nbrOfTelemetry;
    public int nbrOfLogs;

    public IrrDevice() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
        this.metadata = new IrrDeviceMetadata();
        this.state = new IrrDeviceState();
        this.settings = new IrrDeviceSettings();
        this.telemetry_current = new IrrDeviceTelemetry();
        xSeriesVcc = new LineGraphSeries();
        xSeriesValveState =new LineGraphSeries();
        xSeriesHumidity = new LineGraphSeries();
        nbrOfTelemetry = -1;
        nbrOfLogs = -1;

    }


}
