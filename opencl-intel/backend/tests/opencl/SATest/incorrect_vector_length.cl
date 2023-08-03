// RUN: SATest -VAL -config=incorrect_vector_size.cl 2>&1 | FileCheck %s
// CHECK: vector data does not fit its declared size

__kernel void k(__global int8 *v) {}
