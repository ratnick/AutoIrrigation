package com.vanding.datamodel;

import com.google.firebase.database.IgnoreExtraProperties;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

@IgnoreExtraProperties
public class IrrDevice {
    public enum DeviceStatus {OK, WARNING, FAULT};
    public IrrDeviceMetadata metadata;
    public IrrDeviceState state;
    public IrrDeviceSettings settings;
    public IrrDeviceTelemetry telemetry_current;
    public IrrDeviceTelemetry telemetry;
    public LineGraphSeries<DataPoint> xSeriesPrimAxis1;
    public LineGraphSeries<DataPoint> xSeriesPrimAxis2;
    public LineGraphSeries<DataPoint> xSeriesSecAxis1;
    public int nbrOfTelemetry;
    public int nbrOfLogs;

    @SuppressWarnings("unchecked")
    public IrrDevice() {
        // Default constructor required for calls to DataSnapshot.getValue(Comment.class)
        this.metadata = new IrrDeviceMetadata();
        this.state = new IrrDeviceState();
        this.settings = new IrrDeviceSettings();
        this.telemetry_current = new IrrDeviceTelemetry();
        xSeriesPrimAxis1 = new LineGraphSeries();
        xSeriesPrimAxis2 =new LineGraphSeries();
        xSeriesSecAxis1 = new LineGraphSeries();
        nbrOfTelemetry = -1;
        nbrOfLogs = -1;

    }


}
