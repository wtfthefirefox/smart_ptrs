#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <stdio.h>
#include <cstddef>  // std::nullptr_t

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr_);
        }
    };
    SharedPtr(std::nullptr_t){};
    explicit SharedPtr(T* ptr) : block_(new ControlBlockPointer<T>(ptr)), ptr_(ptr) {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr_);
        }
    };
    template <class Y>
    explicit SharedPtr(Y* ptr) : block_(new ControlBlockPointer<Y>(ptr)), ptr_(ptr) {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr_);
        }
    };
    SharedPtr(ControlBlockBase* block, T* ptr) : block_(block), ptr_(ptr) {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr_);
        }
    };

    SharedPtr(const SharedPtr& other) : block_(other.block_), ptr_(other.ptr_) {
        if (block_) {
            ++block_->strong_cnt;
        }

        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr_);
        }
    };
    template <typename Another>
    SharedPtr(const SharedPtr<Another>& other) : block_(other.GetBlock()), ptr_(other.Get()) {
        if (block_) {
            ++block_->strong_cnt;
        }

        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr_);
        }
    };
    template <typename Another>
    SharedPtr(SharedPtr<Another>&& other) : block_(other.GetBlock()), ptr_(other.Get()) {
        other.CreateNullObject();
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr_);
        }
    };
    SharedPtr(SharedPtr<T>&& other) : block_(other.block_), ptr_(other.ptr_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;

        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr_);
        }
    };

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : block_(other.GetBlock()), ptr_(ptr) {
        if (block_) {
            ++block_->strong_cnt;
        }

        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr_);
        }
    };

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) : block_(other.GetBlock()), ptr_(other.GetPtr()) {
        if (block_) {
            if (block_->strong_cnt == 0) {  // weak ptr is dead
                throw BadWeakPtr();         // throw error
            }

            ++block_->strong_cnt;
        }
    };

    explicit SharedPtr(WeakPtr<T>* other) : block_(other->GetBlock()), ptr_(other->GetPtr()) {
        if (block_) {
            if (block_->strong_cnt == 0) {  // weak ptr is dead
                throw BadWeakPtr();         // throw error
            }

            ++block_->strong_cnt;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (&other == this) {
            return *this;
        }

        DeleteBlock();
        ptr_ = other.ptr_;

        if (other.block_) {
            block_ = other.block_;
            ++block_->strong_cnt;
        }

        return *this;
    };
    SharedPtr& operator=(SharedPtr&& other) {
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

    ~SharedPtr() {
        DeleteBlock();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        DeleteBlock();
    };
    void Reset(T* ptr) {
        DeleteBlock();

        ptr_ = ptr;
        block_ = new ControlBlockPointer<T>(ptr);
    };
    template <typename Y>
    void Reset(Y* ptr) {
        DeleteBlock();

        ptr_ = ptr;
        block_ = new ControlBlockPointer<Y>(ptr);
    };
    void Swap(SharedPtr& other) {
        std::swap(other.block_, block_);
        std::swap(other.ptr_, ptr_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    ControlBlockBase* GetBlock() const {
        return block_;
    }
    void CreateNullObject() {
        block_ = nullptr;
        ptr_ = nullptr;
    }

    T* Get() const {
        return ptr_;
    };
    T& operator*() const {
        return *ptr_;
    };
    T* operator->() const {
        return Get();
    };
    size_t UseCount() const {
        if (block_) {
            return block_->strong_cnt;
        } else {
            return 0;
        }
    };
    explicit operator bool() const {
        if (block_) {
            return true;
        } else {
            return false;
        }
    };

private:
    ControlBlockBase* block_ = nullptr;
    T* ptr_ = nullptr;

    void DeleteBlock() {
        if (block_) {
            --block_->strong_cnt;

            if (block_->strong_cnt == 0) {
                size_t prev_count = block_->weak_cnt;
                block_->ClearPtr();

                if (prev_count == 0) {
                    delete block_;
                }
            }

            block_ = nullptr;
        }
        ptr_ = nullptr;
    }

    template <typename Y>
    void InitWeakThis(EnableSharedFromThis<Y>* e) {
        auto temp = new WeakPtr<Y>(this);
        e->weak_this_ = temp;
    }
};

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new ControlBlockEmplace<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(block, block->GetPtr());
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.GetBlock() == right.GetBlock();
};
