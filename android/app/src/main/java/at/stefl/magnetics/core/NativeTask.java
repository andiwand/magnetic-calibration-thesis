package at.stefl.magnetics.core;

public class NativeTask {

    static {
        System.loadLibrary("magnetics-jni");
        redirectStdio();
    }

    private final long handle;

    private NativeTask(long handle) {
        this.handle = handle;
    }

    private static native long createRecorder_(AndroidPlatform platform, String path);

    private static native long createLiveDemo_(AndroidPlatform platform);

    private static native void destroy_(long handle);

    private static native void redirectStdio();

    public static NativeTask createRecorder(AndroidPlatform platform, String path) {
        long handle = createRecorder_(platform, path);
        return new NativeTask(handle);
    }

    public static NativeTask createLiveDemo(AndroidPlatform platform) {
        long handle = createLiveDemo_(platform);
        return new NativeTask(handle);
    }

    @Override
    protected void finalize() {
        destroy_(handle);
    }

    public native String annotation();

    public native void start();

    public native void stop();

}
