; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s
; RUN: opt -passes='vec-clone,intel-ir-optreport-emitter' -mtriple=x86_64 -vec-clone-legalize-enabled -intel-opt-report=high -disable-output < %s 2>&1 | FileCheck %s --strict-whitespace --check-prefix=OPT-REPORT

define noundef i32 @func(ptr nocapture noundef readonly %p) #0 {
; CHECK-LABEL-NOT: @_ZGVZN256u_func
; CHECK-NOT: "vector-variants"="_ZGVZN256u_func"
;
; OPT-REPORT:        Global optimization report for : func
; OPT-REPORT-EMPTY:
; OPT-REPORT-NEXT:   FUNCTION REPORT BEGIN
; OPT-REPORT-NEXT:       remark #15581: 'omp declare' vector variants were skipped due to limited support for vector length and/or argument and/or return type: _ZGVZN256u_func.
; OPT-REPORT-NEXT:   FUNCTION REPORT END
; OPT-REPORT-NEXT:   =================================================================
;
entry:
  %i0 = load i32, ptr %p, align 4
  %i1 = shl nsw i32 %i0, 1
  ret i32 %i1
}

attributes #0 = { "vector-variants"="_ZGVZN256u_func" }
