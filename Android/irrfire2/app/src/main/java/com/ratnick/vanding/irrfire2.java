package com.ratnick.vanding;

import com.google.firebase.FirebaseApp;

public class irrfire2 extends android.app.Application {
    @Override
    public void onCreate() {
        super.onCreate();
        FirebaseApp.initializeApp(this);
    }

}
