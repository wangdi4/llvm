; This testcase verifies that MemInitTrimDown transformation is able
; to recognize destructor, set, get, getCapacity and getSize member
; functions of Arr and Arr1 classes.

; RUN: opt < %s -dtrans-meminitop-recognize-all -passes=dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop,dtrans-soatoaosopclassinfo -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" will be considered as candidate vector fields.
;
; This test verified that the following functions are recognized since
; they are in expected pattern.
; destructor
; set
; get
; getCapacity
; getSize

;  struct Mem {
;   virtual void *allocate() = 0;
;   virtual void *deallocate() = 0;
; };
;
; template <typename S> struct Arr {
;   bool flag;
;   unsigned capacity;
;   unsigned size;
;   S **base;
;   Mem *mem;
;   unsigned getSize() { return size; }
;   unsigned getCapacity() { return capacity; }
;   void resize(int inc) {
;   }
;   void add(S* val) {
;     resize(1);
;   }
;  void set(unsigned i, S* val) {
;    if (i >= size)
;      throw;
;    base[i] = val;
;  }
;  S* get(unsigned i) {
;    if (i >= size)
;      throw;
;    return base[i];
;  }
;  Arr(unsigned c = 2, Mem *mem = 0)
;     : flag(false), capacity(c), size(0), base(0), mem(mem) {
;   base = (S**)malloc(capacity * sizeof(S*));
;   memset(base, 0, capacity * sizeof(S*));
; }
; ~Arr() {
;   free(base);
; }
; Arr(const Arr &A) { }
;};
;
;template <typename S>
;struct Arr1 : public Arr<S> {
; public:
;   Arr1(unsigned c = 2, Mem *mem = 0);
; };
;template <typename S>
; Arr1<S>::Arr1(unsigned c, Mem *mem)
;     : Arr<S>(c, mem) { }
;
;class F {
;  Mem *mem;
;public:
;  Arr<int*>* f1;
;  Arr1<float*>* f2;
;  F() {
;    f1 = new Arr<int *>(10, nullptr);
;    f2 = new Arr1<float *>();
;    int** pi = f1->get(1);
;    float** pf = f2->get(1);
;    f1->set(0, pi);
;    f2->set(0, pf);
;    f2->add(nullptr);
;    f1->add(nullptr);
;    Arr<int*>* f3 = new Arr<int *>(*f1);
;    f3->add(nullptr);
;    f1->getSize();
;    f1->getCapacity();
;    delete f1;
;    delete f2;
;  }
;
;};
;int main() {
;  F *f = new F();
;}

;CHECK: MemInitTrimDown transformation:
;CHECK:  Analyzing Constructor _ZN3ArrIPiEC2EiP3Mem
;CHECK:  Passed: Constructor recognized
;CHECK-DAG:  Functionality of _ZN3ArrIPiE3getEi: Recognized as GetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPiE3setEiPS0_: Recognized as SetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPiE7getSizeEv: Recognized as GetSize
;CHECK-DAG:  Functionality of _ZN3ArrIPiE11getCapacityEv: Recognized as GetCapacity
;CHECK-DAG:  Functionality of _ZN3ArrIPiED2Ev: Recognized as Destructor

;CHECK:  Analyzing Constructor _ZN4Arr1IPfEC2EiP3Mem
;CHECK:  Passed: Constructor recognized
;CHECK-DAG:  Functionality of _ZN3ArrIPfE3getEi: Recognized as GetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPfE3setEiPS0_: Recognized as SetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPfED2Ev: Recognized as Destructor

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { ptr, ptr, ptr }
%struct.Mem = type { ptr }
%struct.Arr = type { i8, i32, i32, ptr, ptr }
%struct.Arr1 = type { %struct.Arr.0 }
%struct.Arr.0 = type { i8, i32, i32, ptr, ptr }

