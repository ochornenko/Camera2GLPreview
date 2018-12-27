package com.media.camera2glpreview;

import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.widget.FrameLayout;

import com.media.camera2glpreview.capture.PreviewFrameHandler;
import com.media.camera2glpreview.capture.VideoCameraPreview;
import com.media.camera2glpreview.gesture.SimpleGestureFilter;
import com.media.camera2glpreview.gesture.SimpleGestureFilter.SimpleGestureListener;
import com.media.camera2glpreview.render.VKVideoRenderer;

public class VKActivity extends FragmentActivity implements PreviewFrameHandler, SimpleGestureListener {

    private VKVideoRenderer mVideoRenderer;
    private VideoCameraPreview mPreview;
    private SimpleGestureFilter mDetector;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_vk);

        mVideoRenderer = new VKVideoRenderer();
        SurfaceView surfaceView = findViewById(R.id.surface_view);
        mVideoRenderer.init(surfaceView);

        mPreview = new VideoCameraPreview(this);
        ((FrameLayout) findViewById(R.id.preview)).addView(mPreview);

        mDetector = new SimpleGestureFilter(this, this);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onResume() {
        super.onResume();
        mPreview.startBackgroundThread();
        mPreview.setVisibility(View.VISIBLE);
    }

    @Override
    public void onPause() {
        mPreview.closeCamera();
        mPreview.stopBackgroundThread();
        mPreview.setVisibility(View.GONE);
        super.onPause();
    }

    @Override
    public void onPreviewFrame(byte[] data, int width, int height) {
        Integer rotation = mPreview.getSensorOrientation();
        mVideoRenderer.drawVideoFrame(data, width, height, rotation);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        mDetector.onTouchEvent(event);
        return super.onTouchEvent(event);
    }

    @Override
    public void onSwipe(SimpleGestureFilter.SwipeDirection direction) {

        switch (direction) {
            case SWIPE_RIGHT:

                break;
            case SWIPE_LEFT:
                finish();
                break;
            default:
                break;
        }
    }
}
