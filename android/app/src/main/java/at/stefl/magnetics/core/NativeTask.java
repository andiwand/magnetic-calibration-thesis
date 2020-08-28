package at.stefl.magnetics.core;

public class NativeTask {

    static {
        System.loadLibrary("magnetics-jni");
    }

    private static native long createDefault_(AndroidPlatform platform);
    private static native long createDefault_(AndroidPlatform platform, String html);
    private static native void destroy(long handle);

    private long handle;

    public static NativeTask createDefault(AndroidPlatform platform) {
        long handle = createDefault_(platform);
        return new NativeTask(handle);
    }

    public static NativeTask createDefault(AndroidPlatform platform, String html) {
        long handle = createDefault_(platform, html);
        return new NativeTask(handle);
    }

    private NativeTask(long handle) {
        this.handle = handle;
    }

    @Override
    protected void finalize() {
        destroy(handle);
    }

    public native String annotation();

    public native void start();
    public native void restart();
    public native void resume();
    public native void pause();
    public native void stop();

}
