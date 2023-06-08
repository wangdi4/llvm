; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s
; RUN: opt -passes='vec-clone,intel-ir-optreport-emitter' -mtriple=x86_64 -vec-clone-legalize-enabled -intel-opt-report=high -disable-output < %s 2>&1 | FileCheck %s --strict-whitespace --check-prefix=OPT-REPORT

define <2 x i32> @func(ptr nocapture noundef readonly %p) #0 {
; CHECK-LABEL-NOT: @_ZGVYN32u_func
; CHECK-NOT: "vector-variants"="_ZGVYN32u_func"
;
; OPT-REPORT:        Global optimization report for : func
; OPT-REPORT-EMPTY:
; OPT-REPORT-NEXT:   FUNCTION REPORT BEGIN
; OPT-REPORT-NEXT:       remark #15581: 'omp declare' vector variants were skipped due to limited support for vector length and/or argument and/or return type: _ZGVYN32u_func.
; OPT-REPORT-NEXT:   FUNCTION REPORT END
; OPT-REPORT-NEXT:   =================================================================
;
entry:
  %i0 = load i32, ptr %p, align 4
  %i1 = shl nsw i32 %i0, 1
  %i2 = insertelement <2 x i32> poison, i32 %i1, i32 0
  %i3 = shufflevector <2 x i32> %i2, <2 x i32> poison, <2 x i32> zeroinitializer
  ret <2 x i32> %i3
}

attributes #0 = { "vector-variants"="_ZGVYN32u_func" }
