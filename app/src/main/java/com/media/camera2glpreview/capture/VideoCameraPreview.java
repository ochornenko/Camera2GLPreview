package com.media.camera2glpreview.capture;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.util.SparseIntArray;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.Arrays;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;


/**
 * Created by oleg on 11/2/17.
 */

public class VideoCameraPreview extends SurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = VideoCameraPreview.class.toString();

    private static final SparseIntArray ORIENTATIONS = new SparseIntArray();
    private VideoCapture mVideoCapture;
    private Context mContext;
    private SurfaceHolder mHolder;
    private CameraCaptureSession mCaptureSession;
    private CameraDevice mCameraDevice;
    private String mCameraId;
    private Handler mBackgroundHandler;
    private HandlerThread mBackgroundThread;
    private ImageReader mImageReader;
    private Semaphore mCameraOpenCloseLock = new Semaphore(1);
    private int mSensorOrientation;

    static {
        ORIENTATIONS.append(Surface.ROTATION_0, 90);
        ORIENTATIONS.append(Surface.ROTATION_90, 0);
        ORIENTATIONS.append(Surface.ROTATION_180, 270);
        ORIENTATIONS.append(Surface.ROTATION_270, 180);
    }

    public VideoCameraPreview(Context context) {
        super(context);

        mContext = context;

        mHolder = getHolder();
        mHolder.addCallback(this);

        mVideoCapture = new VideoCapture((PreviewFrameHandler) context);

        Log.i(TAG, "VCamera VideoCameraPreview");
    }

    public void surfaceCreated(SurfaceHolder holder) {

    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        closeCamera();
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        mImageReader = ImageReader.newInstance(640, 480, ImageFormat.YUV_420_888, 2);
        mImageReader.setOnImageAvailableListener(mVideoCapture, null);
        openCamera();
    }

    private int getOrientation(int rotation) {
        return (ORIENTATIONS.get(rotation) + mSensorOrientation + 270) % 360;
    }

    /**
     * Opens the camera specified by {@link VideoCameraPreview#}.
     */
    private void openCamera() {
        if (ContextCompat.checkSelfPermission(mContext, Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            return;
        }

        Log.i(TAG, "VCamera openCamera");

        //startBackgroundThread();

        CameraManager manager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraId);

                // We don't use a front facing camera in this sample.
                Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
                if (facing != null && facing == CameraCharacteristics.LENS_FACING_FRONT) {
                    continue;
                }
                mCameraId = cameraId;
            }
            manager.openCamera(mCameraId, mStateCallback, null);//mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }
    }

    /**
     * Closes the current {@link CameraDevice}.
     */
    private void closeCamera() {
        try {
            mCameraOpenCloseLock.acquire();
            if (null != mCaptureSession) {
                mCaptureSession.close();
                mCaptureSession = null;
            }
            if (null != mCameraDevice) {
                mCameraDevice.close();
                mCameraDevice = null;
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
            mCameraOpenCloseLock.release();
            //stopBackgroundThread();
        }

        Log.i(TAG, "VCamera closeCamera");
    }

    /**
     * Starts a background thread and its {@link Handler}.
     */
    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("CameraBackground");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    /**
     * Stops the background thread and its {@link Handler}.
     */
    private void stopBackgroundThread() {
        mBackgroundThread.quitSafely();
        try {
            mBackgroundThread.join();
            mBackgroundThread = null;
            mBackgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private CaptureRequest createCaptureRequest() {
        try {
            CaptureRequest.Builder builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
            builder.addTarget(mImageReader.getSurface());
            return builder.build();
        } catch (CameraAccessException e) {
            Log.e(TAG, e.getMessage());
            return null;
        }
    }

    /**
     * {@link CameraDevice.StateCallback} is called when {@link CameraDevice} changes its state.
     */
    private final CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            // This method is called when the camera is opened.  We start camera preview here.
            mCameraOpenCloseLock.release();
            mCameraDevice = cameraDevice;
            createCaptureSession();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice cameraDevice, int error) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
        }
    };

    private void createCaptureSession() {
        try {
            mCameraDevice.createCaptureSession(Arrays.asList(mImageReader.getSurface()), sessionStateCallback, null);

        } catch (CameraAccessException e) {
            Log.e(TAG, "VCamera createCaptureSession " + e.toString());
        }
    }

    /**
     * Creates a new {@link CameraCaptureSession} for camera preview.
     */
    private CameraCaptureSession.StateCallback sessionStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(CameraCaptureSession session) {
            mCaptureSession = session;
            try {
                session.setRepeatingRequest(createCaptureRequest(), null, null);
            } catch (CameraAccessException e){
                Log.e(TAG, "VCamera onConfigured " + e.toString());
            }
        }

        @Override
        public void onConfigureFailed(CameraCaptureSession session) {
            Log.e(TAG, "VCamera onConfigureFailed");
        }
    };
}
