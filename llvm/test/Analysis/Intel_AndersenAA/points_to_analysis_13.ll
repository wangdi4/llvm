; It checks anders-aa helps to detect a and b pointers in _Z3bazv never overlap 
; since they are allocated with two different new calls.
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

@p1 = internal unnamed_addr global ptr null, align 8
@p2 = internal global ptr null, align 8

; Function Attrs: uwtable
define ptr @_Z3foov() {
entry:
  %call = call noalias ptr @_Znwm(i64 4) 
  %0 = bitcast ptr %call to ptr
  ret ptr %0
}

; Function Attrs: nobuiltin
declare noalias ptr @_Znwm(i64) 

; Function Attrs: uwtable
define ptr @_Z3barv()  {
entry:
  %call = call noalias ptr @_Znwm(i64 4) 
  %0 = bitcast ptr %call to ptr
  ret ptr %0
}

; CHECK: Function: _Z3bazv
; CHECK:   NoAlias:      i32* %a, i32* %b

; Function Attrs: uwtable
define void @_Z3bazv()  {
entry:
  %call = call ptr @_Z3foov()
  store ptr %call, ptr @p1, align 8
  %call1 = call ptr @_Z3barv()
  store ptr %call1, ptr @p2, align 8
  %a = load ptr, ptr @p1, align 8
  store i32 17, ptr %a, align 4
  %b = load ptr, ptr @p2, align 8
  store i32 18, ptr %b, align 4
  ret void
}

