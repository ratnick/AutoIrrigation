package com.ratnick.vanding;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.InputType;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.google.firebase.database.Query;
import com.ratnick.vanding.model.IrrDevice;
import com.ratnick.vanding.model.IrrDeviceMetadata;
import com.ratnick.vanding.model.IrrDeviceSettings;
import com.ratnick.vanding.model.IrrDeviceState;
import com.ratnick.vanding.model.IrrDeviceTelemetry;

import com.google.firebase.database.ChildEventListener;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

import com.jjoe64.graphview.*;
import com.jjoe64.graphview.helper.*;
import com.jjoe64.graphview.series.*;

import java.text.NumberFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;

import butterknife.BindView;
import butterknife.ButterKnife;

public class DeviceActivity extends AppCompatActivity {

    @BindView(R.id.executeButton)               Button executeButton;

    @BindView(R.id.tvDeviceID)                  EditText tvDeviceID;
    @BindView(R.id.tvLocation)                  EditText tvLocation;
    @BindView(R.id.tvdeepSleepEnabled)          EditText tvdeepSleepEnabled;
    @BindView(R.id.tvSecsToSleep)               EditText tvSecsToSleep;
    @BindView(R.id.tvMainLoopDelay)             EditText tvMainLoopDelay;
    @BindView(R.id.tvWakeTime)                  EditText tvWakeTime;
    @BindView(R.id.tvOpenDuration)              EditText tvOpenDuration;
    @BindView(R.id.tvSoakTime)                  EditText tvSoakTime;
    @BindView(R.id.tvhumLim)                    EditText tvhumLim;
    @BindView(R.id.tvrunMode)                   EditText tvrunMode;
    @BindView(R.id.tvMacAddr)                   TextView tvMacAddr;
    @BindView(R.id.tvMeasureMode)               TextView tvMeasureMode;
    @BindView(R.id.tvMaxSlpCycles)              TextView tvMaxSlpCycles;
    @BindView(R.id.tvCurrentSleepCycle)         TextView tvCurrentSleepCycle;
    @BindView(R.id.tvWifiSSID)                  TextView tvWifiSSID;
    @BindView(R.id.tvTimestampState)            TextView tvTimestampState;

    @BindView(R.id.tvVcc)                       TextView tvVcc;
    @BindView(R.id.tvHum)                       TextView tvHum;
    @BindView(R.id.tvLastAnalogueReading)       TextView tvLastAnalogueReading;
    @BindView(R.id.tvLastOpenTimestamp)         TextView tvLastOpenTimestamp;
    @BindView(R.id.tvWifi)                      TextView tvWifi;
    @BindView(R.id.tvTimestampTelemetryTxt)     TextView tvTimestampTelemetryTxt;
    @BindView(R.id.tvTimestampTelemetryTime)    TextView tvTimestampTelemetryTime;

    // Firebase setup
    private static final String FB_PROJECT_ID = "irrfire";
    private static final String FB_URL = "https://" + FB_PROJECT_ID + ".firebaseio.com";
    private static final String FB_PATH = "irrdevices";
    private static final String INITIAL_DEVICE_ID = "84:F3:EB: C:38:7A";
    private boolean graphDataInitialized = false;
    private boolean graphInitialized = false;

    // Firebase elements
    private final int MAX_DEVICES = 10;
    private DatabaseReference mAllDevicesReference;
    private DatabaseReference[] mDeviceReference = new DatabaseReference[MAX_DEVICES];
    private String[] mIrrDeviceStrings = new String[MAX_DEVICES];
    private IrrDevice[] mIrrDevice = new IrrDevice[MAX_DEVICES];
    private String mSelectedDeviceId = INITIAL_DEVICE_ID;
    //private IrrDevice mSelectedIrrDevice;
    private int mSelectedIrrDeviceK = 0;

