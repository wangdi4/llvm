; RUN: opt -subscript -scalarize -verify -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check scalarizer uses getTypeAllocSize function 
;; Checked types:
;;                <3 x i32> --> 4 elements
;;                <2 x  i1> --> 2 elements
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

define void @func1(i64* %A, <3 x i32>* %C) nounwind {
entry:
  %addr1 = getelementptr <3 x i32>, <3 x i32>* %C, i32 1
  %x = load <3 x i32>, <3 x i32>* %addr1
  br label %BB1

BB1:
  %addr2 = getelementptr <3 x i32>, <3 x i32>* %C, i32 2
  store <3 x i32> %x, <3 x i32>* %addr2
  br label %end

end:
  ret void
  ;CHECK: define void @func1
;CHECK: entry:
;CHECK: = mul i32 1, 4
;CHECK: = mul i32 1, 4
;CHECK: = mul i32 1, 4
;CHECK: BB1:
;CHECK: = mul i32 2, 4
;CHECK: = mul i32 2, 4
;CHECK: = mul i32 2, 4
;CHECK: ret
}


define void @func2(i64* %A, <2 x i1>* %C) nounwind {
entry:
  %addr1 = getelementptr <2 x i1>, <2 x i1>* %C, i32 1
  %x = load <2 x i1>, <2 x i1>* %addr1
  br label %BB1

BB1:
  %addr2 = getelementptr <2 x i1>, <2 x i1>* %C, i32 2
  store <2 x i1> %x, <2 x i1>* %addr2
  br label %end

end:
  ret void
;CHECK: define void @func2
;CHECK: entry:
;CHECK: = mul i32 1, 2
;CHECK: = mul i32 1, 2
;CHECK: BB1:
;CHECK: = mul i32 2, 2
;CHECK: = mul i32 2, 2
;CHECK: ret
}