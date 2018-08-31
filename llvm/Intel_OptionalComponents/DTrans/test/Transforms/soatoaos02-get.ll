; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:      -debug-only=dtrans-soatoaos-deps \
; RUN:      -passes='require<dtransanalysis>,function(require<soatoaos-approx>)' \
; RUN:      2>&1 | FileCheck %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

; This test checks various approximations for side effects in get-like function.
;   S get(int i) { return base[i]; }
define i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* %this, i32 %i) {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i8*, i8** %tmp, i64 %idxprom
; CHECK:      Load(Func(Arg 1)
; CHECK-NEXT:          (Load(GEP(Arg 0)
; CHECK-NEXT:                    3)))
; CHECK-NEXT: %get = load i8*, i8** %arrayidx, align 8
  %get = load i8*, i8** %arrayidx, align 8
; Return is represent as its operand
; CHECK-NEXT: Load(Func(Arg 1)
; CHECK-NEXT:          (Load(GEP(Arg 0)
; CHECK-NEXT:                    3)))
; CHECK-NEXT: ret i8* %get
  ret i8* %get
}

; CHECK: Deps computed: 7, Queries: 7
; CHECK-NOT: Unknown Dep
