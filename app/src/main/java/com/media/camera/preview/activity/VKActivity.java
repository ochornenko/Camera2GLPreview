package com.media.camera.preview.activity;

import android.os.Bundle;
import android.util.Size;
import android.view.SurfaceView;
import android.widget.FrameLayout;

import com.media.camera.preview.R;
import com.media.camera.preview.gesture.SimpleGestureFilter.SwipeDirection;
import com.media.camera.preview.render.VKVideoRenderer;

public class VKActivity extends BaseActivity {

    private VKVideoRenderer mVideoRenderer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_vk);

        SurfaceView surfaceView = findViewById(R.id.surface_view);
        mVideoRenderer = new VKVideoRenderer();
        mVideoRenderer.init(surfaceView);

        ((FrameLayout) findViewById(R.id.preview)).addView(mPreview);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onResume() {
        super.onResume();
        mPreview.startCamera();
    }

    @Override
    public void onPause() {
        mPreview.stopCamera();
        super.onPause();
    }

    @Override
    public void onPreviewFrame(byte[] data, int width, int height) {
        Integer rotation = mPreview.getSensorOrientation();
        mVideoRenderer.drawVideoFrame(data, width, height, rotation);
    }

    @Override
    public void onSwipe(SwipeDirection direction) {

        switch (direction) {
            case SWIPE_UP:
                showResolutionDialog(mPreview.getOutputSizes());
                break;
            case SWIPE_RIGHT:
                break;
            case SWIPE_LEFT:
                finish();
                break;
            default:
                break;
        }
    }

    @Override
    public void onItemClick(Size item) {
        super.onItemClick(item);
        mPreview.changeResolution(item);
    }
}