package com.media.camera.preview.adapter;

import android.support.annotation.NonNull;
import android.support.v7.util.DiffUtil;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import java.util.List;

public class ItemAdapter<T> extends RecyclerView.Adapter<ItemAdapter.ViewHolder> {

    private List<T> mItems;
    private final ItemListener<T> mListener;
    private final int mLayoutId;

    public ItemAdapter(List<T> items, ItemListener<T> listener, int layoutId) {
        mItems = items;
        mListener = listener;
        mLayoutId = layoutId;
    }

    @NonNull
    @Override
    @SuppressWarnings("unchecked")
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new ViewHolder((RecyclerViewRow<T>) LayoutInflater.from(parent.getContext())
                .inflate(mLayoutId, parent, false));
    }

    @Override
    @SuppressWarnings("unchecked")
    public void onBindViewHolder(@NonNull final ViewHolder holder, int position) {
        holder.mRow.setData(mItems.get(position));
        ((View) holder.mRow).setOnClickListener(view -> {
            if (mListener != null) {
                mListener.onItemClick(mItems.get(holder.getAdapterPosition()));
            }
        });
    }

    @Override
    public int getItemCount() {
        return mItems.size();
    }

    public void setItems(List<T> items) {
        DiffUtil.DiffResult diffResult = DiffUtil.calculateDiff(new DiffUtil.Callback() {
            @Override
            public int getOldListSize() {
                return mItems.size();
            }

            @Override
            public int getNewListSize() {
                return items.size();
            }

            @Override
            public boolean areItemsTheSame(int oldItemPosition, int newItemPosition) {
                return mItems.get(oldItemPosition).equals(items.get(newItemPosition));
            }

            @Override
            public boolean areContentsTheSame(int oldItemPosition, int newItemPosition) {
                return mItems.get(oldItemPosition).equals(items.get(newItemPosition));
            }
        });

        mItems = items;
        diffResult.dispatchUpdatesTo(this);
    }

    static class ViewHolder extends RecyclerView.ViewHolder {
        private final RecyclerViewRow mRow;

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
