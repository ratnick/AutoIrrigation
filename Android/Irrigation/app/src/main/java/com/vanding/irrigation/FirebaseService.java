package com.vanding.irrigation;


import android.app.IntentService;
import android.app.PendingIntent;
import android.content.Intent;
import android.util.Log;

import com.google.firebase.database.FirebaseDatabase;

import static com.vanding.irrigation.db.*;
import static com.vanding.irrigation.db.dbFirebaseLoadingState.*;


public class FirebaseService extends IntentService {

    public FirebaseObject fbo = new FirebaseObject();

    /* Arbitrary int identifier.  Used to Identify the Service as the origin of returning
      results to the calling Activity. */
    public final static int FIREBASE_SERVICE_ID = 111;

    /* String and int constants used either by both the Action Service and caller Activity.  This
       defines the protocol by which we pass information back and forth between the two via
       Intent extras. */
    public final static String FIREBASE_ROOT = FirebaseObject.FB_PATH;
    public final static String DEVICE_NBR = "DEVICE_NBR";
    public final static String PENDING_RESULT = "PENDING_RESULT";
    public final static String ACTION_TYPE = "ACTION_TYPE";
    public final static String INFO_TEXT = "info";
    public final static String RESULT_CODE = "RESULT_CODE";
    public final static String WORK_DONE = "WORK_DONE";
    public final static int CODE_OK = 0;
    public final static int CODE_ERROR = 1;

    /* All the types of actions this Service can perform */
    public enum ActionType {INIT_SERVICE, LOAD_DEVICE_BASICS, LOAD_DEVICE_TELEMETRY, ACTION_INFO, ACTION_NONE, PURGE_LOG_AND_TELE }

    public FirebaseService() {
        super("FirebaseService");
    }

    @Override
    protected void onHandleIntent(Intent commandIntent) {
        //DEBUG: Remove next statement when releasing
        //android.os.Debug.waitForDebugger();

      /* From the Activity, we parse out what the caller wants the Service to do, and the
         callback (PendingIntent) that we are to call when the work is done */
        //ActionType workToDo = (ActionType)commandIntent.getSerializableExtra(ACTION_TYPE, ActionType.ACTION_NONE);
        Log.d("onHandleIntent", "XXX");
        ActionType workToDo = (ActionType)commandIntent.getSerializableExtra(ACTION_TYPE);
        PendingIntent returnIntent = commandIntent.getParcelableExtra(PENDING_RESULT);

        /* The Intent that we will add the results to and send back to the caller */
        Intent resultIntent = new Intent();

        /* The result of this action - assume an error by default, change value on success */
        int workResultCode = CODE_ERROR;

        switch (workToDo) {
            case INIT_SERVICE:
                    if (dbLoadingState == NOT_STARTED) // in all other cases, the service has already been initialized
                    {
                        dbAllDevicesReference = FirebaseDatabase.getInstance().getReference().child(FIREBASE_ROOT);
                        dbLoadingState = INITIALIZED;
                    }
                    workResultCode = CODE_OK;
                break;

            case LOAD_DEVICE_BASICS:
                switch (dbLoadingState) {
                    case NOT_STARTED:
                        while (dbLoadingState != INITIALIZED) { SleepMillis(500); }
                        fbo.LoadDeviceBasics();
                        dbLoadingState = DEVICES_LOADING;
                        break;
                    case INITIALIZED:
                        fbo.LoadDeviceBasics();
                        while (dbLoadingState != DEVICES_LOADED) { SleepMillis(500); }
                        // dbLoadingState set to DEVICES_LOADING in LoadDeviceBasics();
                        break;
                    case DEVICES_LOADING:  // started from another call. Just wait for that one to finish.
                        while (dbLoadingState != DEVICES_LOADED) { SleepMillis(500); }
                        break;
                    case DEVICES_LOADED:
                        break;
                }
                workResultCode = CODE_OK;
                break;

            case LOAD_DEVICE_TELEMETRY:
                while (dbLoadingState != DEVICES_LOADED) {
                    SleepMillis(500);
                }
                int deviceNbr = commandIntent.getIntExtra(DEVICE_NBR, -1);
                if (!dbDeviceLoaded[deviceNbr]) {
                    fbo.LoadDeviceTelemetry(deviceNbr);
                    // dbDeviceLoaded[] set to TRUE in LoadDeviceTelemetry();
                    while (!dbDeviceLoaded[deviceNbr]) {
                        SleepMillis(500);
                    }
                }
                workResultCode = CODE_OK;
                break;

            case ACTION_INFO:  // this is an info call, simply tell the server we want the info
                workResultCode = sendInfoRequest();

            /* In the case of this GET request, our info is returned from the server in the form of a JSON
               String.  The details and error checking are left out here but after parsing the string and
               confirming a successful request, we have access to the raw info via getInfoString() */
                resultIntent.putExtra(INFO_TEXT, "myHttpClass.getJsonResponse.getInfoString()");
                break;
            case ACTION_NONE:  // NOOP
                break;

            case PURGE_LOG_AND_TELE:
                int keepTeleDays = commandIntent.getIntExtra("keepTeleDays",-1);
                int keepLogDays = commandIntent.getIntExtra("keepLogDays",-1);
                int keepEveryN = commandIntent.getIntExtra("keepEveryN",-1);
                int selectedDevice = commandIntent.getIntExtra(FirebaseService.DEVICE_NBR, -1);
                fbo.PurgeLogAndTeleData(selectedDevice, keepTeleDays, keepLogDays, keepEveryN);
                workResultCode = CODE_OK;

                break;
        }

        try {
            /* inform the Activity which of the web service calls these results are for */
            resultIntent.putExtra(WORK_DONE, workToDo);

            /* Call back to the Activity with the result code and the data to parse if successful */
            returnIntent.send(this, workResultCode, resultIntent);
        } catch (PendingIntent.CanceledException e) {
            Log.e(getClass().getSimpleName(), "Pending intent was canceled", e);
        }
    }

    private void SleepMillis(int millisecs){
        try {
            Thread.sleep(millisecs);
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }

    private int sendInfoRequest(){
        return CODE_OK;
    }

}

