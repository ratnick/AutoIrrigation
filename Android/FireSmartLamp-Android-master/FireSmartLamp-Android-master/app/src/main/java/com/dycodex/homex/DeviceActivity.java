package com.dycodex.homex;

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

import com.dycodex.homex.model.HomeXLampParameter;
import com.dycodex.homex.model.HomeXLamp;
import com.dycodex.homex.model.HomeXLampTelemetry;
import com.firebase.client.ChildEventListener;
import com.firebase.client.DataSnapshot;
import com.firebase.client.Firebase;
import com.firebase.client.FirebaseError;
import com.firebase.client.ValueEventListener;

import butterknife.Bind;
import butterknife.ButterKnife;
import butterknife.OnClick;

public class DeviceActivity extends AppCompatActivity {

    @Bind(R.id.buttonOnOff)
    Button buttonOnOff;
    @Bind(R.id.sbDimLevel)
    SeekBar sbDimLevel;
    @Bind(R.id.buttonLightSensor)
    ImageButton buttonLightSensor;

    @Bind(R.id.tvCurrentUsage)
    TextView tvCurrentUsage;

    @Bind(R.id.tvSubTitle)
    TextView tvSubTitle;

    private static final String FB_PROJECT_ID = "[YOUR_OWN_FIREBASE_APP]"; //TODO: CHANGE THIS!!!!

    private static final String FB_URL = "https://" + FB_PROJECT_ID + ".firebaseio.com";
    private static final String FB_PATH = "devices";
    private static final String DEVICE_ID = "smartlamp1";

    private String mDeviceId = DEVICE_ID;
    private HomeXLamp mCurrentLamp = new HomeXLamp();

    private Firebase mFirebase;

    private ProgressDialog mProgressDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device);
        ButterKnife.bind(this);

        //buttonOnOff.setBackgroundDrawable(getResources().getDrawable(R.drawable.ic_incandescent_off));

        sbDimLevel.setOnSeekBarChangeListener(new DimLevelSeekBarListener());

        Drawable lampDrawable = getResources().getDrawable(R.drawable.ic_incandescent_off);
        buttonOnOff.setCompoundDrawablePadding(20);
        buttonOnOff.setCompoundDrawablesWithIntrinsicBounds(null, lampDrawable, null, null);

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

    @OnClick(R.id.buttonLightSensor)
    public void toggleLightSensor(final ImageButton button) {

        //6. Toggle Light Sensor
        mCurrentLamp.parameters.lightSensorEnabled = !mCurrentLamp.parameters.lightSensorEnabled;

        //7. Send to Firebase
        if (mFirebase != null) {
            mFirebase.child("parameters").setValue(mCurrentLamp.parameters, new Firebase.CompletionListener() {

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

    @OnClick(R.id.buttonOnOff)
    public void toggleLamp(final Button button) {

        //4. Button toggle
        button.setEnabled(false);

        mCurrentLamp.parameters.state = !mCurrentLamp.parameters.state;

        //5. Set state to Firebase
        if (mFirebase != null) {
            mFirebase.child("parameters").setValue(mCurrentLamp.parameters, new Firebase.CompletionListener() {

                @Override
                public void onComplete(FirebaseError firebaseError, Firebase firebase) {
                    button.setEnabled(true);

                    //updateUIBasedOnLampStatus(response.body()); --> not called, it will be called later by parameters change listener

                    if (firebaseError != null) {
                        showToast(firebaseError.getMessage());
                    }
                }
            });
        }
    }

    private void loadDevice() {

        mProgressDialog = ProgressDialog.show(this, null, "Loading device...", true, false);

        //1. Firebase ref
        String firebaseURL = FB_URL + "/" + FB_PATH + "/" + mDeviceId;
        mFirebase = new Firebase(firebaseURL);

        buttonOnOff.setEnabled(false);

        //2. Load device details
        mFirebase.addListenerForSingleValueEvent(new ValueEventListener() {

            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                HomeXLamp lamp = dataSnapshot.getValue(HomeXLamp.class);
                if (lamp != null) {
                    mCurrentLamp = lamp;

                    if (lamp.metadata != null && !TextUtils.isEmpty(lamp.metadata.properties.name)) {
                        tvSubTitle.setText(lamp.metadata.properties.name);
                    }

                    updateUIBasedOnLampParameters(lamp.parameters);
                }

                buttonOnOff.setEnabled(true);
                mProgressDialog.dismiss();
            }

            @Override
            public void onCancelled(FirebaseError firebaseError) {
                showToast(firebaseError.getMessage());
                buttonOnOff.setEnabled(true);
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

                if (key.equals("parameters")) {
                    HomeXLampParameter lampParam = dataSnapshot.getValue(HomeXLampParameter.class);
                    if (lampParam != null) {
                        mCurrentLamp.parameters = lampParam;
                        updateUIBasedOnLampParameters(lampParam);
                    }

                } else if (key.equals("telemetry")) {
                    DataSnapshot latestTelemetry = dataSnapshot.child("latest");
                    HomeXLampTelemetry tele = latestTelemetry.getValue(HomeXLampTelemetry.class);

                    if (tele != null) {
                        tvCurrentUsage.setText(String.format("%.02f", tele.wattage) + "w");
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

    private void updateUIBasedOnLampParameters(HomeXLampParameter lampParams) {

        buttonOnOff.setSelected(lampParams.state);
        buttonOnOff.setText((lampParams.state) ? "ON" : "OFF");

        Drawable lampDrawable = getResources().getDrawable(buttonOnOff.isSelected()? R.drawable.ic_incandescent_on: R.drawable.ic_incandescent_off);
        buttonOnOff.setCompoundDrawablesWithIntrinsicBounds(null, lampDrawable, null, null);

        buttonLightSensor.setImageResource(lampParams.lightSensorEnabled ? R.drawable.ic_light_sensor_on : R.drawable.ic_light_sensor);

        sbDimLevel.setProgress(lampParams.dimLevel);
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

            mCurrentLamp.parameters.dimLevel = seekBar.getProgress();

            //Send brightness/dim level to Firebase based on progressbar value

            if (mFirebase != null) {
                mFirebase.child("parameters").setValue(mCurrentLamp.parameters, new Firebase.CompletionListener() {

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
}
