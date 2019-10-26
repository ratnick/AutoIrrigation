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

}
