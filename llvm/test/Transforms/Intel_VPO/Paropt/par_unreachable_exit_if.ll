; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,simplifycfg,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; a() __attribute__((__noreturn__));
; namespace b {
; c() {
; #pragma omp parallel if (c)
;   a();
; }
; } // namespace b
;
; Check that we are able to generate code for parallel if(...) for the region
; without crashing.
; CHECK:         br i1 true, label {{%.*}}, label {{%.*}}
; CHECK:       if.then:
; CHECK-NEXT:    call void {{.*}} @__kmpc_fork_call({{.*}})
; CHECK:       if.else:
; CHECK:         call void @__kmpc_serialized_parallel({{.*}})
; CHECK:         call void @[[OUTLINED_FUNCTION:_ZN1b1cEv.DIR.OMP.PARALLEL[^(]*]]({{.*}})
; CHECK:         call void @__kmpc_end_serialized_parallel({{.*}})

; CHECK:       define internal void @[[OUTLINED_FUNCTION]]({{.*}})
; CHECK:         %call = call i32 @_Z1av()
; CHECK:         unreachable

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local i32 @_ZN1b1cEv() #0 {
entry:
  %retval = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.IF"(i1 true) ]
  %call = call i32 @_Z1av() #3
  unreachable

dummy:                                            ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %1 = load i32, i32* %retval, align 4
  ret i32 %1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noreturn
declare dso_local i32 @_Z1av() #2

attributes #0 = { mustprogress noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { noreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
