package com.example.thehack;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.ProgressBar;

public class PillowSmash extends AppCompatActivity {

    /* Every class deserves an own pillow! */
    private Pillow p1 = new Pillow();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pillow_smash);
    }

    /** Called when the user gets 100% hits */
    public void completedGame(View view) {
        Intent intent = new Intent(this, ExitScene.class);
        startActivity(intent);
    }

    //Init pillow, set hp to 100
    public void InitializePillow(){
        p1.SetHealth(100);
        Log.d("myTag",(Integer.toString(p1.Health())));
    }

    //Hit pillow with desired strength, currently with constant 10hp out of 100hp
    public void HitPillow(View view){

        float alpha;

        p1.Hit(1);
        ProgressBar myProgress = (ProgressBar) findViewById(R.id.punch_progressbar);
        myProgress.setProgress(100-p1.Health());

        /*
        //For "green" image currently invisible
        alpha = (float)(p1.Health()/100.00);
        ImageView imageView = (ImageView) findViewById(R.id.pillow_background);
        imageView.setAlpha(alpha);
        Log.d("myTag",(Float.toString(alpha)));

        //For "red" imageview
        alpha = (float)((100.00-p1.Health())/100.00);
        ImageView imageView2 = (ImageView) findViewById(R.id.red_blood);
        imageView2.setAlpha(alpha);
        Log.d("myTag",(Float.toString(alpha)));

        */
    }

    /* TODO: Next step in development

    //When data comes from sensor
    public void OnImpact(float impact){
        //HitPillow();
    }



     //Hit pillow with desired strength, gets value from sensor and adds it to progressbar depending
     // acceleration
     public void HitPillow(View view, float impact){

     float alpha;

     p1.Hit(impact);
     ProgressBar myProgress = (ProgressBar) findViewById(R.id.punch_progressbar);
     myProgress.setProgress(100-p1.Health());

     //For "green" image currently invisible
     alpha = (float)(p1.Health()/100.00);
     ImageView imageView = (ImageView) findViewById(R.id.pillow_background);
     imageView.setAlpha(alpha);
     Log.d("myTag",(Float.toString(alpha)));

     //For "red" imageview
     alpha = (float)((100.00-p1.Health())/100.00);
     ImageView imageView2 = (ImageView) findViewById(R.id.red_blood);
     imageView2.setAlpha(alpha);
     Log.d("myTag",(Float.toString(alpha)));

     }
     */



}
