package com.vanding.irrigation;

import android.annotation.SuppressLint;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;

import com.jjoe64.graphview.DefaultLabelFormatter;
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GridLabelRenderer;
import com.jjoe64.graphview.helper.DateAsXAxisLabelFormatter;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.text.NumberFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import butterknife.BindView;
import butterknife.ButterKnife;

import static android.graphics.Color.BLUE;
import static android.graphics.Color.CYAN;
import static android.graphics.Color.WHITE;
import static android.graphics.Color.YELLOW;
import static com.vanding.irrigation.FirebaseService.*;
import static com.vanding.irrigation.db.*;


public class SingleDevice extends AppCompatActivity {

    @BindView(R.id.purgeButton)                 Button purgeButton;
    @BindView(R.id.refreshButton)               Button refreshButton;
    @BindView(R.id.detailButton)                Button detailButton;
    @BindView(R.id.executeButton)               Button executeButton;

    @BindView(R.id.tvDeviceID)                  EditText tvDeviceID;
    @BindView(R.id.tvLocation)                  EditText tvLocation;
    @BindView(R.id.tvdeepSleepEnabled)          EditText tvdeepSleepEnabled;
    @BindView(R.id.tvSecsToSleep)               EditText tvSecsToSleep;
    @BindView(R.id.tvOpenDuration)              EditText tvOpenDuration;
    @BindView(R.id.tvSoakTime)                  EditText tvSoakTime;
    @BindView(R.id.tvhumLim)                    EditText tvhumLim;
    @BindView(R.id.tvrunMode)                   EditText tvrunMode;
    @BindView(R.id.tvDb)                        TextView tvDb;
    @BindView(R.id.tvMaxSlpCycles)              TextView tvMaxSlpCycles;
    @BindView(R.id.tvCurrentSleepCycle)         TextView tvCurrentSleepCycle;
    @BindView(R.id.tvWaketime0)                 TextView tvWaketime0;
    @BindView(R.id.tvWaketime1)                 TextView tvWaketime1;
    @BindView(R.id.tvWaketime2)                 TextView tvWaketime2;
    @BindView(R.id.tvWifiSSID)                  TextView tvWifiSSID;
    @BindView(R.id.tvTimestampState)            TextView tvTimestampState;

    @BindView(R.id.tvTmtry1)                    TextView tvTmtry1;
    @BindView(R.id.tvTmtry2)                    TextView tvTmtry2;
    @BindView(R.id.tvTmtry1Txt)                 TextView tvTmtry1Txt;
    @BindView(R.id.tvTmtry2Txt)                 TextView tvTmtry2Txt;
    @BindView(R.id.tvLastAnalogueReading)       TextView tvLastAnalogueReading;
    @BindView(R.id.tvLastOpenTimestamp)         TextView tvLastOpenTimestamp;
    @BindView(R.id.tvWifi)                      TextView tvWifi;
    @BindView(R.id.tvTimestampTelemetryTxt)     TextView tvTimestampTelemetryTxt;
    @BindView(R.id.tvTimestampTelemetryTime)    TextView tvTimestampTelemetryTime;
    public static GraphView graph;

    public class GraphSettings {
        public String titlePrim1;
        public String titlePrim2;
        public String titleSec1;
        public boolean autoScalePrim;
        public boolean autoScaleSec;
        public double minPrim;
        public double maxPrim;
        public double minSec;
        public double maxSec;

        public GraphSettings(
                String _title1, int _thickness1,
                String _title2, int _thickness2,
                String _title3, int _thickness3,
                boolean _autoScalePrim,
                boolean _autoScaleSec,
                double _minPrim,
                double _maxPrim,
                double _minSec,
                double _maxSec
        ) {
            this.titlePrim1 = _title1;
            this.titlePrim2 = _title2;
            this.titleSec1 = _title3;
            this.autoScalePrim = _autoScalePrim;
            this.autoScaleSec = _autoScaleSec;
            this.minPrim = _minPrim;
            this.maxPrim = _maxPrim;
            this.minSec = _minSec;
            this.maxSec = _maxSec;
        }
    }

