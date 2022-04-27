// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

// CHECK: [[ZTV1C:@_ZTV1C]] = linkonce_odr target_declare unnamed_addr addrspace(1) constant { [4 x ptr addrspace(4)] }
// CHECK-SAME: { [4 x ptr addrspace(4)]
// CHECK-SAME:   [ptr addrspace(4) null,
// CHECK-SAME:    ptr addrspace(4) addrspacecast (ptr @_ZTI1C to ptr addrspace(4)),
// CHECK-SAME:    ptr addrspace(4) addrspacecast (ptr @_ZN1C3fooEi to ptr addrspace(4)),
// CHECK_SAME:    ptr addrspace(4) addrspacecast (ptr @_ZN1C3fooEPi to ptr addrspace(4))]
// CHECK-SAME: }

// CHECK: [[ZTV1B:@_ZTV1B]] = linkonce_odr target_declare unnamed_addr addrspace(1) constant { [5 x ptr addrspace(4)], [6 x ptr addrspace(4)] }
// CHECK-SAME:  { [5 x ptr addrspace(4)]
// CHECK-SAME:    [ptr addrspace(4) inttoptr (i64 8 to ptr addrspace(4)),
// CHECK-SAME:     ptr addrspace(4) null,
// CHECK-SAME:     ptr addrspace(4) addrspacecast (ptr @_ZTI1B to ptr addrspace(4)),
// CHECK-SAME:     ptr addrspace(4) addrspacecast (ptr @_ZN1B3fooEi to ptr addrspace(4)),
// CHECK-SAME:     ptr addrspace(4) addrspacecast (ptr @_ZN1B3fooEPi to ptr addrspace(4))],
// CHECK-SAME:    [6 x ptr addrspace(4)]
// CHECK-SAME:    [ptr addrspace(4) inttoptr (i64 -8 to ptr addrspace(4)),
// CHECK-SAME:     ptr addrspace(4) inttoptr (i64 -8 to ptr addrspace(4)),
// CHECK-SAME:     ptr addrspace(4) inttoptr (i64 -8 to ptr addrspace(4)),
// CHECK-SAME:     ptr addrspace(4) addrspacecast (ptr @_ZTI1B to ptr addrspace(4)),
// CHECK-SAME:     ptr addrspace(4) addrspacecast (ptr @_ZTv0_n24_N1B3fooEPi to ptr addrspace(4)),
// CHECK-SAME:     ptr addrspace(4) addrspacecast (ptr @_ZTv0_n32_N1B3fooEi to ptr addrspace(4))]
// CHECK-SAME: }
//
// CHECK: [[ZTV1A:@_ZTV1A]] = linkonce_odr target_declare unnamed_addr addrspace(1) constant { [4 x ptr addrspace(4)] }
// CHECK-SAME: { [4 x ptr addrspace(4)]
// CHECK-SAME:   [ptr addrspace(4) null,
// CHECK-SAME:    ptr addrspace(4) addrspacecast (ptr @_ZTI1A to ptr addrspace(4)),
// CHECK-SAME:    ptr addrspace(4) addrspacecast (ptr @_ZN1A3fooEPi to ptr addrspace(4)),
// CHECK-SAME:    ptr addrspace(4) addrspacecast (ptr @_ZN1A3fooEi to ptr addrspace(4))]
// CHECK-SAME:  }
struct A
{
  #pragma omp declare target
  virtual int foo(int *){return 0;};
  virtual int foo(int){return 0;};
  #pragma omp end declare target
};
struct C
{
  #pragma omp declare target
  virtual int foo(int){ return 1;};
  virtual int foo(int* a) {return 1;}
  #pragma omp end declare target
};

struct B : virtual A , public C
{
  #pragma omp declare target
  B();
  virtual int foo(int* a) {return 2;}
  virtual int foo(int a) {return 2;}
  #pragma omp end declare target
};

