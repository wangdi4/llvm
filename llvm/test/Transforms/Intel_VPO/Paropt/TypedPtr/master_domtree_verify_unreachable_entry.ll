; RUN: opt -vpo-paropt-prepare -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -S %s 2>&1 | FileCheck %s

; Test src:

; a() {
; #pragma omp parallel
;   for (;;)
;     ;
; #pragma omp master
;   ;
; }

; CHECK-NOT: DominatorTree update failed after Master codegen
; CHECK: %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(){{.*}} ]
; CHECK: call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
; CHECK: call i32 @__kmpc_masked({{.*}})
; CHECK: call void @__kmpc_end_masked({{.*}})
; CHECK: master.exit.succ:{{.*}}; preds = %master.entry, %master.exit

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @a() #0 {
entry:
  br label %par.entry

par.entry:                                        ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %par.body

par.body:                                         ; preds = %par.entry
  br label %par.body

par.exit:                                         ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %master.entry

master.entry:                                     ; preds = %par.exit
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  br label %master.body

master.body:                                      ; preds = %master.entry
  br label %master.exit

master.exit:                                      ; master.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.MASTER"() ]
  br label %master.exit.succ

master.exit.succ:                                 ; preds= %master.exit
  br label %exit

exit:                                             ; preds = %master.exit.succ
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!0 = !{i32 1, !"wchar_size", i32 4}
