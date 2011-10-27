// RUN: %clang_cc1 %s -emit-llvm -o - | FileCheck %s

#define __private   __attribute__((address_space(0)))
#define __global    __attribute__((address_space(1)))
#define __constant  __attribute__((address_space(2)))
#define __local     __attribute__((address_space(3)))

struct _image2d_t;
struct _image3d_t;
struct _image2d_array_t;
typedef struct _image2d_t* image2d_t;
typedef struct _image3d_t* image3d_t;
typedef struct _image2d_array_t* image2d_array_t;

#define __rd __attribute__((annotate("__rd")))  
#define __wr __attribute__((annotate("__wr")))  

void __kernel 
__attribute__((vec_type_hint(int))) 
__attribute__((reqd_work_group_size(16, 32, 64)))
foo(int arg1, __global int *arg2, __constant int *arg3, __local int *arg4)
{
}

void __kernel 
__attribute__((work_group_size_hint(17, 33, 65)))
__attribute__((vec_type_hint(float)))
foo2(int arg1, float arg2, char arg3, short arg4)
{
}

void __kernel foo3(image2d_t __rd arg1, image3d_t __wr arg2, image3d_t arg3, image2d_array_t arg4)
{
}

// CHECK: metadata !"__attribute__((reqd_work_group_size(16,32,64))) __attribute__((vec_type_hint(int)))"
// CHECK: metadata !{i32 0, i32 1, i32 2, i32 3}
// CHECK: metadata !{i32 3, i32 3, i32 3, i32 3}
// CHECK: metadata !{metadata !"int", metadata !"int*", metadata !"int*", metadata !"int*"}
// CHECK: metadata !{metadata !"arg1", metadata !"arg2", metadata !"arg3", metadata !"arg4"}

// CHECK: metadata !"__attribute__((work_group_size_hint(17,33,65))) __attribute__((vec_type_hint(float)))"
// CHECK: metadata !{i32 0, i32 0, i32 0, i32 0}
// CHECK: metadata !{metadata !"int", metadata !"float", metadata !"char", metadata !"short"}

// CHECK: metadata !{i32 0, i32 1, i32 2, i32 2}
// CHECK: metadata !{metadata !"image2d_t", metadata !"image3d_t", metadata !"image3d_t", metadata !"image2d_array_t"}

