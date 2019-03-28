; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)'  -S | FileCheck %s
;
; Check whether the compiler generates the call llvm.launder.invariant.group for map global array in vpo-paropt prepare pass.
;
; int arrS[100];
; void foo()
; {
;    #pragma omp target map(arrS[42:20])
;    {
;      arrS[50] = 3;
;    }
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-unknown-linux-gnu"

@arrS = dso_local global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @_Z3foov() #0 {
entry:

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"([100 x i32]* @arrS, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arrS, i64 0, i64 42), i64 80) ]


; CHECK: [[ARR_LAUNDER:%[0-9]+]] = call i8* @llvm.launder.invariant.group.p0i8
; CHECK-SAME: @arrS
; CHECK: [[ARR_BITCAST:%[a-zA-Z._0-9]+]] = bitcast i8* [[ARR_LAUNDER]] to [100 x i32]*
; CHECK: [[GEP:%[0-9]+]] = getelementptr inbounds [100 x i32], [100 x i32]* [[ARR_BITCAST]], i64 0, i64 50
; CHECK: store i32 3, i32* [[GEP]], align 8
  store i32 3, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arrS, i64 0, i64 50), align 8, !tbaa !3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 59, i32 -1938125551, !"_Z3foov", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA100_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
;
; CHECK: llvm.launder.invariant.group.p0i8
