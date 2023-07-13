; This testcase verifies that MemInitTrimDown transformation is able
; to categorize member functions of Arr and Arr1 classes and recognize
; constructors of those classes. It also verifies that size and capacity
; fields are detected.

; RUN: opt < %s -dtrans-meminitop-recognize-all -passes=dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop,dtrans-soatoaosopclassinfo -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" will be considered as candidate vector fields.
; Definitions of most of the member functions are empty or doesn't make
; sense except _ZN3ArrIPiEC2EiP3Mem, _ZN4Arr1IPfEC2EiP3Mem and
; _ZN3ArrIPfEC2EiP3Mem, which are constructor functions.

;  struct Mem {
;   virtual void *allocate() = 0;
;   virtual void *deallocate() = 0;
; };
;
;template <typename S> struct Arr {
;   bool flag;
;   unsigned int capacity;
;   unsigned int size;
;   S **base;
;   Mem *mem;
;   void resize(int inc) {
;   }
;   void add(S* val) {
;     resize(1);
;   }
;  void set(int i, S* val) { }
;  S* get(int i) { return nullptr; }
;  Arr(int c = 2, Mem *mem = 0)
;      : flag(false), capacity(c), size(0), base(0), mem(mem) {
;    base = (S**)malloc(capacity * sizeof(S*));
;    memset(base, 0, capacity * sizeof(S*));
;  }
;  ~Arr() {}
;  Arr(const Arr &A) { }
; };
;
; template <typename S>
; struct Arr1 : public Arr<S> {
;  public:
;    Arr1(int c = 2, Mem *mem = 0);
;  };
; template <typename S>
;  Arr1<S>::Arr1(int c, Mem *mem)
;      : Arr<S>(c, mem) { }
;
; class F {
;   Mem *mem;
; public:
;   Arr<int*>* f1;
;   Arr1<float*>* f2;
;   F() {
;     f1 = new Arr<int *>(10, nullptr);
;     f2 = new Arr1<float *>();
;     int** pi = f1->get(1);
;     float** pf = f2->get(1);
;     f1->set(0, pi);
;     f2->set(0, pf);
;     f2->add(nullptr);
;     f1->add(nullptr);
;     Arr<int*>* f3 = new Arr<int *>(*f1);
;     f3->add(nullptr);
;     delete f1;
;     delete f2;
;   }
;
; };
; int main() {
;   F *f = new F();
; }

;CHECK: MemInitTrimDown transformation:
;CHECK:  Possible candidate structs:
;CHECK:    Candidate: class.F
;CHECK:   Categorize functions using signature:
;CHECK-DAG:       _ZN3ArrIPiEC2EiP3Mem:   Constructor
;CHECK-DAG:       _ZN3ArrIPiE3getEi:   GetElem
;CHECK-DAG:       _ZN3ArrIPiE3setEiPS0_:   SetElem
;CHECK-DAG:       _ZN3ArrIPiE3addEPS0_:   AppendElem
;CHECK-DAG:       _ZN3ArrIPiE6resizeEi:   Resize
;CHECK-DAG:       _ZN3ArrIPiEC2ERKS1_:   CopyConstructor
;CHECK-DAG:       _ZN3ArrIPiED2Ev:   Destructor
;CHECK:  Analyzing Constructor _ZN3ArrIPiEC2EiP3Mem
;CHECK:  Passed: Constructor recognized
;CHECK:    Capacity field: 1
;CHECK:    Size field: 2

;CHECK:  Categorize functions using signature:
;CHECK-DAG:       _ZN4Arr1IPfEC2EiP3Mem:   Constructor
;CHECK-DAG:       _ZN3ArrIPfEC2EiP3Mem:   Constructor
;CHECK-DAG:       _ZN3ArrIPfE3getEi:   GetElem
;CHECK-DAG:       _ZN3ArrIPfE3setEiPS0_:   SetElem
;CHECK-DAG:       _ZN3ArrIPfE3addEPS0_:   AppendElem
;CHECK-DAG:       _ZN3ArrIPfE6resizeEi:   Resize
;CHECK-DAG:       _ZN3ArrIPfED2Ev:   Destructor
;CHECK:  Analyzing Constructor _ZN4Arr1IPfEC2EiP3Mem
;CHECK:  Passed: Constructor recognized
;CHECK:    Capacity field: 1
;CHECK:    Size field: 2

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Arr = type { i8, i32, i32, ptr, ptr }
%struct.Arr1 = type { %struct.Arr.0 }
%struct.Arr.0 = type { i8, i32, i32, ptr, ptr }
%class.F = type { ptr, ptr, ptr }
%struct.Mem = type { ptr }

