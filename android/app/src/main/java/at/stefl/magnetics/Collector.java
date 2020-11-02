package at.stefl.magnetics;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.SystemClock;
import android.util.Log;

import java.util.LinkedList;
import java.util.List;

public class Collector {
    private volatile boolean started = false;
    private long startTime;
    private int defaultMaxReportLatencyUs = 0;
    private final SensorManager sensorManager;

    private final List<SensorRequest> sensorRequests = new LinkedList<>();
    private final List<CollectorListener> listeners = new LinkedList<>();
    private Clock clock = new Clock(100000);
    private Thread clockThread;
    private final SensorEventListener sensorEventListener = new SensorEventListener() {
        @Override
        public void onSensorChanged(SensorEvent sensorEvent) {
            synchronized (Collector.this) {
                double time = getTime();

                for (CollectorListener listener : listeners) {
                    listener.onSensorEvent(time, sensorEvent);
                }
            }
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            synchronized (Collector.this) {
                double time = getTime();

                for (CollectorListener listener : listeners) {
                    listener.onSensorAccuracy(time, sensor, accuracy);
                }
            }
        }
    };
    public Collector(SensorManager sensorManager) {
        this.sensorManager = sensorManager;
    }

    public void setClockInterval(int samplePeriodUs) {
        clock = new Clock(samplePeriodUs);
    }

    public void setDefaultMaxReportLatencyUs(int defaultMaxReportLatencyUs) {
        this.defaultMaxReportLatencyUs = defaultMaxReportLatencyUs;
    }

    public void addRequest(SensorRequest request) {
        sensorRequests.add(request);
    }

    public void addRequest(int sensorType, int samplePeriodUs) {
        addRequest(sensorType, samplePeriodUs, defaultMaxReportLatencyUs);
    }

    public void addRequest(int sensorType, int samplePeriodUs, int maxReportLatencyUs) {
        SensorRequest request = new SensorRequest();
        request.sensor = sensorManager.getDefaultSensor(sensorType);
        request.samplePeriodUs = samplePeriodUs;
        request.maxReportLatencyUs = maxReportLatencyUs;

        if (request.sensor != null) {
            sensorRequests.add(request);
        } else {
            Log.e("magnetics-collector", "sensor " + sensorType + " not available");
        }
    }

    public void addListener(CollectorListener listener) {
        listeners.add(listener);
    }

    public synchronized void start() {
        if (started) throw new RuntimeException("already started");

        startTime = SystemClock.elapsedRealtimeNanos();

        for (CollectorListener listener : listeners) {
            listener.onStart(0);
        }

        clockThread = new Thread(clock);
        clockThread.start();

        for (SensorRequest request : sensorRequests) {
            sensorManager.registerListener(sensorEventListener, request.sensor,
                    request.samplePeriodUs, request.maxReportLatencyUs);
        }

        started = true;
    }

    public synchronized void stop() throws InterruptedException {
        if (!started) throw new RuntimeException("not started");

        double time = getTime();

        clockThread.interrupt();
        clockThread.join();
        clockThread = null;

        sensorManager.unregisterListener(sensorEventListener);

        for (CollectorListener listener : listeners) {
            listener.onStop(time);
        }

        started = false;
    }

    public void tick() {
        double time = getTime();

        for (CollectorListener listener : listeners) {
            listener.onTickEvent(time);
        }
    }

    private double getTime() {
        return (SystemClock.elapsedRealtimeNanos() - startTime) * 1e-9;
    }

    public static class SensorRequest {
        public Sensor sensor;
        public int samplePeriodUs;
        public int maxReportLatencyUs;
    }

    private class Clock implements Runnable {
        private final int samplePeriodMs;

        Clock(int samplePeriodUs) {
            this.samplePeriodMs = (int) (samplePeriodUs * 1e-3);
        }

        @Override
        public void run() {
            while (true) {
                synchronized (Collector.this) {
                    double time = getTime();
                    double linuxUtc = System.currentTimeMillis() * 1e-3;

                    for (CollectorListener listener : listeners) {
                        listener.onClockEvent(time, linuxUtc);
                    }
                }

                try {
                    Thread.sleep(samplePeriodMs);
                } catch (InterruptedException e) {
                    break;
                }
            }
        }
    }
}
