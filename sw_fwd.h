#pragma once

#include <exception>
#include <memory>
#include <stdio.h>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

struct ControlBlockBase {
    ControlBlockBase(){};
    size_t strong_cnt = 1;
    size_t weak_cnt = 0;

    virtual void ClearPtr() = 0;

    virtual ~ControlBlockBase() = default;
};

// magic trait - Hello EBO
class ESFTBase {};

template <typename T>
class EnableSharedFromThis : public ESFTBase {
public:
    SharedPtr<T> SharedFromThis() {
        if (weak_this_ == nullptr) {
            return SharedPtr<T>();
        }

        return SharedPtr<T>(weak_this_);
    };
    SharedPtr<const T> SharedFromThis() const {
        if (weak_this_const_) {
            return SharedPtr<const T>(*weak_this_const_);
        }

        return SharedPtr<const T>();
    };

    WeakPtr<T> WeakFromThis() noexcept {
        if (weak_this_ == nullptr) {
            return WeakPtr<T>();
        }

        return *weak_this_;
    };
    WeakPtr<const T> WeakFromThis() const noexcept {
        if (weak_this_const_) {
            return WeakPtr<const T>(*weak_this_const_);
        }

        return WeakPtr<const T>();
    };

    ~EnableSharedFromThis() {
        if (weak_this_) {
            delete weak_this_;
        }
    };

    WeakPtr<T>* weak_this_ = nullptr;
    WeakPtr<const T>* weak_this_const_ = nullptr;
};

// new shared_ptr
template <typename T>
struct ControlBlockPointer : public ControlBlockBase {
    explicit ControlBlockPointer(T* ptr) : ptr_(ptr){};

    ~ControlBlockPointer() override {
        if (ptr_) {
            delete ptr_;
        }
    }

    void ClearPtr() override {
        if (ptr_) {
            T* temp = ptr_;
            ptr_ = nullptr;
            delete temp;
        }
    }

    T* ptr_ = nullptr;
};

// make_shared
template <typename T>
struct ControlBlockEmplace : public ControlBlockBase {
    template <typename... Args>
    ControlBlockEmplace(Args&&... args) {
        new (&storage_[0]) T(std::forward<Args>(args)...);
    }

    ~ControlBlockEmplace() override {
        if (!is_delete_) {
            std::destroy_at(std::launder(reinterpret_cast<T*>(&storage_[0])));
        }
    };

    T* GetPtr() {
        return reinterpret_cast<T*>(&storage_[0]);
    }

    void ClearPtr() override {
        if (!is_delete_) {
            is_delete_ = true;
            std::destroy_at(std::launder(reinterpret_cast<T*>(&storage_[0])));
        }
    }

    alignas(T) char storage_[sizeof(T)];
    bool is_delete_ = false;
};
