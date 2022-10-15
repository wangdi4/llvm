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

%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }
; CHECK-MOD-DAG: %__SOA_struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], %__SOA_EL_struct.Arr.0*, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr.0 = type { float*, i8* }
; CHECK-OP-MOD-DAG: %__SOA_struct.Arr.0 = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
; CHECK-OP-MOD-DAG: %__SOA_EL_struct.Arr.0 = type { ptr, ptr }

; The following method should be classified as realloc-like method.
; Instructions to transform are shown.
; Transformed instructions are shown.
;   void realloc(int inc) {
;     if (size + inc <= capacilty)
;       return;
;
;     S *new_base = (S *)mem->allocate(5 * (size + inc) * sizeof(S));
;     capacilty = size + inc;
;     for (int i = 0; i < size; ++i) {
;       new_base[5 * i] = base[i];
;     }
;     mem->deallocate(base);
;     base = new_base;
;   }
; CHECK:      Checking array's method _ZN3ArrIPvE7reallocEi
; CHECK-NEXT: Classification: Realloc method
; CHECK-TRANS: ; Dump instructions needing update. Total = 10
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 10
define void @_ZN3ArrIPvE7reallocEi(%struct.Arr.0* "intel_dtrans_func_index"="1"  %this, i32 %inc)  !intel.dtrans.func.type !9 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp = load i32, i32* %size, align 8
  %add = add nsw i32 %tmp, %inc
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  %tmp1 = load i32, i32* %capacilty, align 8
  %cmp = icmp sle i32 %add, %tmp1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %mem = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  %tmp2 = load %struct.Mem*, %struct.Mem** %mem, align 8
  %size2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp3 = load i32, i32* %size2, align 8
  %add3 = add nsw i32 %tmp3, %inc
  %mul = mul nsw i32 5, %add3
  %conv = sext i32 %mul to i64
  %mul4 = mul i64 %conv, 8
; CHECK-TRANS:     ; BasePtrInst: Allocation call
; CHECK-TRANS-NEXT:  %call = call i8* @malloc(i64 %mul4)
; CHECK-OP-TRANS:     ; BasePtrInst: Allocation call
; CHECK-OP-TRANS-NEXT:  %call = call ptr @malloc(i64 %mul4)
; CHECK-MOD:       %nsz = mul nuw i64 %mul4, 2
; CHECK-MOD-NEXT:  %call = call i8* @malloc(i64 %nsz)
; CHECK-OP-MOD:    %nsz = mul nuw i64 %mul4, 2
; CHECK-OP-MOD-NEXT:  %call = call ptr @malloc(i64 %nsz)
  %call = call i8* @malloc(i64 %mul4)
; CHECK-MOD: %tmp6 = bitcast i8* %call to %__SOA_EL_struct.Arr.0*
; CHECK-OP-MOD: %tmp6 = bitcast ptr %call to ptr
  %tmp6 = bitcast i8* %call to i8**
  %size6 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp7 = load i32, i32* %size6, align 8
  %add7 = add nsw i32 %tmp7, %inc
  %capacilty8 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  store i32 %add7, i32* %capacilty8, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc14, %for.inc ]
  %size9 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp8 = load i32, i32* %size9, align 8
  %cmp10 = icmp slt i32 %i.0, %tmp8
  br i1 %cmp10, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:  %tmp9 = load i8**, i8*** %base, align 8
; CHECK-OP-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-OP-TRANS-NEXT:  %tmp9 = load ptr, ptr %base, align 8
; CHECK-MOD: %tmp9 = load %__SOA_EL_struct.Arr.0*, %__SOA_EL_struct.Arr.0** %base, align 8
; CHECK-OP-MOD: %tmp9 = load ptr, ptr %base, align 8
  %tmp9 = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i.0 to i64
; CHECK-TRANS:     ; MemInstGEP: Element load
; CHECK-TRANS-NEXT:  %arrayidx1 = getelementptr inbounds i8*, i8** %tmp9, i64 0
; CHECK-OP-TRANS:     ; MemInstGEP: Element load
; CHECK-OP-TRANS-NEXT:  %arrayidx1 = getelementptr inbounds ptr, ptr %tmp9, i64 0
; CHECK-MOD:         %arrayidx1 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %tmp9, i64 0
; CHECK-OP-MOD:      %arrayidx1 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %tmp9, i64 0
  %arrayidx1 = getelementptr inbounds i8*, i8** %tmp9, i64 0
  br label %for.stop

for.stop:                                         ; preds = %for.body
; CHECK-TRANS:     ; MemInstGEP: Element load
; CHECK-TRANS-NEXT:  %array_phi = phi i8** [ %arrayidx1, %for.body ]
; CHECK-OP-TRANS:     ; MemInstGEP: Element load
; CHECK-OP-TRANS-NEXT:  %array_phi = phi ptr [ %arrayidx1, %for.body ]
; CHECK-MOD:         %array_phi = phi %__SOA_EL_struct.Arr.0* [ %arrayidx1, %for.body ]
; CHECK-OP-MOD:      %array_phi = phi ptr [ %arrayidx1, %for.body ]
  %array_phi = phi i8** [ %arrayidx1, %for.body ]
; CHECK-TRANS:     ; MemInstGEP: Element load
; CHECK-TRANS-NEXT:  %arrayidx = getelementptr inbounds i8*, i8** %array_phi, i64 %idxprom
; CHECK-OP-TRANS:     ; MemInstGEP: Element load
; CHECK-OP-TRANS-NEXT:  %arrayidx = getelementptr inbounds ptr, ptr %array_phi, i64 %idxprom
; CHECK-MOD:         %arrayidx = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %array_phi, i64 %idxprom
; CHECK-OP-MOD:     %arrayidx = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %array_phi, i64 %idxprom
  %arrayidx = getelementptr inbounds i8*, i8** %array_phi, i64 %idxprom
