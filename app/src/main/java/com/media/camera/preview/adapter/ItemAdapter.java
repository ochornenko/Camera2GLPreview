package com.media.camera.preview.adapter;

import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import java.util.List;

public class ItemAdapter<T> extends RecyclerView.Adapter<ItemAdapter.ViewHolder> {

    private List<T> mItems;
    private ItemListener<T> mListener;
    private int mLayoutId;

    public ItemAdapter(List<T> items, ItemListener<T> listener, int layoutId) {
        mItems = items;
        mListener = listener;
        mLayoutId = layoutId;
    }

    @NonNull
    @Override
    @SuppressWarnings("unchecked")
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new ViewHolder((RecyclerViewRow<T>)LayoutInflater.from(parent.getContext())
                .inflate(mLayoutId, parent, false));
    }

    @Override
    @SuppressWarnings("unchecked")
    public void onBindViewHolder(@NonNull final ViewHolder holder, int position) {
        holder.mRow.setData(mItems.get(position));
        ((View) holder.mRow).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mListener != null) {
                    mListener.onItemClick(mItems.get(holder.getAdapterPosition()));
                }
            }
        });
    }

    @Override
    public int getItemCount() {
        return mItems.size();
    }

    public void setItems(List<T> items) {
        mItems = items;
    }

    static class ViewHolder extends RecyclerView.ViewHolder {
        private RecyclerViewRow mRow;

        ViewHolder(RecyclerViewRow itemView) {
            super((View) itemView);
            mRow = itemView;
        }
    }

    public interface RecyclerViewRow<T> {
        void setData(T item);
    }

    public interface ItemListener<T> {
        void onItemClick(T item);
    }
}
