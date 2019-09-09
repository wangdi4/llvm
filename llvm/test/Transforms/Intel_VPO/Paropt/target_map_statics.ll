; RUN: opt < %s -vpo-paropt -S | FileCheck %s
; RUN: opt < %s -passes='vpo-paropt' -S | FileCheck %s
;
;
; Test src:
;
; int foo( ) {
;   static float vx2[100];
;   static float vy2[100];
;   #pragma omp target map(tofrom: vx2[0:30], vy2[0:30])
;   {
;     float dxc;
;   }
;   return 0;
; }

; The test is just intended to check that there is no comp-fail while executing it.
; CHECK: call{{.*}}@__tgt_target

; ModuleID = 'target_map_statics.ll'
source_filename = "target_map_statics.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@foo.vx2 = internal global [100 x float] zeroinitializer, align 16
@foo.vy2 = internal global [100 x float] zeroinitializer, align 16
@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo() #0 {
entry:
  %dxc = alloca float, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.1.split

DIR.OMP.TARGET.1.split:                           ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"([100 x float]* @foo.vx2, float* getelementptr inbounds ([100 x float], [100 x float]* @foo.vx2, i64 0, i64 0), i64 120), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"([100 x float]* @foo.vy2, float* getelementptr inbounds ([100 x float], [100 x float]* @foo.vy2, i64 0, i64 0), i64 120), "QUAL.OMP.PRIVATE"(float* %dxc) ]
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.TARGET.2
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2051, i32 146398040, !"foo", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