    int selectedDevice = -1;
    public static GraphSettings[] gs;
    {
        gs = new GraphSettings[NBR_OF_DEVICE_TYPES];
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_single_device);
        ButterKnife.bind(this);
        graph = (GraphView) findViewById(R.id.graph);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        selectedDevice = this.getIntent().getIntExtra(DEVICE_NBR, -1);
        DisplayDeviceData();

        gs[DEVICE_TYPE_SOIL] = new GraphSettings(
                "Humidity %",2,
                "Valve",2,
                "Batt [V]",2,
                false,
                false,
                0, 1,
                3.3, 4.1 );

        gs[DEVICE_TYPE_GAS] = new GraphSettings(
                "PPM",2,
                "DUTY",2,
                "Wifi",2,
                true,
                true,
                0, 1,
                0, 1);

        gs[DEVICE_TYPE_HUMTEMP] = new GraphSettings(
                "temp C",2,
                "Wifi",2,
                "Hum %",2,
                false,
                false,
                -15, 50,
                0, 100);

        gs[DEVICE_TYPE_WATER] = new GraphSettings(
                "PPM",2,
                "DUTY",2,
                "Wifi",2,
                true,
                true,
                0, 1,
                0, 1);

        Intent commandIntent = new Intent(this, FirebaseService.class);
        commandIntent.putExtra(FirebaseService.DEVICE_NBR, selectedDevice);
        startFirebaseService(FirebaseService.ActionType.LOAD_DEVICE_TELEMETRY, commandIntent);

        purgeButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent intent = new Intent(SingleDevice.this, PurgeLog.class);
                intent.putExtra(DEVICE_NBR, dbSelectedIrrDeviceK);
                intent.putExtra(PurgeLog.PURGE_TYPE, PurgeLog.PURGE_LOG);
                startActivity(intent);

