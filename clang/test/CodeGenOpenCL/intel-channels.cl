// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -emit-llvm -opaque-pointers %s -o - | FileCheck %s

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

channel int ich1 __attribute__((io("myIoChannel")));
channel int ich2 __attribute__((io("testChannel")));

// CHECK-DAG: @arr = {{.*}}local_unnamed_addr addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 4
// CHECK-DAG: @multiarr = {{.*}}local_unnamed_addr addrspace(1) global [2 x [7 x ptr addrspace(1)]] zeroinitializer, align 4, !packet_size ![[MD0:[0-9]+]], !packet_align ![[MD0]], !depth ![[MD3:[0-9]+]]
// CHECK-DAG: @ich = {{.*}}local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 4
// CHECK-DAG: @lch = {{.*}}local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 8, !packet_size ![[MD1:[0-9]+]], !packet_align ![[MD1]], !depth ![[MD2:[0-9]+]]
// CHECK-DAG: @sch = {{.*}}local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 4, !packet_size ![[MD1]], !packet_align ![[MD0]], !depth ![[MD3]]
// CHECK-DAG: @ich1 = {{.*}}local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 4, !packet_size ![[MD0]], !packet_align ![[MD0]], !io ![[MD4:[0-9]+]]
// CHECK-DAG: @ich2 = {{.*}}local_unnamed_addr addrspace(1) global ptr addrspace(1) null, align 4, !packet_size ![[MD0]], !packet_align ![[MD0]], !io ![[MD5:[0-9]+]]

// CHECK-DAG: ![[MD0]] = !{i32 4}
// CHECK-DAG: ![[MD1]] = !{i32 8}
// CHECK-DAG: ![[MD2]] = !{i32 3}
// CHECK-DAG: ![[MD3]] = !{i32 0}
// CHECK-DAG: ![[MD4]] = !{!"myIoChannel"}
// CHECK-DAG: ![[MD5]] = !{!"testChannel"}
