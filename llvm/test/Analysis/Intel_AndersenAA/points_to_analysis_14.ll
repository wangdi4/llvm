; It checks anders-aa helps to detect a and b array pointers in _Z3bazv never
; overlap since they are allocated with two different new [] calls.
; RUN: opt < %s -anders-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -anders-aa -aa-eval -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -convert-to-subscript -S | opt -anders-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -convert-to-subscript -S | opt -anders-aa -aa-eval -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s


@_Z2p1 = internal global i32* null, align 8
@_Z2p2 = internal global i32* null, align 8

; Function Attrs: uwtable
define i32* @_Z3foov() {
entry:
  %call = call noalias i8* @_Znam(i64 20) 
  %0 = bitcast i8* %call to i32*
  ret i32* %0
}

; Function Attrs: nobuiltin
declare noalias i8* @_Znam(i64)

; Function Attrs: uwtable
define i32* @_Z3barv()  {
entry:
  %call = call noalias i8* @_Znam(i64 28) 
  %0 = bitcast i8* %call to i32*
  ret i32* %0
}

; CHECK: Function: _Z3bazv
; CHECK:   NoAlias:      i32* %a, i32* %b

; Function Attrs: uwtable
define void @_Z3bazv() {
entry:
  %call = call i32* @_Z3foov()
  store i32* %call, i32** @_Z2p1, align 8
  %call1 = call i32* @_Z3barv()
  store i32* %call1, i32** @_Z2p2, align 8
  %a = load i32*, i32** @_Z2p1, align 8
  %add.ptr = getelementptr inbounds i32, i32* %a, i64 3
  store i32 17, i32* %add.ptr, align 4
  %b = load i32*, i32** @_Z2p2, align 8
  %add.ptr2 = getelementptr inbounds i32, i32* %b, i64 2
  store i32 18, i32* %add.ptr2, align 4

; dead loads, needed to get aa-eval to trigger
  %ld.b = load i32, i32* %b, align 8
  %ld.a = load i32, i32* %a, align 8

  ret void
}
