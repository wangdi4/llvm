; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos-arrays -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume                                                                        \
; RUN:    -passes=soatoaos-arrays-methods-transform                                                             \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -dtrans-malloc-functions=struct.Mem,0                                                                 \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }
; CHECK-MOD: %__SOA_struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], %__SOA_EL_struct.Arr.0*, i32, [4 x i8] }>
; CHECK-MOD: %__SOA_EL_struct.Arr.0 = type { float*, i8* }

; The following method should be classified as copy-ctor.
; Instructions to transform are shown.
; Transformed instructions are shown.
;   Arr(const Arr &A) {
;     mem = A.mem;
;     capacilty = A.capacilty;
;     size = A.size;
;     base = (S *)mem->allocate(size + capacilty * sizeof(S));
;     for (int i = 0; i < size; ++i)
;       base[size + i] = A.base[i];
;   }
; CHECK:      Checking array's method _ZN3ArrIPvEC2ERKS1_
; CHECK-NEXT: Classification: CCtor method
; CHECK-TRANS:; Dump instructions needing update. Total = 8
define void @_ZN3ArrIPvEC2ERKS1_(%struct.Arr.0* %this, %struct.Arr.0* %A) {
entry:
; CHECK-MOD:  %mem = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %A, i32 0, i32 0
  %mem = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
; CHECK-MOD:  %mem2 = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %this, i32 0, i32 0
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  store %struct.Mem* %tmp, %struct.Mem** %mem2, align 8
; CHECK-MOD:  %capacilty = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %A, i32 0, i32 1
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 1
  %tmp1 = load i32, i32* %capacilty, align 8
; CHECK-MOD:  %capacilty3 = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %this, i32 0, i32 1
  %capacilty3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  store i32 %tmp1, i32* %capacilty3, align 8
; CHECK-MOD:  %size = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %A, i32 0, i32 4
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 4
  %tmp2 = load i32, i32* %size, align 8
; CHECK-MOD:  %size4 = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %this, i32 0, i32 4
  %size4 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  store i32 %tmp2, i32* %size4, align 8
; CHECK-MOD:  %mem5 = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %this, i32 0, i32 0
  %mem5 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  %tmp3 = load %struct.Mem*, %struct.Mem** %mem5, align 8
; CHECK-MOD:  %size6 = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %this, i32 0, i32 4
  %size6 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp4 = load i32, i32* %size6, align 8
  %conv = sext i32 %tmp4 to i64
; CHECK-MOD:  %capacilty7 = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %this, i32 0, i32 1
  %capacilty7 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  %tmp5 = load i32, i32* %capacilty7, align 8
  %conv8 = sext i32 %tmp5 to i64
  %mul = mul i64 %conv8, 8
  %add = add i64 %conv, %mul
  %conv9 = trunc i64 %add to i32
  %tmp6 = bitcast %struct.Mem* %tmp3 to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp6, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp7 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
; CHECK-TRANS:      ; BasePtrInst: Allocation call
; CHECK-TRANS-NEXT:   %call = call i8* %tmp7(%struct.Mem* %tmp3, i32 %conv9)
; CHECK-MOD:  %nsz = zext i32 %conv9 to i64
; CHECK-MOD-NEXT:  %nsz1 = mul nuw i64 %nsz, 2
; CHECK-MOD-NEXT:  %nsz2 = trunc i64 %nsz1 to i32
; CHECK-MOD-NEXT:  %call = call i8* %tmp7(%struct.Mem* %tmp3, i32 %nsz2)
  %call = call i8* %tmp7(%struct.Mem* %tmp3, i32 %conv9)
; CHECK-MOD:  %tmp8 = bitcast i8* %call to %__SOA_EL_struct.Arr.0*
  %tmp8 = bitcast i8* %call to i8**
; CHECK-MOD:  %base = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %this, i32 0, i32 3
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Init base pointer with allocated memory
; CHECK-TRANS-NEXT:   store i8** %tmp8, i8*** %base, align 8
; CHECK-MOD:  store %__SOA_EL_struct.Arr.0* %tmp8, %__SOA_EL_struct.Arr.0** %base, align 8
  store i8** %tmp8, i8*** %base, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
; CHECK-MOD:  %size10 = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %this, i32 0, i32 4
  %size10 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp9 = load i32, i32* %size10, align 8
  %cmp = icmp slt i32 %i.0, %tmp9
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
; CHECK-MOD:  %base11 = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %A, i32 0, i32 3
  %base11 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:   %tmp10 = load i8**, i8*** %base11, align 8
; CHECK-MOD:  %tmp10 = load %__SOA_EL_struct.Arr.0*, %__SOA_EL_struct.Arr.0** %base11, align 8
  %tmp10 = load i8**, i8*** %base11, align 8
  %idxprom = sext i32 %i.0 to i64
; CHECK-TRANS:      ; MemInstGEP: Element load
; CHECK-TRANS-NEXT:   %arrayidx = getelementptr inbounds i8*, i8** %tmp10, i64 %idxprom
; CHECK-MOD:          %arrayidx = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %tmp10, i64 %idxprom
  %arrayidx = getelementptr inbounds i8*, i8** %tmp10, i64 %idxprom
; CHECK-MOD-NEXT:     %elem4 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:     %elem = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx, i64 0, i32 0
; CHECK-TRANS:      ; MemInst: Element load
; CHECK-TRANS-NEXT:   %tmp11 = load i8*, i8** %arrayidx, align 8
; CHECK-MOD-NEXT:     %copy = load float*, float** %elem, align 8
; CHECK-MOD-NEXT:     %tmp11 = load i8*, i8** %elem4, align 8
  %tmp11 = load i8*, i8** %arrayidx, align 8
; CHECK-MOD:  %base12 = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %this, i32 0, i32 3
  %base12 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:   %tmp12 = load i8**, i8*** %base12, align 8
; CHECK-MOD:  %tmp12 = load %__SOA_EL_struct.Arr.0*, %__SOA_EL_struct.Arr.0** %base12, align 8
  %tmp12 = load i8**, i8*** %base12, align 8
; CHECK-MOD:  %size13 = getelementptr inbounds %__SOA_struct.Arr.0, %__SOA_struct.Arr.0* %this, i32 0, i32 4
  %size13 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp13 = load i32, i32* %size13, align 8
  %add14 = add nsw i32 %tmp13, %i.0
  %idxprom15 = sext i32 %add14 to i64
; CHECK-TRANS:      ; MemInstGEP: Element copy
; CHECK-TRANS-NEXT:   %arrayidx16 = getelementptr inbounds i8*, i8** %tmp12, i64 %idxprom15
; CHECK-MOD:          %arrayidx16 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %tmp12, i64 %idxprom15
  %arrayidx16 = getelementptr inbounds i8*, i8** %tmp12, i64 %idxprom15
; CHECK-MOD-NEXT:   %elem5 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx16, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem3 = getelementptr inbounds %__SOA_EL_struct.Arr.0, %__SOA_EL_struct.Arr.0* %arrayidx16, i64 0, i32 0
; CHECK-TRANS:      ; MemInst: Element copy
; CHECK-TRANS-NEXT:   store i8* %tmp11, i8** %arrayidx16, align 8
; CHECK-MOD-NEXT:   store float* %copy, float** %elem3, align 8
; CHECK-MOD-NEXT:   store i8* %tmp11, i8** %elem5, align 8
  store i8* %tmp11, i8** %arrayidx16, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}
