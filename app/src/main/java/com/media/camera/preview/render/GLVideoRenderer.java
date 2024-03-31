package com.media.camera.preview.render;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GLVideoRenderer extends VideoRenderer implements GLSurfaceView.Renderer {

    private GLSurfaceView mGLSurface;

    public GLVideoRenderer() {
        create(Type.GL_YUV420_FILTER.getValue());
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

    public void setVideoParameters(int params) {
        setParameters(params);
    }

    public int getVideoParameters() {
        return getParameters();
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        render();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        init(null, null, width, height);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }
}
