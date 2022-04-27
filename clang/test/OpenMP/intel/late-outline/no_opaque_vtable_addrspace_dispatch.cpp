// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics


// CHECK: [[STRUCTB:%struct.B]] = type { %struct.C, %struct.A }
// CHECK: [[STRUCTC:%struct.C]] = type { i32 (...)* addrspace(4)* }
// CHECK: [[STRUCTA:%struct.A]] = type { i32 (...)* addrspace(4)* }
// CHECK: [[ZTV1C:@_ZTV1C]] = linkonce_odr target_declare unnamed_addr addrspace(1) constant { [4 x i8 addrspace(4)*] } { [4 x i8 addrspace(4)*] [i8 addrspace(4)* null, i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)* }* @_ZTI1C to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (%struct.C addrspace(4)*, i32)* @_ZN1C3fooEi to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (%struct.C addrspace(4)*, i32 addrspace(4)*)* @_ZN1C3fooEPi to i8*) to i8 addrspace(4)*)] }
// CHECK: [[ZTV1B:@_ZTV1B]] = linkonce_odr target_declare unnamed_addr addrspace(1) constant { [5 x i8 addrspace(4)*], [6 x i8 addrspace(4)*] } { [5 x i8 addrspace(4)*] [i8 addrspace(4)* inttoptr (i64 8 to i8 addrspace(4)*), i8 addrspace(4)* null, i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)*, i32, i32, i8 addrspace(4)*, i64, i8 addrspace(4)*, i64 }* @_ZTI1B to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (%struct.B addrspace(4)*, i32)* @_ZN1B3fooEi to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (%struct.B addrspace(4)*, i32 addrspace(4)*)* @_ZN1B3fooEPi to i8*) to i8 addrspace(4)*)], [6 x i8 addrspace(4)*] [i8 addrspace(4)* inttoptr (i64 -8 to i8 addrspace(4)*), i8 addrspace(4)* inttoptr (i64 -8 to i8 addrspace(4)*), i8 addrspace(4)* inttoptr (i64 -8 to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)*, i32, i32, i8 addrspace(4)*, i64, i8 addrspace(4)*, i64 }* @_ZTI1B to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (%struct.B addrspace(4)*, i32 addrspace(4)*)* @_ZTv0_n24_N1B3fooEPi to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (%struct.B addrspace(4)*, i32)* @_ZTv0_n32_N1B3fooEi to i8*) to i8 addrspace(4)*)] }
// CHECK: [[ZTV1A:@_ZTV1A]] = linkonce_odr target_declare unnamed_addr addrspace(1) constant { [4 x i8 addrspace(4)*] } { [4 x i8 addrspace(4)*] [i8 addrspace(4)* null, i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)* }* @_ZTI1A to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (%struct.A addrspace(4)*, i32 addrspace(4)*)* @_ZN1A3fooEPi to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (%struct.A addrspace(4)*, i32)* @_ZN1A3fooEi to i8*) to i8 addrspace(4)*)] }
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
// CHECK: [[VTABLE1:%vtable1]] = load i32 (%struct.A addrspace(4)*, i32)* addrspace(4)*, i32 (%struct.A addrspace(4)*, i32)* addrspace(4)* addrspace(4)*
// CHECK: [[VFN:%vfn]] =  getelementptr inbounds i32 (%struct.A addrspace(4)*, i32)*, i32 (%struct.A addrspace(4)*, i32)* addrspace(4)* [[VTABLE1]]
// CHECK: [[L:%.*]] = load i32 (%struct.A addrspace(4)*, i32)*, i32 (%struct.A addrspace(4)*, i32)* addrspace(4)* [[VFN]]
// CHECK: call spir_func noundef i32 [[L]]
// CHECK: "DIR.OMP.END.TARGET"
// CHECK: ret void
B::B() {}
// CHECK: define{{.*}}spir_func void @_ZN1BC2Ev
// CHECK: [[THIS_ADDR:%[^)]*]] = alloca %struct.B addrspace(4)*
// CHECK: [[VTT_ADDR:%[^)]*]] = alloca i8 addrspace(4)* addrspace(4)*
// CHECK: [[THIS_ADDR_ASCAST:%[^)]*]] = addrspacecast %struct.B addrspace(4)** [[THIS_ADDR]] to %struct.B addrspace(4)* addrspace(4)*
// CHECK: [[VTT_ADDR_ASCAST:%[^)]*]] =  addrspacecast i8 addrspace(4)* addrspace(4)** [[VTT_ADDR]] to i8 addrspace(4)* addrspace(4)* addrspace(4)*
// CHECK: store %struct.B addrspace(4)* %this, %struct.B addrspace(4)* addrspace(4)* [[THIS_ADDR_ASCAST]]
// CHECK: store i8 addrspace(4)* addrspace(4)* %vtt, i8 addrspace(4)* addrspace(4)* addrspace(4)* [[VTT_ADDR_ASCAST]]
// CHECK: [[THIS1:%.*]] = load %struct.B addrspace(4)*, %struct.B addrspace(4)* addrspace(4)* [[THIS_ADDR_ASCAST]]
// CHECK: [[VTT2:%vtt2]] =  load i8 addrspace(4)* addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspace(4)* [[VTT_ADDR_ASCAST]]
// CHECK: [[L:%[^)]*]] = bitcast %struct.B addrspace(4)* [[THIS1]] to %struct.C addrspace(4)*
// CHECK: [[L1:%[^)]*]] = load i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)* [[VTT2]]
// CHECK-NEXT: [[L2:%.*]] = bitcast %struct.B addrspace(4)* [[THIS1]] to i32 (...)* addrspace(4)* addrspace(4)*
// CHECK-NEXT: [[L3:%[^)]*]] = bitcast i8 addrspace(4)* [[L1]] to i32 (...)* addrspace(4)*
// CHECK: store i32 (...)* addrspace(4)* [[L3]], i32 (...)* addrspace(4)* addrspace(4)* [[L2]], align 8
// CHECK: [[L4:%[^)]*]] = getelementptr inbounds i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)* [[VTT2]], i64 1
// CHECK: [[L5:%[^)]*]] = load i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)*
// CHECK: [[L6:%[^)]*]] = bitcast %struct.B addrspace(4)* [[THIS1]] to i8 addrspace(4)* addrspace(4)*
// CHECK: [[VTABLE:%vtable]] = load i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)*  [[L6]]
// CHECK: [[VBASE_OFFSET_PTR:%vbase.offset.ptr]] = getelementptr i8, i8 addrspace(4)* [[VTABLE]]
// CHECK: [[L7:%[^)]*]] = bitcast i8 addrspace(4)* [[VBASE_OFFSET_PTR]] to i64 addrspace(4)*
// CHECK: [[VBASE_OFFSET:%vbase.offset]] = load i64, i64 addrspace(4)* [[L7]]
// CHECK: [[L8:%[^)]*]] = bitcast %struct.B addrspace(4)* [[THIS1]] to i8 addrspace(4)*
// CHECK: [[ADD_PTR:%add.ptr]] = getelementptr inbounds i8, i8 addrspace(4)* %8, i64  [[VBASE_OFFSET]]
// CHECK: [[L9:%[^)]*]] = bitcast i8 addrspace(4)* [[ADD_PTR]] to i32 (...)* addrspace(4)* addrspace(4)*
// CHECK: [[L10:%[^)]*]] = bitcast i8 addrspace(4)* [[L5]] to i32 (...)* addrspace(4)*
// CHECK: store i32 (...)* addrspace(4)* %10, i32 (...)* addrspace(4)* addrspace(4)* [[L9]]
// CHECK: ret void
// CHECK: define{{.*}}spir_func void @_ZN1BC1Ev(%struct.B addrspace(4)* noundef [[THIS:[^)]*]])
// CHECK: [[THIS_ADDR:%[^)]*]] = alloca %struct.B addrspace(4)*
// CHECK: [[THIS_ADDR_ASCAST:%[^)]*]] = addrspacecast %struct.B addrspace(4)** [[THIS_ADDR]] to %struct.B addrspace(4)* addrspace(4)*
// CHECK: store %struct.B addrspace(4)* %this, %struct.B addrspace(4)* addrspace(4)* [[THIS_ADDR_ASCAST]]
// CHECK: [[THIS1:%.*]] = load %struct.B addrspace(4)*, %struct.B addrspace(4)* addrspace(4)* [[THIS_ADDR_ASCAST]]
// CHECK: [[L:%.*]] = bitcast %struct.B addrspace(4)* [[THIS1]] to i8 addrspace(4)*
// CHECK: [[L1:%.*]] = getelementptr inbounds i8, i8 addrspace(4)* %0, i64 8
// CHECK: [[L2:%.*]] =  bitcast i8 addrspace(4)* [[L1]] to %struct.A addrspace(4)*
// CHECK: call spir_func void @_ZN1AC2Ev
// CHECK: [[L3:%.*]] = bitcast %struct.B addrspace(4)* [[THIS1]] to %struct.C addrspace(4)*
// CHECK: call spir_func void @_ZN1CC2Ev
// CHECK: [[L4:%.*]] = bitcast %struct.B addrspace(4)* [[THIS1]]  to i32 (...)* addrspace(4)* addrspace(4)*
// CHECK:store i32 (...)* addrspace(4)* addrspacecast (i32 (...)* addrspace(1)* bitcast (i8 addrspace(4)* addrspace(1)* getelementptr inbounds ({ [5 x i8 addrspace(4)*], [6 x i8 addrspace(4)*] }, { [5 x i8 addrspace(4)*], [6 x i8 addrspace(4)*] } addrspace(1)* [[ZTV1B]], i32 0, inrange i32 0, i32 3) to i32 (...)* addrspace(1)*) to i32 (...)* addrspace(4)*), i32 (...)* addrspace(4)* addrspace(4)* [[L4]]
// CHECK: [[L5:%.*]] = bitcast %struct.B addrspace(4)* [[THIS1]] to i8 addrspace(4)*
// CHECK: [[ADD_PTR:%.*]] = getelementptr inbounds i8, i8 addrspace(4)* [[L5]]
// CHECK: [[L6:%.*]] = bitcast i8 addrspace(4)* [[ADD_PTR]] to i32 (...)* addrspace(4)* addrspace(4)*
// CHECK: store i32 (...)* addrspace(4)* addrspacecast (i32 (...)* addrspace(1)* bitcast (i8 addrspace(4)* addrspace(1)* getelementptr inbounds ({ [5 x i8 addrspace(4)*], [6 x i8 addrspace(4)*] }, { [5 x i8 addrspace(4)*], [6 x i8 addrspace(4)*] } addrspace(1)* [[ZTV1B]], i32 0, inrange i32 1, i32 4) to i32 (...)* addrspace(1)*) to i32 (...)* addrspace(4)*), i32 (...)* addrspace(4)* addrspace(4)* [[L6]],
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
