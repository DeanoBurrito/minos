#pragma once

#include <CppStd.h>
#include <stdint.h>
#include <PlacementNew.h>

namespace sl
{
    template<typename T>
    class [[nodiscard]] Optional
    {
    private:
        alignas(T) uint8_t storage[sizeof(T)]; //cpp has come a long way in some areas
        bool hasValue = false;

    public:
        using ValueType = T;

        Optional() = default;

        ~Optional() 
        {
            Clear();
        }

        Optional(const Optional& other) : hasValue(other.hasValue)
        {
            if (other.hasValue)
                new(&storage) T(other.Value());
        }

        Optional& operator=(const Optional& other) 
        {
            if (this != &other)
            {
                Clear();
                hasValue = other.hasValue;
                if (other.hasValue)
                    new(&storage) T(other.Value());
            }

            return *this;
        }

        Optional(Optional&& from)  : hasValue(from.hasValue)
        {
            if (from.hasValue)
                new(&storage) T(from.ReleaseValue());
        }

        Optional& operator=(Optional&& from) 
        {
            if (this != &from)
            {
                Clear();
                hasValue = from.hasValue;
                if (from.hasValue)
                    new(&storage) T(from.ReleaseValue());
            }

            return *this;
        }

        template<typename U = T>
        explicit Optional(U&& value) : hasValue(true)
        {
            new(&storage) T(Forward<U>(value));
        }

        template<typename O>
        bool operator==(const Optional<O>& other) const
        {
            return (hasValue == other.hasValue) && (!hasValue || Value() == other.Value());
        }

        template<typename O>
        bool operator==(const O& other) const
        {
            return hasValue && (Value() == other);
        }

        void Clear()
        {
            if (hasValue)
                Value().~T();

            hasValue = false;
        }

        template<typename... Parameters>
        void Emplace(Parameters&&... params)
        {
            Clear();
            hasValue = true;
            new(&storage) T(Forward<Parameters>(params)...);
        }

        bool HasValue() const
        {
            return hasValue;
        }

        T& Value() &
        {
            //NOTE: this could explode is !hasValue, but we're assuming the user is checking beforehand.
            return *Launder(reinterpret_cast<T*>(&storage));
        }

        const T& Value() const&
        {
            return *Launder(reinterpret_cast<const T*>(&storage));
        }

        T Value() &&
        {
            return ReleaseValue();
        }

        T ReleaseValue()
        {
            if (!hasValue)
                return;

            T releasedValue = move(Value());
            Value().~T();
            hasValue = false;
            return releasedValue;
        }

        T ValueOr(const T& fallback) const&
        {
            if (hasValue)
                return Value();
            return fallback;
        }

        T ValueOr(T&& fallback) &&
        {
            if (hasValue)
                return move(Value());
            return move(fallback);
        }

        const T& operator*() const
        { return Value(); }

        T& operator*()
        { return Value(); }

        const T* operator->() const
        { return &Value(); }

        T* operator->()
        { return &Value(); }
    };
}
