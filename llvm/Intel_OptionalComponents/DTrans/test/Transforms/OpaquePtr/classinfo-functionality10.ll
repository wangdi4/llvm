; This testcase verifies that MemInitTrimDown transformation is able
; to recognize SetElem(set), GetElem(get) and AppendElem(add)
; member functions of Arr and Arr1 classes when address of element
; is passed as argument/return type.

; SetElem: _ZN3ArrIPiE3setEiPS0_ and _ZN3ArrIPfE3setEiPS0_
; GetElem: _ZN3ArrIPiE3getEi and _ZN3ArrIPfE3getEi
; AppendElem: _ZN3ArrIPiE3addEPS0_ and _ZN3ArrIPfE3addEPS0_

; RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-meminitop-recognize-all -dtrans-meminittrimdownop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop,dtrans-soatoaosopclassinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-meminitop-recognize-all -passes=dtrans-meminittrimdownop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop,dtrans-soatoaosopclassinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -dtrans-meminitop-recognize-all -dtrans-meminittrimdownop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop,dtrans-soatoaosopclassinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -dtrans-meminitop-recognize-all -passes=dtrans-meminittrimdownop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop,dtrans-soatoaosopclassinfo -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" will be considered as candidate vector fields.
;
; This test verified that the following functions are recognized since
; they are in expected pattern.
; resize
; copy-constructor
; add

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
;
;   void resize(int inc) {
;   }
;
;   void add(S** val) {
;     resize(1);
;     base[size] = *val;
;     size++;
;   }
;
;   void set(unsigned i, S** val) {
;    if (i >= size)
;      throw;
;    if (flag)
;      free(base[i]);
;    base[i] = *val;
;  }
;
;  S** get(unsigned i) {
;    if (i >= size)
;      throw;
;    return &base[i];
;  }
;
;   Arr(const Arr &A) :
;     flag(A.flag), capacity(A.capacity), size(A.size), base(0), mem(A.mem) {
;   }
;
;   unsigned getSize() { return size; }
;   unsigned getCapacity() { return capacity; }
;   Arr(unsigned c = 2, Mem *mem = 0)
;      : flag(false), capacity(c), size(0), base(0), mem(mem) {
;    base = (S**)malloc(capacity * sizeof(S*));
;    memset(base, 0, capacity * sizeof(S*));
;  }
;  ~Arr() { }
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
;    int*** pi = f1->get(1);
;    float*** pf = f2->get(1);
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
;CHECK-DAG:  Functionality of _ZN3ArrIPiE3addEPS0_: Recognized as AppendElem
;CHECK-DAG:  Functionality of _ZN3ArrIPiE3getEi: Recognized as GetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPiE3setEiPS0_: Recognized as SetElem

;CHECK:  Analyzing Constructor _ZN4Arr1IPfEC2EiP3Mem
;CHECK:  Passed: Constructor recognized
;CHECK-DAG: Functionality of _ZN3ArrIPfE3getEi: Recognized as GetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPfE3setEiPS0_: Recognized as SetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPfE3addEPS0_: Recognized as AppendElem

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type { i8, i32, i32, i32***, %struct.Mem* }
%struct.Arr1 = type { %struct.Arr.0 }
%struct.Arr.0 = type { i8, i32, i32, float***, %struct.Mem* }

; All fields of Arr class are initialized as expected.
define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !24 {
entry:
  %flag = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  store i8 0, i8* %flag
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  store i32 %c, i32* %capacity
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  store i32 0, i32* %size
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  store %struct.Mem* %mem, %struct.Mem** %mem2
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias i8* @malloc(i64 %mul)
  %0 = bitcast i8* %call to i32***
  store i32*** %0, i32**** %base
  tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

; This is the constructor for Derived class. It just calls constructor of
; base class to initialize fields.
define void @_ZN4Arr1IPfEC2EiP3Mem(%struct.Arr1* "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !25 {
entry:
  %0 = getelementptr inbounds %struct.Arr1, %struct.Arr1* %this, i64 0, i32 0
  tail call void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.0* %0, i32 %c, %struct.Mem* %mem)
  ret void
}

