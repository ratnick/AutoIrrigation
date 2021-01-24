package com.vanding.irrigation;

import com.google.firebase.FirebaseApp;

import android.app.PendingIntent;
import android.content.Intent;
import android.os.Bundle;

import com.vanding.datamodel.DeviceData;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import java.util.ArrayList;
import static com.vanding.irrigation.FirebaseService.*;
import static com.vanding.irrigation.db.*;

public class MainActivity extends AppCompatActivity {

    // UI elements
    ArrayList<DeviceData> dummyList;  // NOTE: This is totally dummy list. We use global variables instead.

    // Create adapter passing in the sample user data
    DevicesAdapter adapter = new DevicesAdapter(dummyList);
    RecyclerView rvContacts;
    private Button btnnext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(false);

        FirebaseApp.initializeApp(this);

        // Initiate loading of basic data for all devices
        Intent initIntent = new Intent(MainActivity.this, FirebaseService.class);
        startFirebaseService(ActionType.INIT_SERVICE, initIntent);

        Intent commandIntent = new Intent(MainActivity.this, FirebaseService.class);
        startFirebaseService(ActionType.LOAD_DEVICE_BASICS, commandIntent);

        // Lookup the recyclerview in activity layout
        rvContacts = (RecyclerView) findViewById(R.id.rvContacts);

        // start sub activity (for test)
        //Intent intent = new Intent(MainActivity.this, testlayout.class);
        //startActivity(intent);

        /*        // start sub activity (for test)
        SleepMillis(500);
        Intent intent = new Intent(MainActivity.this, SingleDevice.class);
        intent.putExtra(DEVICE_NBR, 2);   // TESTING NNR
        startActivity(intent);
*/
    }

    private void SleepMillis(int millisecs){
        try {
            Thread.sleep(millisecs);
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }

    // Generic wrapper called for all commands to FirebaseService
    private void startFirebaseService(ActionType sendType, Intent commandIntent) {
      /* create a pending intent callback for when we send back result
         info from our Service to our Activity.  The Receipt of the
         PendingIntent will go through onActivityResult just as if
         we had called startActivityForResult */
        PendingIntent returnIntent = createPendingResult(
                FirebaseService.FIREBASE_SERVICE_ID,
                new Intent(this, MainActivity.class),0);
        commandIntent.putExtra(FirebaseService.ACTION_TYPE, sendType);
        commandIntent.putExtra(FirebaseService.PENDING_RESULT, returnIntent);
        startService(commandIntent);
    }

    /* Here is the entry point for the returning PendingIntent.
       -- requestCode = The identifier letting us know where these results are coming from.
       -- resultCode = The code telling us whether our webservce calls succeeded or failed.
       -- data = The bundled extras that we can parse our results from once we know the call succeeded.*/
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent returnIntent) {
        if (requestCode == FIREBASE_SERVICE_ID) {
            if (resultCode == CODE_OK) {
                ActionType actionPerformed = (ActionType) returnIntent.getSerializableExtra(WORK_DONE);
                processServiceReturn(actionPerformed, returnIntent);
            } else {
                int fail = 0/0;
                // here we could either fail silently or pop a dialog or Toast up informing the user there was a problem.
            }
        }
    }

    /* Finally, once we are here we know the service call succeeded and we can act accordingly. */
    private void processServiceReturn(ActionType action, Intent returnIntent){
        switch(action){
            case LOAD_DEVICE_BASICS:
                // Attach the adapter to the recyclerview to populate items
                rvContacts.setAdapter(adapter);

                // Set layout manager to position the items
                rvContacts.setLayoutManager(new LinearLayoutManager(this));

                // start sub activity (for test)
//                Intent intent = new Intent(MainActivity.this, testlayout.class);
//                startActivity(intent);

                break;
            case ACTION_INFO:
                //String info = data.getStringExtra(FirebaseService.INFO_TEXT);
                break;
            case ACTION_NONE:  // NOOP
                break;
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
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
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    private long mBackPressed;
    private static final int TIME_INTERVAL = 2000;
    @Override
    public void onBackPressed() {
        if (mBackPressed + TIME_INTERVAL > System.currentTimeMillis()) {
            super.onBackPressed();
            return;
        } else {
            Toast.makeText(getBaseContext(), "Click two times to close an activity",    Toast.LENGTH_SHORT).show();
        }
        mBackPressed = System.currentTimeMillis();
    }

}
