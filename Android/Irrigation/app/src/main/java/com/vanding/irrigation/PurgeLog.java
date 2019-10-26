package com.vanding.irrigation;

import androidx.appcompat.app.AppCompatActivity;

import android.app.PendingIntent;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import butterknife.BindView;
import butterknife.ButterKnife;

import static com.vanding.irrigation.FirebaseService.*;
import static com.vanding.irrigation.db.dbIrrDevice;

public class PurgeLog extends AppCompatActivity {

    @BindView(R.id.executeButton)       Button executeButton;
    @BindView(R.id.tvLocation)          TextView tvLocation;
    @BindView(R.id.tvDeviceID)          TextView tvDeviceID;
    @BindView(R.id.tvNbrOfTelemetry)    TextView tvNbrOfTelemetry;
    @BindView(R.id.tvNbrOfLogs)         TextView tvNbrOfLogs;
    @BindView(R.id.tvKeepLogDays)       EditText tvKeepLogDays;
    @BindView(R.id.tvKeepTeleDays)      EditText tvKeepTeleDays;


    // global constants
    public static String PURGE_TYPE = "PURGE_TYPE";
    public static int PURGE_LOG = 1;
    public static int PURGE_SOMETHING_ELSE = 2;

    private int purgeType = 0;
    private int selectedDevice = -1;
    private int keepLogDays = -1;
    private int keepTeleDays = -1;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_purge_log);

        ButterKnife.bind(this);

        selectedDevice = this.getIntent().getIntExtra(DEVICE_NBR, -1);
        purgeType = this.getIntent().getIntExtra(PURGE_TYPE, 0);

        tvLocation.setText(dbIrrDevice[selectedDevice].metadata.loc);
        tvDeviceID.setText(dbIrrDevice[selectedDevice].metadata.device);
        tvNbrOfTelemetry.setText(String.format("%d", dbIrrDevice[selectedDevice].nbrOfTelemetry));
        tvNbrOfLogs.setText(String.format("%d", dbIrrDevice[selectedDevice].nbrOfLogs));
        tvKeepLogDays.setText(String.format("%d", -1));
        tvKeepTeleDays.setText(String.format("%d", -1));

        executeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                keepLogDays = Integer.valueOf(tvKeepLogDays.getText().toString());
                keepTeleDays = Integer.valueOf(tvKeepTeleDays.getText().toString());
                if (keepLogDays >= 0 || keepTeleDays >= 0) {
                    Intent commandIntent = new Intent(PurgeLog.this, FirebaseService.class);
                    commandIntent.putExtra(FirebaseService.DEVICE_NBR, selectedDevice);
                    commandIntent.putExtra("keepLogDays", keepLogDays);
                    commandIntent.putExtra("keepTeleDays", keepTeleDays);
                    startFirebaseService(FirebaseService.ActionType.PURGE_LOG_AND_TELE, commandIntent);
                }
            }
        });

//        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

    }

    // Generic wrapper called for all commands to FirebaseService
    private void startFirebaseService(ActionType sendType, Intent commandIntent) {
      /* create a pending intent callback for when we send back result
         info from our Service to our Activity.  The Receipt of the
         PendingIntent will go through onActivityResult just as if
         we had called startActivityForResult */
        PendingIntent returnIntent = createPendingResult(
                FirebaseService.FIREBASE_SERVICE_ID,
                new Intent(this, PurgeLog.class),0);
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
        finish();
    }

}
