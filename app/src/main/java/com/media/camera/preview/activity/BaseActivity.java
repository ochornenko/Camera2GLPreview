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
import android.widget.TextView;

import com.media.camera.preview.R;
import com.media.camera.preview.adapter.ItemAdapter;
import com.media.camera.preview.capture.PreviewFrameHandler;
import com.media.camera.preview.capture.VideoCameraPreview;
import com.media.camera.preview.gesture.SimpleGestureFilter;

import java.util.ArrayList;
import java.util.List;

public abstract class BaseActivity extends FragmentActivity implements PreviewFrameHandler,
        SimpleGestureFilter.SimpleGestureListener {

    protected VideoCameraPreview mPreview;
    protected SimpleGestureFilter mDetector;
    protected ResolutionDialog mResolutionDialog;
    protected int mParams;

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

    class BaseDialog extends Dialog {
        RecyclerView mRecyclerView;
        TextView mTextView;

        BaseDialog(@NonNull Context context) {
            super(context);

            setContentView(R.layout.dialog);

            if (getWindow() != null) {
                getWindow().setBackgroundDrawableResource(android.R.color.transparent);
            }
            mRecyclerView = findViewById(R.id.recycler_view);
            mRecyclerView.setHasFixedSize(true);
            mRecyclerView.setLayoutManager(new LinearLayoutManager(BaseActivity.this));

            mTextView = findViewById(R.id.text_view);
        }
    }

    public class ResolutionDialog extends BaseDialog {
        private ItemAdapter<Size> mAdapter;

        ResolutionDialog(@NonNull Context context) {
            super(context);

            mTextView.setText(R.string.select_resolution);
            ArrayList<Size> items = new ArrayList<>();
            mAdapter = new ItemAdapter<>(items, mListener, R.layout.size_list_item);
            mRecyclerView.setAdapter(mAdapter);
        }

        void setItems(List<Size> items) {
            mAdapter.setItems(items);
        }

        private ItemAdapter.ItemListener<Size> mListener = new ItemAdapter.ItemListener<Size>() {
            @Override
            public void onItemClick(Size item) {
                dismiss();
                mPreview.changeResolution(item);
            }
        };
    }
}