                Context context = getApplicationContext();
                Toast toast = Toast.makeText(context, "DATA PURGED", Toast.LENGTH_SHORT);
                toast.show();
            }
        });

        refreshButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent commandIntent2 = new Intent(getApplicationContext(), FirebaseService.class);
                commandIntent2.putExtra(FirebaseService.DEVICE_NBR, selectedDevice);
                startFirebaseService(FirebaseService.ActionType.LOAD_DEVICE_BASICS, commandIntent2);
                startFirebaseService(FirebaseService.ActionType.LOAD_DEVICE_TELEMETRY, commandIntent2);

                DisplayDeviceData();
                Context context = getApplicationContext();
                Toast toast = Toast.makeText(context, "DATA REFRESHED", Toast.LENGTH_SHORT);
                toast.show();
            }
        });

        detailButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent intent = new Intent(SingleDevice.this, DetailedGraph.class);
                intent.putExtra(DEVICE_NBR, dbSelectedIrrDeviceK);
                startActivity(intent);
            }
        });

        executeButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                dbIrrDevice[dbSelectedIrrDeviceK].metadata.loc = tvLocation.getText().toString();
                dbIrrDevice[dbSelectedIrrDeviceK].metadata.device = tvDeviceID.getText().toString();
                dbIrrDevice[dbSelectedIrrDeviceK].settings.slpEnabl = Boolean.valueOf(tvdeepSleepEnabled.getText().toString());
                dbIrrDevice[dbSelectedIrrDeviceK].settings.vlvOpen = Integer.valueOf(tvOpenDuration.getText().toString());
                dbIrrDevice[dbSelectedIrrDeviceK].settings.vlvSoak = Integer.valueOf(tvSoakTime.getText().toString());
                dbIrrDevice[dbSelectedIrrDeviceK].settings.humLim = Integer.valueOf(tvhumLim.getText().toString());
                dbIrrDevice[dbSelectedIrrDeviceK].settings.db = Integer.valueOf(tvDb.getText().toString());
                dbIrrDevice[dbSelectedIrrDeviceK].settings.totSlp = Integer.valueOf(tvSecsToSleep.getText().toString());
                dbIrrDevice[dbSelectedIrrDeviceK].settings.wakeTime0 = tvWaketime0.getText().toString();
                dbIrrDevice[dbSelectedIrrDeviceK].settings.wakeTime1 = tvWaketime1.getText().toString();
                dbIrrDevice[dbSelectedIrrDeviceK].settings.wakeTime2 = tvWaketime2.getText().toString();
                dbIrrDevice[dbSelectedIrrDeviceK].settings.runMode = tvrunMode.getText().toString();
                dbIrrDevice[dbSelectedIrrDeviceK].settings.Updated = true;
                writeStateToFirebase();
                Context context = getApplicationContext();
                Toast toast = Toast.makeText(context, "DATA UPDATED", Toast.LENGTH_SHORT);
                toast.show();
            }
        });

        /*
        OrientationEventListener mOrientationEventListener;
        mOrientationEventListener = new OrientationEventListener(this) {

            // This is either Surface.Rotation_0, _90, _180, _270, or -1 (invalid).
            private int mLastKnownRotation = -1;

            @Override
            public void onOrientationChanged(int orientation) {
                Context context = getApplicationContext();
                Toast toast = Toast.makeText(context, "SCREEN ROTATED TO: " + Integer.toString(orientation), Toast.LENGTH_SHORT);
                if (orientation == OrientationEventListener.ORIENTATION_UNKNOWN) {
                    return;
                }
            }
        };
        mOrientationEventListener.enable();
        */
    }

    public void writeStateToFirebase() {
        dbDeviceReference[dbSelectedIrrDeviceK].child("metadata").child("loc").setValue(dbIrrDevice[dbSelectedIrrDeviceK].metadata.loc);
        dbDeviceReference[dbSelectedIrrDeviceK].child("metadata").child("device").setValue(dbIrrDevice[dbSelectedIrrDeviceK].metadata.device);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("totSlp").setValue(dbIrrDevice[dbSelectedIrrDeviceK].settings.totSlp);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("slpEnabl").setValue(dbIrrDevice[dbSelectedIrrDeviceK].settings.slpEnabl);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("vlvOpen").setValue(dbIrrDevice[dbSelectedIrrDeviceK].settings.vlvOpen);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("vlvSoak").setValue(dbIrrDevice[dbSelectedIrrDeviceK].settings.vlvSoak);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("humLim").setValue(dbIrrDevice[dbSelectedIrrDeviceK].settings.humLim);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("db").setValue(dbIrrDevice[dbSelectedIrrDeviceK].settings.db);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("wakeTime0").setValue(dbIrrDevice[dbSelectedIrrDeviceK].settings.wakeTime0);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("wakeTime1").setValue(dbIrrDevice[dbSelectedIrrDeviceK].settings.wakeTime1);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("wakeTime2").setValue(dbIrrDevice[dbSelectedIrrDeviceK].settings.wakeTime2);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("runMode").setValue(dbIrrDevice[dbSelectedIrrDeviceK].settings.runMode);
        dbDeviceReference[dbSelectedIrrDeviceK].child("settings").child("Updated").setValue(true);
    }

    private void startFirebaseService(FirebaseService.ActionType sendType, Intent commandIntent) {

        PendingIntent returnIntent = createPendingResult(
                FirebaseService.FIREBASE_SERVICE_ID,
                new Intent(this, SingleDevice.class),0);
        commandIntent.putExtra(FirebaseService.ACTION_TYPE, sendType);
        commandIntent.putExtra(FirebaseService.PENDING_RESULT, returnIntent);
        startService(commandIntent);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent returnIntent) {
        if (requestCode == FIREBASE_SERVICE_ID) {
            if (resultCode == CODE_OK) {
                FirebaseService.ActionType actionPerformed = (FirebaseService.ActionType) returnIntent.getSerializableExtra(WORK_DONE);
                processServiceReturn(actionPerformed, returnIntent);
            } else {
                // here we could either fail silently or pop a dialog or Toast up informing the user there was a problem.
            }
        }
    }

    /* Finally, once we are here we know the service call succeeded and we can act accordingly. */
    private void processServiceReturn(FirebaseService.ActionType action, Intent returnIntent){
        switch(action){
            case LOAD_DEVICE_BASICS:
            case LOAD_DEVICE_TELEMETRY:
                DisplayGraphData();
                break;
        }
    }

    // Pure UI updates
    private void DisplayDeviceData() {
        Log.d("HomeX", "selectedDevice=" + Integer.toString(selectedDevice));

        tvDeviceID.setText(dbIrrDevice[selectedDevice].metadata.device);
        updateUIMetadata();
        updateUIState();
        updateUISettings();
        UpdateUICurrentTelemetry();
    }

    private void updateUIMetadata() {
        tvDeviceID.setText(dbIrrDevice[selectedDevice].metadata.device);
        tvLocation.setText(dbIrrDevice[selectedDevice].metadata.loc);
    }

    private void updateUIState() {
        tvMaxSlpCycles.setText(String.format("%d", dbIrrDevice[selectedDevice].state.slpMxCyc));
        tvCurrentSleepCycle.setText(String.format("%d", dbIrrDevice[selectedDevice].state.slpCurCyc));
        tvWifiSSID.setText(dbIrrDevice[selectedDevice].state.SSID);
        SimpleDateFormat sfd = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
        tvTimestampState.setText(sfd.format(new Date(dbIrrDevice[selectedDevice].state.ts)));
    }

    private void updateUISettings() {
        tvdeepSleepEnabled.setText(String.format("%b", dbIrrDevice[selectedDevice].settings.slpEnabl));
        tvSecsToSleep.setText(String.format("%d", dbIrrDevice[selectedDevice].settings.totSlp));
        tvOpenDuration.setText(String.format("%d", dbIrrDevice[selectedDevice].settings.vlvOpen));
        tvSoakTime.setText(String.format("%d", dbIrrDevice[selectedDevice].settings.vlvSoak));
        tvhumLim.setText(String.format("%d", dbIrrDevice[selectedDevice].settings.humLim));
        tvDb.setText(String.format("%d", dbIrrDevice[selectedDevice].settings.db));
        tvWaketime0.setText(dbIrrDevice[selectedDevice].settings.wakeTime0);
        tvWaketime1.setText(dbIrrDevice[selectedDevice].settings.wakeTime1);
        tvWaketime2.setText(dbIrrDevice[selectedDevice].settings.wakeTime2);
        tvrunMode.setText(dbIrrDevice[selectedDevice].settings.runMode);
    }

    @SuppressLint("DefaultLocale")
    private void UpdateUICurrentTelemetry() {
        tvLastAnalogueReading.setText(String.format("%d", dbIrrDevice[selectedDevice].telemetry_current.lastAnalog));
        tvTmtry2.setText(String.format("%.2f", dbIrrDevice[selectedDevice].telemetry_current.Vcc));
        switch (dbIrrDevice[selectedDevice].metadata.sensorType) {
            case DEVICE_TYPE_SOIL_STR:
                tvTmtry1.setText(String.format("%.0f", dbIrrDevice[selectedDevice].telemetry_current.Hum));
                tvTmtry1Txt.setText(String.format("Humidity [%%]"));
                break;
            case DEVICE_TYPE_GAS_STR:
                tvTmtry1.setText(String.format("%.0f", dbIrrDevice[selectedDevice].telemetry_current.cur_ppm));
                tvTmtry1Txt.setText(String.format("ppm"));
                break;
            case DEVICE_TYPE_HUMTEMP_STR:
                tvTmtry1.setText(String.format("%.1f", dbIrrDevice[selectedDevice].telemetry_current.Temp));
                tvTmtry1Txt.setText(String.format("Temperature [C]"));
                break;
            default:
                break;
        }

        tvLastOpenTimestamp.setText(dbIrrDevice[selectedDevice].telemetry_current.lastOpen);
        tvWifi.setText(String.format("%d", dbIrrDevice[selectedDevice].telemetry_current.Wifi));
        SimpleDateFormat sfd = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
        tvTimestampTelemetryTime.setText(sfd.format(new Date(dbIrrDevice[selectedDevice].telemetry_current.timestamp)));
    }

    private void DisplayGraphData() {
        graph.removeAllSeries();
        graph.getSecondScale().removeAllSeries();
        // primary Y-axis:
        graph.addSeries(dbIrrDevice[selectedDevice].xSeriesPrimAxis2);
        graph.addSeries(dbIrrDevice[selectedDevice].xSeriesPrimAxis1);

        // secondary Y-axis:
        graph.getSecondScale().addSeries(dbIrrDevice[selectedDevice].xSeriesSecAxis1);

        FormatSeries(graph);
        FormatGraph(graph);
    }


    private int GetDeviceType() {
        return GetDeviceType(dbIrrDevice[selectedDevice].metadata.sensorType);
    }

    public static int GetDeviceType(String sensorType) {
        switch (sensorType) {
            case DEVICE_TYPE_SOIL_STR:
                return DEVICE_TYPE_SOIL;
            case DEVICE_TYPE_GAS_STR:
                return DEVICE_TYPE_GAS;
            case DEVICE_TYPE_HUMTEMP_STR:
                return DEVICE_TYPE_HUMTEMP;
            default:
                return -1; // will crash
        }
    }

    private void FormatSeries(GraphView graph) {

        int devType = GetDeviceType();

        dbIrrDevice[selectedDevice].xSeriesPrimAxis1.setTitle(gs[devType].titlePrim1);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis1.setThickness(2);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis1.setColor(WHITE);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis1.setDrawDataPoints(false);

        dbIrrDevice[selectedDevice].xSeriesPrimAxis2.setTitle(gs[devType].titlePrim2);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis2.setThickness(2);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis2.setColor(YELLOW);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis2.setDrawDataPoints(false);

        dbIrrDevice[selectedDevice].xSeriesSecAxis1.setTitle(gs[devType].titleSec1);
        dbIrrDevice[selectedDevice].xSeriesSecAxis1.setThickness(2);
        dbIrrDevice[selectedDevice].xSeriesSecAxis1.setColor(BLUE);
        dbIrrDevice[selectedDevice].xSeriesSecAxis1.setDrawDataPoints(false);

    }

    public static long RoundUpToNearestNiceNumber(long in) {
        double x = 1.1 * in;
        return (long) x;
    }

    private void FormatGraph(GraphView graph) {

        long minX, maxX, minY1, maxY1, minY2, maxY2;
        Date minDate, maxDate;
        int devType = GetDeviceType();

        // Common for both Y-axes
        NumberFormat nf = NumberFormat.getInstance();
        nf.setMinimumFractionDigits(1);
        nf.setMaximumFractionDigits(1);
        //nf.setMinimumIntegerDigits(1);
        graph.getGridLabelRenderer().setLabelFormatter(new DefaultLabelFormatter(nf, nf));
        graph.getGridLabelRenderer().setHorizontalLabelsColor(WHITE);

        // Primary Y-axis (Y1) scale
        graph.getViewport().setYAxisBoundsManual(gs[devType].autoScalePrim);
        if(gs[devType].autoScalePrim) {
            minY1 = (long) dbIrrDevice[selectedDevice].xSeriesPrimAxis1.getLowestValueY();
            maxY1 = (long) dbIrrDevice[selectedDevice].xSeriesPrimAxis1.getHighestValueY();
            maxY1 = RoundUpToNearestNiceNumber(maxY1);
            graph.getViewport().setMinY(0); //minY1;
            graph.getViewport().setMaxY(maxY1);
        } else {
            graph.getViewport().setMinY(gs[devType].minPrim);
            graph.getViewport().setMaxY(gs[devType].maxPrim);
        }
        graph.getGridLabelRenderer().setVerticalLabelsColor(WHITE);
        graph.getGridLabelRenderer().setLabelsSpace(30);

        // Secondary Y-axis (Y2) scale
        // set second scale manually (http://www.android-graphview.org/secondary-scale-axis/ : the y bounds are always manual for second scale
        graph.getViewport().setYAxisBoundsManual(gs[devType].autoScaleSec);
        if(gs[devType].autoScaleSec) {
            minY2 = (long) dbIrrDevice[selectedDevice].xSeriesSecAxis1.getLowestValueY();
            maxY2 = (long) dbIrrDevice[selectedDevice].xSeriesSecAxis1.getHighestValueY();
            maxY2 = RoundUpToNearestNiceNumber(maxY2);
            graph.getSecondScale().setMinY(minY2); //minY2
            graph.getSecondScale().setMaxY(maxY2);
        } else {
            graph.getSecondScale().setMinY(gs[devType].minSec);
            graph.getSecondScale().setMaxY(gs[devType].maxSec);
        }
        graph.getGridLabelRenderer().setVerticalLabelsSecondScaleColor(CYAN);
        graph.getGridLabelRenderer().setSecondScaleLabelVerticalWidth(70);
        graph.getGridLabelRenderer().setNumVerticalLabels(7);
        graph.getGridLabelRenderer().setVerticalLabelsSecondScaleAlign(Paint.Align.RIGHT);

        // X-axis auto date labels
        SimpleDateFormat sfd = new SimpleDateFormat("dd/MM-yy\nHH:mm:ss");
        graph.getGridLabelRenderer().setLabelFormatter(new DateAsXAxisLabelFormatter(this,sfd));
        graph.getGridLabelRenderer().setNumHorizontalLabels(3); // only 2 because of the space

        // X-axis
        maxX = (long) dbIrrDevice[selectedDevice].xSeriesSecAxis1.getHighestValueX();
        minX = (long) dbIrrDevice[selectedDevice].xSeriesSecAxis1.getLowestValueX();
        maxDate = new Date(maxX);
        if (maxX - minX > 86400000) {    // 24 hours
            minX = maxX - 86400000;
        }
        minDate = new Date(minX);
        graph.getViewport().setXAxisBoundsManual(true);
        graph.getViewport().setMinX(minX);
        graph.getViewport().setMaxX(maxX);
        //graph.getGridLabelRenderer().setHumanRounding(false);


        // X-axis manual date labels
/*
        String startDateStr = sfd.format(minDate);
        String endDateStr   = sfd.format(maxDate);
        StaticLabelsFormatter staticLabelsFormatter = new StaticLabelsFormatter(graph);
        graph.getGridLabelRenderer().setNumHorizontalLabels(2); // only 2 because of the space
        staticLabelsFormatter.setHorizontalLabels(new String[] {startDateStr, endDateStr});
        graph.getGridLabelRenderer().setLabelFormatter(staticLabelsFormatter);
*/

        // Drawing area
        graph.setTitleColor(YELLOW);
        //graph.setTitle("LIVE");
        graph.setBackgroundColor(Color.DKGRAY);
        graph.getViewport().setBackgroundColor(Color.BLACK);
        graph.getGridLabelRenderer().setGridColor(Color.DKGRAY);
        graph.getGridLabelRenderer().setGridStyle(GridLabelRenderer.GridStyle.NONE);
        //graph.getViewport().setDrawBorder(true);
        //graph.getViewport().setBorderColor(Color.YELLOW);

        // Scaling and moving
        graph.getViewport().setScalable(true); // activate horizontal zooming and scrolling
        graph.getViewport().setScrollable(true);  // activate horizontal scrolling
        //graph.getViewport().setScalableY(true); // activate horizontal and vertical zooming and scrolling
        //graph.getViewport().setScrollableY(true);    // activate vertical scrolling

        // Legend
        graph.getLegendRenderer().setVisible(true);
        graph.getLegendRenderer().setFixedPosition(0,200);
        graph.getLegendRenderer().setTextSize(25);
        graph.getLegendRenderer().setTextColor(Color.WHITE);
        graph.getLegendRenderer().setBackgroundColor(Color.BLACK);

        graph.onDataChanged(true,false);
    }

    public static String getCurrentDate() {
        SimpleDateFormat dateFormat = new SimpleDateFormat(
                "yyyy-MM-dd", Locale.getDefault());
        Date date = new Date();
        return dateFormat.format(date);
    }


}
