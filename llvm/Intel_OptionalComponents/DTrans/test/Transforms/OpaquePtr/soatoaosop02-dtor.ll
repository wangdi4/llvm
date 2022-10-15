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

; This test checks various approximations for side effects in dtor-like function.
;   ~Arr() { mem->deallocate(base); }
; Check that approximations work as expected.
; CHECK-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-WF-NOT: ; Func(GEP
define void @_ZN3ArrIPvED2Ev(%struct.Arr.0* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !9 {
entry:
  %mem = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp1 = load i8**, i8*** %base, align 8
  %tmp2 = bitcast i8** %tmp1 to i8*
; Free 'almost' base pointer, note Func.
; CHECK-TY:      Free ptr(Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                        3)))
; CHECK-TY-NEXT:         (Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                        0)))
; CHECK-TY-NEXT: call void @_ZN10MemManager10deallocateEPv(%struct.Mem* %tmp, i8* %tmp2)
; CHECK-OP:      Free ptr(Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                        3)))
; CHECK-OP-NEXT:         (Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                        0)))
; CHECK-OP-NEXT: call void @_ZN10MemManager10deallocateEPv(ptr %tmp, ptr %tmp2)
  call void @_ZN10MemManager10deallocateEPv(%struct.Mem* %tmp, i8* %tmp2)
  ret void
}

define dso_local void @_ZN10MemManager10deallocateEPv(%struct.Mem* "intel_dtrans_func_index"="1" nocapture readnone %this, i8* "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !12 {
entry:
  tail call void @_ZdlPv(i8* %p)
  ret void
}

declare !intel.dtrans.func.type !13 dso_local void @_ZdlPv(i8* "intel_dtrans_func_index"="1")

; XCHECK: Deps computed: 9, Queries: 10

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
!9 = distinct !{!10}
!10 = !{%struct.Arr.0 zeroinitializer, i32 1}
!11 = !{i8 0, i32 1}
!12 = distinct !{!5, !11}
!13 = distinct !{!11}
