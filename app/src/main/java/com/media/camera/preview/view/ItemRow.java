package com.media.camera.preview.view;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.media.camera.preview.R;

public class ItemRow extends LinearLayout {

    protected TextView mTextView;

    public ItemRow(Context context) {
        super(context);
    }

    public ItemRow(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ItemRow(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mTextView = findViewById(R.id.text_view);
    }
}
