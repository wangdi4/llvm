// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -emit-llvm %s -o - | FileCheck %s

#pragma OPENCL EXTENSION cl_intel_channels : enable

struct st {
  int i1;
  int i2;
};

channel int ich;
channel long lch __attribute__((depth(3)));
channel struct st sch __attribute__((depth(0)));

channel int arr[5];
channel int multiarr[2][7] __attribute__((depth(0)));


// CHECK-DAG: @arr = common local_unnamed_addr addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4
// CHECK-DAG: @multiarr = common local_unnamed_addr addrspace(1) global [2 x [7 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4
// CHECK-DAG: @ich = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
// CHECK-DAG: @lch = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8
// CHECK-DAG: @sch = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4

// CHECK: !opencl.channels = !{![[MD1:[0-9]+]], ![[MD2:[0-9]+]], ![[MD3:[0-9]+]], ![[MD4:[0-9]+]], ![[MD5:[0-9]+]]}
// CHECK-DAG: ![[MD1]] = !{%opencl.channel_t addrspace(1)* addrspace(1)* @ich, ![[MD11:[0-9]+]], ![[MD12:[0-9]+]]}
// CHECK-DAG: ![[MD11]] = !{!"packet_size", i32 4}
// CHECK-DAG: ![[MD12]] = !{!"packet_align", i32 4}
// CHECK-DAG: ![[MD2]] = !{%opencl.channel_t addrspace(1)* addrspace(1)* @lch, ![[MD21:[0-9]+]], ![[MD22:[0-9]+]], ![[MD23:[0-9]+]]}
// CHECK-DAG: ![[MD21]] = !{!"packet_size", i32 8}
// CHECK-DAG: ![[MD22]] = !{!"packet_align", i32 8}
// CHECK-DAG: ![[MD23]] = !{!"depth", i32 3}
// CHECK-DAG: ![[MD3]] = !{%opencl.channel_t addrspace(1)* addrspace(1)* @sch, ![[MD31:[0-9]+]], ![[MD32:[0-9]+]], ![[MD33:[0-9]+]]}
// CHECK-DAG: ![[MD31]] = !{!"packet_size", i32 8}
// CHECK-DAG: ![[MD32]] = !{!"packet_align", i32 4}
// CHECK-DAG: ![[MD33]] = !{!"depth", i32 0}
// CHECK-DAG: ![[MD4]] = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @arr, ![[MD41:[0-9]+]], ![[MD42:[0-9]+]]}
// CHECK-DAG: ![[MD41]] = !{!"packet_size", i32 4}
// CHECK-DAG: ![[MD42]] = !{!"packet_align", i32 4}
// CHECK-DAG: ![[MD5]] = !{[2 x [7 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @multiarr, ![[MD51:[0-9]+]], ![[MD52:[0-9]+]], ![[MD53:[0-9]+]]}
// CHECK-DAG: ![[MD51]] = !{!"packet_size", i32 4}
// CHECK-DAG: ![[MD52]] = !{!"packet_align", i32 4}
// CHECK-DAG: ![[MD53]] = !{!"depth", i32 0}
