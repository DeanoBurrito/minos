#pragma once

#include <CppStd.h>
#include <Memory.h>

namespace sl
{
    namespace priv
    {
        template<typename... Ts>
        struct Tuple{};

        template <typename T>
        struct Tuple<T>
        {
        private:
            T value;
        
        public:
            Tuple(const T& value) : value(value)
            {}

            Tuple(T&& value) : value(Forward<T>(value))
            {}

            template<typename U>
            U& Get()
            {
                //static_assert(IsSame<T, U>, "Funky tuple access attempt."); //TODO: this is always triggering? investigate, currently there's no false positives though.
                return value;
            }

            template<typename U>
            const U& Get() const
            {
                return const_cast<Tuple<T>&>(*this).Get<U>();
            }

            template<typename U, size_t index>
            U& GetWithIndex()
            {
                //static_assert(IsSame<T, U> && index == 0, "Funky tuple access attempt.");
                return value;
            }

            template<typename U, size_t index>
            const U& GetWithIndex() const
            {
                return const_cast<Tuple<T>&>(*this).GetWithIndex<U, index>();
            }
        };

        template<typename T, typename... TRest>
        struct Tuple<T, TRest...> : Tuple<TRest...>
        {
        private:
            T value;

        public:
            template<typename First, typename... Rest>
            Tuple(First&& first, Rest&&... rest) : Tuple<TRest...>(Forward<Rest>(rest)...), value(Forward<First>(first))
            {}

            Tuple(T&& first, TRest&&... rest) : Tuple<TRest...>(sl::move(rest)...), value(sl::move(first))
            {}

            template<typename U>
            U& Get()
            {
                if constexpr (IsSame<T, U>)
                    return value;
                return Tuple<TRest...>::template Get<U>();
            }

            template<typename U>
            const U& Get() const
            {
                return const_cast<Tuple<T, TRest...>&>(*this).Get<U>();
            }

            template<typename U, size_t index>
            U& GetWithIndex()
            {
                if constexpr (IsSame<T, U> && index == 0)
                    return value;
                return Tuple<TRest...>::template GetWithIndex<U, index - 1>();
            }

            template<typename U, size_t index>
            const U& GetWithIndex() const
            {
                return const_cast<Tuple<T, TRest...>&>(*this).GetWithIndex<U, index>();
            }
        };

        //typelist stuff - for internal use
        //TODO: move this to it's own file, its more or less standalone
        template<typename... Types>
        struct TypeList;

        template<size_t index, typename List>
        struct TypeListElement;

        template<size_t index, typename Head, typename... Tail>
        struct TypeListElement<index, TypeList<Head, Tail...>> : TypeListElement<index - 1, TypeList<Tail...>>
        {};

        template<typename Head, typename... Tail>
        struct TypeListElement<0, TypeList<Head, Tail...>>
        {
            using type = Head;
        };

        template<typename... Types>
        struct TypeList
        {
            static constexpr size_t size = sizeof...(Types);

            template<size_t n>
            using type = typename TypeListElement<n, TypeList<Types...>>::type;
        };

        template<typename T>
        struct TypeWrapper
        {
            using type = T;
        };

        template<typename List, typename F, size_t... indices>
        constexpr void ForEachTypeImpl(F&& f, IndexSequence<indices...>)
        {
            (Forward<F>(f)(TypeWrapper<typename List::template type<indices>> {}), ...);
        }
        
        template<typename List, typename F>
        constexpr void ForEachType(F&& f)
        {
            ForEachTypeImpl<List>(Forward<F>(f), MakeIndexSequence<List::size> {});
        }

        template<typename A, typename B, typename F, size_t... indices>
        constexpr void ForEachTypeZippedImpl(F&& f, IndexSequence<indices...>)
        {
            (Forward<F>(f)(TypeWrapper<typename A::template type<indices>> {}, TypeWrapper<typename B::template type<indices>> {}), ...);
        }

        template<typename A, typename B, typename F>
        constexpr void ForEachTypeZipped(F&& f)
        {
            static_assert(A::size == B::size, "Cant zip typelists that arent the same size!");
            ForEachTypeZippedImpl<A, B>(Forward<F>(f), MakeIndexSequence<A::size> {});
        }
    }

    template<typename... Ts>
    struct Tuple : priv::Tuple<Ts...>
    {
    private:
        template<size_t... Is>
        Tuple(const Tuple& other, IndexSequence<Is...>) : priv::Tuple<Ts...>(other.Get<Is>()...)
        {}

        template<size_t... Is>
        Tuple(Tuple&& from, IndexSequence<Is...>) : priv::Tuple<Ts...>(sl::move(from.Get<Is>())...)
        {}

        template<size_t... Is>
        void Set(const Tuple& other, IndexSequence<Is...>)
        {
            ((Get<Is>() = other.Get<Is>()), ...);
        }

        template<size_t... Is>
        void Set(Tuple&& from, IndexSequence<Is...>)
        {
            ((Get<Is>() = sl::move(from.Get<Is>())), ...);
        }

        template<typename F, size_t... Is>
        auto ApplyAsArgs(F&& f, IndexSequence<Is...>)
        {
            return Forward<F>(f)(Get<Is>()...);
        }

        template<typename F, size_t... Is>
        auto ApplyAsArgs(F&& f, IndexSequence<Is...>) const
        {
            return Forward<F>(f)(Get<Is>()...);
        }

    public:
        using types = priv::TypeList<Ts...>;
        using priv::Tuple<Ts...>::Tuple;
        using indices = MakeIndexSequence<sizeof...(Ts)>;

        Tuple(const Tuple& other) : Tuple(other, indices())
        {}

        Tuple& operator=(const Tuple& other)
        {
            Set(other, indices());
            return *this;
        }

        Tuple(Tuple&& from) : Tuple(sl::move(from), indices())
        {}

        Tuple& operator=(Tuple&& from)
        {
            Set(move(from), indices());
            return *this;
        }

        template<typename T>
        auto& Get()
        {
            return priv::Tuple<Ts...>::template Get<T>();
        }

        template<size_t index>
        auto& Get()
        {
            return priv::Tuple<Ts...>::template GetWithIndex<typename types::template type<index>, index>();
        }

        template<typename T>
        auto& Get() const
        {
            return priv::Tuple<Ts...>::template Get<T>();
        }

        template<size_t index>
        auto& Get() const
        {
            return priv::Tuple<Ts...>::template GetWithIndex<typename types::template type<index>, index>();
        }

        template<typename F>
        auto ApplyAsArgs(F&& f)
        {
            return ApplyAsArgs(Forward<F>(f), indices());
        }

        template<typename F>
        auto ApplyAsArgs(F&& f) const
        {
            return ApplyAsArgs(Forward<F>(f), indices());
        }

        static constexpr size_t Size()
        {
            return sizeof...(Ts);
        }
    };

    //handy shortcut, not strictly needed
    template<typename... Args>
    Tuple(Args... args) -> Tuple<Args...>;
}
