#pragma once

#include <stdint.h>
#include <stddef.h>

typedef uintptr_t hash_value;
typedef uintptr_t pointer_value;

typedef struct source_location
{
    const char *filename;
    uint32_t    line;
    uint32_t    column;
} source_location;

typedef struct type_descriptor
{
    uint16_t type_kind;
    uint16_t type_info;
    uint8_t  type_name[];
} type_descriptor;

typedef struct type_mismatch_info
{
    source_location location;
    const type_descriptor *type;
    uint8_t log_alignment;
    uint8_t type_check_kind;
} type_mismatch_info;

typedef struct out_of_bounds_data
{
    source_location location;
    const type_descriptor *array_type;
    const type_descriptor *index_type;
} out_of_bounds_data;

typedef struct overflow_data {
    source_location location;
    const type_descriptor *type;
} overflow_data;

typedef struct shift_out_of_bounds_data {
    source_location location;
    const type_descriptor *lhs_type;
    const type_descriptor *rhs_type;
} shift_out_of_bounds_data;

typedef struct unreachable_data
{
    source_location location;
} unreachable_data;

typedef struct invalid_value_data
{
    source_location location;
    const type_descriptor *type;
} invalid_value_data;

typedef struct invalid_builtin_data
{
    source_location location;
    uint8_t kind;
} invalid_builtin_data;

typedef struct nonnull_return_data
{
    source_location attribute_location;
} nonnull_return_data;

typedef struct nonnull_arg_data
{
    source_location location;
    source_location attribute_location;
    int arg_index;
} nonnull_arg_data;

typedef struct pointer_overflow_data
{
    source_location location;
} pointer_overflow_data;

typedef struct dynamic_type_cache_miss_data
{
    source_location location;
    const type_descriptor *type;
    void *typeinfo;
    uint8_t typecheck_kind;
} dynamic_type_cache_miss_data;

typedef struct dynamic_type_info
{
    const char *most_derived_typename;
    intptr_t offset;
    const char *subobject_typename;
} dynamic_type_info;