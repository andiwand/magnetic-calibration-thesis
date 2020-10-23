package at.stefl.magnetics;

import android.hardware.Sensor;
import android.hardware.SensorEvent;

public interface CollectorListener {

    void onStart(double time);

    void onStop(double time);

    void onTickEvent(double time);

    void onClockEvent(double time, double linuxUtc);

    void onSensorEvent(double time, SensorEvent event);

    void onSensorAccuracy(double time, Sensor sensor, int accuracy);

}
