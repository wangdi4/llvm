; It checks anders-aa helps to detect a and b array pointers in _Z3bazv never
; overlap since they are allocated with two different new [] calls.
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s


@_Z2p1 = internal global ptr null, align 8
@_Z2p2 = internal global ptr null, align 8

; Function Attrs: uwtable
define ptr @_Z3foov() {
entry:
  %call = call noalias ptr @_Znam(i64 20) 
  %0 = bitcast ptr %call to ptr
  ret ptr %0
}

; Function Attrs: nobuiltin
declare noalias ptr @_Znam(i64)

; Function Attrs: uwtable
define ptr @_Z3barv()  {
entry:
  %call = call noalias ptr @_Znam(i64 28) 
  %0 = bitcast ptr %call to ptr
  ret ptr %0
}

; CHECK: Function: _Z3bazv
; CHECK:   NoAlias:      i32* %a, i32* %b

; Function Attrs: uwtable
define void @_Z3bazv() {
entry:
  %call = call ptr @_Z3foov()
  store ptr %call, ptr @_Z2p1, align 8
  %call1 = call ptr @_Z3barv()
  store ptr %call1, ptr @_Z2p2, align 8
  %a = load ptr, ptr @_Z2p1, align 8
  %add.ptr = getelementptr inbounds i32, ptr %a, i64 3
  store i32 17, ptr %add.ptr, align 4
  %b = load ptr, ptr @_Z2p2, align 8
  %add.ptr2 = getelementptr inbounds i32, ptr %b, i64 2
  store i32 18, ptr %add.ptr2, align 4

; dead loads, needed to get aa-eval to trigger
  %ld.b = load i32, ptr %b, align 8
  %ld.a = load i32, ptr %a, align 8

  ret void
}
