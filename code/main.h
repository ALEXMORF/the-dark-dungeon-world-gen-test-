#pragma once

#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float r32;
typedef double r64;
typedef i32 b32;

#define internal static
#define local_persist static
#define global static
#define true 1
#define false 0

struct v2
{
    r32 x, y;
};

struct v2i
{
    i32 x, y;
};

