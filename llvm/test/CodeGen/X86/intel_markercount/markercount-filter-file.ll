; REQUIRES: intel_feature_markercount
; RUN: llc < %s -mark-prolog-epilog -mtriple=x86_64-- -stop-after=tailduplication -filtered-markercount-file=%S/../../Inputs/intel-filtered-markercount-file.txt | FileCheck --check-prefix=TAIL %s
; RUN: llc < %s -mark-prolog-epilog -mtriple=x86_64-- -filtered-markercount-file=%S/../../Inputs/intel-filtered-markercount-file.txt | FileCheck %s
; RUN: llc < %s -mark-prolog-epilog -mtriple=x86_64-- -filtered-markercount-file=%S/../../Inputs/fake-file.txt 2>&1 | FileCheck --check-prefix=FAKE %s

; TAIL: kernel_a
; TAIL: PSEUDO_FUNCTION_PROLOG
; TAIL: PSEUDO_FUNCTION_EPILOG

; TAIL: kernel_b
; TAIL-NOT: PSEUDO_FUNCTION_PROLOG
; TAIL-NOT: PSEUDO_FUNCTION_EPILOG

; CHECK: kernel_a
; CHECK: markercount_function                    # PROLOG
; CHECK: markercount_function                    # EPILOG

; CHECK: kernel_b
; CHECK-NOT: markercount_function

; FAKE: pseudo-markercount-inserter: failed to read file {{.*}}
; FAKE: kernel_a
; FAKE: markercount_function                    # PROLOG
; FAKE: markercount_function                    # EPILOG

; FAKE: kernel_b
; FAKE: markercount_function                    # PROLOG
; FAKE: markercount_function                    # EPILOG
define i32 @kernel_a(i32 %x) {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
  %1 = load i32, i32* %x.addr, align 4
  %mul = mul nsw i32 %0, %1
  %mul1 = mul nsw i32 %mul, 2
  ret i32 %mul1
}

define i32 @kernel_b(i32 %x) {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
  %1 = load i32, i32* %x.addr, align 4
  %mul = mul nsw i32 %0, %1
  %mul1 = mul nsw i32 %mul, 2
  ret i32 %mul1
}

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
