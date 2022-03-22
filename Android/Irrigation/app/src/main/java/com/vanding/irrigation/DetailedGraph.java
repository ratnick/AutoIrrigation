package com.vanding.irrigation;

import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;

import com.jjoe64.graphview.DefaultLabelFormatter;
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GridLabelRenderer;
import com.jjoe64.graphview.helper.DateAsXAxisLabelFormatter;
import com.vanding.datamodel.DeviceSettings;

import androidx.appcompat.app.AppCompatActivity;

import java.text.NumberFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import static android.graphics.Color.BLUE;
import static android.graphics.Color.WHITE;
import static android.graphics.Color.YELLOW;
import static com.vanding.irrigation.FirebaseService.DEVICE_NBR;
import static com.vanding.irrigation.Common.*;
//import static com.vanding.irrigation.SingleDevice.gs;
//import static com.vanding.datamodel.DeviceSettings.gs;
import static com.vanding.irrigation.db.*;


public class DetailedGraph extends AppCompatActivity {

    private GraphView graph;

    // global constants
    private static DeviceSettings ds; {  ds = new DeviceSettings(); }
    private int selectedDevice = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_detailed_graph);
        graph = (GraphView) findViewById(R.id.graph);
        selectedDevice = this.getIntent().getIntExtra(DEVICE_NBR, -1);
        DisplayGraphData();
    }

    private void DisplayGraphData() {
        graph.removeAllSeries();
        graph.getSecondScale().removeAllSeries();
        // primary Y-axis:
        graph.addSeries(dbIrrDevice[selectedDevice].xSeriesPrimAxis1);
        graph.addSeries(dbIrrDevice[selectedDevice].xSeriesPrimAxis2);
        // secondary Y-axis:
        graph.getSecondScale().addSeries(dbIrrDevice[selectedDevice].xSeriesSecAxis1);

        FormatSeries(graph);
        FormatGraph(graph);
    }


    private void FormatSeries(GraphView graph) {

        int devType = SingleDevice.GetDeviceType(dbIrrDevice[selectedDevice].metadata.sensorType);

        dbIrrDevice[selectedDevice].xSeriesPrimAxis1.setTitle(ds.gs[devType].titlePrim1);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis1.setThickness(2);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis1.setColor(WHITE);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis1.setDrawDataPoints(false);

        dbIrrDevice[selectedDevice].xSeriesPrimAxis2.setTitle(ds.gs[devType].titlePrim2);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis2.setThickness(2);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis2.setColor(BLUE);
        dbIrrDevice[selectedDevice].xSeriesPrimAxis2.setDrawDataPoints(false);

        dbIrrDevice[selectedDevice].xSeriesSecAxis1.setTitle(ds.gs[devType].titleSec1);
        dbIrrDevice[selectedDevice].xSeriesSecAxis1.setThickness(2);
        dbIrrDevice[selectedDevice].xSeriesSecAxis1.setColor(YELLOW);
        dbIrrDevice[selectedDevice].xSeriesSecAxis1.setDrawDataPoints(false);
    }

    private void FormatGraph(GraphView graph) {

        long minX, maxX;
        double minY1, minY1b, maxY1, maxY1b, minY2, maxY2;
        Date minDate, maxDate;
        int devType = SingleDevice.GetDeviceType(dbIrrDevice[selectedDevice].metadata.sensorType);

        // Common for both Y-axes
        NumberFormat nf = NumberFormat.getInstance();
        nf.setMinimumFractionDigits(1);
        nf.setMaximumFractionDigits(1);
        //nf.setMinimumIntegerDigits(1);
        graph.getGridLabelRenderer().setLabelFormatter(new DefaultLabelFormatter(nf, nf));
        graph.getGridLabelRenderer().setHorizontalLabelsColor(WHITE);

        // Primary Y-axis (Y1) scale
        graph.getViewport().setYAxisBoundsManual(ds.gs[devType].autoScalePrim);
        if(ds.gs[devType].autoScalePrim) {
            minY1 = dbIrrDevice[selectedDevice].xSeriesPrimAxis1.getLowestValueY();
            minY1b = dbIrrDevice[selectedDevice].xSeriesPrimAxis2.getLowestValueY();
            minY1 = RoundToNearestNiceNumber(Math.min(minY1, minY1b), false);
            maxY1 = dbIrrDevice[selectedDevice].xSeriesPrimAxis1.getHighestValueY();
            maxY1b = dbIrrDevice[selectedDevice].xSeriesPrimAxis2.getHighestValueY();
            maxY1 = RoundToNearestNiceNumber(Math.max(maxY1, maxY1b), true);

/*            minY1 = dbIrrDevice[selectedDevice].xSeriesPrimaryTm.getLowestValueY();
            minY1 = RoundToNearestNiceNumber(minY1, false);
            maxY1 = dbIrrDevice[selectedDevice].xSeriesPrimaryTm.getHighestValueY();
            maxY1 = RoundToNearestNiceNumber(maxY1, true);
*/
            graph.getViewport().setMinY(minY1); //minY1;
            graph.getViewport().setMaxY(maxY1);
        } else {
            graph.getViewport().setMinY(ds.gs[devType].minPrim);
            graph.getViewport().setMaxY(ds.gs[devType].maxPrim);
        }
        graph.getGridLabelRenderer().setVerticalLabelsColor(WHITE);
        graph.getGridLabelRenderer().setLabelsSpace(20);
        graph.getGridLabelRenderer().setLabelVerticalWidth(100);

        // Secondary Y-axis (Y2) scale
        // set second scale manually (http://www.android-graphview.org/secondary-scale-axis/ : the y bounds are always manual for second scale
        graph.getViewport().setYAxisBoundsManual(ds.gs[devType].autoScaleSec);
        if(ds.gs[devType].autoScaleSec) {
            minY2 = dbIrrDevice[selectedDevice].xSeriesSecAxis1.getLowestValueY();
            minY2 = RoundToNearestNiceNumber(minY2, false);
            maxY2 = dbIrrDevice[selectedDevice].xSeriesSecAxis1.getHighestValueY();
            maxY2 = RoundToNearestNiceNumber(maxY2, true);
            graph.getSecondScale().setMinY(minY2); //minY2
            graph.getSecondScale().setMaxY(maxY2);
        } else {
            graph.getSecondScale().setMinY(ds.gs[devType].minSec);
            graph.getSecondScale().setMaxY(ds.gs[devType].maxSec);
        }
        graph.getGridLabelRenderer().setVerticalLabelsSecondScaleColor(YELLOW);
        graph.getGridLabelRenderer().setSecondScaleLabelVerticalWidth(70);
        graph.getGridLabelRenderer().setNumVerticalLabels(7);
        graph.getGridLabelRenderer().setVerticalLabelsSecondScaleAlign(Paint.Align.RIGHT);

        // X-axis auto date labels
        SimpleDateFormat sfd = new SimpleDateFormat("dd/MM-yy\nHH:mm:ss");
        graph.getGridLabelRenderer().setLabelFormatter(new DateAsXAxisLabelFormatter(this,sfd));
        graph.getGridLabelRenderer().setNumHorizontalLabels(5); // only 2 because of the space

        // X-axis
        minX = (long) dbIrrDevice[selectedDevice].xSeriesSecAxis1.getLowestValueX();
        maxX = (long) dbIrrDevice[selectedDevice].xSeriesSecAxis1.getHighestValueX();
        if (maxX - minX > 86400000) {  // 24 hours
            minX = maxX - 86400000;
        }
        minDate = new Date(minX);
        maxDate = new Date(maxX);
        graph.getViewport().setXAxisBoundsManual(true);
        graph.getViewport().setMinX(minX);
        graph.getViewport().setMaxX(maxX);
        graph.getGridLabelRenderer().setHumanRounding(false);

        // Drawing area
        graph.setTitleColor(YELLOW);
        graph.setTitle(dbIrrDevice[selectedDevice].metadata.loc + " - " + dbIrrDevice[selectedDevice].metadata.sensorType);
        graph.setBackgroundColor(Color.DKGRAY);
        graph.getViewport().setBackgroundColor(Color.BLACK);
        graph.getGridLabelRenderer().setGridColor(Color.DKGRAY);
        graph.getGridLabelRenderer().setGridStyle(GridLabelRenderer.GridStyle.BOTH);
        //graph_.getViewport().setDrawBorder(true);
        //graph_.getViewport().setBorderColor(Color.YELLOW);

        // Scaling and moving
        graph.getViewport().setScalable(true); // activate horizontal zooming and scrolling
        graph.getViewport().setScrollable(true);  // activate horizontal scrolling
        //graph_.getViewport().setScalableY(true); // activate horizontal and vertical zooming and scrolling
        //graph_.getViewport().setScrollableY(true);    // activate vertical scrolling

        // Legend
        graph.getLegendRenderer().setVisible(true);
        graph.getLegendRenderer().setFixedPosition(0,200);
        graph.getLegendRenderer().setTextSize(25);
        graph.getLegendRenderer().setTextColor(Color.WHITE);
        graph.getLegendRenderer().setBackgroundColor(Color.BLACK);

        graph.onDataChanged(true,false);
    }
}
