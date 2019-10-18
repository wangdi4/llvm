; This testcase verifies that MemInitTrimDown transformation is able
; to detect possible candidates and collect member functions of potential
; candidate vector fields.

; RUN: opt < %s -dtrans-meminittrimdown -enable-dtrans-meminittrimdown -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdown -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=dtrans-meminittrimdown -enable-dtrans-meminittrimdown -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdown -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" will be considered as candidate vector fields.
; Definitions of most of the member functions are empty or doesn't make
; sense.

; struct Mem {
;   virtual void *allocate() = 0;
;   virtual void *deallocate() = 0;
; };
;
; foo, which is an alias to bar, is called in F routine to make sure
; MemInitTrimDown is able to handle aliases.
;
; void bar(void) {
;   return;
; }
;
; // Note that mangled name _Z3barv is used instead of bar to make
; // clang to compile this code without syntax error.
; void foo(void) __attribute__((alias("_Z3barv")));
;
;template <typename S> struct Arr {
;   bool flag;
;   int capacity;
;   int size;
;   S **base;
;   Mem *mem;
;   void resize(void) {
;   }
;   void add(S* val) {
;     resize();
;   }
;  void set(int i, S* val) { }
;  Arr(int c = 1, Mem *mem = 0)
;      : mem(mem), capacity(c), size(0), base(0) {}
;  ~Arr() {}
; };
;
; template <typename S>
; struct Arr1 : public Arr<S> {
; };
;
; class F {
;   Mem *mem;
; public:
;   Arr<int*>* f1;
;   Arr1<float*>* f2;
;   F() {
;     f1 = new Arr<int *>(10, nullptr);
;     f2 = new Arr1<float *>();
;     f1->set(0, nullptr);
;     f2->set(0, nullptr);
;     f2->add(nullptr);
;     f1->add(nullptr);
;     foo();
;   }
;
; };
; int main() {
;   F *f = new F();
; }
;

;CHECK: MemInitTrimDown transformation:
;CHECK:  Possible candidate structs:
;CHECK:    Candidate: class.F
;CHECK: Member functions for 1 candidate field:
;CHECK-DAG:       _ZN3ArrIPiE3setEiPS0_
;CHECK-DAG:       _ZN3ArrIPiEC2EiP3Mem
;CHECK-DAG:       _ZN3ArrIPiE3addEPS0_
;CHECK-DAG:       _ZN3ArrIPiE6resizeEv

;CHECK:  Member functions for 2 candidate field:
;CHECK-DAG:       _ZN4Arr1IPfEC2Ev
;CHECK-DAG:       _ZN3ArrIPfEC2EiP3Mem
;CHECK-DAG:       _ZN3ArrIPfE3setEiPS0_
;CHECK-DAG:       _ZN3ArrIPfE3addEPS0_
;CHECK-DAG:       _ZN3ArrIPfE6resizeEv
;

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type { i8, i32, i32, i32***, %struct.Mem* }
%struct.Arr1 = type { %struct.Arr.0 }
%struct.Arr.0 = type { i8, i32, i32, float***, %struct.Mem* }
@_Z3foov = internal unnamed_addr alias void (), void ()* @_Z3barv

define dso_local i32 @main() {
entry:
  %call = call i8* @_Znwm(i64 24)
  %0 = bitcast i8* %call to %class.F*
  call void @_ZN1FC2Ev(%class.F* %0)
  ret i32 0
}

define void @_ZN1FC2Ev(%class.F* %this) {
entry:
  %call = tail call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %struct.Arr*
  tail call void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* nonnull %0, i32 10, %struct.Mem* null)
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  store %struct.Arr* %0, %struct.Arr** %f1, align 8
  %call2 = tail call i8* @_Znwm(i64 32)
  %1 = bitcast i8* %call2 to %struct.Arr1*
  call void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* nonnull %1)
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 2
  store %struct.Arr1* %1, %struct.Arr1** %f2, align 8
  %2 = load %struct.Arr*, %struct.Arr** %f1, align 8
  tail call void @_ZN3ArrIPiE3setEiPS0_(%struct.Arr* %2, i32 0, i32** null)
  %3 = bitcast %struct.Arr1** %f2 to %struct.Arr.0**
  %4 = load %struct.Arr.0*, %struct.Arr.0** %3, align 8
  tail call void @_ZN3ArrIPfE3setEiPS0_(%struct.Arr.0* %4, i32 0, float** null)
  %5 = load %struct.Arr.0*, %struct.Arr.0** %3, align 8
  tail call void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* %5, float** null)
  %6 = load %struct.Arr*, %struct.Arr** %f1, align 8
  tail call void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* %6, i32** null)
  call void @_Z3foov()
  ret void
}

define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %this, i32 %c, %struct.Mem* %mem) {
entry:
  ret void
}

define void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* %this) {
entry:
  %0 = getelementptr inbounds %struct.Arr1, %struct.Arr1* %this, i64 0, i32 0
  tail call void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.0* %0, i32 1, %struct.Mem* null)
  ret void
}

define void @_ZN3ArrIPiE3setEiPS0_(%struct.Arr* %this, i32 %i, i32** %val) {
entry:
  ret void
}

define void @_ZN3ArrIPfE3setEiPS0_(%struct.Arr.0* %this, i32 %i, float** %val) {
entry:
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* %this, float** %val) {
entry:
  tail call void @_ZN3ArrIPfE6resizeEv(%struct.Arr.0* %this)
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* %this, i32** %val) {
entry:
  tail call void @_ZN3ArrIPiE6resizeEv(%struct.Arr* %this)
  ret void
}

define void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.0* %this, i32 %c, %struct.Mem* %mem) {
entry:
  ret void
}

define void @_ZN3ArrIPfE6resizeEv(%struct.Arr.0* %this) {
entry:
  ret void
}

define void @_ZN3ArrIPiE6resizeEv(%struct.Arr* %this) {
entry:
  ret void
}

define void @_Z3barv() {
entry:
  ret void
}

declare dso_local noalias i8* @_Znwm(i64)
