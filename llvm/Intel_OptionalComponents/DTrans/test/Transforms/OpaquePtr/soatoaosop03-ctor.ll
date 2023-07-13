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

; The following method should be classified as ctor.
; Instructions to transform are shown.
; Transformed instructions are shown.
;   Arr(int c = 1, Mem *mem = nullptr)
;       : mem(mem), capacilty(c), size(0), base(nullptr) {
;     base = (S *)mem->allocate(capacilty * sizeof(S));
;   }
; CHECK:      Checking array's method _ZN3ArrIPiEC2EiP3Mem
; CHECK-NEXT: Classification: Ctor method
; CHECK-TRANS: ; Dump instructions needing update. Total = 3

define void @_ZN3ArrIPiEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:  %mem2 = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 0
  %mem2 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 0
  store ptr %mem, ptr %mem2, align 8
; CHECK-MOD:  %capacilty = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 1
  %capacilty = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 1
  store i32 %c, ptr %capacilty, align 8
; CHECK-MOD:  %base = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 3
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Nullify base pointer
; CHECK-TRANS-NEXT:  store ptr null, ptr %base, align 8
; CHECK-MOD:  store ptr null, ptr %base, align 8
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
  store i32 0, ptr %size, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 1
  %tmp = load i32, ptr %capacilty3, align 8
  %conv = sext i32 %tmp to i64
  %mul = mul i64 %conv, 8
; CHECK-TRANS:     ; BasePtrInst: Allocation call
; CHECK-TRANS-NEXT:  %call = call ptr @malloc(i64 %mul)
; CHECK-MOD:  %nsz = mul nuw i64 %mul, 2
; CHECK-MOD-NEXT:  %call = call ptr @malloc(i64 %nsz)
  %call = call ptr @malloc(i64 %mul)
; CHECK-MOD-NEXT:  %tmp3 = bitcast ptr %call to ptr
  %tmp3 = bitcast ptr %call to ptr
; CHECK-MOD-NEXT:  %base5 = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 3
  %base5 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-TRANS-NEXT:  store ptr %tmp3, ptr %base5, align 8
; CHECK-MOD-NEXT:  store ptr %tmp3, ptr %base5, align 8
  store ptr %tmp3, ptr %base5, align 8
  ret void
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !11 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

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
!9 = distinct !{!10, !5}
!10 = !{%struct.Arr zeroinitializer, i32 1}
!11 = distinct !{!12}
!12 = !{i8 0, i32 1}