; All fields of Arr class are initialized as expected.
define void @_ZN3ArrIPiEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !26 {
entry:
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0
  store i8 0, ptr %flag
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  store i32 0, ptr %size
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %mem2 = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias ptr @malloc(i64 %mul)
  store ptr %call, ptr %base
  tail call void @llvm.memset.p0i8.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

; This is the constructor for Derived class. It just calls constructor of
; base class to initialize fields.
define void @_ZN4Arr1IPfEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !27 {
entry:
  %i0 = getelementptr inbounds %struct.Arr1, ptr %this, i64 0, i32 0
  tail call void @_ZN3ArrIPfEC2EiP3Mem(ptr %i0, i32 %c, ptr %mem)
  ret void
}

; This is constructor of base class of Arr1. It initialize all fields
; as expected.
define void @_ZN3ArrIPfEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !41 {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 0
  store i8 0, ptr %flag
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  store i32 0, ptr %size
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %mem2 = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias ptr @malloc(i64 %mul)
  store ptr %call, ptr %base
  tail call void @llvm.memset.p0i8.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define void @_ZN3ArrIPiEC2ERKS1_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !35 {
entry:
  ret void
}

define void @_ZN3ArrIPiE3setEiPS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !31 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i0 = load i32, ptr %size
  %cmp = icmp ugt i32 %i0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i1 = load ptr, ptr %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  store ptr %val, ptr %arrayidx
  ret void
}

define void @_ZN3ArrIPfE3setEiPS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !32 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i0 = load i32, ptr %size
  %cmp = icmp ugt i32 %i0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i1 = load ptr, ptr %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  store ptr %val, ptr %arrayidx
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !33 {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(ptr %this, i32 1)
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !34 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(ptr %this, i32 1)
  ret void
}

define void @_ZN3ArrIPfE6resizeEi(ptr "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !42 {
entry:
  ret void
}

define void @_ZN3ArrIPiE6resizeEi(ptr "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !43 {
entry:
  ret void
}

define "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPiE3getEi(ptr "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !28 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i0 = load i32, ptr %size
  %cmp = icmp ugt i32 %i0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i1 = load ptr, ptr %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  %i2 = load ptr, ptr %arrayidx
  ret ptr %i2
}

define "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPfE3getEi(ptr "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !29 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i0 = load i32, ptr %size, align 8
  %cmp = icmp ugt i32 %i0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i1 = load ptr, ptr %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  %i2 = load ptr, ptr %arrayidx
  ret ptr %i2
}

define void @_ZN3ArrIPiED2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !38 {
entry:
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i1 = load ptr, ptr %base
  tail call void @free(ptr %i1)
  ret void
}

define void @_ZN3ArrIPfED2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !45 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i1 = load ptr, ptr %base
  tail call void @free(ptr %i1)
  ret void
}

define i32 @_ZN3ArrIPiE7getSizeEv(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !36 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i0 = load i32, ptr %size
  ret i32 %i0
}

