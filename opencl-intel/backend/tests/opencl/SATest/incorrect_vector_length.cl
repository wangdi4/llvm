//RUN: SATest -OCL -VAL -config=incorrect_vector_size.cl |& FileCheck %s
//CHECK: vector data does not fit its declared size

__kernel void k( __global int8* v ){
}
