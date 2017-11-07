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
        // Create an OpenGL ES 2.0 context.
        mGLSurface.setEGLContextClientVersion(2);
        mGLSurface.setRenderer(this);
        mGLSurface.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public GLSurfaceView getGL() {
        return mGLSurface;
    }

    public void requestRender() {
        if (mGLSurface != null) {
            mGLSurface.requestRender();
        }
    }

    public void destroyRender() {
        destroy();
    }

    public void drawVideoFrame(byte[] data, int width, int height) {
        draw(data, width, height);
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
    private native void draw(byte[] data, int width, int height);

    static {
        System.loadLibrary("libmedia");
    }
}
