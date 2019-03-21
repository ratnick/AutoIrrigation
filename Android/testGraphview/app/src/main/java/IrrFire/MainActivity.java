package IrrFire;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import IrrFire.AutoIrrigation.R;

import com.firebase.client.FirebaseApp;
import com.jjoe64.graphview.*;
import com.jjoe64.graphview.series.*;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.InputType;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import IrrFire.model.IrrDeviceMetadata;
import IrrFire.model.IrrDeviceState;
import IrrFire.model.IrrDevice;
import IrrFire.model.IrrDeviceTelemetry;
import com.firebase.client.ChildEventListener;
import com.firebase.client.DataSnapshot;
import com.firebase.client.Firebase;
import com.firebase.client.FirebaseError;
import com.firebase.client.ValueEventListener;
import com.jjoe64.graphview.*;
import com.jjoe64.graphview.series.*;

import java.text.SimpleDateFormat;
import java.util.Date;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;


public class MainActivity extends AppCompatActivity {

    //    @BindView(R.id.buttonOnOff)
//    Button buttonOnOff;

    @BindView(R.id.tvDeviceID)              TextView tvDeviceID;
    @BindView(R.id.tvLocation)              TextView tvLocation;
    @BindView(R.id.tvMacAddr)               TextView tvMacAddr;
    @BindView(R.id.tvMeasureMode)           TextView tvMeasureMode;
    @BindView(R.id.tvUseDeepSleep)          TextView tvUseDeepSleep;
    @BindView(R.id.tvSecsToSleep)           TextView tvSecsToSleep;
    @BindView(R.id.tvMaxSlpCycles)          TextView tvMaxSlpCycles;
    @BindView(R.id.tvCurrentSleepCycle)     TextView tvCurrentSleepCycle;
    @BindView(R.id.tvWifiSSID)              TextView tvWifiSSID;
    @BindView(R.id.tvTimestampState)        TextView tvTimestampState;

    @BindView(R.id.tvVcc)                   TextView tvVcc;
    @BindView(R.id.tvLastAnalogueReading)   TextView tvLastAnalogueReading;
    @BindView(R.id.tvLastOpenTimestamp )    TextView tvLastOpenTimestamp;
    @BindView(R.id.tvWifi )                 TextView tvWifi;
    @BindView(R.id.tvTimestampTelemetryTxt )  TextView tvTimestampTelemetryTxt;
    @BindView(R.id.tvTimestampTelemetryTime ) TextView tvTimestampTelemetryTime;

    private static final String FB_PROJECT_ID = "irrfire";

    private static final String FB_URL = "https://" + FB_PROJECT_ID + ".firebaseio.com";
    private static final String FB_PATH = "irrdevices";
    private static final String DEVICE_ID = "5C:CF:7F:AB:7F:D4";

    private String mDeviceId = DEVICE_ID;
    private IrrDevice mIrrDevice = new IrrDevice();

    private Firebase mFirebase;

    private ProgressDialog mProgressDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //Newer version of Firebase
        if(!FirebaseApp.getApps(this).isEmpty()) {
            FirebaseDatabase.getInstance().setPersistenceEnabled(true);
        }

        setContentView(R.layout.activity_main);
        ShowGraph();

        ButterKnife.bind(this);
        loadDevice();

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
        if (id == R.id.action_settings) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Device");

// Set up the input
            final EditText input = new EditText(this);
            input.setTextColor(Color.BLACK);
            input.setText(mDeviceId);
// Specify the type of input expected; this, for example, sets the input as a password, and will mask the text
            input.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI);
            builder.setView(input);

// Set up the buttons
            builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    mDeviceId = input.getText().toString();
                    loadDevice();
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
        }
        else if (id == R.id.action_refresh) {
            loadDevice();
        }

        return super.onOptionsItemSelected(item);
    }

