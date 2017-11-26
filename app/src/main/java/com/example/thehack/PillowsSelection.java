package com.example.thehack;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

public class PillowsSelection extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pillows_selection);
    }

    /** Called when the user taps the pillow button */
    public void choosePillow(View view) {
        Intent intent = new Intent(this, PillowSmash.class);
        startActivity(intent);
    }

}