; This is constructor of base class of Arr1. It initialize all fields
; as expected.
define void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.0*  "intel_dtrans_func_index"="1"  %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !39 {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  store i8 0, i8* %flag
  %capacity = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  store i32 %c, i32* %capacity
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  store i32 0, i32* %size
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 4
  store %struct.Mem* %mem, %struct.Mem** %mem2
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias i8* @malloc(i64 %mul)
  %0 = bitcast i8* %call to float***
  store float*** %0, float**** %base
  tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* "intel_dtrans_func_index"="1" %this, %struct.Arr* "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !33 {
entry:
  ret void
}

define void @_ZN3ArrIPiE3setEiPS0_(%struct.Arr* "intel_dtrans_func_index"="1" %this, i32 %i, i32*** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !29 {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  %cmp = icmp ugt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %flag = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  %1 = load i8, i8* %flag
  %tobool = icmp eq i8 %1, 0
  br i1 %tobool, label %if.end3, label %if.then2

if.then2:                                         ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %2 = load i32***, i32**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds i32**, i32*** %2, i64 %idxprom
  %3 = bitcast i32*** %arrayidx to i8**
  %4 = load i8*, i8** %3
  tail call void @free(i8* %4)
  br label %if.end3

if.end3:                                          ; preds = %if.end, %if.then2
  %base4 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %5 = load i32***, i32**** %base4
  %idxprom5 = zext i32 %i to i64
  %arrayidx6 = getelementptr inbounds i32**, i32*** %5, i64 %idxprom5
  %ldv = load i32**, i32*** %val
  store i32** %ldv, i32*** %arrayidx6
  ret void
}

define void @_ZN3ArrIPfE3setEiPS0_(%struct.Arr.0* "intel_dtrans_func_index"="1" %this, i32 %i, float*** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !30 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  %cmp = icmp ugt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %flag = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  %1 = load i8, i8* %flag, align 8
  %tobool = icmp eq i8 %1, 0
  br i1 %tobool, label %if.end3, label %if.then2

if.then2:                                         ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %2 = load float***, float**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds float**, float*** %2, i64 %idxprom
  %3 = bitcast float*** %arrayidx to i8**
  %4 = load i8*, i8** %3
  tail call void @free(i8* %4)
  br label %if.end3

if.end3:                                          ; preds = %if.end, %if.then2
  %base4 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %5 = load float***, float**** %base4
  %idxprom5 = zext i32 %i to i64
  %arrayidx6 = getelementptr inbounds float**, float*** %5, i64 %idxprom5
  %ldv = load float**, float*** %val
  store float** %ldv, float*** %arrayidx6
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* "intel_dtrans_func_index"="1" %this, float*** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !31 {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* %this, i32 1)
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %0 = load float***, float**** %base
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %1 = load i32, i32* %size
  %idxprom = zext i32 %1 to i64
  %arrayidx = getelementptr inbounds float**, float*** %0, i64 %idxprom
  %ldv = load float**, float*** %val
  store float** %ldv, float*** %arrayidx
  %inc = add i32 %1, 1
  store i32 %inc, i32* %size
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* "intel_dtrans_func_index"="1" %this, i32*** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !32 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(%struct.Arr* %this, i32 1)
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %0 = load i32***, i32**** %base
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %1 = load i32, i32* %size
  %idxprom = zext i32 %1 to i64
  %arrayidx = getelementptr inbounds i32**, i32*** %0, i64 %idxprom
  %ldv = load i32**, i32*** %val
  store i32** %ldv, i32*** %arrayidx
  %inc = add i32 %1, 1
  store i32 %inc, i32* %size
  ret void
}

define void @_ZN3ArrIPiE6resizeEi(%struct.Arr* "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !42 {
entry:
  ret void
}

define void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !41 {
entry:
  ret void
}

define "intel_dtrans_func_index"="1" i32*** @_ZN3ArrIPiE3getEi(%struct.Arr* "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !26 {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  %cmp = icmp ugt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %1 = load i32***, i32**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds i32**, i32*** %1, i64 %idxprom
  ret i32*** %arrayidx
}

define "intel_dtrans_func_index"="1" float*** @_ZN3ArrIPfE3getEi(%struct.Arr.0* "intel_dtrans_func_index"="2"  %this, i32 %i) !intel.dtrans.func.type !27 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %0 = load i32, i32* %size, align 8
  %cmp = icmp ugt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %1 = load float***, float**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds float**, float*** %1, i64 %idxprom
  ret float*** %arrayidx
}

