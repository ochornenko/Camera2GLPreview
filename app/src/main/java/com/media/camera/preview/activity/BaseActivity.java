package com.media.camera.preview.activity;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.FragmentActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Size;
import android.view.MotionEvent;

import com.media.camera.preview.R;
import com.media.camera.preview.adapter.ItemAdapter;
import com.media.camera.preview.capture.PreviewFrameHandler;
import com.media.camera.preview.capture.VideoCameraPreview;
import com.media.camera.preview.gesture.SimpleGestureFilter;

import java.util.ArrayList;
import java.util.List;

public abstract class BaseActivity extends FragmentActivity implements PreviewFrameHandler,
        SimpleGestureFilter.SimpleGestureListener, ItemAdapter.ItemListener {

    protected VideoCameraPreview mPreview;
    protected SimpleGestureFilter mDetector;
    protected ResolutionDialog mResolutionDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mPreview = new VideoCameraPreview(this);
        mDetector = new SimpleGestureFilter(this, this);
        mResolutionDialog = new ResolutionDialog(this);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        mDetector.onTouchEvent(event);
        return super.onTouchEvent(event);
    }

    public void showResolutionDialog(List<Size> items) {
        mResolutionDialog.setItems(items);
        mResolutionDialog.show();
    }

    public class ResolutionDialog extends Dialog {
        private RecyclerView mRecyclerView;
        private ItemAdapter mAdapter;

        ResolutionDialog(@NonNull Context context) {
            super(context);

            setContentView(R.layout.resolution_dialog);

            if (getWindow() != null) {
                getWindow().setBackgroundDrawableResource(android.R.color.transparent);
            }
            mRecyclerView = findViewById(R.id.recycler_view);

            mRecyclerView.setHasFixedSize(true);
            mRecyclerView.setLayoutManager(new LinearLayoutManager(BaseActivity.this));

            ArrayList<Size> items = new ArrayList<>();
            mAdapter = new ItemAdapter(items, BaseActivity.this);
            mRecyclerView.setAdapter(mAdapter);
        }

        void setItems(List<Size> items) {
            mAdapter.setItems(items);
        }
    }

    @Override
    public void onItemClick(Size item) {
        mResolutionDialog.dismiss();
    }
}
