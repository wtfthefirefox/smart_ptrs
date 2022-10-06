#pragma once

#include <stdio.h>
#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr(){};

    WeakPtr<const T>(std::nullptr_t){};

    WeakPtr(const WeakPtr& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            ++block_->weak_cnt;
        }
    };
    WeakPtr(WeakPtr&& other) : block_(other.GetBlock()), ptr_(other.GetPtr()) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    };

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : block_(other.GetBlock()), ptr_(other.Get()) {
        if (block_) {
            ++block_->weak_cnt;
        }
    };

    template <typename Y>
    WeakPtr(SharedPtr<Y>* other) : block_(other->GetBlock()), ptr_(other->Get()) {
        if (block_) {
            ++block_->weak_cnt;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (&other == this) {
            return *this;
        }

        DeleteBlock();
        ptr_ = other.ptr_;

        if (other.block_) {
            block_ = other.block_;
            ++block_->weak_cnt;
        }

        return *this;
    };
    WeakPtr& operator=(WeakPtr&& other) {
        if (&other == this) {
            return *this;
        }

        DeleteBlock();

        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);

        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        DeleteBlock();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        DeleteBlock();
    };
    void Swap(WeakPtr& other) {
        std::swap(other.block_, block_);
        std::swap(other.ptr_, ptr_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_) {
            return block_->strong_cnt;
        } else {
            return 0;
        }
    };
    bool Expired() const {
        if (block_) {
            return block_->strong_cnt == 0;
        } else {
            return true;
        }
    };
    SharedPtr<T> Lock() const {
        return Expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
    };

    ControlBlockBase* GetBlock() const {
        return block_;
    }
    T* GetPtr() const {
        return ptr_;
    }

private:
    ControlBlockBase* block_ = nullptr;
    T* ptr_ = nullptr;

    void DeleteBlock() {
        if (block_) {
            --block_->weak_cnt;

            if (block_->strong_cnt == 0 && block_->weak_cnt == 0) {
                delete block_;
            }

            block_ = nullptr;
        }

        ptr_ = nullptr;
    }
};
