// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp \
// RUN:  -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fopenmp-declare-target-global-default-map \
// RUN:  -triple x86_64-unknown-linux-gnu -x c++ %s | FileCheck %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp \
// RUN:  -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fopenmp-declare-target-scalar-defaultmap-firstprivate \
// RUN:  -fopenmp-declare-target-global-default-map \
// RUN:  -triple x86_64-unknown-linux-gnu -x c++ %s | FileCheck %s \
// RUN:  --check-prefix=FPRIVATE

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp \
// RUN:  -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu -x c++ %s | FileCheck %s \
// RUN:  --check-prefix=LIVEIN

// CHECK-LABEL: foo_one
void foo_one()
{
  int a[10];
  int share = 0;

// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// FPRIVATE: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// LIVEIN: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %share
// FPRIVATE-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %share
// LIVEIN-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %share
#pragma omp target teams distribute  shared(share) defaultmap(tofrom:scalar)
  for (int i = 0 ; i < 10; i++)
    share = share + a[i];
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
// FPRIVATE: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
// LIVEIN: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
}

#pragma omp declare target
static int x[10];
#pragma omp end declare target

// CHECK-LABEL: foo_two
void foo_two()
{
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// FPRIVATE: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// LIVEIN: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @{{.*}}x
// FPRIVATE-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @{{.*}}x
// LIVEIN-SAME: "QUAL.OMP.LIVEIN:TYPED"(ptr @{{.*}}x
  #pragma omp target
    {
      x[1]=1;;
    }
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET"() ]
// FPRIVATE: region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET"() ]
}


#pragma omp declare target
static int x1;
#pragma omp end declare target
int a;
#pragma omp declare target to(a)device_type(nohost)
// CHECK-LABEL: foo_three
void foo_three()
{
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// FPRIVATE: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// LIVEIN: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @{{.*}}x1
// FPRIVATE-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @{{.*}}x1
// LIVEIN-SAME: "QUAL.OMP.LIVEIN:TYPED"(ptr @{{.*}}x1
  #pragma omp target
    x1 = 1;
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.TARGET"() ]
// FPRIVATE: region.exit(token [[TV3]]) [ "DIR.OMP.END.TARGET"() ]
// LIVEIN: region.exit(token [[TV3]]) [ "DIR.OMP.END.TARGET"() ]
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// FPRIVATE: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// LIVEIN: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @a
// FPRIVATE-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @a
// LIVEIN: "QUAL.OMP.LIVEIN:TYPED"(ptr @a
  #pragma omp target
    a = 10;
// CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
// FPRIVATE: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
// LIVEIN: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
}

// CHECK-LABEL: foo_four
void foo_four()
{
  int y[10];
// CHECK: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// FPRIVATE: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// LIVEIN: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK:"QUAL.OMP.MAP.TOFROM"(ptr %y{{.*}}, ptr %y{{.*}}, i64 40, i64 547
// FPRIVATE:"QUAL.OMP.MAP.TOFROM"(ptr %y{{.*}}, ptr %y{{.*}}, i64 40, i64 547
// LIVEIN:"QUAL.OMP.MAP.TOFROM"(ptr %y{{.*}}, ptr %y{{.*}}, i64 40, i64 547
// CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y
// FPRIVATE-NOT: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y
// LIVEIN-NOT: "QUAL.OMP.LIVEIN:TYPED"(ptr %y
  #pragma omp target
    y[1] = 1;
// CHECK: region.exit(token [[TV4]]) [ "DIR.OMP.END.TARGET"() ]
// FPRIVATE: region.exit(token [[TV4]]) [ "DIR.OMP.END.TARGET"() ]
// LIVEIN: region.exit(token [[TV4]]) [ "DIR.OMP.END.TARGET"() ]
}

class SharedArray {
public:
  double data;
};

#pragma omp declare target
SharedArray data_a;
#pragma omp end declare target
// CHECK-LABEL: foo_five
void foo_five() {
// CHECK: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// FPRIVATE: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// LIVEIN: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @data_a
// FPRIVATE-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @data_a
// LIVEIN-SAME: "QUAL.OMP.LIVEIN:TYPED"(ptr @data_a
  #pragma omp target
  data_a.data = 1.23;
// CHECK: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
// FPRIVATE: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
// LIVEIN: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
}

// end INTEL_COLLAB
