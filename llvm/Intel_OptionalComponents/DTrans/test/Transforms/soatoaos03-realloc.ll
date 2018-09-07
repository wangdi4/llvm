; RUN: opt < %s -whole-program-assume -disable-output                                                           \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output                                                           \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos-arrays                                                                    \
; RUN:    -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1                             \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume                                                                        \
; RUN:    -passes=soatoaos-arrays-methods-transform                                                             \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1                             \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }
; CHECK-MOD-DAG: %__SOA_struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], %__SOA_EL_struct.Arr.0*, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr.0 = type { float*, i8* }

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
define void @_ZN3ArrIPvE7reallocEi(%struct.Arr.0* %this, i32 %inc) {
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
  %conv5 = trunc i64 %mul4 to i32
  %tmp4 = bitcast %struct.Mem* %tmp2 to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp4, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp5 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
; CHECK-TRANS:     ; BasePtrInst: Allocation call
; CHECK-TRANS-NEXT:  %call = call i8* %tmp5(%struct.Mem* %tmp2, i32 %conv5)
; CHECK-MOD:       %nsz = zext i32 %conv5 to i64
; CHECK-MOD-NEXT:  %nsz1 = mul nuw i64 %nsz, 2
; CHECK-MOD-NEXT:  %nsz2 = trunc i64 %nsz1 to i32
; CHECK-MOD-NEXT:  %call = call i8* %tmp5(%struct.Mem* %tmp2, i32 %nsz2)
  %call = call i8* %tmp5(%struct.Mem* %tmp2, i32 %conv5)
; CHECK-MOD: %tmp6 = bitcast i8* %call to %__SOA_EL_struct.Arr.0*
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
; CHECK-MOD: %tmp9 = load %__SOA_EL_struct.Arr.0*, %__SOA_EL_struct.Arr.0** %base, align 8
  %tmp9 = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i.0 to i64
; CHECK-TRANS:     ; MemInstGEP: Element load
; CHECK-TRANS-NEXT:  %arrayidx1 = getelementptr inbounds i8*, i8** %tmp9, i64 0
; CHECK-MOD:         %arrayidx1 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %tmp9, i64 0
  %arrayidx1 = getelementptr inbounds i8*, i8** %tmp9, i64 0
  br label %for.stop

for.stop:                                         ; preds = %for.body
; CHECK-TRANS:     ; MemInstGEP: Element load
; CHECK-TRANS-NEXT:  %array_phi = phi i8** [ %arrayidx1, %for.body ]
; CHECK-MOD:         %array_phi = phi %__SOA_EL_struct.Arr.0* [ %arrayidx1, %for.body ]
  %array_phi = phi i8** [ %arrayidx1, %for.body ]
; CHECK-TRANS:     ; MemInstGEP: Element load
; CHECK-TRANS-NEXT:  %arrayidx = getelementptr inbounds i8*, i8** %array_phi, i64 %idxprom
; CHECK-MOD:         %arrayidx = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %array_phi, i64 %idxprom
  %arrayidx = getelementptr inbounds i8*, i8** %array_phi, i64 %idxprom
; CHECK-MOD-NEXT:    %elem4 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:    %elem = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx, i64 0, i32 0
; CHECK-TRANS:     ; MemInst: Element load
; CHECK-TRANS-NEXT:  %tmp10 = load i8*, i8** %arrayidx, align 8
; CHECK-MOD:        %copy = load float*, float** %elem, align 8
; CHECK-MOD-NEXT:   %tmp10 = load i8*, i8** %elem4, align 8
  %tmp10 = load i8*, i8** %arrayidx, align 8
  %mul11 = mul nsw i32 5, %i.0
  %idxprom12 = sext i32 %mul11 to i64
; CHECK-TRANS:     ; MemInstGEP: Element store to new mem
; CHECK-TRANS-NEXT:  %arrayidx13 = getelementptr inbounds i8*, i8** %tmp6, i64 %idxprom12
; CHECK-MOD:         %arrayidx13 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %tmp6, i64 %idxprom12
  %arrayidx13 = getelementptr inbounds i8*, i8** %tmp6, i64 %idxprom12
; CHECK-MOD-NEXT:    %elem5 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx13, i64 0, i32 1
; CHECK-MOD-NEXT:    %elem3 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx13, i64 0, i32 0
; CHECK-TRANS:     ; MemInst: Element store to new mem
; CHECK-TRANS-NEXT:  store i8* %tmp10, i8** %arrayidx13, align 8
; CHECK-MOD-NEXT:  store float* %copy, float** %elem3, align 8
; CHECK-MOD-NEXT:  store i8* %tmp10, i8** %elem5, align 8
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
; CHECK-MOD: %tmp12 = load %__SOA_EL_struct.Arr.0*, %__SOA_EL_struct.Arr.0** %base16, align 8
  %tmp12 = load i8**, i8*** %base16, align 8
  %tmp13 = bitcast i8** %tmp12 to i8*
  %tmp14 = bitcast %struct.Mem* %tmp11 to void (%struct.Mem*, i8*)***
  %vtable17 = load void (%struct.Mem*, i8*)**, void (%struct.Mem*, i8*)*** %tmp14, align 8
  %vfn18 = getelementptr inbounds void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vtable17, i64 1
  %tmp15 = load void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vfn18, align 8
  call void %tmp15(%struct.Mem* %tmp11, i8* %tmp13)
  %base19 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-TRANS-NEXT:  store i8** %tmp6, i8*** %base19, align 8
; CHECK-MOD:         store %__SOA_EL_struct.Arr.0* %tmp6, %__SOA_EL_struct.Arr.0** %base19, align 8
  store i8** %tmp6, i8*** %base19, align 8
  br label %return

return:                                           ; preds = %for.end, %if.then
  ret void
}
