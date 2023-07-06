package com.kamal.subsurface;

import androidx.appcompat.app.AppCompatActivity;

import java.io.File;
import java.io.InputStream;
import java.io.RandomAccessFile;
import android.os.Bundle;
import android.os.Build;
import android.app.Activity;
import android.app.ActivityManager;
import android.content.res.AssetManager;
import android.util.Log;
import android.view.MotionEvent;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.widget.Toast;
import android.content.res.AssetManager;
import android.view.Window;
import android.view.WindowManager;
import android.opengl.GLSurfaceView.EGLContextFactory;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGL10;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity {

    private static android.content.Context applicationContext = null;
    private static String                  assetsDirectory    = null;
    private static String                  LOGTAG             = "libNative";
    private GLSurfaceView view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        view = new GLSurfaceView(this);
        view.setEGLContextClientVersion(2);
        view.setEGLConfigChooser(8, 8, 8, 8, 16, 8); // OBS! Nonzero stencil buffer size!
        view.setRenderer(new Renderer());
        view.getHolder().setFixedSize(1280, 720);
        setContentView(view);

        applicationContext = getApplicationContext();
        assetsDirectory    = applicationContext.getFilesDir().getPath() + "/";

        Log.d("assetsDirectory", assetsDirectory);


        extractAsset("thickness.vs");
        extractAsset("thickness.fs");

        extractAsset("resolve.vs");
        extractAsset("resolve.fs");

        extractAsset("prepass.vs");
        extractAsset("prepass.fs");

        extractAsset("scattering.vs");
        extractAsset("scattering.fs");

        extractAsset("opaque.vs");
        extractAsset("opaque.fs");

        extractAsset("teapot.bin");
    }

    @Override protected void onPause()
    {
        super.onPause();
        view.onPause();
        NativeLibrary.uninit();
    }

    @Override protected void onResume()
    {
        super.onResume();
        view.onResume();
    }

    @Override public boolean onTouchEvent(MotionEvent event)
    {
        float x = event.getRawX();
        float y = event.getRawY();
        if (event.getAction() == MotionEvent.ACTION_UP)
            NativeLibrary.onpointerup(x, y);
        else if (event.getAction() == MotionEvent.ACTION_MOVE)
            NativeLibrary.onpointerdown(x, y);
        return true;
    }

    private void extractAsset(String assetName)
    {
        File file = new File(assetsDirectory + assetName);

        if(file.exists()) {
            Log.d(LOGTAG, assetName +  " already exists. No extraction needed.\n");
        } else {
            Log.d(LOGTAG, assetName + " doesn't exist. Extraction needed. \n");

            try {
                RandomAccessFile randomAccessFile = new RandomAccessFile(assetsDirectory + assetName,"rw");
                AssetManager assetManager     = applicationContext.getResources().getAssets();
                InputStream inputStream      = assetManager.open(assetName);

                byte buffer[] = new byte[1024];
                int count     = inputStream.read(buffer, 0, 1024);

                while (count > 0) {
                    randomAccessFile.write(buffer, 0, count);

                    count = inputStream.read(buffer, 0, 1024);
                }

                randomAccessFile.close();
                inputStream.close();
            } catch(Exception e) {
                Log.e(LOGTAG, "Failure in extractAssets(): " + e.toString() + " " + assetsDirectory + assetName);
            }

            if(file.exists()) {
                Log.d(LOGTAG,"File extracted successfully");
            }
        }
    }

    private static class Renderer implements GLSurfaceView.Renderer
    {
        public void onDrawFrame(GL10 gl)
        {
            NativeLibrary.step();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height)
        {
            NativeLibrary.init(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config)
        {

        }
    }
}