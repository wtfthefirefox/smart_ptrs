#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>

template <typename T>
struct DefaultDelete {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

template <typename T>
struct DefaultDelete<T[]> {
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDelete<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) {
        data_pair_.GetFirst() = ptr;
    };
    UniquePtr(T* ptr, Deleter deleter) : data_pair_(ptr, deleter){};

    UniquePtr(UniquePtr&& other) noexcept
        : data_pair_(other.data_pair_.GetFirst(), other.data_pair_.GetSecond()) {
        other.data_pair_.GetFirst() = nullptr;
    };

    template <typename Another, typename AnotherDeleter>  // upcast constructor
    UniquePtr(UniquePtr<Another, AnotherDeleter>&& other) {
        data_pair_.GetFirst() = other.data_pair_.GetFirst();
        other.data_pair_.GetFirst() = nullptr;
    };

    UniquePtr(UniquePtr&) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (&other == this) {
            return *this;
        }

        if (data_pair_.GetFirst() != nullptr) {
            data_pair_.GetSecond()(data_pair_.GetFirst());
        }

        std::swap(other.data_pair_, data_pair_);

        other.data_pair_.GetFirst() = nullptr;

        return *this;
    };
    //    template <typename Derived, typename DerivedDeleter>
    //    UniquePtr& operator=(UniquePtr<Derived, DerivedDeleter>&& other) noexcept {
    //        if (data_pair_.GetFirst() != nullptr) {
    //            data_pair_.GetSecond()(data_pair_.GetFirst());
    //        }
    //
    //        data_pair_.GetFirst() = other.data_pair_.GetFirst();
    //        data_pair_.GetSecond() = other.data_pair_.GetSecond();
    //
    //        other.data_pair_.GetFirst() = nullptr;
    //        other.data_pair_.GetSecond() = nullptr;
    //
    //        return *this;
    //    };
    UniquePtr& operator=(std::nullptr_t) {
        if (data_pair_.GetFirst() != nullptr) {
            data_pair_.GetSecond()(data_pair_.GetFirst());
        }

        data_pair_.GetFirst() = nullptr;

        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto temp = data_pair_.GetFirst();
        data_pair_.GetFirst() = nullptr;
        return temp;
    };
    void Reset(T* ptr = nullptr) {
        std::swap(data_pair_.GetFirst(), ptr);

        if (ptr != nullptr) {
            data_pair_.GetSecond()(ptr);
        }
    };
    void Reset(T* ptr = nullptr) const {
        std::swap(data_pair_.GetFirst(), ptr);

        if (ptr != nullptr && data_pair_.GetSecond() != nullptr) {
            data_pair_.GetSecond()(ptr);
        }
    };
    void Swap(UniquePtr& other) {
        std::swap(other.data_pair_, data_pair_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() {
        return data_pair_.GetFirst();
    };

    T* Get() const {
        return data_pair_.GetFirst();
    };
    Deleter& GetDeleter() {
        return data_pair_.GetSecond();
    };
    const Deleter& GetDeleter() const {
        return data_pair_.GetSecond();
    };
    explicit operator bool() const {
        return data_pair_.GetFirst() != nullptr;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {  // fix void
        return *data_pair_.GetFirst();
    };
    T* operator->() const {
        return data_pair_.GetFirst();
    };

    // private:
    CompressedPair<T*, Deleter> data_pair_;
};

template <typename T>
class UniquePtr<T[], DefaultDelete<T[]>*> {
public:
    UniquePtr(T* ptr) {
        data_pair_.GetFirst() = ptr;
        data_pair_.GetSecond() = DefaultDelete<T[]>();
    };

    ~UniquePtr() {
        Reset();
    };

    void Reset(T* ptr = nullptr) {
        std::swap(data_pair_.GetFirst(), ptr);

        if (ptr != nullptr) {
            data_pair_.GetSecond()(ptr);
        }
    };
    void Reset(T* ptr = nullptr) const {
        std::swap(data_pair_.GetFirst(), ptr);

        if (ptr != nullptr) {
            data_pair_.GetSecond()(ptr);
        }
    };

    T* Get() {
        return data_pair_.GetFirst();
    };

    T* Get() const {
        return data_pair_.GetFirst();
    };

    T& operator[](size_t idx) {
        return Get()[idx];
    }

    CompressedPair<T*, DefaultDelete<T[]>> data_pair_;
};

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    UniquePtr(T* ptr) {
        data_pair_.GetFirst() = ptr;
    };
    UniquePtr(T* ptr, Deleter deleter) : data_pair_(ptr, deleter){};

    ~UniquePtr() {
        Reset();
    };

    void Reset(T* ptr = nullptr) {
        std::swap(data_pair_.GetFirst(), ptr);

        if (ptr != nullptr) {
            data_pair_.GetSecond()(ptr);
        }
    };
    void Reset(T* ptr = nullptr) const {
        std::swap(data_pair_.GetFirst(), ptr);

        if (ptr != nullptr) {
            data_pair_.GetSecond()(ptr);
        }
    };

    T* Get() {
        return data_pair_.GetFirst();
    };

    T* Get() const {
        return data_pair_.GetFirst();
    };

    T& operator[](size_t idx) {
        return Get()[idx];
    }

    CompressedPair<T*, Deleter> data_pair_;
};
