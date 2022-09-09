; Test verifies that dummyAlloc is treated as dummy allocation function, which
; actually doesn't allocate any memory and just calls _CxxThrowException.
; This test is verified only on Windows.

; REQUIRES: system-windows
; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,require<soatoaos-approx>,function(require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos 2>&1 | FileCheck %s

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...) ** }
%struct.MemImpl = type { %struct.Mem*, i32, i32, i8** }
%struct.bad_alloc = type { %struct.exception }
%struct.exception = type { i32 (...)** }

%"class.std::bad_alloc" = type { %"class.std::exception" }
%"class.std::exception" = type { i32 (...)**, %struct.__std_exception_data }
%struct.__std_exception_data = type { i8*, i8 }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

; CHECK-MOD-DAG: %__SOA_struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], %__SOA_EL_struct.Arr.0*, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr.0 = type { float*, i8* }

; The following method should be classified as copy-ctor.
;   Arr(const Arr &A) {
;     mem = A.mem;
;     capacilty = A.capacilty;
;     size = A.size;
;     base = (S *)mem->allocate(size + capacilty * sizeof(S));
;     for (int i = 0; i < size; ++i)
;       base[size + i] = A.base[i];
;   }
; The test specifically checks for dummy allocation function.
; CHECK:      Checking array's method _ZN3ArrIPvEC2ERKS1_
; CHECK-NEXT: Classification: CCtor method
define void @_ZN3ArrIPvEC2ERKS1_(%struct.Arr.0* %this, %struct.Arr.0* %A) {
entry:
  %mem = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  store %struct.Mem* %tmp, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 1
  %tmp1 = load i32, i32* %capacilty, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  store i32 %tmp1, i32* %capacilty3, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 4
  %tmp2 = load i32, i32* %size, align 8
  %size4 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  store i32 %tmp2, i32* %size4, align 8
  %mem5 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  %tmp3 = load %struct.Mem*, %struct.Mem** %mem5, align 8
  %size6 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp4 = load i32, i32* %size6, align 8
  %conv = sext i32 %tmp4 to i64
  %capacilty7 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  %tmp5 = load i32, i32* %capacilty7, align 8
  %conv8 = sext i32 %tmp5 to i64
  %mul = mul i64 %conv8, 8
  %add = add i64 %conv, %mul
  %conv9 = trunc i64 %add to i32
  %call = call i8* bitcast (i8* (%struct.MemImpl*, i32)* @dummyAlloc to i8* (%struct.Mem*, i32)*)(%struct.Mem* nonnull %tmp3, i32 %conv9)
  %tmp8 = bitcast i8* %call to i8**
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  store i8** %tmp8, i8*** %base, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %size10 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp9 = load i32, i32* %size10, align 8
  %cmp = icmp slt i32 %i.0, %tmp9
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %base11 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 3
  %tmp10 = load i8**, i8*** %base11, align 8
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i8*, i8** %tmp10, i64 %idxprom
  %tmp11 = load i8*, i8** %arrayidx, align 8
  %base12 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp12 = load i8**, i8*** %base12, align 8
  %size13 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp13 = load i32, i32* %size13, align 8
  %add14 = add nsw i32 %tmp13, %i.0
  %idxprom15 = sext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds i8*, i8** %tmp12, i64 %idxprom15
  store i8* %tmp11, i8** %arrayidx16, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define internal i8* @dummyAlloc(%struct.MemImpl* %this, i32 %conv4) {
entry:
  %a = alloca %"class.std::bad_alloc", align 8
  %g1 = getelementptr inbounds %"class.std::bad_alloc", %"class.std::bad_alloc"* %a, i64 0, i32 0, i32 1, i32 1
  %bc1 = bitcast i8* %g1 to i64*
  store i64 0, i64* %bc1, align 8
  %g3 = getelementptr inbounds %"class.std::bad_alloc", %"class.std::bad_alloc"* %a, i64 0, i32 0, i32 1, i32 0
  store i8* getelementptr inbounds ([15 x i8], [15 x i8]* null, i64 0, i64 0), i8** %g3, align 8
  %g2 = getelementptr inbounds %"class.std::bad_alloc", %"class.std::bad_alloc"* %a, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** null to i32 (...)**), i32 (...)*** %g2, align 8
  %bc2 = bitcast %"class.std::bad_alloc"* %a to i8*
  call void @_CxxThrowException(i8* %bc2, %eh.ThrowInfo* null)
  unreachable
}

declare noalias i8* @malloc(i32)
declare void @_CxxThrowException(i8*, %eh.ThrowInfo*)
