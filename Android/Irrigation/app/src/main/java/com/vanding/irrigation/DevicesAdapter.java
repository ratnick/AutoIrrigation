package com.vanding.irrigation;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.recyclerview.widget.RecyclerView;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;

import com.vanding.datamodel.DeviceData;

import org.json.JSONArray;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import static android.graphics.Color.BLACK;
import static android.graphics.Color.RED;
import static android.graphics.Color.YELLOW;
import static com.vanding.irrigation.FirebaseService.DEVICE_NBR;
import static com.vanding.irrigation.db.DEVICE_TYPE_DIST_STR;
import static com.vanding.irrigation.db.DEVICE_TYPE_GAS_STR;
import static com.vanding.irrigation.db.DEVICE_TYPE_HUMTEMP;
import static com.vanding.irrigation.db.DEVICE_TYPE_HUMTEMP_STR;
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
        public TextView tVal1;
        public TextView tVal2;
        public TextView tVal3;
        public TextView tVal4;
        public TextView tUnit1;
        public TextView tUnit2;
        public TextView tUnit3;
        public TextView tUnit4;
        public ImageView imStatusColor;
        protected ConstraintLayout deviceLine ;


        // We also create a constructor that accepts the entire item row
        // and does the view lookups to find each subview
        public ViewHolder(View itemView) {
            // Stores the itemView in a public final member variable that can be used
            // to access the context from any ViewHolder instance.
            super(itemView);

            tLocation = (TextView) itemView.findViewById(R.id.tLocation);
            tSensorType = (TextView) itemView.findViewById(R.id.tSensorType);
            tTimestamp = (TextView) itemView.findViewById(R.id.tTimestamp);
            tVal1 = (TextView) itemView.findViewById(R.id.tVal1);
            tVal2 = (TextView) itemView.findViewById(R.id.tVal2);
            tVal3 = (TextView) itemView.findViewById(R.id.tVal3);
            tVal4 = (TextView) itemView.findViewById(R.id.tVal4);
            tUnit1 = (TextView) itemView.findViewById(R.id.tUnit1);
            tUnit2 = (TextView) itemView.findViewById(R.id.tUnit2);
            tUnit3 = (TextView) itemView.findViewById(R.id.tUnit3);
            tUnit4 = (TextView) itemView.findViewById(R.id.tUnit4);
            imStatusColor = (ImageView) itemView.findViewById(R.id.imStatusColor);
            deviceLine = (ConstraintLayout) itemView.findViewById(R.id.deviceLine);
            deviceLine.setOnClickListener(this);
        }

        // onClick Listener for view
        // Not sure if it's ever used. But a method to transfer view "ID" to caller
        @Override
        public void onClick(View v) {
            View tempview = (View) deviceLine.getTag(dbSelectedIrrDeviceK);
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
        TextView tVal1 = viewHolder.tVal1;
        TextView tVal2 = viewHolder.tVal2;
        TextView tVal3 = viewHolder.tVal3;
        TextView tVal4 = viewHolder.tVal4;
        TextView tUnit1 = viewHolder.tUnit1;
        TextView tUnit2 = viewHolder.tUnit2;
        TextView tUnit3 = viewHolder.tUnit3;
        TextView tUnit4 = viewHolder.tUnit4;
        ImageView imStatusColor = viewHolder.imStatusColor;
        ConstraintLayout deviceLine = viewHolder.deviceLine;

        // Set item views based on your views and data model
        tLocation.setText(dbIrrDevice[position].metadata.loc);
        tSensorType.setText(dbIrrDevice[position].metadata.sensorType);
        SimpleDateFormat sfd = new SimpleDateFormat("dd-MM-yyyy HH:mm");
        tTimestamp.setText(sfd.format(new Date(dbIrrDevice[position].telemetry_current.timestamp)));

        switch (dbIrrDevice[position].metadata.sensorType) {
            case DEVICE_TYPE_SOIL_STR:
                tVal1.setText(String.format("%.0f", dbIrrDevice[position].telemetry_current.Hum));
                tUnit1.setText(String.format("%%"));
                tVal3.setText(String.format("%.1f", dbIrrDevice[position].telemetry_current.Vcc));
                tUnit3.setText(String.format("V"));
                break;
            case DEVICE_TYPE_GAS_STR:
                tVal1.setText(String.format("%.0f", dbIrrDevice[position].telemetry_current.cur_ppm));
                tUnit1.setText(String.format("ppm"));
                break;
            case DEVICE_TYPE_HUMTEMP_STR:
                tVal1.setText(String.format("%.0f", dbIrrDevice[position].telemetry_current.Hum));
                tUnit1.setText(String.format("%%"));
                tVal2.setText(String.format("%.1f", dbIrrDevice[position].telemetry_current.Temp));
                tUnit2.setText(String.format("C"));
                break;
            case DEVICE_TYPE_DIST_STR:
                tVal1.setText(String.format("%.0f", dbIrrDevice[position].telemetry_current.Dist));
                tUnit1.setText(String.format("cm"));
                tVal2.setText(String.format("%.1f", dbIrrDevice[position].telemetry_current.Temp));
                tUnit2.setText(String.format("C"));
                break;
            default:
                break;
        }
        tVal4.setText(String.format("%d", dbIrrDevice[position].telemetry_current.Wifi));
        tUnit4.setText(String.format("db"));

        if (dbIrrDevice[position].state.deviceStatus >= 10) {
            imStatusColor.setBackgroundColor(RED);
        } else if (dbIrrDevice[position].state.deviceStatus > 0) {
            imStatusColor.setBackgroundColor(YELLOW);
        } else {
            imStatusColor.setBackgroundColor(BLACK);
        }

        deviceLine.setOnClickListener(new View.OnClickListener() {
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
