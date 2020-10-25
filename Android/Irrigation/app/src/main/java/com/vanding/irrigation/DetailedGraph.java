package com.vanding.irrigation;

import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;

import com.jjoe64.graphview.DefaultLabelFormatter;
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GridLabelRenderer;
import com.jjoe64.graphview.helper.DateAsXAxisLabelFormatter;
import androidx.appcompat.app.AppCompatActivity;

import java.text.NumberFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import static android.graphics.Color.CYAN;
import static android.graphics.Color.GREEN;
import static android.graphics.Color.MAGENTA;
import static android.graphics.Color.WHITE;
import static android.graphics.Color.YELLOW;
import static com.vanding.irrigation.FirebaseService.DEVICE_NBR;
import static com.vanding.irrigation.SingleDevice.RoundUpToNearestNiceNumber;
import static com.vanding.irrigation.SingleDevice.graph;
import static com.vanding.irrigation.SingleDevice.gs;
import static com.vanding.irrigation.db.*;


public class DetailedGraph extends AppCompatActivity {

    private GraphView graph;

    // global constants
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
        graph.addSeries(dbIrrDevice[selectedDevice].xSeriesSecTm);
        graph.addSeries(dbIrrDevice[selectedDevice].xSeriesPrimaryTm);

        // secondary Y-axis:
        graph.getSecondScale().addSeries(dbIrrDevice[selectedDevice].xSeriesVcc);

        FormatSeries(graph);
        FormatGraph(graph);
    }


    private void FormatSeries(GraphView graph) {

        int devType = SingleDevice.GetDeviceType(dbIrrDevice[selectedDevice].metadata.sensorType);

        // Raw
        //dbIrrDevice[selectedDevice].xSeriesPrimaryTm.setTitle("Humidity %");
        dbIrrDevice[selectedDevice].xSeriesPrimaryTm.setTitle(gs[devType].titlePrim);
        dbIrrDevice[selectedDevice].xSeriesPrimaryTm.setThickness(2);
        dbIrrDevice[selectedDevice].xSeriesPrimaryTm.setColor(WHITE);
        dbIrrDevice[selectedDevice].xSeriesPrimaryTm.setDrawDataPoints(false);

        // Valve position
        dbIrrDevice[selectedDevice].xSeriesSecTm.setTitle(gs[devType].titleSec);
        dbIrrDevice[selectedDevice].xSeriesSecTm.setThickness(2);
        dbIrrDevice[selectedDevice].xSeriesSecTm.setColor(YELLOW);
        dbIrrDevice[selectedDevice].xSeriesSecTm.setDrawDataPoints(false);

        // Vcc
        dbIrrDevice[selectedDevice].xSeriesVcc.setTitle(gs[devType].titleVcc);
        dbIrrDevice[selectedDevice].xSeriesVcc.setThickness(2);
        dbIrrDevice[selectedDevice].xSeriesVcc.setColor(CYAN);
        dbIrrDevice[selectedDevice].xSeriesVcc.setDrawDataPoints(false);

    }

    private void FormatGraph(GraphView graph) {

        long minX, maxX, minY1, maxY1, minY2, maxY2;
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
        graph.getViewport().setYAxisBoundsManual(gs[devType].autoScalePrim);
        if(gs[devType].autoScalePrim) {
            minY1 = (long) dbIrrDevice[selectedDevice].xSeriesPrimaryTm.getLowestValueY();
            maxY1 = (long) dbIrrDevice[selectedDevice].xSeriesPrimaryTm.getHighestValueY();
            maxY1 = RoundUpToNearestNiceNumber(maxY1);
            graph.getViewport().setMinY(0); //minY1;
            graph.getViewport().setMaxY(maxY1);
        } else {
            graph.getViewport().setMinY(gs[devType].minPrim);
            graph.getViewport().setMaxY(gs[devType].maxPrim);
        }
        graph.getGridLabelRenderer().setVerticalLabelsColor(WHITE);
        graph.getGridLabelRenderer().setLabelsSpace(20);

        // Secondary Y-axis (Y2) scale
        // set second scale manually (http://www.android-graphview.org/secondary-scale-axis/ : the y bounds are always manual for second scale
        graph.getViewport().setYAxisBoundsManual(gs[devType].autoScaleSec);
        if(gs[devType].autoScaleSec) {
            minY2 = (long) dbIrrDevice[selectedDevice].xSeriesVcc.getLowestValueY();
            maxY2 = (long) dbIrrDevice[selectedDevice].xSeriesVcc.getHighestValueY();
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
        graph.getGridLabelRenderer().setNumHorizontalLabels(5); // only 2 because of the space

        // X-axis
        minX = (long) dbIrrDevice[selectedDevice].xSeriesVcc.getLowestValueX();
        maxX = (long) dbIrrDevice[selectedDevice].xSeriesVcc.getHighestValueX();
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
        graph.getGridLabelRenderer().setGridStyle(GridLabelRenderer.GridStyle.VERTICAL);
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
