#pragma once

#include <type_traits>
#include <utility>

template <typename T, std::size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairElement {
    explicit CompressedPairElement() : value(){};
    template <typename T2>
    explicit CompressedPairElement(T2&& el) : value(std::forward<T2>(el)){};

    T& GetValue() {
        return value;
    }

    const T& GetValue() const {
        return value;
    }

    T value;
};

template <typename T, std::size_t I>
struct CompressedPairElement<T, I, true> : public T {
    explicit CompressedPairElement(){};
    template <typename T2>
    explicit CompressedPairElement(T2&&){};

    T& GetValue() {
        return *this;
    }

    const T& GetValue() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : private CompressedPairElement<F, 0>, private CompressedPairElement<S, 1> {
    using First = CompressedPairElement<F, 0>;
    using Second = CompressedPairElement<S, 1>;

public:
    CompressedPair() : First(), Second(){};
    template <typename STemp, typename FTemp>
    CompressedPair(STemp&& first, FTemp&& second)
        : First(std::forward<F>(first)), Second(std::forward<S>(second)){};

    F& GetFirst() {
        return First::GetValue();
    }

    const F& GetFirst() const {
        return First::GetValue();
    }

    S& GetSecond() {
        return Second::GetValue();
    };

    const S& GetSecond() const {
        return Second::GetValue();
    };
};