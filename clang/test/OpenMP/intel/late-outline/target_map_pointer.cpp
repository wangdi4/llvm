// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -fopenmp-new-depend-ir %s \
// RUN:  | FileCheck %s
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
// CHECK: [[F_ADDR:%f.addr]] = alloca ptr,
// CHECK: [[MAP_ADDR:%f.map.ptr.tmp]] = alloca ptr,
// CHECK: [[MAP_ADDR2:%f.map.ptr.tmp2]] = alloca ptr,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[MAP_ADDR]])
// CHECK: store {{.*}}, ptr [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME: "QUAL.OMP.SHARED:BYREF"(ptr [[MAP_ADDR]])
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED:BYREF"(ptr [[MAP_ADDR]])
// CHECK: load ptr, ptr [[MAP_ADDR]]
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target teams distribute parallel for
  for (int i = 0 ; i < 10 ; i ++)
    f->a = 10;
// CHECK: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[MAP_ADDR2]]
// CHECK: store {{.*}}, ptr [[MAP_ADDR2]],
// CHECK: load ptr, ptr [[MAP_ADDR2]]
// CHECK: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target
  for (int i = 0 ; i < 10 ; i ++)
    f->a = 10;
// CHECK: [[L:%[0-9]+]] = load ptr, ptr [[F_ADDR]]
// CHECK: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.PARALLEL
// CHECK-SAME:"QUAL.OMP.SHARED:BYREF"(ptr [[F_ADDR]])
#pragma omp parallel
  for (int i = 0 ; i < 10 ; i ++)
    f->a = 10;
}

// CHECK-LABEL: foo_one
void foo_one(A & f)
{
// CHECK: [[F_ADDR:%f.addr]] = alloca ptr,
// CHECK: [[MAP_ADDR:%f.map.ptr.tmp]] = alloca ptr,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[MAP_ADDR]])
// CHECK: store {{.*}}, ptr [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME: "QUAL.OMP.SHARED:BYREF"(ptr [[MAP_ADDR]])
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED:BYREF"(ptr [[MAP_ADDR]])
// CHECK: load ptr, ptr [[MAP_ADDR]]
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
// CHECK: [[F_ADDR:%f.addr]] = alloca ptr,
// CHECK: [[MAP_ADDR:%f.map.ptr.tmp]] = alloca ptr,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[MAP_ADDR]])
// CHECK: store {{.*}}, ptr [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME: "QUAL.OMP.SHARED:BYREF"(ptr [[MAP_ADDR]])
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED:BYREF"(ptr [[MAP_ADDR]])
// CHECK: load ptr, ptr [[MAP_ADDR]]
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
// CHECK: [[F_ADDR:%f.addr]] = alloca ptr,
// CHECK: [[MAP_ADDR:%f.map.ptr.tmp]] = alloca ptr,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[MAP_ADDR]])
// CHECK: store {{.*}}, ptr [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME: "QUAL.OMP.SHARED"(ptr [[MAP_ADDR]])
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED"(ptr [[MAP_ADDR]])
// CHECK: load ptr, ptr [[MAP_ADDR]]
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
// CHECK: [[B_ADDR:%b]] = alloca ptr,
// CHECK: [[MAP_ADDR:%b.map.ptr.tmp]] = alloca ptr,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[MAP_ADDR]])
// CHECK: store {{.*}}, ptr [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME: "QUAL.OMP.SHARED"(ptr [[MAP_ADDR]])
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED"(ptr [[MAP_ADDR]])
// CHECK: load ptr, ptr [[MAP_ADDR]]
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target teams distribute parallel for
  for (int i = 0 ; i < 10 ; i ++)
    b->a = 10;
}

void foo_five(int &y)
{
  int *yp = &y;
  volatile int size = 1;
// CHECK-DAG: [[Y_ADDR:%y.addr]] = alloca ptr,
// CHECK-DAG: [[YP_ADDR:%yp]] = alloca ptr, align 8
// CHECK-DAG: [[SI:%size]] = alloca i32, align 4
// CHECK-DAG: [[Y_MAP:%y.map.ptr.tmp]] = alloca ptr, align 8
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TASK
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[Y_MAP]])
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[Y_MAP]]
// CHECK: store {{.*}}, ptr [[Y_MAP]]
// CHECK: load {{.*}}, ptr [[Y_MAP]]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TASK"() ]
  #pragma omp target map(tofrom:yp[0:size])  nowait depend(out:y)
   y = 3;
}

void foo_six(int &y) {
// CHECK: [[Y_ADDR:%y.addr]] = alloca ptr,
// CHECK-NEXT: [[Y_MAP:%y.map.ptr.tmp]] = alloca ptr, align 8
// CHECK: [[L:%[0-9]+]] = load ptr, ptr [[Y_ADDR]]
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(ptr [[L]]
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[Y_MAP]]
// CHECK:  store ptr [[L]],  ptr [[Y_MAP]]
// CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target
  y = 3;
}
void foo_seven(int *&y) {
// CHECK: [[Y_ADDR:%y.addr]] = alloca ptr,
// CHECK-NEXT: [[Y_MAP:%y.map.ptr.tmp]] = alloca ptr, align 8
// CHECK: [[L:%[0-9]+]] = load ptr, ptr [[Y_ADDR]]
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L]]
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[Y_MAP]]
// CHECK:  store ptr [[L]],  ptr [[Y_MAP]]
// CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target
  (*y) = 3;
}

void foo_eight(int *&v) {
// CHECK: [[V_ADDR:%v.addr]] = alloca ptr,
// CHECK: [[V_MAP:%v.map.ptr.tmp]] = alloca ptr,
// CHECK: [[L0:%[0-9]+]] = load ptr, ptr [[V_ADDR]]
// CHECK: [[L:%[0-9]+]] = load ptr, ptr [[V_ADDR]]
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[L]]
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[V_MAP]]
// CHECK: store ptr [[L]], ptr [[V_MAP]]
// CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(to: v[0:3])
    v[2]++;
}
char * host_ptr = nullptr;
void foo_nigth() {
// CHECK: [[HOST_MAP:%host_ptr.map.ptr.tmp]] = alloca ptr
// CHECK: [[HOST_MAP1:%host_ptr.map.ptr.tmp1]] = alloca ptr
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[HOST_MAP]]
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME:  "QUAL.OMP.SHARED"(ptr [[HOST_MAP]]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target teams
    host_ptr[1] = (char) 1;
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[HOST_MAP1]]
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK-SAME:  "QUAL.OMP.SHARED"(ptr [[HOST_MAP1]]
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME:  "QUAL.OMP.SHARED"(ptr [[HOST_MAP1]]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target teams distribute parallel for
  for (int i = 0 ; i < 1 ; i ++)
    host_ptr[0] = (char) 1;
}
