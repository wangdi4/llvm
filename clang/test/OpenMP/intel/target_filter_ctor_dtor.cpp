// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t1-host.bc
//
// RUN: %clang_cc1 -verify -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t1-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// RUN: %clang_cc1 -verify -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t2-host.bc
//
// RUN: %clang_cc1 -verify -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t2-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s -check-prefix=ALIAS
//
// expected-no-diagnostics

// Verifies that constructors/destructors are handled

//CHECK-DAG: define{{.*}}called_from_ctor_target_region
//ALIAS-DAG: define{{.*}}called_from_ctor_target_region
void called_from_ctor_target_region() {}
//CHECK-DAG: define{{.*}}called_from_dtor_target_region
//ALIAS-DAG: define{{.*}}called_from_dtor_target_region
void called_from_dtor_target_region() {}

// This contructor is not referenced from any use on the target but it
// shouldn't cause a compiler crash whether it is emitted on not.
struct InlineConstructorNotUsedOnTarget {
  InlineConstructorNotUsedOnTarget() {
    #pragma omp target
    {}
  }
};

// This destructor is not referenced from any use on the target but it
// shouldn't cause a compiler crash whether it is emitted on not.
struct InlineDestructorNotUsedOnTarget {
  ~InlineDestructorNotUsedOnTarget() {
    #pragma omp target
    {}
  }
};

// Function not called from a target region
void foo() {
  InlineConstructorNotUsedOnTarget A;
  InlineDestructorNotUsedOnTarget D;
}

// This one has a constructor defined here with a target region. It should
// be defined on the target, along with anything called in the region.
struct ObjectWithOutOfLineCtor {
  ObjectWithOutOfLineCtor();
};

//CHECK-DAG: define{{.*}}ObjectWithOutOfLineCtorC1
//CHECK-DAG: define{{.*}}ObjectWithOutOfLineCtorC2
//ALIAS-DAG: ObjectWithOutOfLineCtorC1{{.*}}alias{{.*}}ObjectWithOutOfLineCtorC2
//ALIAS-DAG: define{{.*}}ObjectWithOutOfLineCtorC2
ObjectWithOutOfLineCtor::ObjectWithOutOfLineCtor() {
  #pragma omp target
  {
    called_from_ctor_target_region();
  }
}

// This one has a destructor defined here with a target region. It should
// be defined on the target, along with anything called in the region.
struct ObjectWithOutOfLineDtor {
  ~ObjectWithOutOfLineDtor();
};

//CHECK-DAG: define{{.*}}ObjectWithOutOfLineDtorD1
//CHECK-DAG: define{{.*}}ObjectWithOutOfLineDtorD2
//ALIAS-DAG: ObjectWithOutOfLineDtorD1{{.*}}alias{{.*}}ObjectWithOutOfLineDtorD2
//ALIAS-DAG: define{{.*}}ObjectWithOutOfLineDtorD2
ObjectWithOutOfLineDtor::~ObjectWithOutOfLineDtor() {
  #pragma omp target
  {
    called_from_dtor_target_region();
  }
}

// This contructor is referenced from a target region.
struct InlineConstructorUsedOnTarget {
  InlineConstructorUsedOnTarget() {
    #pragma omp target
    {}
  }
};
//CHECK-DAG: define{{.*}}InlineConstructorUsedOnTargetC
//ALIAS-DAG: define{{.*}}InlineConstructorUsedOnTargetC


// This destructor is referenced from a target region.
struct InlineDestructorUsedOnTarget {
  ~InlineDestructorUsedOnTarget() {
    #pragma omp target
    {}
  }
};
//CHECK-DAG: define{{.*}}InlineDestructorUsedOnTargetD
//ALIAS-DAG: define{{.*}}InlineDestructorUsedOnTargetD

//CHECK-DAG: define{{.*}}target_with_ctor_object
//ALIAS-DAG: define{{.*}}target_with_ctor_object
void target_with_ctor_object() {
  #pragma omp target
  {
    InlineConstructorUsedOnTarget C;
  }
}

//CHECK-DAG: define{{.*}}target_with_dtor_object
//ALIAS-DAG: define{{.*}}target_with_dtor_object
void target_with_dtor_object() {
  #pragma omp target
  {
    InlineDestructorUsedOnTarget F;
  }
}
