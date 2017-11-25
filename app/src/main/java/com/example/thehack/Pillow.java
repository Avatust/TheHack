package com.example.thehack;

/**
 * Created by on 25.11.2017.
 */

class Pillow{
    private int TotalHealth;

    public void SetHealth(int Health){
        TotalHealth = Health;
    }

    public int Hit(int Damage){
        TotalHealth=TotalHealth-Damage;
        return TotalHealth;
    }

    public int Health(){
        return TotalHealth;
    }
}
