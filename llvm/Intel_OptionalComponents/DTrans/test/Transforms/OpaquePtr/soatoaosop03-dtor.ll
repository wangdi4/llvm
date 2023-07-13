; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                    \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:    -debug-only=dtrans-soatoaosop                                                                         \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                    \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:    -debug-only=dtrans-soatoaosop-arrays                                                                  \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                                 \
; RUN:    -passes=soatoaosop-arrays-methods-transform                                                           \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Mem = type { ptr }
; CHECK-MOD-DAG: %__SOA_struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr = type { ptr, ptr }

; The following method should be classified as dtor.
; Instructions to transform are shown.
; Transformed instructions are shown.
;   ~Arr() { mem->deallocate(base); }
; CHECK:      Checking array's method _ZN3ArrIPiED2Ev
; CHECK-NEXT: Classification: Dtor method
; CHECK-TRANS: ; Dump instructions needing update. Total = 1

define void @_ZN3ArrIPiED2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:   %mem = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 0
  %mem = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 0
  %tmp = load ptr, ptr %mem, align 8
; CHECK-MOD:   %base = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 3
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:  %tmp1 = load ptr, ptr %base, align 8
; CHECK-MOD:   %tmp1 = load ptr, ptr %base, align 8
  %tmp1 = load ptr, ptr %base, align 8
; CHECK-MOD:   %tmp2 = bitcast ptr %tmp1 to ptr
  %tmp2 = bitcast ptr %tmp1 to ptr
  call void @_ZdlPv(ptr %tmp2)
  ret void
}

declare !intel.dtrans.func.type !11 dso_local void @_ZdlPv(ptr "intel_dtrans_func_index"="1")

!intel.dtrans.types = !{!0, !4}

!0 = !{!"S", %struct.Mem zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %struct.Arr zeroinitializer, i32 6, !5, !3, !6, !8, !3, !6}
!5 = !{%struct.Mem zeroinitializer, i32 1}
!6 = !{!"A", i32 4, !7}
!7 = !{i8 0, i32 0}
!8 = !{i32 0, i32 2}
!9 = distinct !{!10}
!10 = !{%struct.Arr zeroinitializer, i32 1}
!11 = distinct !{!12}
!12 = !{i8 0, i32 1}
