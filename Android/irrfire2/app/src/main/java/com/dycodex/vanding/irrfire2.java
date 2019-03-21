package com.dycodex.vanding;

import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.FirebaseApp;

import static com.google.firebase.database.FirebaseDatabase.*;

public class irrfire2 extends android.app.Application {
    @Override
    public void onCreate() {
        super.onCreate();
        FirebaseApp.initializeApp(this);
    }

}
