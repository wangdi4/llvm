// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
class A
{
public:
  A();
  ~A();
  int a;
};


// CHECK-LABEL: foo
void foo(A *& f)
{
// CHECK: [[F_ADDR:%f.addr]] = alloca %class.A**,
// CHECK: [[MAP_ADDR:%f.map.ptr.tmp]] = alloca %class.A**,
// CHECK: [[MAP_ADDR2:%f.map.ptr.tmp2]] = alloca %class.A**,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(%class.A*** [[MAP_ADDR]])
// CHECK: store {{.*}}, %class.A*** [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME: "QUAL.OMP.SHARED"(%class.A*** [[MAP_ADDR]])
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED"(%class.A*** [[MAP_ADDR]])
// CHECK: load %class.A**, %class.A*** [[MAP_ADDR]]
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target teams distribute parallel for
  for (int i = 0 ; i < 10 ; i ++)
    f->a = 10;
// CHECK: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(%class.A*** [[MAP_ADDR2]]
// CHECK: store {{.*}}, %class.A*** [[MAP_ADDR2]],
// CHECK: load %class.A**, %class.A*** [[MAP_ADDR2]]
// CHECK: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target
  for (int i = 0 ; i < 10 ; i ++)
    f->a = 10;
// CHECK: [[L:%[0-9]+]] = load %class.A**, %class.A*** [[F_ADDR]]
// CHECK: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.PARALLEL
// CHECK-SAME:"QUAL.OMP.SHARED"(%class.A** [[L]])
#pragma omp parallel
  for (int i = 0 ; i < 10 ; i ++)
    f->a = 10;
}

// CHECK-LABEL: foo_one
void foo_one(A & f)
{
// CHECK: [[F_ADDR:%f.addr]] = alloca %class.A*,
// CHECK: [[MAP_ADDR:%f.map.ptr.tmp]] = alloca %class.A*,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(%class.A** [[MAP_ADDR]])
// CHECK: store {{.*}}, %class.A** [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME: "QUAL.OMP.SHARED"(%class.A** [[MAP_ADDR]])
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED"(%class.A** [[MAP_ADDR]])
// CHECK: load %class.A*, %class.A** [[MAP_ADDR]]
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target teams distribute parallel for
  for (int i = 0 ; i < 10 ; i ++)
    f.a = 10;
}
// CHECK-LABEL: foo_two
void foo_two(A && f)
{
// CHECK: [[F_ADDR:%f.addr]] = alloca %class.A*,
// CHECK: [[MAP_ADDR:%f.map.ptr.tmp]] = alloca %class.A*,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(%class.A** [[MAP_ADDR]])
// CHECK: store {{.*}}, %class.A** [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME: "QUAL.OMP.SHARED"(%class.A** [[MAP_ADDR]])
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED"(%class.A** [[MAP_ADDR]])
// CHECK: load %class.A*, %class.A** [[MAP_ADDR]]
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target teams distribute parallel for
  for (int i = 0 ; i < 10 ; i ++)
    f.a = 10;
}
// CHECK-LABEL: foo_three
void foo_three(A * f)
{
// CHECK: [[F_ADDR:%f.addr]] = alloca %class.A*,
// CHECK: [[MAP_ADDR:%f.map.ptr.tmp]] = alloca %class.A*,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(%class.A** [[MAP_ADDR]])
// CHECK: store {{.*}}, %class.A** [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME: "QUAL.OMP.SHARED"(%class.A** [[MAP_ADDR]])
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED"(%class.A** [[MAP_ADDR]])
// CHECK: load %class.A*, %class.A** [[MAP_ADDR]]
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target teams distribute parallel for
  for (int i = 0 ; i < 10 ; i ++)
    f->a = 10;
}

// CHECK-LABEL: foo_four
void foo_four()
{
  A *b;
// CHECK: [[B_ADDR:%b]] = alloca %class.A*,
// CHECK: [[MAP_ADDR:%b.map.ptr.tmp]] = alloca %class.A*,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(%class.A** [[MAP_ADDR]])
// CHECK: store {{.*}}, %class.A** [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME: "QUAL.OMP.SHARED"(%class.A** [[MAP_ADDR]])
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED"(%class.A** [[MAP_ADDR]])
// CHECK: load %class.A*, %class.A** [[MAP_ADDR]]
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target teams distribute parallel for
  for (int i = 0 ; i < 10 ; i ++)
    b->a = 10;
}

// CHECK-LABEL: foo_five
void foo_five(int &y)
{
  int *yp = &y;
  volatile int size = 1;
// CHECK: [[Y_ADDR:%y.addr]] = alloca i32*,
// CHECK-NEXT: [[YP_ADDR:%yp]] = alloca i32*, align 8
// CHECK-NEXT: [[SI:%size]] = alloca i32, align 4
// CHECK-NEXT: [[Y_MAP:%y.map.ptr.tmp]] = alloca i32*, align 8
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TASK
// CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[Y_MAP]])
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[Y_MAP]]
// CHECK: store {{.*}}, i32** [[Y_MAP]]
// CHECK: load {{.*}}, i32** [[Y_MAP]]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TASK"() ]
  #pragma omp target map(tofrom:yp[0:size])  nowait depend(out:y)
   y = 3;
}
