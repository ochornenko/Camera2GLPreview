package com.media.camera.preview.activity;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.DialogFragment;

import com.media.camera.preview.R;
import com.media.camera.preview.controller.CameraController;
import com.media.camera.preview.gesture.SimpleGestureFilter.SwipeDirection;
import com.media.camera.preview.render.GLVideoRenderer;

public class GLActivity extends BaseActivity implements ActivityCompat.OnRequestPermissionsResultCallback {

    private static final int REQUEST_CAMERA_PERMISSION = 1;
    private static final String FRAGMENT_DIALOG = "dialog";
    private static final String[] CAMERA_PERMISSIONS = {
            Manifest.permission.CAMERA
    };

    private GLVideoRenderer mVideoRenderer;
    private ErrorDialog mErrorDialog;
    private int mFilter = 0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_gl);

        GLSurfaceView glSurfaceView = findViewById(R.id.preview);
        mVideoRenderer = new GLVideoRenderer();
        mVideoRenderer.init(glSurfaceView);

        mCameraController = new CameraController(this, mVideoRenderer);

        setup(glSurfaceView);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mCameraController.destroy();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (!hasPermissionsGranted()) {
            requestCameraPermission();
        } else {
            mCameraController.startCamera();
        }
    }

    @Override
    public void onPause() {
        if (hasPermissionsGranted()) {
            mCameraController.stopCamera();
        }
        super.onPause();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == REQUEST_CAMERA_PERMISSION) {
            if (grantResults.length == CAMERA_PERMISSIONS.length) {
                for (int result : grantResults) {
                    if (result != PackageManager.PERMISSION_GRANTED) {
                        if (null == mErrorDialog || mErrorDialog.isHidden()) {
                            mErrorDialog = ErrorDialog.newInstance(getString(R.string.request_permission));
                            mErrorDialog.show(getSupportFragmentManager(), FRAGMENT_DIALOG);
                        }
                        break;
                    } else {
                        if (null != mErrorDialog) {
                            mErrorDialog.dismiss();
                        } else {
                            recreate();
                        }
                    }
                }
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    private boolean hasPermissionsGranted() {
        for (String permission : GLActivity.CAMERA_PERMISSIONS) {
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
    public void onSwipe(SwipeDirection direction) {

        switch (direction) {
            case SWIPE_UP:
                showResolutionDialog(mCameraController.getOutputSizes());
                break;
            case SWIPE_RIGHT:
                if (mFilter > 0) {
                    mFilter--;
                    mParams = (mParams & 0xFFFFFFF0) | mFilter;
                    mVideoRenderer.setVideoParameters(mParams);
                } else {
                    Intent intent = new Intent(this, VKActivity.class);
                    startActivity(intent);
                }
                break;
            case SWIPE_LEFT:
                mParams = mVideoRenderer.getVideoParameters();
                int maxFilter = (mParams & 0x000000F0) >>> 4;
                if (mFilter < maxFilter - 1) {
                    mFilter++;
                    mParams = (mParams & 0xFFFFFFF0) | mFilter;
                    mVideoRenderer.setVideoParameters(mParams);
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

        @NonNull
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Activity activity = getActivity();
            assert activity != null;
            assert getArguments() != null;
            return new AlertDialog.Builder(activity)
                    .setMessage(getArguments().getString(ARG_MESSAGE))
                    .setPositiveButton(android.R.string.ok, (dialogInterface, i) -> activity.finish())
                    .create();
        }
    }

    /**
     * Shows OK/Cancel confirmation dialog about camera permission.
     */
    public static class ConfirmationDialog extends DialogFragment {

        @NonNull
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Activity activity = getActivity();
            assert activity != null;
            return new AlertDialog.Builder(activity)
                    .setMessage(R.string.request_permission)
                    .setPositiveButton(android.R.string.ok, (dialog, which) -> ActivityCompat
                            .requestPermissions(activity, CAMERA_PERMISSIONS, REQUEST_CAMERA_PERMISSION))
                    .setNegativeButton(android.R.string.cancel, (dialog, which) -> activity.finish())
                    .create();
        }
    }
}
