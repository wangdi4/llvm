; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
;
; typedef struct { int a; double *b; } C;
; #pragma omp declare mapper(id: C s) map(s.a, s.b[0:2])
;
; int main() {
;   C s;
;   s.a = 10;
;   double x[2];
;   x[0] = 20;
;   s.b = &x[0];
; //  printf("%d %lf %p\n", s.a, s.b[0], s.b);
;
; #pragma omp target map(mapper(id), tofrom : s)
; //  printf("%d %lf %p\n", s.a, s.b[0], s.b);
; ;
; }

; Check that the mapper function is being passed to tgt_target_mapper.
; CHECK: %.offload_mappers = alloca [1 x i8*], align 8
; CHECK: %[[MAPPER_GEP1:[^ ]+]] = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_mappers, i32 0, i32 0
; CHECK: store i8* bitcast (void (i8*, i8*, i8*, i64, i64)* @.omp_mapper._ZTS1C.id to i8*), i8** %[[MAPPER_GEP1]], align 8
; CHECK: %[[MAPPER_GEP2:[^ ]+]] = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_mappers, i32 0, i32 0
; CHECK: %{{[^ ]+}} = call i32 @__tgt_target_mapper(%struct.ident_t* @{{[^ ,]+}}, i64 %{{[^ ,]+}}, i8* @{{[^ ,]+}}, i32 1, i8** %{{[^ ,]+}}, i8** %{{[^ ,]+}}, i64* getelementptr inbounds ([1 x i64], [1 x i64]* @{{[^ ,]+}}, i32 0, i32 0), i64* getelementptr inbounds ([1 x i64], [1 x i64]* @{{[^ ,]+}}, i32 0, i32 0), i8** null, i8** %[[MAPPER_GEP2]])


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

%struct.C = type { i32, double* }

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define hidden i32 @main() #0 {
entry:
  %s = alloca %struct.C, align 8
  %x = alloca [2 x double], align 16
  %a = getelementptr inbounds %struct.C, %struct.C* %s, i32 0, i32 0
  store i32 10, i32* %a, align 8
  %arrayidx = getelementptr inbounds [2 x double], [2 x double]* %x, i64 0, i64 0
  store double 2.000000e+01, double* %arrayidx, align 16
  %arrayidx1 = getelementptr inbounds [2 x double], [2 x double]* %x, i64 0, i64 0
  %b = getelementptr inbounds %struct.C, %struct.C* %s, i32 0, i32 1
  store double* %arrayidx1, double** %b, align 8

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(%struct.C* %s, %struct.C* %s, i64 16, i64 35, i8* null, void (i8*, i8*, i8*, i64, i64)* @.omp_mapper._ZTS1C.id) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind uwtable
declare void @.omp_mapper._ZTS1C.id(i8* %0, i8* %1, i8* %2, i64 %3, i64 %4) #2

; Function Attrs: nounwind
declare void @__tgt_push_mapper_component(i8*, i8*, i8*, i64, i64) #1

; Function Attrs: nounwind
declare i64 @__tgt_mapper_num_components(i8*) #1

attributes #0 = { noinline norecurse nounwind optnone uwtable mustprogress "contains-openmp-target"="true" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 2055, i32 151394014, !"_Z4main", i32 14, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
