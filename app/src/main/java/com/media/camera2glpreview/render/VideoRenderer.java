package com.media.camera2glpreview.render;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by oleg on 11/2/17.
 */

public class VideoRenderer implements GLSurfaceView.Renderer {
    private long mNativeContext; // using by native
    private GLSurfaceView mGLSurface;

    public VideoRenderer() {
        create();
    }

    public void init(GLSurfaceView glSurface) {
        mGLSurface = glSurface;
        // Create an OpenGL ES 2 context.
        mGLSurface.setEGLContextClientVersion(2);
        mGLSurface.setRenderer(this);
        mGLSurface.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void requestRender() {
        if (mGLSurface != null) {
            mGLSurface.requestRender();
        }
    }

    public void destroyRender() {
        destroy();
    }

    public void drawVideoFrame(byte[] data, int width, int height, int rotation) {
        draw(data, width, height, rotation);
    }

    public void applyVideoFilter(int filter) {
        applyFilter(filter);
    }

    public int getMaxVideoFilter() {
        return getMaxFilter();
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        render();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        init(width, height);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }

    private native void create();
    private native void destroy();
    private native void init(int width, int height);
    private native void render();
    private native void draw(byte[] data, int width, int height, int rotation);
    private native void applyFilter(int filter);
    private native int getMaxFilter();

    static {
        System.loadLibrary("libmedia");
    }
}