; CHECK-MOD-NEXT:    %elem2 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:    %elem = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx, i64 0, i32 0
; CHECK-OP-MOD-NEXT:    %elem2 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %arrayidx, i64 0, i32 1
; CHECK-OP-MOD-NEXT:    %elem = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %arrayidx, i64 0, i32 0
; CHECK-TRANS:     ; MemInst: Element load
; CHECK-TRANS-NEXT:  %tmp10 = load i8*, i8** %arrayidx, align 8
; CHECK-OP-TRANS:     ; MemInst: Element load
; CHECK-OP-TRANS-NEXT:  %tmp10 = load ptr, ptr %arrayidx, align 8
; CHECK-MOD:        %copy = load float*, float** %elem, align 8
; CHECK-MOD-NEXT:   %tmp10 = load i8*, i8** %elem2, align 8
; CHECK-OP-MOD:        %copy = load ptr, ptr %elem, align 8
; CHECK-OP-MOD-NEXT:   %tmp10 = load ptr, ptr %elem2, align 8
  %tmp10 = load i8*, i8** %arrayidx, align 8
  %mul11 = mul nsw i32 5, %i.0
  %idxprom12 = sext i32 %mul11 to i64
; CHECK-TRANS:     ; MemInstGEP: Element store to new mem
; CHECK-TRANS-NEXT:  %arrayidx13 = getelementptr inbounds i8*, i8** %tmp6, i64 %idxprom12
; CHECK-OP-TRANS:     ; MemInstGEP: Element store to new mem
; CHECK-OP-TRANS-NEXT:  %arrayidx13 = getelementptr inbounds ptr, ptr %tmp6, i64 %idxprom12
; CHECK-MOD:         %arrayidx13 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %tmp6, i64 %idxprom12
; CHECK-OP-MOD:         %arrayidx13 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %tmp6, i64 %idxprom12
  %arrayidx13 = getelementptr inbounds i8*, i8** %tmp6, i64 %idxprom12
; CHECK-MOD-NEXT:    %elem3 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx13, i64 0, i32 1
; CHECK-MOD-NEXT:    %elem1 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx13, i64 0, i32 0
; CHECK-OP-MOD-NEXT:    %elem3 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %arrayidx13, i64 0, i32 1
; CHECK-OP-MOD-NEXT:    %elem1 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %arrayidx13, i64 0, i32 0
; CHECK-TRANS:     ; MemInst: Element store to new mem
; CHECK-TRANS-NEXT:  store i8* %tmp10, i8** %arrayidx13, align 8
; CHECK-OP-TRANS:     ; MemInst: Element store to new mem
; CHECK-OP-TRANS-NEXT:  store ptr %tmp10, ptr %arrayidx13, align 8
; CHECK-MOD-NEXT:  store float* %copy, float** %elem1, align 8
; CHECK-MOD-NEXT:  store i8* %tmp10, i8** %elem3, align 8
; CHECK-OP-MOD-NEXT:  store ptr %copy, ptr %elem1, align 8
; CHECK-OP-MOD-NEXT:  store ptr %tmp10, ptr %elem3, align 8
  store i8* %tmp10, i8** %arrayidx13, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc14 = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %mem15 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  %tmp11 = load %struct.Mem*, %struct.Mem** %mem15, align 8
  %base16 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:  %tmp12 = load i8**, i8*** %base16, align 8
; CHECK-OP-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-OP-TRANS-NEXT:  %tmp12 = load ptr, ptr %base16, align 8
; CHECK-MOD: %tmp12 = load %__SOA_EL_struct.Arr.0*, %__SOA_EL_struct.Arr.0** %base16, align 8
; CHECK-OP-MOD: %tmp12 = load ptr, ptr %base16, align 8
  %tmp12 = load i8**, i8*** %base16, align 8
  %tmp13 = bitcast i8** %tmp12 to i8*
  call void @_ZdlPv(i8* %tmp13)
  %base19 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-TRANS-NEXT:  store i8** %tmp6, i8*** %base19, align 8
; CHECK-OP-TRANS:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-OP-TRANS-NEXT:  store ptr %tmp6, ptr %base19, align 8
; CHECK-MOD:         store %__SOA_EL_struct.Arr.0* %tmp6, %__SOA_EL_struct.Arr.0** %base19, align 8
; CHECK-OP-MOD:         store ptr %tmp6, ptr %base19, align 8
  store i8** %tmp6, i8*** %base19, align 8
  br label %return

return:                                           ; preds = %for.end, %if.then
  ret void
}

declare !intel.dtrans.func.type !12 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !13 dso_local void @_ZdlPv(i8* "intel_dtrans_func_index"="1")

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!intel.dtrans.types = !{!0, !4}

!0 = !{!"S", %struct.Mem zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %struct.Arr.0 zeroinitializer, i32 6, !5, !3, !6, !8, !3, !6}
!5 = !{%struct.Mem zeroinitializer, i32 1}
!6 = !{!"A", i32 4, !7}
!7 = !{i8 0, i32 0}
!8 = !{i8 0, i32 2}
!9 = distinct !{!10}
!10 = !{%struct.Arr.0 zeroinitializer, i32 1}
!11 = !{i8 0, i32 1}
!12 = distinct !{!11}
!13 = distinct !{!11}

