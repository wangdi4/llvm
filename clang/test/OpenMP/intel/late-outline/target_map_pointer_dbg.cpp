// INTEL_COLLAB
// RUN: %clang_cc1 -debug-info-kind=limited -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

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
// CHECK: [[MAP_ADDR2:%f.map.ptr.tmp[0-9]+]] = alloca ptr,
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[MAP_ADDR]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[MAP_ADDR]], metadata !30, metadata !DIExpression()), !dbg !31
// CHECK: store {{.*}}, ptr [[MAP_ADDR]],
// CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
// CHECK-SAME: "QUAL.OMP.SHARED:BYREF.TYPED"(ptr [[MAP_ADDR]]
// CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
// CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target teams distribute parallel for
  for (int i = 0 ; i < 10 ; i ++)
    f->a = 10;
// CHECK: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[MAP_ADDR2]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[MAP_ADDR2]],  metadata !55, metadata !DIExpression()), !dbg !56
// CHECK: store {{.*}}, ptr [[MAP_ADDR2]],
// CHECK: load ptr, ptr [[MAP_ADDR2]]
// CHECK: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target
  for (int i = 0 ; i < 10 ; i ++)
    f->a = 10;
// CHECK: [[L:%[0-9]+]] = load ptr, ptr [[F_ADDR]]
// CHECK: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.PARALLEL
// CHECK-SAME:"QUAL.OMP.SHARED:BYREF.TYPED"(ptr [[F_ADDR]]
#pragma omp parallel
  for (int i = 0 ; i < 10 ; i ++)
    f->a = 10;
}

int main() {
// CHECK: [[A_ADDR:%a]] = alloca ptr,
// CHECK: [[MAP_ADDR:%a.map.ptr.tmp]] = alloca ptr,
  int *a;
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[MAP_ADDR]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[MAP_ADDR]], metadata !103, metadata !DIExpression()), !dbg !104
#pragma omp target map(to: a[0:1])
  a[0]++;
}

// !29 = !DILocalVariable(name: "f", scope: !27, file: !9, line: 19, type: !12)
// !54 = !DILocalVariable(name: "f", scope: !52, file: !9, line: 19, type: !12)
// !103 = !DILocalVariable(name: "a", scope: !101, file: !10, line: 55, type: !97)
// end INTEL_COLLAB
