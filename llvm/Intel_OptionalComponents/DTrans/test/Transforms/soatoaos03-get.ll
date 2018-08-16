; RUN: opt -S < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-approx-typename=struct.Arr.0                                                         \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos -dtrans-free-functions=struct.Mem,1 \
; RUN:  2>&1 | FileCheck %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

$_ZN3ArrIPvE3getEi = comdat any

; The following method should be classified as get-like method.
;   S* get(int i) {
;     if (size > 7)
;       return base + i * i + 1;
;     return base + i;
;   }
; CHECK:      Checking array's method _ZN3ArrIPvE3getEi
; CHECK-NEXT: Classification: Get pointer to element method
define i8** @_ZN3ArrIPvE3getEi(%struct.Arr.0* nocapture readonly %this, i32 %i) #0 comdat align 2 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp = load i32, i32* %size, align 8
  %cmp = icmp sgt i32 %tmp, 7
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp1 = load i8**, i8*** %base, align 8
  %mul = mul nsw i32 %i, %i
  %idx.ext = sext i32 %mul to i64
  %add.ptr = getelementptr inbounds i8*, i8** %tmp1, i64 %idx.ext
  %add.ptr2 = getelementptr inbounds i8*, i8** %add.ptr, i64 1
  br label %return

if.end:                                           ; preds = %entry
  %base3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp2 = load i8**, i8*** %base3, align 8
  %idx.ext4 = sext i32 %i to i64
  %add.ptr5 = getelementptr inbounds i8*, i8** %tmp2, i64 %idx.ext4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i8** [ %add.ptr2, %if.then ], [ %add.ptr5, %if.end ]
  ret i8** %retval.0
}

attributes #0 = { noinline norecurse nounwind readonly uwtable }
