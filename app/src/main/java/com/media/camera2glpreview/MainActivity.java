package com.media.camera2glpreview;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.FragmentActivity;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;

import com.media.camera2glpreview.capture.PreviewFrameHandler;
import com.media.camera2glpreview.capture.VideoCameraPreview;
import com.media.camera2glpreview.gesture.SimpleGestureFilter;
import com.media.camera2glpreview.gesture.SimpleGestureFilter.SimpleGestureListener;
import com.media.camera2glpreview.gesture.SimpleGestureFilter.SwipeDirection;
import com.media.camera2glpreview.render.VideoRenderer;

public class MainActivity extends FragmentActivity implements PreviewFrameHandler, SimpleGestureListener,
        ActivityCompat.OnRequestPermissionsResultCallback {

    private static final int REQUEST_CAMERA_PERMISSION = 1;
    private static final String FRAGMENT_DIALOG = "dialog";
    private static final String[] CAMERA_PERMISSIONS = {
            Manifest.permission.CAMERA
    };

    private VideoRenderer mVideoRenderer;
    private VideoCameraPreview mPreview;
    private SimpleGestureFilter mDetector;
    private ErrorDialog errorDialog;
    private int mFilter = 0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mVideoRenderer = new VideoRenderer();
        GLSurfaceView glSurfaceView = findViewById(R.id.gl_surface_view);

        mVideoRenderer.init(glSurfaceView);

        mPreview = new VideoCameraPreview(this);
        ((FrameLayout) findViewById(R.id.preview)).addView(mPreview);

        mDetector = new SimpleGestureFilter(this, this);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        mVideoRenderer.destroyRender();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (!hasPermissionsGranted(CAMERA_PERMISSIONS)) {
            requestCameraPermission();
        } else {
            mPreview.startBackgroundThread();
            mPreview.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onPause() {
        if (hasPermissionsGranted(CAMERA_PERMISSIONS)) {
            mPreview.closeCamera();
            mPreview.stopBackgroundThread();
            mPreview.setVisibility(View.GONE);
        }
        super.onPause();
    }

    @Override
    public void onPreviewFrame(byte[] data, int width, int height) {

        Integer rotation = mPreview.getSensorOrientation();
        mVideoRenderer.drawVideoFrame(data, width, height, rotation);
        mVideoRenderer.requestRender();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == REQUEST_CAMERA_PERMISSION) {
            if (grantResults.length == CAMERA_PERMISSIONS.length) {
                for (int result : grantResults) {
                    if (result != PackageManager.PERMISSION_GRANTED) {
                        if (null == errorDialog || errorDialog.isHidden()) {
                            errorDialog = ErrorDialog.newInstance(getString(R.string.request_permission));
                            errorDialog.show(getSupportFragmentManager(), FRAGMENT_DIALOG);
                        }
                        break;
                    } else {
                        if (null != errorDialog) errorDialog.dismiss();
                    }
                }
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    private boolean hasPermissionsGranted(String[] permissions) {
        for (String permission : permissions) {
            if (ActivityCompat.checkSelfPermission(this, permission)
                    != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    private void requestCameraPermission() {
        if (shouldShowRequestPermissionRationale(Manifest.permission.CAMERA)) {
            new ConfirmationDialog().show(getSupportFragmentManager(), FRAGMENT_DIALOG);
        } else {
            requestPermissions(CAMERA_PERMISSIONS, REQUEST_CAMERA_PERMISSION);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        mDetector.onTouchEvent(event);
        return super.onTouchEvent(event);
    }

    @Override
    public void onSwipe(SwipeDirection direction) {

        switch (direction) {
            case SWIPE_RIGHT:
                if (mFilter > 0) {
                    mFilter--;
                    mVideoRenderer.applyVideoFilter(mFilter);
                }
                break;
            case SWIPE_LEFT:
                if (mFilter < mVideoRenderer.getMaxVideoFilter() - 1) {
                    mFilter++;
                    mVideoRenderer.applyVideoFilter(mFilter);
                }
                break;
            default:
                break;
        }
    }

    /**
     * Shows an error message dialog.
     */
    public static class ErrorDialog extends DialogFragment {

        private static final String ARG_MESSAGE = "message";

        public static ErrorDialog newInstance(String message) {
            ErrorDialog dialog = new ErrorDialog();
            Bundle args = new Bundle();
            args.putString(ARG_MESSAGE, message);
            dialog.setArguments(args);
            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Activity activity = getActivity();
            return new AlertDialog.Builder(activity)
                    .setMessage(getArguments().getString(ARG_MESSAGE))
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int i) {
                            activity.finish();
                        }
                    })
                    .create();
        }
    }

    /**
     * Shows OK/Cancel confirmation dialog about camera permission.
     */
    public static class ConfirmationDialog extends DialogFragment {

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Activity activity = getActivity();
            return new AlertDialog.Builder(activity)
                    .setMessage(R.string.request_permission)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            ActivityCompat.requestPermissions(activity, CAMERA_PERMISSIONS,
                                    REQUEST_CAMERA_PERMISSION);
                        }
                    })
                    .setNegativeButton(android.R.string.cancel,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    activity.finish();
                                }
                            })
                    .create();
        }
    }
}
