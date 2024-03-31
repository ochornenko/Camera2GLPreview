package com.media.camera.preview.render;

import android.content.Context;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class VKVideoRenderer extends VideoRenderer implements SurfaceHolder.Callback {

    private final Context mContext;

    public VKVideoRenderer(Context context) {
        mContext = context;
    }

    public void init(SurfaceView surface) {
        surface.getHolder().addCallback(this);
    }

    public void drawVideoFrame(byte[] data, int width, int height, int rotation) {
        draw(data, width, height, rotation);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        create(Type.VK_YUV420.getValue());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        init(holder.getSurface(), mContext.getAssets(), width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        destroy();
    }
}
