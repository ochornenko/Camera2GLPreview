package com.media.camera.preview.render;

import android.content.res.AssetManager;
import android.view.Surface;

/**
 * Created by oleg on 11/2/17.
 */

public abstract class VideoRenderer {
    protected enum Type {
        GL_YUV420(0), VK_YUV420(1), GL_YUV420_FILTER(2);

        private final int mValue;

        Type(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }
    }

    private long mNativeContext; // using by native

    protected native void create(int type);

    protected native void destroy();

    protected native void init(Surface surface, AssetManager assetManager, int width, int height);

    protected native void render();

    protected native void draw(byte[] data, int width, int height, int rotation, boolean mirror);

    protected native void setParameters(int params);

    protected native int getParameters();

    public abstract void drawVideoFrame(byte[] data, int width, int height, int rotation, boolean mirror);

    public void destroyRenderer() {
        destroy();
    }

    static {
        System.loadLibrary("media-lib");
    }
}
