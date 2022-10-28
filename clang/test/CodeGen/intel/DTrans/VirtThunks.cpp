// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-pc -fexceptions -fcxx-exceptions -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-pc -fexceptions -fcxx-exceptions -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

// Ensure that metadata is correctly attached to the 'call' in a virtual thunk.

struct Struct {
  virtual void func(int, double, float) = 0;
};

void use() {
  (void)&Struct::func;
}

// PTR: define linkonce_odr void @"??_9Struct@@$BA@AA"(%"struct..?AUStruct@@.Struct"* noundef %this, ...)
// OPQ: define linkonce_odr void @"??_9Struct@@$BA@AA"(ptr noundef %this, ...)
// PTR: alloca %"struct..?AUStruct@@.Struct"*, align 8, !intel_dtrans_type ![[STRUCT_PTR:[0-9]+]]
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[STRUCT_PTR:[0-9]+]]
// PTR: musttail call void (%"struct..?AUStruct@@.Struct"*, ...) %{{[0-9]+}}(%"struct..?AUStruct@@.Struct"* noundef %{{.*}}, ...), !intel_dtrans_type ![[MUSTTAIL_CALL:[0-9]+]]
// OPQ: musttail call void (ptr, ...) %{{[0-9]+}}(ptr noundef %{{.*}}, ...), !intel_dtrans_type ![[MUSTTAIL_CALL:[0-9]+]]

// CHECK: ![[STRUCT_PTR]] = !{%"struct..?AUStruct@@.Struct" zeroinitializer, i32 1} 
// CHECK: ![[MUSTTAIL_CALL]] = !{!"F", i1 true, i32 1, ![[VOID:[0-9]+]], ![[STRUCT_PTR]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
