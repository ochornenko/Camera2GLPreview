package com.media.camera.preview.view;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Size;

import com.media.camera.preview.adapter.ItemAdapter;

public class SizeItemRow extends ItemRow implements ItemAdapter.RecyclerViewRow<Size> {
    public SizeItemRow(Context context) {
        super(context);
    }

    public SizeItemRow(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public SizeItemRow(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    public void setData(Size item) {
        mTextView.setText(item.toString());
    }
}