; All fields of Arr class are initialized as expected.
define void @_ZN3ArrIPiEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !15 {
entry:
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0
  store i8 0, ptr %flag, align 1
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  store i32 0, ptr %size, align 4
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %mem2 = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias ptr @malloc(i64 %mul)
  store ptr %call, ptr %base, align 8
  tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

; This is the constructor for Derived class. It just calls constructor of
; base class to initialize fields.
define void @_ZN4Arr1IPfEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !16 {
entry:
  %0 = getelementptr inbounds %struct.Arr1, ptr %this, i64 0, i32 0
  tail call void @_ZN3ArrIPfEC2EiP3Mem(ptr %0, i32 %c, ptr %mem)
  ret void
}

; This is constructor of base class of Arr1. It initialize all fields
; as expected.
define void @_ZN3ArrIPfEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !17 {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 0
  store i8 0, ptr %flag, align 1
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  store i32 0, ptr %size, align 4
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %mem2 = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias ptr @malloc(i64 %mul)
  store ptr %call, ptr %base, align 8
  tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define void @_ZN3ArrIPiEC2ERKS1_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !19 {
entry:
  ret void
}

define void @_ZN3ArrIPiE3setEiPS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !20 {
entry:
  ret void
}

define void @_ZN3ArrIPfE3setEiPS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !22 {
entry:
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !24 {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(ptr %this, i32 1)
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !25 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(ptr %this, i32 1)
  ret void
}

define void @_ZN3ArrIPfE6resizeEi(ptr "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !26 {
entry:
  ret void
}

define void @_ZN3ArrIPiE6resizeEi(ptr "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !27 {
entry:
  ret void
}

define "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPiE3getEi(ptr "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !28 {
entry:
  ret ptr null
}

define "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPfE3getEi(ptr "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !29 {
entry:
  ret ptr null
}

define void @_ZN3ArrIPiED2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !30 {
entry:
  ret void
}

define void @_ZN3ArrIPfED2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !31 {
entry:
  ret void
}

define i32 @main() {
entry:
  %call = tail call ptr @_Znwm(i64 24)
  tail call void @_ZN1FC2Ev(ptr %call)
  ret i32 0
}

define void @_ZN1FC2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !32 {
entry:
  %call = tail call ptr @_Znwm(i64 32)
  tail call void @_ZN3ArrIPiEC2EiP3Mem(ptr %call, i32 10, ptr null)
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  store ptr %call, ptr %f1, align 8
  %call2 = tail call ptr @_Znwm(i64 32)
  tail call void @_ZN4Arr1IPfEC2EiP3Mem(ptr %call2, i32 2, ptr null)
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 2
  store ptr %call2, ptr %f2, align 8
  %i4 = load ptr, ptr %f1, align 8
  %call6 = tail call ptr @_ZN3ArrIPiE3getEi(ptr %i4, i32 1)
  %i6 = load ptr, ptr %f2, align 8
  %call8 = tail call ptr @_ZN3ArrIPfE3getEi(ptr %i6, i32 1)
  %i7 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiE3setEiPS0_(ptr %i7, i32 0, ptr %call6)
  %i8 = load ptr, ptr %f2, align 8
  tail call void @_ZN3ArrIPfE3setEiPS0_(ptr %i8, i32 0, ptr %call8)
  %i9 = load ptr, ptr %f2, align 8
  tail call void @_ZN3ArrIPfE3addEPS0_(ptr %i9, ptr null)
  %i10 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiE3addEPS0_(ptr %i10, ptr null)
  %call13 = tail call ptr @_Znwm(i64 32)
  %i12 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiEC2ERKS1_(ptr %call13, ptr %i12)
  tail call void @_ZN3ArrIPiE3addEPS0_(ptr %call13, ptr null)
  %i13 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiED2Ev(ptr %i13)
  %i14 = load ptr, ptr %f2, align 8
  %i15 = getelementptr inbounds %struct.Arr1, ptr %i14, i64 0, i32 0
  tail call void @_ZN3ArrIPfED2Ev(ptr %i15)
  ret void
}

declare !intel.dtrans.func.type !34 noalias "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !36 noalias "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: write) }

!intel.dtrans.types = !{!0, !4, !8, !11, !13}

!0 = !{!"S", %class.F zeroinitializer, i32 3, !1, !2, !3}
!1 = !{%struct.Mem zeroinitializer, i32 1}
!2 = !{%struct.Arr zeroinitializer, i32 1}
!3 = !{%struct.Arr1 zeroinitializer, i32 1}
!4 = !{!"S", %struct.Mem zeroinitializer, i32 1, !5}
!5 = !{!6, i32 2}
!6 = !{!"F", i1 true, i32 0, !7}
!7 = !{i32 0, i32 0}
!8 = !{!"S", %struct.Arr zeroinitializer, i32 5, !9, !7, !7, !10, !1}
!9 = !{i8 0, i32 0}
!10 = !{i32 0, i32 3}
!11 = !{!"S", %struct.Arr1 zeroinitializer, i32 1, !12}
!12 = !{%struct.Arr.0 zeroinitializer, i32 0}
!13 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !9, !7, !7, !14, !1}
!14 = !{float 0.000000e+00, i32 3}
!15 = distinct !{!2, !1}
!16 = distinct !{!3, !1}
!17 = distinct !{!18, !1}
!18 = !{%struct.Arr.0 zeroinitializer, i32 1}
!19 = distinct !{!2, !2}
!20 = distinct !{!2, !21}
!21 = !{i32 0, i32 2}
!22 = distinct !{!18, !23}
!23 = !{float 0.000000e+00, i32 2}
!24 = distinct !{!18, !23}
!25 = distinct !{!2, !21}
!26 = distinct !{!18}
!27 = distinct !{!2}
!28 = distinct !{!21, !2}
!29 = distinct !{!23, !18}
!30 = distinct !{!2}
!31 = distinct !{!18}
!32 = distinct !{!33}
!33 = !{%class.F zeroinitializer, i32 1}
!34 = distinct !{!35}
!35 = !{i8 0, i32 1}
!36 = distinct !{!35}
