package com.vanding.irrigation;

import com.google.firebase.database.DatabaseReference;
import com.vanding.datamodel.IrrDevice;

// Firebase Static data
public class db {

    public static int MAX_DEVICES = 10;
    public enum dbFirebaseLoadingState {NOT_STARTED, INITIALIZED, DEVICES_LOADING, DEVICES_LOADED};
    public static dbFirebaseLoadingState dbLoadingState = dbFirebaseLoadingState.NOT_STARTED;
    public static boolean[] dbDeviceLoaded = new boolean[MAX_DEVICES];
    public static DatabaseReference dbAllDevicesReference;
    public static DatabaseReference[] dbDeviceReference = new DatabaseReference[MAX_DEVICES];
    public static String[] dbIrrDeviceStrings = new String[MAX_DEVICES];
    public static IrrDevice[] dbIrrDevice = new IrrDevice[MAX_DEVICES];
    public static int dbSelectedIrrDeviceK = 0;
    public static int dbNbrOfDevices = 0;

    public static final String DEVICE_TYPE_SOIL_STR = "soil";
    public static final String DEVICE_TYPE_WATER_STR = "water";
    public static final String DEVICE_TYPE_GAS_STR = "gas";
    public static final String DEVICE_TYPE_HUMTEMP_STR = "Temp+hum";

    public static final int DEVICE_TYPE_SOIL = 0;
    public static final int DEVICE_TYPE_GAS = 1;
    public static final int DEVICE_TYPE_HUMTEMP = 2;
    public static final int DEVICE_TYPE_WATER = 3;

    public static final int NBR_OF_DEVICE_TYPES = 4;
//    public static enum DEVICE_TYPE_ENUM {SOIL, GAS, HUMTEMP, WATER};

}
