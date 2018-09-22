; RUN: opt -S < %s -dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume -debug-only=dtrans-soatoaos -dtrans-soatoaos-typename=noname -disable-output 2>&1 | FileCheck %s
; RUN: opt -S < %s -passes=dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume -debug-only=dtrans-soatoaos -dtrans-soatoaos-typename=noname -disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts

; This test check essential layout requirements. See comments inlined.
;
; Optional memory interface (only vtable).
; struct Mem {
;   virtual void *allocate() = 0;
;   virtual void *deallocate() = 0;
; };
;
; struct S1;
; struct S2;
;
; _Array_ like structure in 'SOA'.
; base pointer, at least 2 interger fields, optional memory interface.
; template <typename S> struct Arr {
;   Mem *mem;
;   int capacilty;
;   int size;
;   S **base;
; };
;
; template <typename S>
; struct Arr1 : public Arr<S> {
; };
;
; _Structure_ in 'SOA'. Arrays should be verysimilar in layout.
; Some arrays like Arr1 are rejected for non-trivial base class (or vtable).
; Optional memory interface is the same in _structure_ and _arrays_.
;
; class F {
;   Mem *mem;
; public:
;   Arr<int>* f1;
;   Arr<void*>* f2;
;   Arr1<float>* f3;
; };
;
; int main() {
;   F *f = new F();
;   f->f1 = new Arr<int>();
;   f->f2 = new Arr<void*>();
;   f->f3 = new Arr1<float>();
; }
; CHECK: Rejecting %class.F because it does not look like a candidate from CFG analysis.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type { %struct.Mem*, i32, i32, i32** }
%struct.Arr.0 = type { %struct.Mem*, i32, i32, i8*** }
%struct.Arr1 = type { %struct.Arr.1 }
%struct.Arr.1 = type { %struct.Mem*, i32, i32, float** }

define dso_local i32 @main() #0 {
entry:
  %f = alloca %class.F*, align 8
  %call = call i8* @_Znwm(i64 32) #3
  %0 = bitcast i8* %call to %class.F*
  %1 = bitcast %class.F* %0 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %1, i8 0, i64 32, i1 false)
  store %class.F* %0, %class.F** %f, align 8
  %call1 = call i8* @_Znwm(i64 24) #3
  %2 = bitcast i8* %call1 to %struct.Arr*
  %3 = bitcast %struct.Arr* %2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %3, i8 0, i64 24, i1 false)
  %4 = load %class.F*, %class.F** %f, align 8
  %f1 = getelementptr inbounds %class.F, %class.F* %4, i32 0, i32 1
  store %struct.Arr* %2, %struct.Arr** %f1, align 8
  %call2 = call i8* @_Znwm(i64 24) #3
  %5 = bitcast i8* %call2 to %struct.Arr.0*
  %6 = bitcast %struct.Arr.0* %5 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %6, i8 0, i64 24, i1 false)
  %7 = load %class.F*, %class.F** %f, align 8
  %f2 = getelementptr inbounds %class.F, %class.F* %7, i32 0, i32 2
  store %struct.Arr.0* %5, %struct.Arr.0** %f2, align 8
  %call3 = call i8* @_Znwm(i64 24) #3
  %8 = bitcast i8* %call3 to %struct.Arr1*
  %9 = bitcast %struct.Arr1* %8 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %9, i8 0, i64 24, i1 false)
  %10 = load %class.F*, %class.F** %f, align 8
  %f3 = getelementptr inbounds %class.F, %class.F* %10, i32 0, i32 3
  store %struct.Arr1* %8, %struct.Arr1** %f3, align 8
  ret i32 0
}

declare dso_local noalias i8* @_Znwm(i64) #1

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #2

attributes #1 = { nobuiltin }
attributes #2 = { argmemonly nounwind }
attributes #3 = { builtin }

