; RUN: opt -enable-new-pm=0 -vpo-wrncollection -analyze %s | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -disable-output %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test checks that WRegionUtils::hasTargetDirective() correctly detects
; whether a function contains OpenMP TARGET constructs or not.
; The original C test has two functions, one with a TARGET pragma, and the
; other without:
;
; int fn_with_target() {
;   int x = 123;
;   #pragma omp target map(x)
;     x = x + 456;
;   return x;
; }
; void bar();
; void fn_without_target() {
;   #pragma omp parallel
;      bar();
; }
;
; Dumping The WRN Graph from WRegionCollection should show this:
;
; Printing analysis 'VPO Work-Region Collection' for function 'fn_with_target':
;
; Function contains OpenMP Target construct(s).        <--- CHECK this
;
; BEGIN TARGET ID=1 {
;
;   IF_EXPR: UNSPECIFIED
;   DEVICE: UNSPECIFIED
;   NOWAIT: false
;   DEFAULTMAP: UNSPECIFIED
;   OFFLOAD_ENTRY_IDX: 0
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
;
; Printing analysis 'VPO Work-Region Collection' for function 'fn_without_target':
;
; Function does not contain OpenMP Target constructs.  <--- CHECK this
;
; BEGIN PARALLEL ID=2 {
;
;   IF_EXPR: UNSPECIFIED
;   NUM_THREADS: UNSPECIFIED
;   DEFAULT: UNSPECIFIED
;   PROCBIND: UNSPECIFIED
;   SHARED clause: UNSPECIFIED
;   PRIVATE clause: UNSPECIFIED
;   FIRSTPRIVATE clause: UNSPECIFIED
;   REDUCTION clause: UNSPECIFIED
;   COPYIN clause: UNSPECIFIED
;
;   EntryBB: entry
;   ExitBB: DIR.OMP.END.PARALLEL.2
;
; } END PARALLEL ID=2


; CHECK: Function contains OpenMP Target
; CHECK: Function does not contain OpenMP Target


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; FUNCTION WITH A TARGET CONSTRUCT ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Function Attrs: nounwind uwtable
define dso_local i32 @fn_with_target() local_unnamed_addr #0 {
entry:
  %x = alloca i32, align 4
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  store i32 123, i32* %x, align 4, !tbaa !3
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32* %x) ]
  br label %DIR.OMP.TARGET.11

DIR.OMP.TARGET.11:                                ; preds = %DIR.OMP.TARGET.1
  %2 = load i32, i32* %x, align 4, !tbaa !3
  %add = add nsw i32 %2, 456
  store i32 %add, i32* %x, align 4, !tbaa !3
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.TARGET.11
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.2

DIR.OMP.END.TARGET.2:                             ; preds = %DIR.OMP.END.TARGET.3
  %3 = load i32, i32* %x, align 4, !tbaa !3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #2
  ret i32 %3
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; FUNCTION WITH NO TARGET CONSTRUCTS ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Function Attrs: nounwind uwtable
define dso_local void @fn_without_target() local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  call void (...) @bar() #2
  br label %DIR.OMP.END.PARALLEL.2

DIR.OMP.END.PARALLEL.2:                           ; preds = %DIR.OMP.PARALLEL.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.21

DIR.OMP.END.PARALLEL.21:                          ; preds = %DIR.OMP.END.PARALLEL.2
  ret void
}

declare dso_local void @bar(...) #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 84, i32 -672425036, !"fn_with_target", i32 3, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 5ce3ba55ac0820d6015de166c9df98f8074a8b8a) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 8388a769ba89cd5885402ed875319f2f43272f7d)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
