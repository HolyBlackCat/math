#pragma once

#include "em/macros/meta/common.h"
#include "em/macros/portable/canonical_typedefs.h"

#include <cstddef>
#include <cstdint>

// Short spellings for types, to bake them into type names.

#define EM_MATH_TYPE_SHORTHANDS(X, ...) \
    X( b   , bool               __VA_OPT__(,)__VA_ARGS__ ) \
    X( c   , char               __VA_OPT__(,)__VA_ARGS__ ) \
    X( uc  , unsigned char      __VA_OPT__(,)__VA_ARGS__ ) \
    X( sc  , signed char        __VA_OPT__(,)__VA_ARGS__ ) \
    X( s   , short              __VA_OPT__(,)__VA_ARGS__ ) \
    X( us  , unsigned short     __VA_OPT__(,)__VA_ARGS__ ) \
    X( i   , int                __VA_OPT__(,)__VA_ARGS__ ) \
    X( u   , unsigned int       __VA_OPT__(,)__VA_ARGS__ ) \
    X( l   , long               __VA_OPT__(,)__VA_ARGS__ ) \
    X( ul  , unsigned long      __VA_OPT__(,)__VA_ARGS__ ) \
    X( ll  , long long          __VA_OPT__(,)__VA_ARGS__ ) \
    X( ull , unsigned long long __VA_OPT__(,)__VA_ARGS__ ) \
    X( f   , float              __VA_OPT__(,)__VA_ARGS__ ) \
    X( d   , double             __VA_OPT__(,)__VA_ARGS__ ) \
    X( ld  , long double        __VA_OPT__(,)__VA_ARGS__ ) \
    X( i8  , std::int8_t        __VA_OPT__(,)__VA_ARGS__ ) \
    X( u8  , std::uint8_t       __VA_OPT__(,)__VA_ARGS__ ) \
    X( i16 , std::int16_t       __VA_OPT__(,)__VA_ARGS__ ) \
    X( u16 , std::uint16_t      __VA_OPT__(,)__VA_ARGS__ ) \
    X( i32 , std::int32_t       __VA_OPT__(,)__VA_ARGS__ ) \
    X( u32 , std::uint32_t      __VA_OPT__(,)__VA_ARGS__ ) \
    X( i64 , std::int64_t       __VA_OPT__(,)__VA_ARGS__ ) \
    X( u64 , std::uint64_t      __VA_OPT__(,)__VA_ARGS__ ) \
    X( x   , std::ptrdiff_t     __VA_OPT__(,)__VA_ARGS__ ) \
    X( z   , std::size_t        __VA_OPT__(,)__VA_ARGS__ ) \

// Same syntax as `EM_CANONICAL_TYPEDEFS(...)`, but without the type list, since that's generated automatically.
#define EM_MATH_TYPE_SHORTHANDS_VEC(kind_, vec_) \
    EM_CANONICAL_TYPEDEFS( kind_, vec_, EM_MATH_TYPE_SHORTHANDS(DETAIL_EM_MATH_TYPE_SHORTHANDS_VEC_ELEM_CANONICAL, vec_) ) \
    template <typename T> using EM_CAT(vec_,2) = vec_<T,2>; \
    template <typename T> using EM_CAT(vec_,3) = vec_<T,3>; \
    template <typename T> using EM_CAT(vec_,4) = vec_<T,4>; \
    EM_MATH_TYPE_SHORTHANDS(DETAIL_EM_MATH_TYPE_SHORTHANDS_VEC_ELEM_TEMPLATE, vec_)

#define DETAIL_EM_MATH_TYPE_SHORTHANDS_VEC_ELEM_CANONICAL(t_, type_, vec_) \
    (EM_CAT3(t_,vec_,2), vec_<type_,2>)\
    (EM_CAT3(t_,vec_,3), vec_<type_,3>)\
    (EM_CAT3(t_,vec_,4), vec_<type_,4>)

#define DETAIL_EM_MATH_TYPE_SHORTHANDS_VEC_ELEM_TEMPLATE(t_, type_, vec_) \
    template <int N> using EM_CAT(t_,vec_) = vec_<type_,N>;

// Imports the typedefs generated by a `EM_MATH_TYPE_SHORTHANDS_VEC(...)`.
#define EM_MATH_IMPORT_TYPE_SHORTHANDS_VEC(qual_, vec_) \
    using qual_ EM_CAT(vec_,2); \
    using qual_ EM_CAT(vec_,3); \
    using qual_ EM_CAT(vec_,4); \
    EM_MATH_TYPE_SHORTHANDS(DETAIL_EM_MATH_IMPORT_TYPE_SHORTHANDS_VEC_ELEM, qual_, vec_)

#define DETAIL_EM_MATH_IMPORT_TYPE_SHORTHANDS_VEC_ELEM(t_, type_, qual_, vec_) \
    using qual_ EM_CAT(t_,vec_); \
    using qual_ EM_CAT3(t_,vec_,2); \
    using qual_ EM_CAT3(t_,vec_,3); \
    using qual_ EM_CAT3(t_,vec_,4);