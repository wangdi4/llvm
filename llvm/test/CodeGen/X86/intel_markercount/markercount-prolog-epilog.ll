; REQUIRES: intel_feature_markercount
; RUN: llc < %s -mark-prolog-epilog -mtriple=x86_64-- -print-after=finalize-isel 2>&1 1>/dev/null | FileCheck --check-prefix=ISEL %s
; RUN: llc < %s -mark-prolog-epilog -mtriple=x86_64-- -stop-after=tailduplication | FileCheck --check-prefix=TAIL %s
; RUN: llc < %s -mark-prolog-epilog -mtriple=x86_64-- -print-after=tailduplication 2>&1 1>/dev/null | FileCheck --check-prefixes=TAIL,TAILNO %s
; RUN: llc < %s -mark-prolog-epilog -mtriple=x86_64-- -stop-after=x86-markercount | FileCheck --check-prefix=MARKER %s
; RUN: llc < %s -mark-prolog-epilog -mtriple=x86_64-- | FileCheck %s

; ISEL-NOT: markercount.loop_header
; ISEL: kernel
; ISEL: bb.0.entry
; ISEL-NEXT: ;  markercount.prolog markercount.epilog
; ISEL-NOT: ;  markercount.prolog markercount.epilog

; ISEL: for_looper
; ISEL: bb.0.entry
; ISEL: ;  markercount.prolog
; ISEL-NOT: ;  markercount.prolog
; ISEL: bb.{{.*}}.for.end
; ISEL: ;  markercount.epilog
; ISEL-NOT: ;  markercount.epilog

; TAILNO-NOT: markercount.prolog
; TAILNO-NOT: markercount.epilog
; TAIL-NOT: PSEUDO_LOOP_HEADER
; TAIL: kernel
; TAIL: bb.0.entry
; TAIL: PSEUDO_FUNCTION_PROLOG
; TAIL: PSEUDO_FUNCTION_EPILOG

; TAIL: for_looper
; TAIL: bb.0.entry
; TAIL: PSEUDO_FUNCTION_PROLOG
; TAIL-NOT: PSEUDO_FUNCTION_PROLOG
; TAIL: bb.{{.*}}.for.end
; TAIL: PSEUDO_FUNCTION_EPILOG
; TAIL-NOT: PSEUDO_FUNCTION_EPILOG

; MARKER-NOT: PSEUDO_FUNCTION_PROLOG
; MARKER-NOT: PSEUDO_FUNCTION_EPILOG
; MARKER: kernel
; MARKER: bb.0.entry
; MARKER: MARKER_COUNT_FUNCTION
; MARKER: MARKER_COUNT_FUNCTION

; MARKER: for_looper
; MARKER: bb.0.entry
; MARKER: MARKER_COUNT_FUNCTION
; MARKER: bb.{{.*}}.for.end
; MARKER: MARKER_COUNT_FUNCTION
; MARKER-NOT: MARKER_COUNT_FUNCTION

; CHECK: kernel
; CHECK: markercount_function                    # PROLOG
; CHECK: markercount_function                    # EPILOG
; CHECK: retq

; CHECK: for_looper
; CHECK: markercount_function                    # PROLOG
; CHECK: markercount_function                    # EPILOG
; CHECK: retq
define i32 @kernel(i32 %x) {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
  %1 = load i32, i32* %x.addr, align 4
  %mul = mul nsw i32 %0, %1
  %mul1 = mul nsw i32 %mul, 2
  ret i32 %mul1
}

define i32 @for_looper(i32 noundef %n) {
entry:
  %n.addr = alloca i32, align 4
  %res = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  store i32 %0, i32* %res, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4
  %2 = load i32, i32* %n.addr, align 4
  %cmp = icmp slt i32 %1, %2
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load i32, i32* %i, align 4
  %4 = load i32, i32* %res, align 4
  %add = add nsw i32 %4, %3
  store i32 %add, i32* %res, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %5 = load i32, i32* %i, align 4
  %inc = add nsw i32 %5, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond, !llvm.loop !4

for.end:                                          ; preds = %for.cond
  %6 = load i32, i32* %res, align 4
  ret i32 %6
}

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