define i32 @_ZN3ArrIPiE11getCapacityEv(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !37 {
entry:
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  %i0 = load i32, ptr %capacity
  ret i32 %i0
}

define i32 @main() {
entry:
  %call = tail call ptr @_Znwm(i64 24)
  tail call void @_ZN1FC2Ev(ptr %call)
  ret i32 0
}

define void @_ZN1FC2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !22 {
entry:
  %call = tail call ptr @_Znwm(i64 32)
  tail call void @_ZN3ArrIPiEC2EiP3Mem(ptr %call, i32 10, ptr null)
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  store ptr %call, ptr %f1
  %call2 = tail call ptr @_Znwm(i64 32)
  tail call void @_ZN4Arr1IPfEC2EiP3Mem(ptr %call2, i32 2, ptr null)
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 2
  store ptr %call2, ptr %f2
  %i4 = load ptr, ptr %f1
  %call6 = tail call ptr @_ZN3ArrIPiE3getEi(ptr %i4, i32 1)
  %i6 = load ptr, ptr %f2
  %call8 = tail call ptr @_ZN3ArrIPfE3getEi(ptr %i6, i32 1)
  %i7 = load ptr, ptr %f1
  tail call void @_ZN3ArrIPiE3setEiPS0_(ptr %i7, i32 0, ptr %call6)
  %i8 = load ptr, ptr %f2
  tail call void @_ZN3ArrIPfE3setEiPS0_(ptr %i8, i32 0, ptr %call8)
  %i9 = load ptr, ptr %f2
  tail call void @_ZN3ArrIPfE3addEPS0_(ptr %i9, ptr null)
  %i10 = load ptr, ptr %f1
  tail call void @_ZN3ArrIPiE3addEPS0_(ptr %i10, ptr null)
  %call13 = tail call ptr @_Znwm(i64 32)
  %i12 = load ptr, ptr %f1
  tail call void @_ZN3ArrIPiEC2ERKS1_(ptr %call13, ptr %i12)
  tail call void @_ZN3ArrIPiE3addEPS0_(ptr %call13, ptr null)
  %s = load ptr, ptr %f1, align 8
  %call18 = tail call i32 @_ZN3ArrIPiE7getSizeEv(ptr %s)
  %d = load ptr, ptr %f1, align 8
  %call20 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(ptr %d)
  %i13 = load ptr, ptr %f1
  tail call void @_ZN3ArrIPiED2Ev(ptr %i13)
  %i14 = load ptr, ptr %f2
  %i15 = getelementptr inbounds %struct.Arr1, ptr %i14, i64 0, i32 0
  tail call void @_ZN3ArrIPfED2Ev(ptr %i15)
  ret void
}

declare !intel.dtrans.func.type !20 noalias "intel_dtrans_func_index"="1" ptr @_Znwm(i64)
declare !intel.dtrans.func.type !40 noalias "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare void @llvm.memset.p0i8.i64(ptr nocapture writeonly, i8, i64, i1 immarg)
declare void @__cxa_rethrow()
declare !intel.dtrans.func.type !44 void @free(ptr "intel_dtrans_func_index"="1" nocapture) #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!3, !7, !11, !14, !16}

!3 = !{!"S", %class.F zeroinitializer, i32 3, !4, !5, !6}
!4 = !{%struct.Mem zeroinitializer, i32 1}
!5 = !{%struct.Arr zeroinitializer, i32 1}
!6 = !{%struct.Arr1 zeroinitializer, i32 1}
!7 = !{!"S", %struct.Mem zeroinitializer, i32 1, !8}
!8 = !{!9, i32 2}
!9 = !{!"F", i1 true, i32 0, !10}
!10 = !{i32 0, i32 0}
!11 = !{!"S", %struct.Arr zeroinitializer, i32 5, !12, !10, !10, !13, !4}
!12 = !{i8 0, i32 0}
!13 = !{i32 0, i32 3}
!14 = !{!"S", %struct.Arr1 zeroinitializer, i32 1, !15}
!15 = !{%struct.Arr.0 zeroinitializer, i32 0}
!16 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !12, !10, !10, !17, !4}
!17 = !{float 0.000000e+00, i32 3}
!18 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!19 = !{%class.F zeroinitializer, i32 1}
!20 = distinct !{!21}
!21 = !{i8 0, i32 1}
!22 = distinct !{!19}
!23 = !{i32 0, i32 2}
!24 = !{float 0.000000e+00, i32 2}
!25 = distinct !{!21}
!26 = distinct !{!5, !4}
!27 = distinct !{!6, !4}
!28 = distinct !{!23, !5}
!29 = distinct !{!24, !30}
!30 = !{%struct.Arr.0 zeroinitializer, i32 1}
!31 = distinct !{!5, !23}
!32 = distinct !{!30, !24}
!33 = distinct !{!30, !24}
!34 = distinct !{!5, !23}
!35 = distinct !{!5, !5}
!36 = distinct !{!5}
!37 = distinct !{!5}
!38 = distinct !{!5}
!39 = distinct !{!6}
!40 = distinct !{!21}
!41 = distinct !{!30, !4}
!42 = distinct !{!30}
!43 = distinct !{!5}
!44 = distinct !{!21}
!45 = distinct !{!30}
