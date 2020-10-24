package at.stefl.magnetics.core;

public class NativeTask {

    static {
        System.loadLibrary("magnetics-jni");
    }

    private static native long createFilter_(AndroidPlatform platform);
    private static native void destroy(long handle);

    private final long handle;

    public static NativeTask createFilter(AndroidPlatform platform) {
        long handle = createFilter_(platform);
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
    public native void stop();

}
