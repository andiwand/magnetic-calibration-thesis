package at.stefl.magnetics;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.LinkedList;
import java.util.List;

import at.stefl.magnetics.core.AndroidPlatform;
import at.stefl.magnetics.core.NativeTask;

public class MainActivity extends AppCompatActivity {
    private Collector collector;
    private AndroidPlatform androidPlatform;
    private List<NativeTask> nativeTasks = new LinkedList<>();

    private static String streamToString(InputStream inputStream) throws IOException {
        ByteArrayOutputStream result = new ByteArrayOutputStream();
        byte[] buffer = new byte[1024];
        int length;
        while ((length = inputStream.read(buffer)) != -1) {
            result.write(buffer, 0, length);
        }
        return result.toString("UTF-8");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        collector = new Collector((SensorManager) getSystemService(Context.SENSOR_SERVICE));
        collector.setClockInterval(100000);
        collector.addRequest(Sensor.TYPE_ACCELEROMETER, 1000);
        collector.addRequest(Sensor.TYPE_GYROSCOPE, 1000);
        collector.addRequest(Sensor.TYPE_MAGNETIC_FIELD_UNCALIBRATED, 1000);

        androidPlatform = new AndroidPlatform();
        collector.addListener(androidPlatform.getCollectorListener());

        nativeTasks.add(NativeTask.createDefault(androidPlatform));
    }

    @Override
    protected void onStart() {
        super.onStart();
        for (NativeTask task : nativeTasks) {
            task.start();
        }
        collector.start();

        WebView webView = (WebView) findViewById(R.id.webview);

        WebSettings settings = webView.getSettings();
        settings.setJavaScriptEnabled(true);
        settings.setLoadWithOverviewMode(true);
        settings.setUseWideViewPort(true);
        settings.setSupportZoom(true);
        settings.setBuiltInZoomControls(false);
        settings.setLayoutAlgorithm(WebSettings.LayoutAlgorithm.SINGLE_COLUMN);
        settings.setCacheMode(WebSettings.LOAD_NO_CACHE);
        settings.setDomStorageEnabled(true);
        webView.setScrollBarStyle(WebView.SCROLLBARS_OUTSIDE_OVERLAY);
        webView.setScrollbarFadingEnabled(true);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            webView.setLayerType(View.LAYER_TYPE_HARDWARE, null);
        } else {
            webView.setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        }

        String html = null;
        try {
            html = streamToString(getAssets().open("html/index.html"));
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        webView.loadDataWithBaseURL("file:///android_asset/html/", html, "text/html", "utf-8", null);
    }

    @Override
    protected void onRestart() {
        super.onRestart();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
        collector.stop();
        for (NativeTask task : nativeTasks) {
            task.stop();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}
