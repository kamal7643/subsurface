package com.kamal.subsurface;

public class NativeLibrary {

    static
    {
        System.loadLibrary("native");
    }
    public static native void init(int width, int height);
    public static native void uninit();
    public static native void step();
    public static native void onpointerdown(float x, float y);
    public static native void onpointerup(float x, float y);
}
