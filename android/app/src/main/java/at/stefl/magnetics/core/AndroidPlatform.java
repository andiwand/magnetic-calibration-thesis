package at.stefl.magnetics.core;

import android.hardware.Sensor;
import android.hardware.SensorEvent;

import at.stefl.magnetics.CollectorListener;

public class AndroidPlatform {

    static {
        System.loadLibrary("magnetics-jni");
    }

    private final long handle;
    private final CollectorListener collectorListener = new CollectorListener() {
        @Override
        public void onStart(double time) {
            pushStart(time);
        }

        @Override
        public void onStop(double time) {
            pushStop(time);
        }

        @Override
        public void onTickEvent(double time) {
            pushTick(time);
        }

        @Override
        public void onClockEvent(double time, double unixUtc) {
            pushClock(time, unixUtc);
        }

        @Override
        public void onSensorEvent(double time, SensorEvent event) {
            switch (event.sensor.getType()) {
                case Sensor.TYPE_ACCELEROMETER:
                    pushAccelerometer(time, event.values[0], event.values[1], event.values[2]);
                    break;
                case Sensor.TYPE_GYROSCOPE:
                    pushGyroscope(time, event.values[0], event.values[1], event.values[2]);
                    break;
                case Sensor.TYPE_MAGNETIC_FIELD:
                    pushMagnetometer(time, event.values[0], event.values[1], event.values[2]);
                    break;
                case Sensor.TYPE_MAGNETIC_FIELD_UNCALIBRATED:
                    pushMagnetometer(time, event.values[0] - event.values[3],
                            event.values[1] - event.values[4],
                            event.values[2] - event.values[5]);
                    pushMagnetometerUncalibrated(time, event.values[0], event.values[1], event.values[2]);
                    pushMagnetometerBias(time, event.values[3], event.values[4], event.values[5]);
                    break;
                case Sensor.TYPE_ROTATION_VECTOR:
                    double w;
                    if (event.values.length > 3) {
                        w = event.values[3];
                    } else {
                        w = Math.sqrt(event.values[0] * event.values[0] + event.values[1] * event.values[1] + event.values[2] * event.values[2]);
                    }
                    pushOrientation(time, w, event.values[0], event.values[1], event.values[2]);
                    break;
            }
        }

        @Override
        public void onSensorAccuracy(double time, Sensor sensor, int accuracy) {
            // TODO
        }
    };

    public AndroidPlatform() {
        this.handle = create();
    }

    private static native long create();

    private static native void destroy(long handle);

    @Override
    protected void finalize() {
        destroy(handle);
    }

    public CollectorListener getCollectorListener() {
        return collectorListener;
    }

    public native void pushStart(double time);

    public native void pushStop(double time);

    public native void pushTick(double time);

    public native void pushClock(double time, double linuxUtc);

    public native void pushAccelerometer(double time, double x, double y, double z);

    public native void pushGyroscope(double time, double x, double y, double z);

    public native void pushMagnetometer(double time, double x, double y, double z);

    public native void pushMagnetometerUncalibrated(double time, double x, double y, double z);

    public native void pushMagnetometerBias(double time, double x, double y, double z);

    public native void pushOrientation(double time, double w, double x, double y, double z);
}
