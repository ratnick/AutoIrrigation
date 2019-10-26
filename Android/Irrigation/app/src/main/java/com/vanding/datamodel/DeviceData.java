package com.vanding.datamodel;

import java.util.ArrayList;

public class DeviceData {
    private String mName;
    private boolean mOnline;

    public DeviceData(String name, boolean online) {
        mName = name;
        mOnline = online;
    }

    public String getName() {
        return mName;
    }

    public boolean isOnline() {
        return mOnline;
    }

    private static int lastContactId = 0;

    public static ArrayList<DeviceData> createContactsList(int numContacts) {
        ArrayList<DeviceData> contacts = new ArrayList<DeviceData>();

        for (int i = 1; i <= numContacts; i++) {
            contacts.add(new DeviceData("Person " + ++lastContactId, i <= numContacts / 2));
        }

        return contacts;
    }
}