; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s
;
; Test support for the "CLOSE" map-type modifier.
; C source:
;
; void foo() {
;   int xyz;
;   #pragma omp target map(close,tofrom:xyz)
;     xyz = 456;
; }
;
; The MAP_TYPE flags for the map in the example is expected to be
;              TO  | FROM | PARAM | CLOSE
;           =  0x1 | 0x2  | 0x20  | 0x400
;           =  0x423 = 1059 (decimal)
; CHECK: @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 1059]

source_filename = "t_close.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %xyz = alloca i32, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM:CLOSE"(i32* %xyz, i32* %xyz, i64 4, i64 1059, i8* null, i8* null) ]
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2
  store i32 456, i32* %xyz, align 4
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.END.TARGET.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1


attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 58, i32 -684619672, !"_Z3foov", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
