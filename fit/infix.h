/*=============================================================================
    Copyright (c) 2014 Paul Fultz II
    infix.h
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef FIT_GUARD_FUNCTION_INFIX_H
#define FIT_GUARD_FUNCTION_INFIX_H

/// infix
/// =====
/// 
/// Description
/// -----------
/// 
/// The `infix` function adaptor allows the function to be used as an infix
/// operator. The operator must be placed inside the angle brackets(ie `<`
/// and `>`).
/// 
/// Synopsis
/// --------
/// 
///     template<class F>
///     constexpr infix_adaptor<F> infix(F f);
/// 
/// Semantics
/// ---------
/// 
///     assert((x <infix(f)> y) == f(x, y));
/// 
/// Requirements
/// ------------
/// 
/// F must be:
/// 
///     BinaryFunctionObject
///     MoveConstructible
/// 
/// Example
/// -------
/// 
///     struct plus_f
///     {
///         template<class T, class U>
///         T operator()(T x, U y) const
///         {
///             return x+y;
///         }
///     };
///     
///     constexpr infix_adaptor<plus_f> plus = {};
///     int r = 3 <plus> 2;
///     assert(r == 5);
/// 

#include <fit/detail/delegate.h>
#include <fit/detail/result_of.h>
#include <fit/always.h>
#include <fit/detail/move.h>
#include <fit/detail/make.h>
#include <fit/detail/static_const_var.h>
#include <fit/detail/compressed_pair.h>

namespace fit {
 
namespace detail{
template<class T, class F>
struct postfix_adaptor
: compressed_pair<T, F>
{
    typedef compressed_pair<T, F> base_type;
    // template<class X, class XF>
    // constexpr postfix_adaptor(X&& x, XF&& f) 
    // : F(fit::forward<XF>(f)), x(fit::forward<X>(x))
    // {}

    FIT_INHERIT_CONSTRUCTOR(postfix_adaptor, base_type);

    // template<class... Ts>
    // constexpr const F& base_function(Ts&&... xs) const
    // {
    //     return always_ref(*this)(xs...);
    // }

    template<class... Ts>
    constexpr const F& base_function(Ts&&... xs) const
    {
        return this->second(xs...);
    }

    template<class... Ts>
    constexpr const T& get_x(Ts&&... xs) const
    {
        return this->first(xs...);
    }

    FIT_RETURNS_CLASS(postfix_adaptor);

    template<class... Ts>
    constexpr FIT_SFINAE_RESULT(const F&, id_<T&&>, id_<Ts>...)
    operator()(Ts&&... xs) const FIT_SFINAE_RETURNS
    (
        (FIT_MANGLE_CAST(const F&)(FIT_CONST_THIS->base_function(xs...)))(
            FIT_MANGLE_CAST(T&&)(fit::move(FIT_CONST_THIS->get_x(xs...))), fit::forward<Ts>(xs)...
        )
    );

    template<class A>
    constexpr FIT_SFINAE_RESULT(const F&, id_<T&&>, id_<A>)
    operator>(A&& a) const FIT_SFINAE_RETURNS
    (
        (*FIT_CONST_THIS)(fit::forward<A>(a))
        // (FIT_MANGLE_CAST(const F&)(FIT_CONST_THIS->base_function(a)))(
        //     FIT_MANGLE_CAST(T&&)(fit::move(FIT_CONST_THIS->get_x(a))), fit::forward<A>(a)
        // )
    );
};

template<class T, class F>
constexpr postfix_adaptor<T, F> make_postfix_adaptor(T&& x, F f)
{
    return postfix_adaptor<T, F>(fit::forward<T>(x), fit::move(f));
}
}

template<class F>
struct infix_adaptor : F
{
    typedef infix_adaptor fit_rewritable1_tag;
    FIT_INHERIT_CONSTRUCTOR(infix_adaptor, F);

    template<class... Ts>
    constexpr const F& base_function(Ts&&... xs) const
    {
        return always_ref(*this)(xs...);
    }

    template<class... Ts>
    constexpr const F& infix_base_function(Ts&&... xs) const
    {
        return always_ref(*this)(xs...);
    }

    FIT_RETURNS_CLASS(infix_adaptor);

    template<class... Ts>
    constexpr auto operator()(Ts&&... xs) const FIT_RETURNS
    (
        (FIT_MANGLE_CAST(const F&)(FIT_CONST_THIS->base_function(xs...)))(fit::forward<Ts>(xs)...)
    );
};

template<class T, class F>
constexpr auto operator<(T&& x, const infix_adaptor<F>& i) FIT_RETURNS
(detail::make_postfix_adaptor(fit::forward<T>(x), fit::move(i.base_function(x))));

// TODO: Operators for static_

namespace detail {

template<class F>
struct static_function_wrapper;

// Operators for static_function_wrapper adaptor
template<class T, class F>
auto operator<(T&& x, const fit::detail::static_function_wrapper<F>& f) FIT_RETURNS
(
    detail::make_postfix_adaptor(fit::forward<T>(x), fit::move(f.base_function().infix_base_function()))
);

template<class F>
struct static_default_function;

// Operators for static_default_function adaptor
template<class T, class F>
auto operator<(T&& x, const fit::detail::static_default_function<F>&) FIT_RETURNS
(
    detail::make_postfix_adaptor(fit::forward<T>(x), fit::move(F().infix_base_function()))
);
}

FIT_DECLARE_STATIC_VAR(infix, detail::make<infix_adaptor>);

}

#endif