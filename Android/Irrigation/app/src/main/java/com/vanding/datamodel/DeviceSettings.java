package com.vanding.datamodel;

import com.vanding.irrigation.SingleDevice;

import static com.vanding.irrigation.db.DEVICE_TYPE_DIST;
import static com.vanding.irrigation.db.DEVICE_TYPE_GAS;
import static com.vanding.irrigation.db.DEVICE_TYPE_HUMTEMP;
import static com.vanding.irrigation.db.DEVICE_TYPE_PUMP;
import static com.vanding.irrigation.db.DEVICE_TYPE_SOIL;
import static com.vanding.irrigation.db.DEVICE_TYPE_WATER;
import static com.vanding.irrigation.db.NBR_OF_DEVICE_TYPES;

public class DeviceSettings {

    public static DeviceSetting[] gs;
    {
        gs = new DeviceSetting[NBR_OF_DEVICE_TYPES];
    }

    public DeviceSettings () {
        gs[DEVICE_TYPE_SOIL] = new DeviceSetting(
                "Humidity %",2, "H",
                "Valve",2, "vlvState",
                "Batt [V]",2, "Vcc",
                false,
                false,
                0, 1,
                3.3, 4.1 );

        gs[DEVICE_TYPE_GAS] = new DeviceSetting(
                "PPM",2, "cur_ppm",
                "DUTY",2, "phase",
                "Wifi",2, "W",
                true,
                true,
                0, 1,
                0, 1);

        gs[DEVICE_TYPE_HUMTEMP] = new DeviceSetting(
                "temp C",2, "T",
                "n/a",2, "N/A",
                "Hum %",2, "H", //"DewTemp"
                // "Dug tmp",2,
                false,
                false,
                5, 30,
                0, 100);

        gs[DEVICE_TYPE_WATER] = new DeviceSetting(
                "PPM",2, "ppm",
                "DUTY",2, "duty",
                "Wifi",2, "W",
                true,
                true,
                0, 1,
                0, 1);

        gs[DEVICE_TYPE_DIST] = new DeviceSetting(
                "dist cm",2, "Dist",
                "n/a",2, "N/A",
                "Temp C",2, "T",
                false,
                false,
                -20, 20,
                -20, 30);

        gs[DEVICE_TYPE_PUMP] = new DeviceSetting(
                "State",2, "state",
                "n/a",2, "N/A",
                "Wifi",2, "W",
                false,
                false,
                -1, 2,
                -80, -50);

    }

    public class DeviceSetting {

        public String titlePrim1;
        public String dataNamePrim1;
        public String titlePrim2;
        public String dataNamePrim2;
        public String titleSec1;
        public String dataNameSec1;
        public boolean autoScalePrim;
        public boolean autoScaleSec;
        public double minPrim;
        public double maxPrim;
        public double minSec;
        public double maxSec;

        public DeviceSetting(
                String _titlePrim1, int _thickness1, String _dataNamePrim1,
                String _titlePrim2, int _thickness2, String _dataNamePrim2,
                String _titleSec1, int _thickness3, String _dataNameSec1,
                boolean _autoScalePrim,
                boolean _autoScaleSec,
                double _minPrim,
                double _maxPrim,
                double _minSec,
                double _maxSec
        ) {
            this.titlePrim1 = _titlePrim1;
            this.titlePrim2 = _titlePrim2;
            this.titleSec1 = _titleSec1;
            this.dataNamePrim1 = _dataNamePrim1;
            this.dataNamePrim2 = _dataNamePrim2;
            this.dataNameSec1 = _dataNameSec1;
            this.autoScalePrim = _autoScalePrim;
            this.autoScaleSec = _autoScaleSec;
            this.minPrim = _minPrim;
            this.maxPrim = _maxPrim;
            this.minSec = _minSec;
            this.maxSec = _maxSec;
        }
    }

}

