package com.media.camera.preview.activity;

import android.os.Bundle;
import android.view.SurfaceView;

import com.media.camera.preview.R;
import com.media.camera.preview.controller.CameraController;
import com.media.camera.preview.gesture.SimpleGestureFilter.SwipeDirection;
import com.media.camera.preview.render.VKVideoRenderer;

public class VKActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_vk);

        SurfaceView surfaceView = findViewById(R.id.preview);
        VKVideoRenderer videoRenderer = new VKVideoRenderer(getApplicationContext());
        videoRenderer.init(surfaceView);

        mCameraController = new CameraController(this, videoRenderer);

        setup(surfaceView);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mCameraController.destroy();
    }

    @Override
    public void onResume() {
        super.onResume();
        mCameraController.startCamera();
    }

    @Override
    public void onPause() {
        mCameraController.stopCamera();
        super.onPause();
    }

    @Override
    public void onSwipe(SwipeDirection direction) {

        switch (direction) {
            case SWIPE_UP:
                showResolutionDialog(mCameraController.getOutputSizes());
                break;
            case SWIPE_LEFT:
                finish();
                break;
            default:
                break;
        }
    }
}