/*    @OnClick(R.id.buttonLightSensor)
    public void toggleLightSensor(final ImageButton button) {

        //6. Toggle Light Sensor
        mIrrDevice.state.useDeepSleep = !mIrrDevice.state.useDeepSleep;

        //7. Send to Firebase
        if (mFirebase != null) {
            mFirebase.child("state").setValue(mIrrDevice.state, new Firebase.CompletionListener() {

                @Override
                public void onComplete(FirebaseError firebaseError, Firebase firebase) {
                    button.setEnabled(true);

                    if (firebaseError != null) {
                        showToast(firebaseError.getMessage());
                    }
                }
            });
        }

    }
*/

    /*    @OnClick(R.id.buttonOnOff)
        public void toggleLamp(final Button button) {

            //4. Button toggle
            button.setEnabled(false);

            mIrrDevice.state.measureMode = !mIrrDevice.state.measureMode;

            //5. Set measureMode to Firebase
            if (mFirebase != null) {
                mFirebase.child("state").setValue(mIrrDevice.state, new Firebase.CompletionListener() {

                    @Override
                    public void onComplete(FirebaseError firebaseError, Firebase firebase) {
                        button.setEnabled(true);

                        //updateUIBasedOnLampStatus(response.body()); --> not called, it will be called later by state change listener

                        if (firebaseError != null) {
                            showToast(firebaseError.getMessage());
                        }
                    }
                });
            }
        }
    */
    private void loadDevice() {

        mProgressDialog = ProgressDialog.show(this, null, "Loading device...", true, false);

        //1. Firebase ref
        String firebaseURL = FB_URL + "/" + FB_PATH + "/" + mDeviceId;
        mFirebase = new Firebase(firebaseURL);

//NNR        buttonOnOff.setEnabled(false);

        //2. Load device details
        mFirebase.addListenerForSingleValueEvent(new ValueEventListener() {

            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                IrrDevice irrDevice = dataSnapshot.getValue(IrrDevice.class);
                if (irrDevice != null) {
                    mIrrDevice = irrDevice;

                    if (irrDevice.metadata != null && !TextUtils.isEmpty(irrDevice.metadata.location)) {
                        tvDeviceID.setText(irrDevice.metadata.deviceID);
                    }

                    updateUIBasedOnMetadata(irrDevice.metadata);
                    updateUIBasedOnState(irrDevice.state);
                    updateUIBasedOnTelemetry(irrDevice.telemetry_current);
                }

//NNR                buttonOnOff.setEnabled(true);
                mProgressDialog.dismiss();
            }

            @Override
            public void onCancelled(FirebaseError firebaseError) {
                showToast(firebaseError.getMessage());
//NNR                buttonOnOff.setEnabled(true);
                mProgressDialog.dismiss();
            }
        });

        //3. Listen for parameter change
        mFirebase.addChildEventListener(new ChildEventListener() {
            @Override
            public void onChildAdded(DataSnapshot dataSnapshot, String s) {

            }

            @Override
            public void onChildChanged(DataSnapshot dataSnapshot, String s) {
                String key = dataSnapshot.getKey();
                Log.d("HomeX", "Key is " + key);

                if (key.equals("metadata")) {
                    IrrDeviceMetadata meta = dataSnapshot.getValue(IrrDeviceMetadata.class);
                    if (meta != null) {
                        mIrrDevice.metadata = meta;
                        updateUIBasedOnMetadata(meta);
                    }
                }

                if (key.equals("state")) {
                    IrrDeviceState devState = dataSnapshot.getValue(IrrDeviceState.class);
                    if (devState != null) {
                        mIrrDevice.state = devState;
                        updateUIBasedOnState(devState);
                    }
                }

                if (key.equals("telemetry_current")) {
                    IrrDeviceTelemetry tele = dataSnapshot.getValue(IrrDeviceTelemetry.class);
                    if (tele != null) {
                        mIrrDevice.telemetry_current = tele;
                        updateUIBasedOnTelemetry(tele);
                    }
                }
            }

            @Override
            public void onChildRemoved(DataSnapshot dataSnapshot) {

            }

            @Override
            public void onChildMoved(DataSnapshot dataSnapshot, String s) {

            }

            @Override
            public void onCancelled(FirebaseError firebaseError) {
                showToast(firebaseError.getMessage());
            }
        });
    }

    private void updateUIBasedOnMetadata(IrrDeviceMetadata meta) {

        tvDeviceID.setText(meta.deviceID);
        tvLocation.setText(meta.location);
        tvMacAddr.setText(meta.macAddr);
    }

    private void updateUIBasedOnState(IrrDeviceState state) {

        tvMeasureMode.setText(state.measureMode);
        tvUseDeepSleep.setText(String.format("%b", state.useDeepSleep));
        tvSecsToSleep.setText(String.format("%d", state.secsToSleep));
        tvMaxSlpCycles.setText(String.format("%d", state.maxSlpCycles));
        tvCurrentSleepCycle.setText(String.format("%d", state.currentSleepCycle));
        tvWifiSSID.setText(state.wifiSSID);
        SimpleDateFormat sfd = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
        tvTimestampState.setText(sfd.format(new Date(state.timestampState)));

// TODO: update parqameters from app to device
 /*       buttonOnOff.setSelected(irrDeviceState.measureMode);
        buttonOnOff.setText((irrDeviceState.measureMode) ? "ON" : "OFF");

        Drawable irrDeviceDrawable = getResources().getDrawable(buttonOnOff.isSelected()? R.drawable.ic_incandescent_on: R.drawable.ic_incandescent_off);
        buttonOnOff.setCompoundDrawablesWithIntrinsicBounds(null, irrDeviceDrawable, null, null);

        buttonLightSensor.setImageResource(irrDeviceState.useDeepSleep ? R.drawable.ic_light_sensor_on : R.drawable.ic_light_sensor);

        sbDimLevel.setProgress(irrDeviceState.secsToSleep);
*/
    }

    @SuppressLint("DefaultLocale")
    private void updateUIBasedOnTelemetry(IrrDeviceTelemetry tele) {

        tvLastAnalogueReading.setText(String.format("%d", tele.lastAnalogueReading));
        tvVcc.setText(String.format("%.2f", tele.vcc));
        tvLastOpenTimestamp.setText(tele.lastOpenTimestamp);
        tvWifi.setText(String.format("%d", tele.wifi));
        SimpleDateFormat sfd = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
        tvTimestampTelemetryTime.setText(sfd.format(new Date(tele.timestampTelemetry)));

    }

    private void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }

    private class DimLevelSeekBarListener implements SeekBar.OnSeekBarChangeListener {

        public void onProgressChanged(SeekBar seekBar, int progress,
                                      boolean fromUser) {
            //Log.d("HomeX", "Value is " + progress);
        }

        public void onStartTrackingTouch(SeekBar seekBar) {}

        public void onStopTrackingTouch(final SeekBar seekBar) {
            Log.d("HomeX", "Final value is " + seekBar.getProgress());

            seekBar.setEnabled(false);

            mIrrDevice.state.secsToSleep = seekBar.getProgress();

            //Send brightness/dim level to Firebase based on progressbar value

            if (mFirebase != null) {
                mFirebase.child("state").setValue(mIrrDevice.state, new Firebase.CompletionListener() {

                    @Override
                    public void onComplete(FirebaseError firebaseError, Firebase firebase) {
                        seekBar.setEnabled(true);

                        if (firebaseError != null) {
                            showToast(firebaseError.getMessage());
                        }
                    }
                });
            }
        }

    }

    private void ShowGraph() {
        GraphView graph = (GraphView) findViewById(R.id.graph);
        LineGraphSeries<DataPoint> series = new LineGraphSeries<>(new DataPoint[] {
                new DataPoint(0, 1),
                new DataPoint(1, 5),
                new DataPoint(2, 3),
                new DataPoint(3, 2),
                new DataPoint(4, 6)
        });
        graph.addSeries(series);
    }

}
