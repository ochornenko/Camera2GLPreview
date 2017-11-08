package com.media.camera2glpreview.capture;

import android.media.Image;
import android.media.ImageReader;

import java.nio.ByteBuffer;

/**
 * Created by oleg on 11/2/17.
 */

public class VideoCapture implements ImageReader.OnImageAvailableListener {

    private PreviewFrameHandler mPreviewFrameHandler = null;

    public VideoCapture(PreviewFrameHandler frameHandler) {
        mPreviewFrameHandler = frameHandler;
    }

    @Override
    public void onImageAvailable(ImageReader imageReader) {

        Image image = imageReader.acquireLatestImage();
        if (image != null) {
            if (mPreviewFrameHandler != null) {
                mPreviewFrameHandler.onPreviewFrame(YUV_420_888_data(image), image.getWidth(), image.getHeight());
            }

            image.close();
        }
    }

    private static byte[] YUV_420_888_data(Image image) {
        ByteBuffer yBuffer = image.getPlanes()[0].getBuffer();
        ByteBuffer uBuffer = image.getPlanes()[1].getBuffer();
        ByteBuffer vBuffer = image.getPlanes()[2].getBuffer();

        int ySize = yBuffer.remaining();
        int uSize = uBuffer.remaining();
        int vSize = vBuffer.remaining();

        byte[] data = new byte[ySize + uSize + vSize];

        yBuffer.get(data, 0, ySize);
        uBuffer.get(data, ySize, uSize);
        vBuffer.get(data, ySize + uSize, vSize);

        return data;
    }
}
