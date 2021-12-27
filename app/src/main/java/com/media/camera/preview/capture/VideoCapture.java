package com.media.camera.preview.capture;

import android.graphics.ImageFormat;
import android.media.Image;
import android.media.ImageReader;

import java.nio.ByteBuffer;

/**
 * Created by oleg on 11/2/17.
 */

public class VideoCapture implements ImageReader.OnImageAvailableListener {

    private final PreviewFrameHandler mPreviewFrameHandler;

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
        final int imageWidth = image.getWidth();
        final int imageHeight = image.getHeight();
        final Image.Plane[] planes = image.getPlanes();
        byte[] data = new byte[imageWidth * imageHeight *
                ImageFormat.getBitsPerPixel(ImageFormat.YUV_420_888) / 8];
        int offset = 0;

        for (int plane = 0; plane < planes.length; ++plane) {
            final ByteBuffer buffer = planes[plane].getBuffer();
            final int rowStride = planes[plane].getRowStride();
            // Experimentally, U and V planes have |pixelStride| = 2, which
            // essentially means they are packed.
            final int pixelStride = planes[plane].getPixelStride();
            final int planeWidth = (plane == 0) ? imageWidth : imageWidth / 2;
            final int planeHeight = (plane == 0) ? imageHeight : imageHeight / 2;
            if (pixelStride == 1 && rowStride == planeWidth) {
                // Copy whole plane from buffer into |data| at once.
                buffer.get(data, offset, planeWidth * planeHeight);
                offset += planeWidth * planeHeight;
            } else {
                // Copy pixels one by one respecting pixelStride and rowStride.
                byte[] rowData = new byte[rowStride];
                for (int row = 0; row < planeHeight - 1; ++row) {
                    buffer.get(rowData, 0, rowStride);
                    for (int col = 0; col < planeWidth; ++col) {
                        data[offset++] = rowData[col * pixelStride];
                    }
                }
                // Last row is special in some devices and may not contain the full
                // |rowStride| bytes of data.
                // See http://developer.android.com/reference/android/media/Image.Plane.html#getBuffer()
                buffer.get(rowData, 0, Math.min(rowStride, buffer.remaining()));
                for (int col = 0; col < planeWidth; ++col) {
                    data[offset++] = rowData[col * pixelStride];
                }
            }
        }

        return data;
    }
}
