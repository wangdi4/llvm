// INTEL_COLLAB

// RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc -o %t-host.bc %s
//
// RUN: %clang_cc1 -no-opaque-pointers -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc -emit-llvm -o - %s | \
// RUN:  FileCheck %s

//CHECK: [[DT:%struct.Default]] = type { i32 }
struct Default {
  int i;
};

//CHECK: [[CT:%struct.CopyCtor]] = type { i32 }
struct CopyCtor {
  CopyCtor();
  CopyCtor(const CopyCtor&);
  int i;
};

#pragma omp declare target
//CHECK: define {{.*}}_Z3Foo7Default([[DT]] [[AS:addrspace\(4\)]]* noundef byval([[DT]])
int Foo(Default D) {
  return D.i;
}

//CHECK: define {{.*}}_Z3Foo8CopyCtor([[CT]] [[AS]]* noundef %C)
int Foo(CopyCtor C) {
  return C.i;
}

//CHECK: define {{.*}}_Z6Callerv()
int Caller() {
  //CHECK: [[D:%d]] = alloca [[DT]], align 4
  //CHECK: [[C:%c]] = alloca [[CT]], align 4
  //CHECK: [[ATD:%agg.tmp.*]] = alloca [[DT]], align 4
  //CHECK: [[ATC:%agg.tmp.*]] = alloca [[CT]], align 4
  //CHECK: [[DA:%d.ascast]] = addrspacecast [[DT]]* [[D]] to [[DT]] [[AS]]*
  Default d = {42};
  //CHECK: [[CA:%c.ascast]] = addrspacecast [[CT]]* [[C]] to [[CT]] [[AS]]*
  CopyCtor c;
  //CHECK-NEXT: [[ATDA:%agg.tmp.ascast.*]] =
  //CHECK-SAME: addrspacecast [[DT]]* [[ATD]] to [[DT]] [[AS]]*
  //CHECK-NEXT: [[ATCA:%agg.tmp.*.ascast]] =
  //CHECK-SAME: addrspacecast [[CT]]* [[ATC]] to [[CT]] [[AS]]*

  // Default copy construct the agg.tmp with a memcpy.
  //CHECK: [[DST:%[0-9]+]] = bitcast [[DT]] [[AS]]* [[ATDA]] to i8 [[AS]]*
  //CHECK: [[SRC:%[0-9]+]] = bitcast [[DT]] [[AS]]* [[DA]] to i8 [[AS]]*
  //CHECK: call void @llvm.memcpy{{.*}}(i8 [[AS]]* align 4 [[DST]],
  //CHECK-SAME: i8 [[AS]]* align 4 [[SRC]], i64 4, i1 false)

  // Call passing agg.tmp byval
  //CHECK: call {{.*}}_Z3Foo7Default([[DT]] [[AS]]* noundef byval([[DT]]){{.*}}[[ATDA]])

  // Copy construct the agg.tmp calling copy constructor.
  //CHECK: call {{.*}}_ZN8CopyCtorC1ERKS_([[CT]] [[AS]]* {{[^,]*}} [[ATCA]],
  //CHECK-SAME: [[CT]] [[AS]]* noundef align 4 dereferenceable(4) [[CA]])

  //Call passing address of agg.tmp
  //CHECK: call {{.*}}_Z3Foo8CopyCtor([[CT]] [[AS]]*  noundef [[ATCA]])

  return Foo(d) + Foo(c);
}

#pragma omp end declare target

// end INTEL_COLLAB
