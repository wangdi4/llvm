// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//

// CHECK-LABEL: foo_one
void foo_one()
{
  int a[10];
  int share = 0;

// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(i32* %share
#pragma omp target teams distribute  shared(share) defaultmap(tofrom:scalar)
  for (int i = 0 ; i < 10; i++)
    share = share + a[i];
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
}

#pragma omp declare target
static int x[10];
#pragma omp end declare target

// CHECK-LABEL: foo_two
void foo_two()
{
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"([10 x i32]* @x
  #pragma omp target
    {
      x[1]=1;;
    }
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET"() ]
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
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(i32* @x1
  #pragma omp target
    x1 = 1;
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.TARGET"() ]
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* @a
  #pragma omp target
    a = 10;
// CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
}

// CHECK-LABEL: foo_four
void foo_four()
{
  int y[10];
// CHECK: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// "QUAL.OMP.MAP.TOFROM"([10 x i32]* %y{{.*}}, [10 x i32]* %y{{.*}}, i64 40, i64 547)
// CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"(i32* %y
  #pragma omp target
    y[1] = 1;
// CHECK: region.exit(token [[TV4]]) [ "DIR.OMP.END.TARGET"() ]
}
