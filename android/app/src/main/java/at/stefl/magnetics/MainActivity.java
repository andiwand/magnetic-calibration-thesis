package at.stefl.magnetics;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.LinkedList;
import java.util.List;

import at.stefl.magnetics.core.AndroidPlatform;
import at.stefl.magnetics.core.NativeTask;

public class MainActivity extends AppCompatActivity {
    private static String TAG = "magnetics";

    private Collector collector;
    private AndroidPlatform androidPlatform;
    private List<NativeTask> nativeTasks = new LinkedList<>();

    private File internalDirectory;
    private File htmlDirectory;

    private void copyAsset(AssetManager assetManager, String source, File destination) throws IOException {
        InputStream in = assetManager.open(source);
        OutputStream out = new FileOutputStream(destination);
        byte[] buffer = new byte[1024];
        int read = in.read(buffer);
        while (read != -1) {
            out.write(buffer, 0, read);
            read = in.read(buffer);
        }
    }

    private void initStorage() throws IOException {
        AssetManager assetManager = getAssets();

        internalDirectory = new File(Environment.getExternalStorageDirectory(), "magnetics");
        if (!internalDirectory.isDirectory() && !internalDirectory.mkdirs()) {
            throw new IOException("could not create directory " + internalDirectory);
        }

        htmlDirectory = new File(internalDirectory, "html");
        if (!htmlDirectory.isDirectory() && !htmlDirectory.mkdirs()) {
            throw new IOException("could not create directory " + internalDirectory);
        }
        copyAsset(assetManager, "html/index.html", new File(htmlDirectory, "index.html"));
        copyAsset(assetManager, "html/bundle.js", new File(htmlDirectory, "bundle.js"));
    }

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
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        for (int i = 0; i < permissions.length; ++i) {
            Log.v(TAG, "permission: " + permissions[i] + " was " + grantResults[i]);
        }
    }

    public boolean checkStoragePermissionsGranted() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (checkSelfPermission(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    == PackageManager.PERMISSION_GRANTED) {
                return true;
            } else {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
                return false;
            }
        } else {
            return true;
        }
    }

    public void checkPermissionsGranted() {
        if (!checkStoragePermissionsGranted()) {
            Log.w(TAG, "storage permissions not granted");
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        checkPermissionsGranted();

        try {
            initStorage();
        } catch (IOException e) {
            e.printStackTrace();
        }

        collector = new Collector((SensorManager) getSystemService(Context.SENSOR_SERVICE));
        collector.setClockInterval(100000);
        collector.addRequest(Sensor.TYPE_ACCELEROMETER, 1000);
        collector.addRequest(Sensor.TYPE_GYROSCOPE, 1000);
        collector.addRequest(Sensor.TYPE_MAGNETIC_FIELD_UNCALIBRATED, 1000);
        collector.addRequest(Sensor.TYPE_ROTATION_VECTOR, 10000);

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
        //settings.setLoadWithOverviewMode(true);
        //settings.setUseWideViewPort(true);
        //settings.setSupportZoom(true);
        //settings.setBuiltInZoomControls(false);
        //settings.setLayoutAlgorithm(WebSettings.LayoutAlgorithm.SINGLE_COLUMN);
        //settings.setCacheMode(WebSettings.LOAD_NO_CACHE);
        //settings.setDomStorageEnabled(true);
        //webView.setScrollBarStyle(WebView.SCROLLBARS_OUTSIDE_OVERLAY);
        //webView.setScrollbarFadingEnabled(true);
        //if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
        //    webView.setLayerType(View.LAYER_TYPE_HARDWARE, null);
        //} else {
        //    webView.setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        //}

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
