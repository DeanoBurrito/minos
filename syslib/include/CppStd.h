#pragma once

/*
    A bit of a dumping ground for standard c++ features that we need for various things.
    Will refactor as needed. Try not to look too closely.
*/

#include <stddef.h>

namespace sl
{
    template<typename T>
    struct RemoveReference
    { using type = T; };

    template<typename T>
    struct RemoveReference<T&>
    { using type = T; };

    template<typename T>
    struct RemoveReference<T&&>
    { using type = T; };

    template<typename T>
    using RemoveReferenceType = typename RemoveReference<T>::type;

    template<typename T>
    inline constexpr bool IsLValueReference = false;

    template<typename T>
    inline constexpr bool IsLValueReference<T&> = true;

    template<typename T>
    constexpr T&& Forward(RemoveReferenceType<T>& param)
    { return static_cast<T&&>(param); }

    template<typename T>
    constexpr T&& Forward(RemoveReferenceType<T>&& param)
    {
        static_assert(!IsLValueReference<T>, "Cant forward an rvalue as lvalue.");
        return static_cast<T&&>(param);
    }

    template<typename T>
    constexpr  T* Launder(T* p)
    {
        return __builtin_launder(p);
    }

    template<typename T, T... Ts>
    struct IntegerSequence
    {
        using type = T;
        static constexpr size_t size() { return sizeof...(Ts); };
    };

    template<size_t... Indices>
    using IndexSequence = IntegerSequence<unsigned, Indices...>;

    namespace priv
    {
        template<typename T, T N, T... Ts>
        auto MakeIntegerSequenceImpl()
        {
            if constexpr (N == 0)
                return IntegerSequence<T, Ts...> {};
            else
                return MakeIntegerSequenceImpl<T, N - 1, N - 1, Ts...>();
        }
    }

    template<typename T, T N>
    using MakeIntegerSequence = decltype(priv::MakeIntegerSequenceImpl<T, N>());

    template<size_t N>
    using MakeIndexSequence = MakeIntegerSequence<size_t, N>;

    template<bool B, typename T = void>
    struct EnableIf 
    {};

    template<typename T>
    struct EnableIf<true, T>
    {
        using type = T;
    };

    template<typename T>
    struct __MakeUnsigned { using Type = void; };

    template<>
    struct __MakeUnsigned<signed char> { using Type = unsigned char; };
    template<>
    struct __MakeUnsigned<signed short> { using Type = unsigned short; };
    template<>
    struct __MakeUnsigned<signed int> { using Type = unsigned int; };
    template<>
    struct __MakeUnsigned<signed long> { using Type = unsigned long; };
    template<>
    struct __MakeUnsigned<signed long long> { using Type = unsigned long long; };
    template<>
    struct __MakeUnsigned<unsigned char> { using Type = unsigned char; };
    template<>
    struct __MakeUnsigned<unsigned short> { using Type = unsigned short; };
    template<>
    struct __MakeUnsigned<unsigned int> { using Type = unsigned int; };
    template<>
    struct __MakeUnsigned<unsigned long> { using Type = unsigned long; };
    template<>
    struct __MakeUnsigned<unsigned long long> { using Type = unsigned long long; };

    template<typename T>
    using MakeUnsigned = typename __MakeUnsigned<T>::Type;

    template<typename T>
    using AddConst = const T;

    template<typename T>
    struct __RemoveConst { using Type = T; };

    template<typename T>
    struct __RemoveConst<const T> { using Type = T; };

    template<typename T>
    using RemoveConst = typename __RemoveConst<T>::Type;

    template<typename T>
    struct __RemoveVolatile { using Type = T; };

    template <typename T>
    struct __RemoveVolatile<volatile T> { using Type = T; };

    template<typename T>
    using RemoveVolatile = typename __RemoveVolatile<T>::Type;

    template<typename T>
    using RemoveCV = RemoveVolatile<RemoveConst<T>>;

    template<typename T, typename U>
    inline constexpr bool IsSame = false;

    template<typename T>
    inline constexpr bool IsSame<T, T> = true;

    template<typename T>
    inline constexpr bool __IsIntegral = false;

    template<>
    inline constexpr bool __IsIntegral<bool> = true;
    template<>
    inline constexpr bool __IsIntegral<unsigned char> = true;
    template<>
    inline constexpr bool __IsIntegral<unsigned short> = true;
    template<>
    inline constexpr bool __IsIntegral<unsigned int> = true;
    template<>
    inline constexpr bool __IsIntegral<unsigned long> = true;
    template<>
    inline constexpr bool __IsIntegral<unsigned long long> = true;

    template<typename T>
    inline constexpr bool IsIntegral = __IsIntegral<MakeUnsigned<RemoveCV<T>>>;
}
