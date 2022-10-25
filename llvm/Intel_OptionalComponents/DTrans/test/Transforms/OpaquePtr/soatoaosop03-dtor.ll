; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                            \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:    -debug-only=dtrans-soatoaosop                                                                         \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                            \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:    -debug-only=dtrans-soatoaosop-arrays                                                                  \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                                         \
; RUN:    -passes=soatoaosop-arrays-methods-transform                                                           \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                          \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:    -debug-only=dtrans-soatoaosop                                                                         \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                          \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:    -debug-only=dtrans-soatoaosop-arrays                                                                  \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-OP-TRANS %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                                       \
; RUN:    -passes=soatoaosop-arrays-methods-transform                                                           \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:  | FileCheck --check-prefix=CHECK-OP-MOD %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }
; CHECK-MOD-DAG: %__SOA_struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], %__SOA_EL_struct.Arr*, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr = type { float*, i32* }
; CHECK-OP-MOD-DAG: %__SOA_struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
; CHECK-OP-MOD-DAG: %__SOA_EL_struct.Arr = type { ptr, ptr }

; The following method should be classified as dtor.
; Instructions to transform are shown.
; Transformed instructions are shown.
;   ~Arr() { mem->deallocate(base); }
; CHECK:      Checking array's method _ZN3ArrIPiED2Ev
; CHECK-NEXT: Classification: Dtor method
; CHECK-TRANS: ; Dump instructions needing update. Total = 1
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 1
define void @_ZN3ArrIPiED2Ev(%struct.Arr*  "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:   %mem = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 0
; CHECK-OP-MOD:   %mem = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 0
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
; CHECK-MOD:   %base = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 3
; CHECK-OP-MOD:   %base = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 3
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:  %tmp1 = load i32**, i32*** %base, align 8
; CHECK-OP-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-OP-TRANS-NEXT:  %tmp1 = load ptr, ptr %base, align 8
; CHECK-MOD:   %tmp1 = load %__SOA_EL_struct.Arr*, %__SOA_EL_struct.Arr** %base, align 8
; CHECK-OP-MOD:   %tmp1 = load ptr, ptr %base, align 8
  %tmp1 = load i32**, i32*** %base, align 8
; CHECK-MOD:   %tmp2 = bitcast %__SOA_EL_struct.Arr* %tmp1 to i8*
; CHECK-OP-MOD:   %tmp2 = bitcast ptr %tmp1 to ptr
  %tmp2 = bitcast i32** %tmp1 to i8*
  call void @_ZdlPv(i8* %tmp2)
  ret void
}

declare !intel.dtrans.func.type !12 dso_local void @_ZdlPv(i8* "intel_dtrans_func_index"="1")

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
!11 = !{i8 0, i32 1}
!12 = distinct !{!11}