define void @_ZN3ArrIPiED2Ev(%struct.Arr* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !36 {
entry:
  ret void
}

define void @_ZN3ArrIPfED2Ev(%struct.Arr.0* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !37 {
entry:
  ret void
}

define i32 @_ZN3ArrIPiE7getSizeEv(%struct.Arr* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !34 {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  ret i32 %0
}

define i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !35 {
entry:
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %0 = load i32, i32* %capacity
  ret i32 %0
}

define i32 @main() {
entry:
  %call = tail call i8* @_Znwm(i64 24)
  %0 = bitcast i8* %call to %class.F*
  tail call void @_ZN1FC2Ev(%class.F* %0)
  ret i32 0
}

define void @_ZN1FC2Ev(%class.F* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !22 {
entry:
  %call = tail call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %struct.Arr*
  tail call void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %0, i32 10, %struct.Mem* null)
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %1 = bitcast %struct.Arr** %f1 to i8**
  store %struct.Arr* %0, %struct.Arr** %f1
  %call2 = tail call i8* @_Znwm(i64 32)
  %2 = bitcast i8* %call2 to %struct.Arr1*
  tail call void @_ZN4Arr1IPfEC2EiP3Mem(%struct.Arr1* %2, i32 2, %struct.Mem* null)
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 2
  %3 = bitcast %struct.Arr1** %f2 to i8**
  store %struct.Arr1* %2, %struct.Arr1** %f2
  %4 = load %struct.Arr*, %struct.Arr** %f1
  %call6 = tail call i32*** @_ZN3ArrIPiE3getEi(%struct.Arr* %4, i32 1)
  %5 = bitcast %struct.Arr1** %f2 to %struct.Arr.0**
  %6 = load %struct.Arr.0*, %struct.Arr.0** %5
  %call8 = tail call float*** @_ZN3ArrIPfE3getEi(%struct.Arr.0* %6, i32 1)
  %7 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiE3setEiPS0_(%struct.Arr* %7, i32 0, i32*** %call6)
  %8 = load %struct.Arr.0*, %struct.Arr.0** %5
  tail call void @_ZN3ArrIPfE3setEiPS0_(%struct.Arr.0* %8, i32 0, float*** %call8)
  %9 = load %struct.Arr.0*, %struct.Arr.0** %5
  tail call void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* %9, float*** null)
  %10 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* %10, i32*** null)
  %call13 = tail call i8* @_Znwm(i64 32)
  %11 = bitcast i8* %call13 to %struct.Arr*
  %12 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* %11, %struct.Arr* %12)
  tail call void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* %11, i32*** null)
  %s = load %struct.Arr*, %struct.Arr** %f1, align 8
  %call18 = tail call i32 @_ZN3ArrIPiE7getSizeEv(%struct.Arr* %s)
  %d = load %struct.Arr*, %struct.Arr** %f1, align 8
  %call20 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* %d)
  %13 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiED2Ev(%struct.Arr* %13)
  %14 = load %struct.Arr1*, %struct.Arr1** %f2
  %15 = getelementptr inbounds %struct.Arr1, %struct.Arr1* %14, i64 0, i32 0
  tail call void @_ZN3ArrIPfED2Ev(%struct.Arr.0* %15)
  ret void
}

declare !intel.dtrans.func.type !20 dso_local nonnull "intel_dtrans_func_index"="1" i8* @_Znwm(i64)
declare dso_local void @__cxa_rethrow()
declare !intel.dtrans.func.type !40 dso_local void @free(i8* "intel_dtrans_func_index"="1")
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
declare !intel.dtrans.func.type !38 dso_local noalias "intel_dtrans_func_index"="1" i8* @malloc(i64)

!intel.dtrans.types = !{!3, !7, !11, !14, !16}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
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
!23 = distinct !{!21}
!24 = distinct !{!5, !4}
!25 = distinct !{!6, !4}
!26 = distinct !{!13, !5}
!27 = distinct !{!17, !28}
!28 = !{%struct.Arr.0 zeroinitializer, i32 1}
!29 = distinct !{!5, !13}
!30 = distinct !{!28, !17}
!31 = distinct !{!28, !17}
!32 = distinct !{!5, !13}
!33 = distinct !{!5, !5}
!34 = distinct !{!5}
!35 = distinct !{!5}
!36 = distinct !{!5}
!37 = distinct !{!6}
!38 = distinct !{!21}
!39 = distinct !{!28, !4}
!40 = distinct !{!21}
!41 = distinct !{!28}
!42 = distinct !{!5}
!43 = distinct !{!28}
