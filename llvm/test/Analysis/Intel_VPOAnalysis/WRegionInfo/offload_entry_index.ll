; RUN: opt -enable-new-pm=0 -vpo-wrncollection -analyze %s | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -disable-output %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test checks that QUAL.OMP.OFFLOAD.ENTRY.IDX is parsed
; and properly represented in the WRNTargetNode.
; The test was created from this C source, plus the hand-added
; OperandBundle for "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 77)
;
; int foo() {
;   int x = 123;
;   #pragma omp target map(x)
;     x = x + 456;
;   return x;
; }
;
; The WRN Graph should have this:
;  BEGIN TARGET ID=1 {
;
;   IF_EXPR: UNSPECIFIED
;   DEVICE: UNSPECIFIED
;   NOWAIT: false
;   DEFAULTMAP: UNSPECIFIED
;   OFFLOAD_ENTRY_IDX: 77              <--- CHECK this
;   PRIVATE clause: UNSPECIFIED
;   FIRSTPRIVATE clause: UNSPECIFIED
;   MAP clause (size=1): (i32* %x)
;   IS_DEVICE_PTR clause: UNSPECIFIED
;   DEPEND clause: UNSPECIFIED
;
;   EntryBB: DIR.OMP.TARGET.1
;   ExitBB: DIR.OMP.END.TARGET.3
;
; } END TARGET ID=1

; CHECK: OFFLOAD_ENTRY_IDX: 77

; Function Attrs: nounwind uwtable
define dso_local i32 @foo() #0 {
entry:
  %x = alloca i32, align 4
  %0 = bitcast i32* %x to i8*
  store i32 123, i32* %x, align 4, !tbaa !3
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.MAP.TOFROM"(i32* %x), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 77)]
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %2 = load i32, i32* %x, align 4, !tbaa !3
  %add = add nsw i32 %2, 456
  store i32 %add, i32* %x, align 4, !tbaa !3
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.TARGET.2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.END.TARGET.3
  %3 = load i32, i32* %x, align 4, !tbaa !3
  %4 = bitcast i32* %x to i8*
  ret i32 %3
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 84, i32 -676652413, !"foo", i32 3, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 105f3862bb742d6adede6b26d29ed74003e24523) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 3ee81ef1db62f9ca5dfe2e38ae3deb128d4423ac)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
