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
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 3
define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:  %mem2 = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 0
; CHECK-OP-MOD:  %mem2 = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 0
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
; CHECK-MOD:  %capacilty = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 1
; CHECK-OP-MOD:  %capacilty = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 1
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
; CHECK-MOD:  %base = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 3
; CHECK-OP-MOD:  %base = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 3
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Nullify base pointer
; CHECK-TRANS-NEXT:  store i32** null, i32*** %base, align 8
; CHECK-OP-TRANS:     ; BasePtrInst: Nullify base pointer
; CHECK-OP-TRANS-NEXT:  store ptr null, ptr %base, align 8
; CHECK-MOD:  store %__SOA_EL_struct.Arr* null, %__SOA_EL_struct.Arr** %base, align 8
; CHECK-OP-MOD:  store ptr null, ptr %base, align 8
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp = load i32, i32* %capacilty3, align 8
  %conv = sext i32 %tmp to i64
  %mul = mul i64 %conv, 8
; CHECK-TRANS:     ; BasePtrInst: Allocation call
; CHECK-TRANS-NEXT:  %call = call i8* @malloc(i64 %mul)
; CHECK-OP-TRANS:     ; BasePtrInst: Allocation call
; CHECK-OP-TRANS-NEXT:  %call = call ptr @malloc(i64 %mul)
; CHECK-MOD:       %nsz = mul nuw i64 %mul, 2
; CHECK-MOD-NEXT:  %call = call i8* @malloc(i64 %nsz)
; CHECK-OP-MOD:  %nsz = mul nuw i64 %mul, 2
; CHECK-OP-MOD-NEXT:  %call = call ptr @malloc(i64 %nsz)
  %call = call i8* @malloc(i64 %mul)
; CHECK-MOD-NEXT:  %tmp3 = bitcast i8* %call to %__SOA_EL_struct.Arr*
; CHECK-OP-MOD-NEXT:  %tmp3 = bitcast ptr %call to ptr
  %tmp3 = bitcast i8* %call to i32**
; CHECK-MOD-NEXT:  %base5 = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 3
; CHECK-OP-MOD-NEXT:  %base5 = getelementptr inbounds %__SOA_struct.Arr, ptr %this, i32 0, i32 3
  %base5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-TRANS-NEXT:  store i32** %tmp3, i32*** %base5, align 8
; CHECK-OP-TRANS:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-OP-TRANS-NEXT:  store ptr %tmp3, ptr %base5, align 8
; CHECK-MOD-NEXT:  store %__SOA_EL_struct.Arr* %tmp3, %__SOA_EL_struct.Arr** %base5, align 8
; CHECK-OP-MOD-NEXT:  store ptr %tmp3, ptr %base5, align 8
  store i32** %tmp3, i32*** %base5, align 8
  ret void
}

declare !intel.dtrans.func.type !12 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

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
!11 = !{i8 0, i32 1}
!12 = distinct !{!11}