// CHECK: define{{.*}}spir_func void @_Z4testv()
// CHECK: "DIR.OMP.TARGET"
// CHECK: [[VTABLE1:%vtable1]] = load ptr addrspace(4), ptr addrspace(4)
// CHECK: [[VFN:%vfn]] =  getelementptr inbounds ptr, ptr addrspace(4) [[VTABLE1]]
// CHECK: [[L:%.*]] = load ptr, ptr addrspace(4) [[VFN]]
// CHECK: call spir_func noundef i32 [[L]]
// CHECK: "DIR.OMP.END.TARGET"
// CHECK: ret void
B::B() {}
// CHECK: define{{.*}}spir_func void @_ZN1BC2Ev
// CHECK: [[THIS_ADDR:%[^)]*]] = alloca ptr addrspace(4)
// CHECK: [[VTT_ADDR:%[^)]*]] = alloca ptr addrspace(4)
// CHECK: [[THIS_ADDR_ASCAST:%[^)]*]] = addrspacecast ptr [[THIS_ADDR]] to ptr addrspace(4)
// CHECK: [[VTT_ADDR_ASCAST:%[^)]*]] = addrspacecast ptr [[VTT_ADDR]] to ptr addrspace(4)
// CHECK: store ptr addrspace(4) %this, ptr addrspace(4) [[THIS_ADDR_ASCAST]]
// CHECK: store ptr addrspace(4) %vtt, ptr addrspace(4) [[VTT_ADDR_ASCAST]]
// CHECK: [[THIS1:%.*]] = load ptr addrspace(4), ptr addrspace(4) [[THIS_ADDR_ASCAST]]
// CHECK: [[VTT2:%vtt2]] = load ptr addrspace(4), ptr addrspace(4) [[VTT_ADDR_ASCAST]]
// CHECK: [[L1:%[^)]*]] = load ptr addrspace(4), ptr addrspace(4) [[VTT2]]
// CHECK: store ptr addrspace(4) [[L1]], ptr addrspace(4) [[THIS1]], align 8
// CHECK: [[L4:%[^)]*]] = getelementptr inbounds ptr addrspace(4), ptr addrspace(4) [[VTT2]], i64 1
// CHECK: [[L5:%[^)]*]] = load ptr addrspace(4), ptr addrspace(4) [[L4]]
// CHECK: [[VTABLE:%vtable]] = load ptr addrspace(4), ptr addrspace(4) [[THIS1]]
// CHECK: [[VBASE_OFFSET_PTR:%vbase.offset.ptr]] = getelementptr i8, ptr addrspace(4) [[VTABLE]]
// CHECK: [[VBASE_OFFSET:%vbase.offset]] = load i64, ptr addrspace(4) [[VBASE_OFFSET_PTR]]
// CHECK: [[ADD_PTR:%add.ptr]] = getelementptr inbounds i8, ptr addrspace(4) [[THIS1]], i64 [[VBASE_OFFSET]]
// CHECK: store ptr addrspace(4) [[L5]], ptr addrspace(4) [[ADD_PTR]]
// CHECK: ret void

// CHECK: define{{.*}}spir_func void @_ZN1BC1Ev(ptr addrspace(4) noundef [[THIS:[^)]*]])
// CHECK: [[THIS_ADDR:%[^)]*]] = alloca ptr addrspace(4)
// CHECK: [[THIS_ADDR_ASCAST:%[^)]*]] = addrspacecast ptr [[THIS_ADDR]] to ptr addrspace(4)
// CHECK: store ptr addrspace(4) %this, ptr addrspace(4) [[THIS_ADDR_ASCAST]]
// CHECK: [[THIS1:%.*]] = load ptr addrspace(4), ptr addrspace(4) [[THIS_ADDR_ASCAST]]
// CHECK: call spir_func void @_ZN1AC2Ev
// CHECK: call spir_func void @_ZN1CC2Ev
// CHECK:store ptr addrspace(4) addrspacecast (ptr addrspace(1) getelementptr inbounds ({ [5 x ptr addrspace(4)], [6 x ptr addrspace(4)] }, ptr addrspace(1) [[ZTV1B]], i32 0, inrange i32 0, i32 3) to ptr addrspace(4)), ptr addrspace(4) [[THIS1]]
// CHECK: [[ADD_PTR:%.*]] = getelementptr inbounds i8, ptr addrspace(4) [[THIS1]]
// CHECK: store ptr addrspace(4) addrspacecast (ptr addrspace(1) getelementptr inbounds ({ [5 x ptr addrspace(4)], [6 x ptr addrspace(4)] }, ptr addrspace(1) [[ZTV1B]], i32 0, inrange i32 1, i32 4) to ptr addrspace(4)), ptr addrspace(4) [[ADD_PTR]],
// CHECK: ret void

void test() {
  B *obj = new B();
  A *x =  obj;
  int i = 1;
  #pragma omp target
  x->foo(i);
}
// CHECK: define{{.*}}spir_func noundef i32 @_ZN1B3fooEi
// CHECK: define{{.*}}spir_func noundef i32 @_ZN1B3fooEPi
// CHECK: define{{.*}}spir_func noundef i32 @_ZTv0_n24_N1B3fooEPi
// CHECK: define{{.*}}spir_func noundef i32 @_ZTv0_n32_N1B3fooEi
// CHECK: define{{.*}}spir_func noundef i32 @_ZN1A3fooEPi
// CHECK: define{{.*}}spir_func noundef i32 @_ZN1A3fooEi
// INTEL_COLLAB
