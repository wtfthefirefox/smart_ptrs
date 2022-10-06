#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap
#include <stddef.h>

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;

        return count_;
    };
    size_t DecRef() {
        --count_;

        return count_;
    };
    size_t RefCount() const {
        return count_;
    };

    ~SimpleCounter() = default;

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    };

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (RefCount() <= 1) {
            deleter_.Destroy(static_cast<Derived*>(this));
        } else {
            counter_.DecRef();
        }
    };

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    };

    ~RefCounted() = default;

private:
    Counter counter_;
    Deleter deleter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() {
        IncreaseCount();
    };
    IntrusivePtr(std::nullptr_t) {
        IncreaseCount();
    };
    IntrusivePtr(T* ptr) : ptr_(ptr) {
        IncreaseCount();
    };

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) : ptr_(other.Get()) {
        IncreaseCount();
    };

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) : ptr_(other.Get()) {
        other.ptr_ = nullptr;
    };

    IntrusivePtr(const IntrusivePtr& other) : ptr_(other.Get()) {
        IncreaseCount();
    };
    IntrusivePtr(IntrusivePtr&& other) : ptr_(other.Get()) {
        other.ptr_ = nullptr;
    };

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (&other == this) {
            return *this;
        }

        DeletePtr();

        ptr_ = other.Get();
        IncreaseCount();

        return *this;
    };
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (&other == this) {
            return *this;
        }

        DeletePtr();

        ptr_ = other.Get();
        other.ptr_ = nullptr;

        return *this;
    };

    // Destructor
    ~IntrusivePtr() {
        DeletePtr();
    };

    // Modifiers
    void Reset() {
        DecreaseCount();
        ptr_ = nullptr;
    };
    void Reset(T* ptr) {
        Reset();
        ptr_ = ptr;
        IncreaseCount();
    };
    void Swap(IntrusivePtr& other) {
        std::swap(other.ptr_, ptr_);
    };

    // Observers
    T* Get() const {
        return ptr_;
    };
    T& operator*() const {
        return *Get();
    };
    T* operator->() const {
        return Get();
    };
    size_t UseCount() const {
        if (ptr_) {
            return ptr_->RefCount();
        }

        return 0;
    };
    explicit operator bool() const {
        return ptr_;
    };

private:
    T* ptr_ = nullptr;

    void DeletePtr() {
        if (ptr_) {
            ptr_->DecRef();
            ptr_ = nullptr;
        }
    }

    void IncreaseCount() {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    void DecreaseCount() {
        if (ptr_) {
            size_t prev_count = UseCount();
            ptr_->DecRef();

            if (prev_count == 1) {
                ptr_ = nullptr;
            }
        }
    }
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}
