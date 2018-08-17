; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos -dtrans-free-functions=struct.Mem,1 \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos-arrays -dtrans-free-functions=struct.Mem,1 \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

; The following method should be classified as get-like method.
; Instructions to transform are shown.
;   S* get(int i) {
;     if (size > 7)
;       return base + i * i + 1;
;     return base + i;
;   }
; CHECK:      Checking array's method _ZN3ArrIPvE3getEi
; CHECK-NEXT: Classification: Get pointer to element method
; CHECK-TRANS:; Dump instructions needing update. Total = 6
define i8** @_ZN3ArrIPvE3getEi(%struct.Arr.0* %this, i32 %i) {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp = load i32, i32* %size, align 8
  %cmp = icmp sgt i32 %tmp, 7
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:   %tmp1 = load i8**, i8*** %base, align 8
  %tmp1 = load i8**, i8*** %base, align 8
  %mul = mul nsw i32 %i, %i
  %idx.ext = sext i32 %mul to i64
; CHECK-TRANS:     ; MemInstGEP: Address in ret
; CHECK-TRANS-NEXT:  %add.ptr = getelementptr inbounds i8*, i8** %tmp1, i64 %idx.ext
  %add.ptr = getelementptr inbounds i8*, i8** %tmp1, i64 %idx.ext
; CHECK-TRANS:     ; MemInstGEP: Address in ret
; CHECK-TRANS-NEXT:  %add.ptr2 = getelementptr inbounds i8*, i8** %add.ptr, i64 1
  %add.ptr2 = getelementptr inbounds i8*, i8** %add.ptr, i64 1
  br label %return

if.end:                                           ; preds = %entry
  %base3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:  %tmp2 = load i8**, i8*** %base3, align 8
  %tmp2 = load i8**, i8*** %base3, align 8
  %idx.ext4 = sext i32 %i to i64
; CHECK-TRANS:     ; MemInstGEP: Address in ret
; CHECK-TRANS-NEXT:  %add.ptr5 = getelementptr inbounds i8*, i8** %tmp2, i64 %idx.ext4
  %add.ptr5 = getelementptr inbounds i8*, i8** %tmp2, i64 %idx.ext4
  br label %return

return:                                           ; preds = %if.end, %if.then
; CHECK-TRANS:     ; MemInstGEP: Address in ret
; CHECK-TRANS-NEXT:  %retval.0 = phi i8** [ %add.ptr2, %if.then ], [ %add.ptr5, %if.end ]
  %retval.0 = phi i8** [ %add.ptr2, %if.then ], [ %add.ptr5, %if.end ]
  ret i8** %retval.0
}
