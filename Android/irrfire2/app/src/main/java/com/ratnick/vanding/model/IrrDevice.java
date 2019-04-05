package com.ratnick.vanding.model;

import com.google.firebase.database.IgnoreExtraProperties;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

@IgnoreExtraProperties
public class IrrDevice {
    public IrrDeviceMetadata metadata;
    public IrrDeviceState state;
    public IrrDeviceSettings settings;
    public IrrDeviceTelemetry telemetry_current;
    public IrrDeviceTelemetry telemetry;
    public LineGraphSeries<DataPoint> xSeriesVcc;
    public LineGraphSeries<DataPoint> xSeriesValveState;
    public LineGraphSeries<DataPoint> xSerieslastAnalogueReading;

    public IrrDevice() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
        this.metadata = new IrrDeviceMetadata();
        this.state = new IrrDeviceState();
        this.settings = new IrrDeviceSettings();
        this.telemetry_current = new IrrDeviceTelemetry();
        xSeriesVcc = new LineGraphSeries();
        xSeriesValveState =new LineGraphSeries();
        xSerieslastAnalogueReading = new LineGraphSeries();

    }


}
