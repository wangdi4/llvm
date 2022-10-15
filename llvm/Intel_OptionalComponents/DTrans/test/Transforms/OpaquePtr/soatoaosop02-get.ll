; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-TY %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

; This test checks various approximations for side effects in get-like function.
;   S get(int i) { return base[i]; }
; Check that approximations work as expected.
; CHECK-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-WF-NOT: ; Func(GEP
define "intel_dtrans_func_index"="1" i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !9 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i8*, i8** %tmp, i64 %idxprom
; CHECK-TY:      Load(Func(Arg 1)
; CHECK-TY-NEXT:          (Load(GEP(Arg 0)
; CHECK-TY-NEXT:                    3)))
; CHECK-TY-NEXT: %get = load i8*, i8** %arrayidx, align 8
; CHECK-OP:      Load(Func(Arg 1)
; CHECK-OP-NEXT:          (Load(GEP(Arg 0)
; CHECK-OP-NEXT:                    3)))
; CHECK-OP-NEXT: %get = load ptr, ptr %arrayidx, align 8
  %get = load i8*, i8** %arrayidx, align 8
; Return is represent as its operand
; CHECK-TY-NEXT: Load(Func(Arg 1)
; CHECK-TY-NEXT:          (Load(GEP(Arg 0)
; CHECK-TY-NEXT:                    3)))
; CHECK-TY-NEXT: ret i8* %get
; CHECK-OP-NEXT: Load(Func(Arg 1)
; CHECK-OP-NEXT:          (Load(GEP(Arg 0)
; CHECK-OP-NEXT:                    3)))
; CHECK-OP-NEXT: ret ptr %get
  ret i8* %get
}

; XCHECK: Deps computed: 7, Queries: 7

!intel.dtrans.types = !{!0, !4}

!0 = !{!"S", %struct.Mem zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %struct.Arr.0 zeroinitializer, i32 6, !5, !3, !6, !8, !3, !6}
!5 = !{%struct.Mem zeroinitializer, i32 1}
!6 = !{!"A", i32 4, !7}
!7 = !{i8 0, i32 0}
!8 = !{i32 0, i32 2}
!9 = distinct !{!10, !11}
!10 = !{i8 0, i32 1}
!11 = !{%struct.Arr.0 zeroinitializer, i32 1}
