; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that we create a correct LocalPointerInfo for bitcast corresponding to zero element access.

%struct.mystruct = type { i32, i32 }
%struct.nextstruct = type { i8*, i64 }

define internal i32 @foo(%struct.nextstruct* nocapture) {
  %base = getelementptr inbounds %struct.nextstruct, %struct.nextstruct* %0, i64 0, i32 0
  %ld = load i8*, i8** %base, align 8
  %cmp = icmp eq i8* %ld, null
  br i1 %cmp, label %3, label %2

; <label>:2:
  %bc1 = bitcast %struct.nextstruct* %0 to %struct.mystruct**
  br label %4

; <label>:3:
  %malloc = tail call i8* @malloc(i64 16)
  store i8* %malloc, i8** %base, align 8
  %bc2 = bitcast %struct.nextstruct* %0 to %struct.mystruct**
  br label %4

; <label>:4:
  %res = phi %struct.mystruct** [ %bc1, %2 ], [ %bc2, %3 ]
  ret i32 0
}

declare noalias i8* @malloc(i64)

; CHECK-LABEL:  LLVMType: %struct.mystruct = type { i32, i32 }
; CHECK: Bad casting
; CHECK-NOT: Unsafe pointer merge

; CHECK-LABEL:  LLVMType: %struct.nextstruct = type { i8*, i64 }
; CHECK-NOT: Bad casting
; CHECK-NOT: Unsafe pointer merge
