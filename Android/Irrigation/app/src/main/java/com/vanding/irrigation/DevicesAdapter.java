package com.vanding.irrigation;

import android.content.Context;
import android.content.Intent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.recyclerview.widget.RecyclerView;

import com.vanding.datamodel.DeviceData;

import java.text.SimpleDateFormat;
import java.util.List;

import static android.graphics.Color.BLACK;
import static android.graphics.Color.RED;
import static android.graphics.Color.YELLOW;
import static com.vanding.irrigation.FirebaseService.DEVICE_NBR;
import static com.vanding.irrigation.db.DEVICE_TYPE_GAS_STR;
import static com.vanding.irrigation.db.DEVICE_TYPE_SOIL_STR;
import static com.vanding.irrigation.db.dbIrrDevice;
import static com.vanding.irrigation.db.dbSelectedIrrDeviceK;

// Create the basic adapter extending from RecyclerView.Adapter
// Note that we specify the custom ViewHolder which gives us access to our views
public class DevicesAdapter extends
        RecyclerView.Adapter<DevicesAdapter.ViewHolder> {

    // Provide a direct reference to each of the views within a data item
    // Used to cache the views within the item layout for fast access
    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        // Your holder should contain a member variable
        // for any view that will be set as you render a row

        public TextView tLocation ;
        public TextView tSensorType;
        public TextView tTimestamp ;
        public TextView tHumidity ;
        public TextView tVcc ;
        public ImageView imStatusColor;
        protected Button btnSelectDevice ;


        // We also create a constructor that accepts the entire item row
        // and does the view lookups to find each subview
        public ViewHolder(View itemView) {
            // Stores the itemView in a public final member variable that can be used
            // to access the context from any ViewHolder instance.
            super(itemView);

            tLocation = (TextView) itemView.findViewById(R.id.tLocation);
            tSensorType = (TextView) itemView.findViewById(R.id.tSensorType);
            tTimestamp = (TextView) itemView.findViewById(R.id.tTimestamp);
            tHumidity = (TextView) itemView.findViewById(R.id.tHumidity);
            tVcc = (TextView) itemView.findViewById(R.id.tVcc);
            imStatusColor = (ImageView) itemView.findViewById(R.id.imStatusColor);
            btnSelectDevice = (Button) itemView.findViewById(R.id.btnSelectDevice);
            //btnSelectDevice.setTag(R.integer.btnSelectDevice, itemView);
            btnSelectDevice.setOnClickListener(this);

        }

        // onClick Listener for view
        // Not sure if it's ever used. But a method to transfer view "ID" to caller
        @Override
        public void onClick(View v) {
            View tempview = (View) btnSelectDevice.getTag(dbSelectedIrrDeviceK);
        }
    }

    // Store a member variable for the dummyList
    private List<DeviceData> mContacts;

    // Pass in the contact array into the constructor
    public DevicesAdapter(List<DeviceData> contacts) {
        mContacts = contacts;
    }

    // Usually involves inflating a layout from XML and returning the holder
    @Override
    public DevicesAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        Context context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(context);

        // Inflate the custom layout
        View contactView = inflater.inflate(R.layout.device_line_layout, parent, false);

        // Return a new holder instance
        ViewHolder viewHolder = new ViewHolder(contactView);
        return viewHolder;
    }

    // Involves populating data into the item through holder
    @Override
    public void onBindViewHolder(final DevicesAdapter.ViewHolder viewHolder, int position) {
        // Get the data model based on position0

        TextView tLocation = viewHolder.tLocation;
        TextView tSensorType = viewHolder.tSensorType;
        TextView tTimestamp = viewHolder.tTimestamp;
        TextView tHumidity = viewHolder.tHumidity;
        TextView tVcc = viewHolder.tVcc;
        ImageView imStatusColor = viewHolder.imStatusColor;
        Button btnSelectDevice = viewHolder.btnSelectDevice;

        // Set item views based on your views and data model
        tLocation.setText(dbIrrDevice[position].metadata.loc);
        tSensorType.setText(dbIrrDevice[position].metadata.sensorType);
        SimpleDateFormat sfd = new SimpleDateFormat("dd-MM-yyyy HH:mm");
        tTimestamp.setText("reimplement in Arduino");
//TODO:reimplement in Arduino        tTimestamp.setText(sfd.format(new Date(dbIrrDevice[position].telemetry_current.timestamp)));
        switch (dbIrrDevice[position].metadata.sensorType) {
            case DEVICE_TYPE_SOIL_STR:
                tHumidity.setText(String.format("%.0f", dbIrrDevice[position].telemetry_current.Hum));
                break;
            case DEVICE_TYPE_GAS_STR:
                tHumidity.setText(String.format("%.0f", dbIrrDevice[position].telemetry_current.cur_ppm));
                break;
            default:
                break;
        }
        tVcc.setText(String.format("%.2f",dbIrrDevice[position].telemetry_current.Vcc));

        if (dbIrrDevice[position].state.deviceStatus >= 10) {
            imStatusColor.setBackgroundColor(RED);
        } else if (dbIrrDevice[position].state.deviceStatus > 0) {
            imStatusColor.setBackgroundColor(YELLOW);
        } else {
            imStatusColor.setBackgroundColor(BLACK);
        }

        btnSelectDevice.setText("DETAILS");
        btnSelectDevice.setEnabled(true);

        btnSelectDevice.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                dbSelectedIrrDeviceK = viewHolder.getAdapterPosition();
                Intent intent = new Intent(view.getContext(), SingleDevice.class);
                intent.putExtra(DEVICE_NBR, dbSelectedIrrDeviceK);   // TESTING NNR
                view.getContext().startActivity(intent);
            }
        });
    }

    // Returns the total count of items in the list
    @Override
    public int getItemCount() {
        return db.dbNbrOfDevices;
    }

}