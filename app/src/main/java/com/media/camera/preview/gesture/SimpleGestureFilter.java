package com.media.camera.preview.gesture;

import android.content.Context;
import android.view.GestureDetector;
import android.view.MotionEvent;

public class SimpleGestureFilter extends GestureDetector.SimpleOnGestureListener {

    public enum SwipeDirection {
        SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT
    }

    private static final int SWIPE_MIN_DISTANCE = 100;
    private static final int SWIPE_THRESHOLD_VELOCITY = 100;

    private final GestureDetector mDetector;
    private final SimpleGestureListener mListener;

    public SimpleGestureFilter(Context context, SimpleGestureListener listener) {
        mDetector = new GestureDetector(context, this);
        mListener = listener;
    }

    public void onTouchEvent(MotionEvent event) {
        mDetector.onTouchEvent(event);
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        if (e1.getX() - e2.getX() > SWIPE_MIN_DISTANCE && Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
            mListener.onSwipe(SwipeDirection.SWIPE_LEFT);
            return true;
        } else if (e2.getX() - e1.getX() > SWIPE_MIN_DISTANCE && Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
            mListener.onSwipe(SwipeDirection.SWIPE_RIGHT);
            return true;
        }

        if (e1.getY() - e2.getY() > SWIPE_MIN_DISTANCE && Math.abs(velocityY) > SWIPE_THRESHOLD_VELOCITY) {
            mListener.onSwipe(SwipeDirection.SWIPE_UP);
            return true;
        } else if (e2.getY() - e1.getY() > SWIPE_MIN_DISTANCE && Math.abs(velocityY) > SWIPE_THRESHOLD_VELOCITY) {
            mListener.onSwipe(SwipeDirection.SWIPE_DOWN);
            return true;
        }
        return false;
    }

    @Override
    public boolean onDoubleTap(MotionEvent e) {
        mListener.onDoubleTap();
        return true;
    }

    public interface SimpleGestureListener {
        void onSwipe(SwipeDirection direction);
        void onDoubleTap();
    }
}
