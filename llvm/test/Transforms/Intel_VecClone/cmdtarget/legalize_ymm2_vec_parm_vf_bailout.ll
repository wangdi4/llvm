; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s
; RUN: opt -passes='vec-clone,intel-ir-optreport-emitter' -mtriple=x86_64 -vec-clone-legalize-enabled -intel-opt-report=high -disable-output < %s 2>&1 | FileCheck %s --strict-whitespace --check-prefix=OPT-REPORT


define <2 x i32> @func(<2 x i32> %v) #0 {
; CHECK-LABEL-NOT: @_ZGVYN32v_func
; CHECK-NOT: "vector-variants"="_ZGVYN32v_func"
;
; OPT-REPORT:        Global optimization report for : func
; OPT-REPORT-EMPTY:
; OPT-REPORT-NEXT:   FUNCTION REPORT BEGIN
; OPT-REPORT-NEXT:       remark #15581: 'omp declare' vector variants were skipped due to limited support for vector length and/or argument and/or return type: _ZGVYN32v_func.
; OPT-REPORT-NEXT:   FUNCTION REPORT END
; OPT-REPORT-NEXT:   =================================================================
;
entry:
  %i1 = shl nsw <2 x i32> %v, <i32 1, i32 1>
  ret <2 x i32> %i1
}

attributes #0 = { "vector-variants"="_ZGVYN32v_func" }
