// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -emit-llvm %s -o - | FileCheck %s

#pragma OPENCL EXTENSION cl_altera_channels : enable

struct st {
  int i1;
  int i2;
};

channel int ich;
channel long lch ;
channel struct st sch;

// CHECK: @ich = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
// CHECK: @lch = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8
// CHECK: @sch = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4

// CHECK: !opencl.channels = !{![[MD1:[0-9]+]], ![[MD2:[0-9]+]], ![[MD3:[0-9]+]]}
// CHECK-DAG: ![[MD1]] = !{%opencl.channel_t addrspace(1)* addrspace(1)* @ich, i32 4, i32 4}
// CHECK-DAG: ![[MD2]] = !{%opencl.channel_t addrspace(1)* addrspace(1)* @lch, i32 8, i32 8}
// CHECK-DAG: ![[MD3]] = !{%opencl.channel_t addrspace(1)* addrspace(1)* @sch, i32 8, i32 4}

__kernel void k1() {
}
