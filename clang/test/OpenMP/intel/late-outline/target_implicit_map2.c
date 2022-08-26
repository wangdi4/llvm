// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp \
// RUN:  -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp \
// RUN:  -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fopenmp-declare-target-scalar-defaultmap-firstprivate \
// RUN:  -fopenmp-declare-target-global-default-map \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s \
// RUN:  --check-prefix=FPRIVATE

//CHECK-LABEL: foo_local_data
void foo_local_data()
{
  int local_array[10];
  int local_scalar = 0;

  // With defaultmap

  //CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %local_scalar
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %local_array
  #pragma omp target teams distribute shared(local_scalar) \
                                      defaultmap(tofrom:scalar)
  for (int i = 0 ; i < 10; i++)
    local_scalar = local_scalar + local_array[i];
  //CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]

  // Without defaultmap

  //CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %local_array
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %local_scalar
  #pragma omp target teams distribute shared(local_scalar)
  for (int i = 0 ; i < 10; i++)
    local_scalar = local_scalar + local_array[i];
  //CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
}

#pragma omp declare target
static int dt_array[10];
int dt_scalar;
#pragma omp end declare target
int glob_array[10];
int glob_scalar;


//CHECK-LABEL: foo_global_data
void foo_global_data()
{
  // With defaultmap

  //CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @dt_scalar
  //FPRIVATE: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @dt_scalar
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @dt_array
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @glob_scalar
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @glob_array
  #pragma omp target teams distribute defaultmap(tofrom:scalar)
  for (int i = 0 ; i < 10; i++) {
    dt_scalar = dt_scalar + dt_array[i];
    glob_scalar = glob_scalar + glob_array[i];
  }
  //CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]

  // Without defaultmap

  //CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @glob_array
  //CHECK-SAME: "QUAL.OMP.LIVEIN:TYPED"(ptr @dt_array
  //CHECK-SAME: "QUAL.OMP.LIVEIN:TYPED"(ptr @dt_scalar
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @glob_scalar
  #pragma omp target teams distribute
  for (int i = 0 ; i < 10; i++) {
    dt_scalar = dt_scalar + dt_array[i];
    glob_scalar = glob_scalar + glob_array[i];
  }
  //CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
}

void foo() {
  //CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  //FPRIVATE: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.LIVEIN:TYPED"(ptr @dt_scalar
  //FPRIVATE-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @dt_scalar
  #pragma omp target
    dt_scalar++;
  //CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  //FPRIVATE: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  //CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  //FPRIVATE: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @dt_scalar
  //FPRIVATE-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @dt_scalar
  #pragma omp target defaultmap(tofrom:scalar)
    dt_scalar++;
  //CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  //FPRIVATE: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  //CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  //FPRIVATE: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @dt_scalar
  //FPRIVATE-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @dt_scalar
  #pragma omp target map(tofrom:dt_scalar)
    dt_scalar++;
  //CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  //FPRIVATE: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
}
// end INTEL_COLLAB
