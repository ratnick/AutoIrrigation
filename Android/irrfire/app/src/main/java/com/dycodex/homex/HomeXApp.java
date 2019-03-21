package com.dycodex.homex;

import com.firebase.client.Firebase;

/**
 * Created by andri on 10/15/15.
 */
public class HomeXApp extends android.app.Application {
    @Override
    public void onCreate() {
        super.onCreate();
        Firebase.setAndroidContext(this);
    }
}
