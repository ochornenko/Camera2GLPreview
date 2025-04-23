package com.media.camera.preview.render;

import android.content.Context;
import android.support.annotation.NonNull;
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

    @Override
    public void drawVideoFrame(byte[] data, int width, int height, int rotation, boolean mirror) {
        draw(data, width, height, rotation, mirror);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        create(Type.VK_YUV420.getValue());
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        init(holder.getSurface(), mContext.getAssets(), width, height);
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
    }
}