    // UI elements
    private GraphView graph;
//    private LineGraphSeries<DataPoint> xSeriesVcc = new LineGraphSeries();
//    private LineGraphSeries<DataPoint>[] mSeriesVcc; // = new LineGraphSeries[MAX_DEVICES];
    private ProgressDialog mProgressDialog;
    private Spinner mSpinnerDevice;
    private Button btnDeviceChoice;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device);
        mAllDevicesReference     = FirebaseDatabase.getInstance().getReference().child("irrdevices");
        ButterKnife.bind(this);
        graph = (GraphView) findViewById(R.id.graph);
        DetectDevicesAndLoadSpinner();

        executeButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                mIrrDevice[mSelectedIrrDeviceK].metadata.loc = tvLocation.getText().toString();
                mIrrDevice[mSelectedIrrDeviceK].metadata.device = tvDeviceID.getText().toString();
                mIrrDevice[mSelectedIrrDeviceK].settings.totSlp = Integer.valueOf(tvSecsToSleep.getText().toString());
                mIrrDevice[mSelectedIrrDeviceK].settings.slpEnabl = Boolean.valueOf(tvdeepSleepEnabled.getText().toString());
                mIrrDevice[mSelectedIrrDeviceK].settings.loopSec = Integer.valueOf(tvMainLoopDelay.getText().toString());
                mIrrDevice[mSelectedIrrDeviceK].settings.wakeTime = tvWakeTime.getText().toString();
                mIrrDevice[mSelectedIrrDeviceK].settings.vlvOpen = Integer.valueOf(tvOpenDuration.getText().toString());
                mIrrDevice[mSelectedIrrDeviceK].settings.vlvSoak = Integer.valueOf(tvSoakTime.getText().toString());
                mIrrDevice[mSelectedIrrDeviceK].settings.humLim = Integer.valueOf(tvhumLim.getText().toString());
                mIrrDevice[mSelectedIrrDeviceK].settings.runMode = tvrunMode.getText().toString();
                mIrrDevice[mSelectedIrrDeviceK].settings.Updated = true;
                writeStateToFirebase();
            }
        });
    }

    public void writeStateToFirebase() {
        mDeviceReference[mSelectedIrrDeviceK].child("metadata").child("loc").setValue(mIrrDevice[mSelectedIrrDeviceK].metadata.loc);
        mDeviceReference[mSelectedIrrDeviceK].child("metadata").child("device").setValue(mIrrDevice[mSelectedIrrDeviceK].metadata.device);
        mDeviceReference[mSelectedIrrDeviceK].child("settings").child("totSlp").setValue(mIrrDevice[mSelectedIrrDeviceK].settings.totSlp);
        mDeviceReference[mSelectedIrrDeviceK].child("settings").child("slpEnabl").setValue(mIrrDevice[mSelectedIrrDeviceK].settings.slpEnabl);
        mDeviceReference[mSelectedIrrDeviceK].child("state").child("slpMxCyc").setValue(mIrrDevice[mSelectedIrrDeviceK].state.slpMxCyc);
        mDeviceReference[mSelectedIrrDeviceK].child("settings").child("loopSec").setValue(mIrrDevice[mSelectedIrrDeviceK].settings.loopSec);
        mDeviceReference[mSelectedIrrDeviceK].child("settings").child("vlvOpen").setValue(mIrrDevice[mSelectedIrrDeviceK].settings.vlvOpen);
        mDeviceReference[mSelectedIrrDeviceK].child("settings").child("vlvSoak").setValue(mIrrDevice[mSelectedIrrDeviceK].settings.vlvSoak);
        mDeviceReference[mSelectedIrrDeviceK].child("settings").child("humLim").setValue(mIrrDevice[mSelectedIrrDeviceK].settings.humLim);
        mDeviceReference[mSelectedIrrDeviceK].child("settings").child("runMode").setValue(mIrrDevice[mSelectedIrrDeviceK].settings.runMode);
        mDeviceReference[mSelectedIrrDeviceK].child("state").child("slpDura").setValue(mIrrDevice[mSelectedIrrDeviceK].state.slpDura);
        mDeviceReference[mSelectedIrrDeviceK].child("settings").child("Updated").setValue(true);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_device, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_purge_data) {

            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Keep X days of telemetry:");

            // Set up the input
            final EditText input = new EditText(this);
            input.setTextColor(Color.BLACK);
            input.setText("21");

            // Specify the type of input expected; this, for example, sets the input as a password, and will mask the text
            input.setInputType(InputType.TYPE_CLASS_NUMBER);
            builder.setView(input);

            // Set up the buttons
            builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    int keepXdays = Integer.valueOf(input.getText().toString());
                    PurgeTelemetryData(keepXdays);
                }
            });
            builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    dialog.cancel();
                }
            });

            builder.show();

            return true;

        } else if (id == R.id.action_refresh) {
            DisplayDeviceData();
        }

        return super.onOptionsItemSelected(item);
    }

    // Spinner code
    private void DetectDevicesAndLoadSpinner() {

        // set up spinner for choice of device
        mAllDevicesReference.addListenerForSingleValueEvent(
            new ValueEventListener() {

                @Override
                public void onDataChange(DataSnapshot dataSnapshot) {
                    addDevicesOnSpinner(dataSnapshot);
                }

                @Override
                public void onCancelled(DatabaseError firebaseError) {
                    showToast(firebaseError.toString());
                    mProgressDialog.dismiss();
                }
            }
        );
    }

    public void addDevicesOnSpinner(DataSnapshot dataSnapshot) {

        int count = (int) dataSnapshot.getChildrenCount();

        int i=0;
        for (DataSnapshot postSnapshot : dataSnapshot.getChildren()) {
            mIrrDeviceStrings[i] = postSnapshot.getKey();
            mIrrDevice[i] = new IrrDevice();
            LoadDeviceDataFromFirebase(i);
            i++;
        }

//        mSelectedIrrDeviceK = 0;
        addListenerOnButton();
        addListenerOnSpinnerItemSelection();

        mSpinnerDevice = (Spinner) findViewById(R.id.spinnerDevice);
        List<String> list = new ArrayList<String>();

        for(i=0;i<count;i++) {
            list.add(mIrrDeviceStrings[i]);
        }
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, list);
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinnerDevice.setAdapter(dataAdapter);
    }

    public void addListenerOnSpinnerItemSelection() {
        mSpinnerDevice = (Spinner) findViewById(R.id.spinnerDevice);
        mSpinnerDevice.setOnItemSelectedListener(new CustomOnItemSelectedListener());
    }


    // get the selected dropdown list value
    public void addListenerOnButton() {

        mSpinnerDevice = (Spinner) findViewById(R.id.spinnerDevice);

/*
        btnDeviceChoice = (Button) findViewById(R.id.btnDeviceChoice);
        btnDeviceChoice.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mSelectedDeviceId = String.valueOf(mSpinnerDevice.getSelectedItem());
                mSelectedIrrDeviceK = FindKeyInList(mSelectedDeviceId);
                Toast.makeText(DeviceActivity.this,
                        "OnClickListener : " +
                                "\nSpinnerDevice choice : " + String.valueOf(mSpinnerDevice.getSelectedItem()),
                        Toast.LENGTH_SHORT).show();
                DisplayDeviceData();
            }

        });
*/
    }

    public class CustomOnItemSelectedListener implements AdapterView.OnItemSelectedListener {

        public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {

            mSelectedDeviceId = String.valueOf(mSpinnerDevice.getSelectedItem());
            mSelectedIrrDeviceK = FindKeyInList(mSelectedDeviceId);
//            Toast.makeText(DeviceActivity.this,
//                    "OnClickListener : \nSpinnerDevice choice : " + String.valueOf(mSpinnerDevice.getSelectedItem()),
//                    Toast.LENGTH_SHORT).show();
            DisplayDeviceData();
//            Toast.makeText(parent.getContext(),
//                    "OnItemSelectedListener : " + parent.getItemAtPosition(pos).toString(),
//                    Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onNothingSelected(AdapterView<?> arg0) {
            // TODO Auto-generated method stub
        }

    }

    private void LoadDeviceDataFromFirebase(int _k) {
        final int k = _k;
        mProgressDialog = ProgressDialog.show(this, null, "Loading device...", true, false);

        mDeviceReference[k] = FirebaseDatabase.getInstance().getReference().child("irrdevices").child(mIrrDeviceStrings[k]);
        // Load device details
        mDeviceReference[k].addListenerForSingleValueEvent(
                new ValueEventListener() {

                    @Override
                    // Called once on creation + every time a change (incl children) happen (https://www.learnhowtoprogram.com/android/data-persistence/firebase-reading-data-and-event-listeners)
                    public void onDataChange(DataSnapshot dataSnapshot) {
                        String dsKey = dataSnapshot.getKey();
                        int i = FindKeyInList(dsKey);
                        if (i == -1) {
                            i = mIrrDeviceStrings.length;
                            mIrrDevice[i] = new IrrDevice();
                        }
                        mIrrDevice[i].metadata = dataSnapshot.child("metadata").getValue(IrrDeviceMetadata.class);
                        mIrrDevice[i].telemetry_current = dataSnapshot.child("telemetry_current").getValue(IrrDeviceTelemetry.class);
                        mIrrDevice[i].state = dataSnapshot.child("state").getValue(IrrDeviceState.class);
                        mIrrDevice[i].settings = dataSnapshot.child("settings").getValue(IrrDeviceSettings.class);
                        mIrrDevice[i].xSeriesVcc.resetData(readAllData(dataSnapshot, "Vcc"));
                        mIrrDevice[i].xSeriesHumidity.resetData(readAllData(dataSnapshot, "Hum"));
                        mIrrDevice[i].xSeriesValveState.resetData(readAllData(dataSnapshot, "vlvState"));

//                        if (dsKey == INITIAL_DEVICE_ID) {
                            DisplayDeviceData();
//                            graphDataInitialized = true;
//                        }
                    }

                    @Override
                    public void onCancelled(DatabaseError firebaseError) {
                        showToast(firebaseError.toString());
                        mProgressDialog.dismiss();
                    }
                });

        // Listen for parameter change
        mDeviceReference[k].addChildEventListener(new ChildEventListener() {
            @Override
            public void onChildAdded(DataSnapshot dataSnapshot, String s) {
                String key = dataSnapshot.getKey();
                Log.d("HomeX", "Key is " + key);

                if (key.equals("telemetry")) {
                    String deviceKey = dataSnapshot.getRef().getParent().getKey();
                    if (deviceKey == mSelectedDeviceId) {
                        Log.e("Count ", "" + dataSnapshot.getChildrenCount());
//                        Add point to data set
                    }
//                    if (!graphDataInitialized) {
                        DisplayDeviceData();
//                        graphInitialized = true;
//                    }
                }
            }

            @Override
            public void onChildChanged(DataSnapshot dataSnapshot, String s) {
                String key = dataSnapshot.getKey();
                Log.d("HomeX", "Key is " + key);

                if (key.equals("metadata")) {
                    IrrDeviceMetadata meta = dataSnapshot.getValue(IrrDeviceMetadata.class);
                    if (meta != null) {
                        mIrrDevice[k].metadata = meta;
                    }
                }

                if (key.equals("state")) {
                    IrrDeviceState devState = dataSnapshot.getValue(IrrDeviceState.class);
                    if (devState != null) {
                        mIrrDevice[k].state = devState;
                        updateUIBasedOnState(devState);
                    }
                }

                if (key.equals("settings")) {
                    IrrDeviceSettings devSettings = dataSnapshot.getValue(IrrDeviceSettings.class);
                    if (devSettings != null) {
                        mIrrDevice[k].settings = devSettings;
                        updateUIBasedOnSettings(devSettings);
                    }
                }

                if (key.equals("telemetry_current")) {
                    IrrDeviceTelemetry tele = dataSnapshot.getValue(IrrDeviceTelemetry.class);
                    if (tele != null) {
                        mIrrDevice[k].telemetry_current = tele;
                        updateUIBasedOnTelemetry(tele);
                    }
                }

                if (key.equals("telemetry")) {
                    String deviceKey = dataSnapshot.getRef().getParent().getKey();
                    if (deviceKey == mSelectedDeviceId) {
                        Log.e("Count ", "" + dataSnapshot.getChildrenCount());
                        mIrrDevice[k].xSeriesVcc.resetData(readAllData(dataSnapshot, "Vcc"));
                        mIrrDevice[k].xSeriesHumidity.resetData(readAllData(dataSnapshot, "Hum"));
                        mIrrDevice[k].xSeriesValveState.resetData(readAllData(dataSnapshot, "vlvState"));
                    }
                }

                if (key.equals(mSelectedDeviceId)) {
                    DisplayDeviceData();
                }
            }

            @Override
            public void onChildRemoved(DataSnapshot dataSnapshot) {

            }

            @Override
            public void onChildMoved(DataSnapshot dataSnapshot, String s) {

            }

            @Override
            public void onCancelled(DatabaseError firebaseError) {
                showToast(firebaseError.toString());
            }
        });

        mProgressDialog.dismiss();

    }

    private int FindKeyInList(String key) {  //return -1 if not found
        int i = 0;
        for(String str: mIrrDeviceStrings) {
            if(str.trim().contains(key))
                return i;
            i++;
        }
        return (int) -1;
    }

    private DataPoint[] readAllData(DataSnapshot dataSnapshot, String parameterName) {
        // are we on telemetry level or on child level?
        DataSnapshot teledataSnapshot;
        String parentKey = dataSnapshot.getRef().getParent().getKey();
        if (parentKey.equals("irrdevices")) {
            teledataSnapshot = dataSnapshot.child("telemetry");
        }
        else if (dataSnapshot.getKey().equals("telemetry")) {
            teledataSnapshot = dataSnapshot;
        } else {

            teledataSnapshot = null;
            int i = 0/0;  // GENERATE ERROR
        }
        int count = (int) teledataSnapshot.getChildrenCount();

        if (parameterName.equals("vlvState")) {
            DataPoint[] values = new DataPoint[count*2];
            int i = 0;
            IrrDeviceTelemetry post;

            float prevValveState = 0;
            for (DataSnapshot postSnapshot : teledataSnapshot.getChildren()) {
                float factor = 20;
                post = postSnapshot.getValue(IrrDeviceTelemetry.class);
                if (prevValveState != post.vlvState) {  // insert extra point before real point to make a sharp edge on the graph
                    DataPoint x = new DataPoint(post.timestamp -1, (float) factor * prevValveState);
                    values[i++] = x;
                }
                DataPoint v = new DataPoint(post.timestamp, (float) factor * post.vlvState);
                values[i++] = v;
                prevValveState = post.vlvState;
            }
            Log.d("data fetch", "i=" + Integer.toString(i) + "  count=" + Integer.toString(count*2));
            while (i < count*2) {
                DataPoint x = new DataPoint(values[i-1].getX()+2, (float) 0.0);
                values[i++] = x;
            }
            Log.d("data fetch", "i=" + Integer.toString(i) + "  count=" + Integer.toString(count*2));
            return values;

        } else {

            DataPoint[] values = new DataPoint[count];
            int i = 0;
            IrrDeviceTelemetry post;

            switch (parameterName) {
                case "Vcc":
                    for (DataSnapshot postSnapshot : teledataSnapshot.getChildren()) {
                        post = postSnapshot.getValue(IrrDeviceTelemetry.class);
                        DataPoint v = new DataPoint(post.timestamp, post.Vcc);
                        values[i++] = v;
                    }
                    break;
                case "Hum":
                    for (DataSnapshot postSnapshot : teledataSnapshot.getChildren()) {
                        post = postSnapshot.getValue(IrrDeviceTelemetry.class);
                        DataPoint v = new DataPoint(post.timestamp, (float) post.Hum);
                        values[i++] = v;
                    }
                    break;
                default: i=0/0; //crash
            }
            return values;
        }

    }

    private void FormatSeries(GraphView graph) {

        // Raw
        mIrrDevice[mSelectedIrrDeviceK].xSeriesHumidity.setTitle("Humidity %");
        mIrrDevice[mSelectedIrrDeviceK].xSeriesHumidity.setThickness(2);
        mIrrDevice[mSelectedIrrDeviceK].xSeriesHumidity.setColor(Color.WHITE);
        mIrrDevice[mSelectedIrrDeviceK].xSeriesHumidity.setDrawDataPoints(false);

        // Valve position
        mIrrDevice[mSelectedIrrDeviceK].xSeriesValveState.setTitle("Valve");
        mIrrDevice[mSelectedIrrDeviceK].xSeriesValveState.setThickness(2);
        mIrrDevice[mSelectedIrrDeviceK].xSeriesValveState.setColor(Color.YELLOW);
        mIrrDevice[mSelectedIrrDeviceK].xSeriesValveState.setDrawDataPoints(false);

        // Vcc
        mIrrDevice[mSelectedIrrDeviceK].xSeriesVcc.setTitle("Batt [V]");
        mIrrDevice[mSelectedIrrDeviceK].xSeriesVcc.setThickness(2);
        mIrrDevice[mSelectedIrrDeviceK].xSeriesVcc.setColor(Color.MAGENTA);
        mIrrDevice[mSelectedIrrDeviceK].xSeriesVcc.setDrawDataPoints(false);

    }

    private void FormatGraph(GraphView graph) {

        long minX, maxX, minY1, maxY1, minY2, maxY2;
        Date minDate, maxDate;


        // Common for both Y-axes
        NumberFormat nf = NumberFormat.getInstance();
        nf.setMinimumFractionDigits(1);
        nf.setMaximumFractionDigits(1);
        //nf.setMinimumIntegerDigits(1);
        graph.getGridLabelRenderer().setLabelFormatter(new DefaultLabelFormatter(nf, nf));
        graph.getGridLabelRenderer().setHorizontalLabelsColor(Color.WHITE);

        // Primary Y-axis (Y1) scale
        graph.getViewport().setYAxisBoundsManual(true);
        graph.getViewport().setMinY(0);
        graph.getViewport().setMaxY(99);
        graph.getGridLabelRenderer().setVerticalLabelsColor(Color.WHITE);
        graph.getGridLabelRenderer().setLabelsSpace(20);

        // Secondary Y-axis (Y2) scale
        // set second scale manually (http://www.android-graphview.org/secondary-scale-axis/ : the y bounds are always manual for second scale
        minY2 = (long) mIrrDevice[mSelectedIrrDeviceK].xSeriesVcc.getLowestValueY();
        maxY2 = (long) mIrrDevice[mSelectedIrrDeviceK].xSeriesVcc.getHighestValueY();
        //graph.getSecondScale().setMinY(minY2);
        //graph.getSecondScale().setMaxY(maxY2);
        graph.getSecondScale().setMinY(3.3);
        graph.getSecondScale().setMaxY(4.1);
        graph.getGridLabelRenderer().setVerticalLabelsSecondScaleColor(Color.MAGENTA);
        graph.getGridLabelRenderer().setSecondScaleLabelVerticalWidth(70);
        graph.getGridLabelRenderer().setNumVerticalLabels(7);
        graph.getGridLabelRenderer().setVerticalLabelsSecondScaleAlign(Paint.Align.RIGHT);

        // X-axis auto date labels
        SimpleDateFormat sfd = new SimpleDateFormat("dd/MM-yy\nHH:mm:ss");
        graph.getGridLabelRenderer().setLabelFormatter(new DateAsXAxisLabelFormatter(this,sfd));
        graph.getGridLabelRenderer().setNumHorizontalLabels(3); // only 2 because of the space

        // X-axis
        minX = (long) mIrrDevice[mSelectedIrrDeviceK].xSeriesVcc.getLowestValueX();
        maxX = (long) mIrrDevice[mSelectedIrrDeviceK].xSeriesVcc.getHighestValueX();
        minDate = new Date(minX);
        maxDate = new Date(maxX);
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
        graph.setTitleColor(Color.YELLOW);
        graph.setTitle("LIVE");
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

//        graph.onDataChanged(true,false);
    }


    public static String getCurrentDate() {
        SimpleDateFormat dateFormat = new SimpleDateFormat(
                "yyyy-MM-dd", Locale.getDefault());
        Date date = new Date();
        return dateFormat.format(date);
    }

    private void PurgeTelemetryData(int daysToKeep) {

        Date dateFilter = new Date(System.currentTimeMillis() - (daysToKeep * 1000 * 60 * 60 * 24));

        final DatabaseReference allTele = mDeviceReference[mSelectedIrrDeviceK].child("telemetry");
        allTele.orderByChild("timestamp").endAt(dateFilter.getTime()).addListenerForSingleValueEvent(new ValueEventListener() {
            // .endAt(1556001761734)
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {

                int count = (int) dataSnapshot.getChildrenCount();
                if (count > 0) {
                    IrrDeviceTelemetry post;
                    for (DataSnapshot postSnapshot : dataSnapshot.getChildren()) {
                        post = postSnapshot.getValue(IrrDeviceTelemetry.class);
                        //System.out.println(" timestamp " + post.timestamp + "  Vcc=" + post.Vcc);
                        allTele.child(postSnapshot.getKey()).removeValue();
                    }
                }
                showToast("Deleted " + count + " telemetry posts");
            }

            @Override
            public void onCancelled(DatabaseError databaseError) {
                // ...
            }
        });
    }

    // Pure UI updates
    private void DisplayDeviceData() {
        Log.d("HomeX", "mSelectedIrrDeviceK=" + Integer.toString(mSelectedIrrDeviceK));

        tvDeviceID.setText(mIrrDevice[mSelectedIrrDeviceK].metadata.device);
        updateUIBasedOnMetadata(mIrrDevice[mSelectedIrrDeviceK].metadata);
        updateUIBasedOnState(mIrrDevice[mSelectedIrrDeviceK].state);
        updateUIBasedOnSettings(mIrrDevice[mSelectedIrrDeviceK].settings);
        updateUIBasedOnTelemetry(mIrrDevice[mSelectedIrrDeviceK].telemetry_current);
        graph.removeAllSeries();
        // primary Y-axis:
        graph.addSeries(mIrrDevice[mSelectedIrrDeviceK].xSeriesValveState);
        graph.addSeries(mIrrDevice[mSelectedIrrDeviceK].xSeriesHumidity);

        // secondary Y-axis:
        graph.getSecondScale().addSeries(mIrrDevice[mSelectedIrrDeviceK].xSeriesVcc);

        FormatSeries(graph);
        FormatGraph(graph);
    }

    private void updateUIBasedOnMetadata(IrrDeviceMetadata meta) {
        tvDeviceID.setText(meta.device);
        tvLocation.setText(meta.loc);
        tvMacAddr.setText(meta.mac);
    }

    private void updateUIBasedOnState(IrrDeviceState state) {
        tvMeasureMode.setText(state.mode);
        tvMaxSlpCycles.setText(String.format("%d", state.slpMxCyc));
        tvCurrentSleepCycle.setText(String.format("%d", state.slpCurCyc));
        tvWifiSSID.setText(state.SSID);
        SimpleDateFormat sfd = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
        tvTimestampState.setText(sfd.format(new Date(state.ts)));
    }

    private void updateUIBasedOnSettings(IrrDeviceSettings settings) {
        tvdeepSleepEnabled.setText(String.format("%b", settings.slpEnabl));
        tvSecsToSleep.setText(String.format("%d", settings.totSlp));
        tvMainLoopDelay.setText(String.format("%d", settings.loopSec));
        tvWakeTime.setText(settings.wakeTime);
        tvOpenDuration.setText(String.format("%d", settings.vlvOpen));
        tvSoakTime.setText(String.format("%d", settings.vlvSoak));
        tvhumLim.setText(String.format("%d", settings.humLim));
        tvrunMode.setText(settings.runMode);
    }

    @SuppressLint("DefaultLocale")
    private void updateUIBasedOnTelemetry(IrrDeviceTelemetry tele) {
        tvLastAnalogueReading.setText(String.format("%d", tele.lastAnalog));
        tvVcc.setText(String.format("%.2f", tele.Vcc));
        tvHum.setText(String.format("%.0f", tele.Hum));
        tvLastOpenTimestamp.setText(tele.lastOpen);
        tvWifi.setText(String.format("%d", tele.Wifi));
        SimpleDateFormat sfd = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
        tvTimestampTelemetryTime.setText(sfd.format(new Date(tele.timestamp)));
    }

    private void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }

}
