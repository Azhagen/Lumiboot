#include "runtime/sanitizer.h"

void __ubsan_handle_type_mismatch(const type_mismatch_info *data,
    uintptr_t pointer)
{
    (void)data;
    (void)pointer;
}

void __ubsan_handle_missing_return(const unreachable_data *data)
{
    (void)data;
}

void __ubsan_handle_load_invalid_value(const invalid_value_data *data,
    uintptr_t value)
{
    (void)data;
    (void)value;
}

void __ubsan_handle_shift_out_of_bounds(const shift_out_of_bounds_data *data,
    uintptr_t lhs, uintptr_t rhs)
{
    (void)data;
    (void)lhs;
    (void)rhs;
}

void __ubsan_handle_out_of_bounds(const out_of_bounds_data *data,
    uintptr_t index)
{
    (void)data;
    (void)index;
}

void __ubsan_handle_negate_overflow(const overflow_data *data,
    uintptr_t oldval)
{
    (void)data;
    (void)oldval;
}

void __ubsan_handle_divrem_overflow(const overflow_data *data,
    uintptr_t lhs, uintptr_t rhs)
{
    (void)data;
    (void)lhs;
    (void)rhs;
}

void __ubsan_handle_pointer_overflow(const pointer_overflow_data *data,
    uintptr_t base, uintptr_t result)
{
    (void)data;
    (void)base;
    (void)result;
}

void __ubsan_handle_invalid_builtin(const invalid_builtin_data *data)
{
    (void)data;
}

void __ubsan_handle_builtin_unreachable(const unreachable_data *data)
{
    (void)data;
}

void __ubsan_handle_type_mismatch_v1(const type_mismatch_info *data,
    uintptr_t pointer)
{
    (void)data;
    (void)pointer;
}

void __ubsan_handle_dynamic_type_cache_miss(dynamic_type_cache_miss_data *data,
    pointer_value pointer, hash_value hash)
{
    (void)data;
    (void)pointer;
    (void)hash;
}

void __ubsan_handle_dynamic_type_cache_miss_abort(dynamic_type_cache_miss_data *data,
    pointer_value pointer, hash_value hash)
{
    (void)data;
    (void)pointer;
    (void)hash;
}

void __ubsan_handle_add_overflow(const overflow_data *data,
    uintptr_t lhs, uintptr_t rhs)
{
    (void)data;
    (void)lhs;
    (void)rhs;
}

void __ubsan_handle_sub_overflow(const overflow_data *data,
    uintptr_t lhs, uintptr_t rhs)
{
    (void)data;
    (void)lhs;
    (void)rhs;
}

void __ubsan_handle_mul_overflow(const overflow_data *data,
    uintptr_t lhs, uintptr_t rhs)
{
    (void)data;
    (void)lhs;
    (void)rhs;
}

void __ubsan_handle_nonnull_arg(const nonnull_arg_data *data)
{
    (void)data;
}