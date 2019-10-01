; This testcase verifies that MemInitTrimDown transformation is not
; recognizing destructor, set and get member functions of Arr and Arr1
; classes.

; RUN: opt < %s -dtrans-meminit-recognize-all -dtrans-meminittrimdown -enable-dtrans-meminittrimdown -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdown,dtrans-soatoaosclassinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtrans-meminit-recognize-all -passes=dtrans-meminittrimdown -enable-dtrans-meminittrimdown -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdown,dtrans-soatoaosclassinfo -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" will be considered as candidate vector fields.
; This test verified that the following functions are not recognized since
; they are not in expected pattern.
;
; destructor: Missing "free(base);".
;
; set: Freeing "free(base[i + 1]);" instead of "free(base[i]);".
;
; get: "if (i >= size)" check is missing.
;
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
;   unsigned getSize() { return size + 1; }
;   unsigned getCapacity() { return capacity << 1; }
;   void resize(int inc) {
;   }
;   void add(S* val) {
;     resize(1);
;   }
;  void set(unsigned i, S* val) {
;    if (i >= size)
;      throw;
;    if (flag)
;      free(base[i + 1]);
;    base[i] = val;
;  }
;  S* get(unsigned i) {
;    return base[i - 1];
;  }
;  Arr(unsigned c = 2, Mem *mem = 0)
;     : flag(false), capacity(c), size(0), base(0), mem(mem) {
;   base = (S**)malloc(capacity * sizeof(S*));
;   memset(base, 0, capacity * sizeof(S*));
; }
; ~Arr() {
;   if (flag) {
;     for(unsigned i = 0; i < size; i++)
;       free(base[i]);
;   }
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
;
;CHECK-DAG:  Functionality of _ZN3ArrIPiE3getEi: Failed to recognize as GetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPiE3setEiPS0_: Failed to recognize as SetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPiED2Ev: Failed to recognize as Destructor
;
;CHECK:  Analyzing Constructor _ZN4Arr1IPfEC2EiP3Mem
;CHECK:  Passed: Constructor recognized
;
;CHECK-DAG:  Functionality of _ZN3ArrIPfE3getEi: Failed to recognize as GetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPfE3setEiPS0_: Failed to recognize as SetElem
;CHECK-DAG:  Functionality of _ZN3ArrIPfED2Ev: Failed to recognize as Destructor

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type { i8, i32, i32, i32***, %struct.Mem* }
%struct.Arr1 = type { %struct.Arr.0 }
%struct.Arr.0 = type { i8, i32, i32, float***, %struct.Mem* }

; All fields of Arr class are initialized as expected.
define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %this, i32 %c, %struct.Mem* %mem) {
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
define void @_ZN4Arr1IPfEC2EiP3Mem(%struct.Arr1* %this, i32 %c, %struct.Mem* %mem) {
entry:
  %0 = getelementptr inbounds %struct.Arr1, %struct.Arr1* %this, i64 0, i32 0
  tail call void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.0* %0, i32 %c, %struct.Mem* %mem)
  ret void
}

; This is constructor of base class of Arr1. It initialize all fields
; as expected.
define void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.0* %this, i32 %c, %struct.Mem* %mem) {
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

define void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* %this, %struct.Arr* %A) {
entry:
  ret void
}

define void @_ZN3ArrIPiE3setEiPS0_(%struct.Arr* %this, i32 %i, i32** %val) {
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
  %add = add i32 %i, 1
  %idxprom = zext i32 %add to i64
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
  store i32** %val, i32*** %arrayidx6
  ret void
}

define void @_ZN3ArrIPfE3setEiPS0_(%struct.Arr.0* %this, i32 %i, float** %val) {
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
  %1 = load i8, i8* %flag
  %tobool = icmp eq i8 %1, 0
  br i1 %tobool, label %if.end3, label %if.then2

if.then2:                                         ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %2 = load float***, float**** %base
  %add = add i32 %i, 1
  %idxprom = zext i32 %add to i64
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
  store float** %val, float*** %arrayidx6
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* %this, float** %val) {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* %this, i32 1)
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* %this, i32** %val) {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(%struct.Arr* %this, i32 1)
  ret void
}

define void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* %this, i32 %inc) {
entry:
  ret void
}

define void @_ZN3ArrIPiE6resizeEi(%struct.Arr* %this, i32 %inc) {
entry:
  ret void
}

define i32** @_ZN3ArrIPiE3getEi(%struct.Arr* %this, i32 %i) {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %0 = load i32***, i32**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds i32**, i32*** %0, i64 %idxprom
  %1 = load i32**, i32*** %arrayidx
  ret i32** %1
}

define float** @_ZN3ArrIPfE3getEi(%struct.Arr.0* %this, i32 %i) {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %0 = load float***, float**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds float**, float*** %0, i64 %idxprom
  %1 = load float**, float*** %arrayidx
  ret float** %1
}

define void @_ZN3ArrIPiED2Ev(%struct.Arr* %this) {
entry:
  %flag = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  %0 = load i8, i8* %flag
  %tobool = icmp eq i8 %0, 0
  br i1 %tobool, label %if.end, label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %1 = load i32, i32* %size
  %cmp6 = icmp eq i32 %1, 0
  br i1 %cmp6, label %if.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %2 = load i32***, i32**** %base
  %arrayidx = getelementptr inbounds i32**, i32*** %2, i64 %indvars.iv
  %3 = bitcast i32*** %arrayidx to i8**
  %4 = load i8*, i8** %3
  tail call void @free(i8* %4)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %5 = load i32, i32* %size
  %6 = zext i32 %5 to i64
  %cmp = icmp ult i64 %indvars.iv.next, %6
  br i1 %cmp, label %for.body, label %if.end

if.end:                                           ; preds = %for.body, %for.cond.preheader, %entry
  %base2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  ret void
}

define void @_ZN3ArrIPfED2Ev(%struct.Arr.0* %this) {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  %0 = load i8, i8* %flag
  %tobool = icmp eq i8 %0, 0
  br i1 %tobool, label %if.end, label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %1 = load i32, i32* %size
  %cmp6 = icmp eq i32 %1, 0
  br i1 %cmp6, label %if.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %2 = load float***, float**** %base
  %arrayidx = getelementptr inbounds float**, float*** %2, i64 %indvars.iv
  %3 = bitcast float*** %arrayidx to i8**
  %4 = load i8*, i8** %3
  tail call void @free(i8* %4)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %5 = load i32, i32* %size
  %6 = zext i32 %5 to i64
  %cmp = icmp ult i64 %indvars.iv.next, %6
  br i1 %cmp, label %for.body, label %if.end

if.end:                                           ; preds = %for.body, %for.cond.preheader, %entry
  ret void
}

define i32 @_ZN3ArrIPiE7getSizeEv(%struct.Arr* %this) {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  %add = add i32 %0, 1
  ret i32 %add
}

define i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* %this) {
entry:
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %0 = load i32, i32* %capacity
  %shl = shl i32 %0, 1
  ret i32 %shl

}

define i32 @main() {
entry:
  %call = tail call i8* @_Znwm(i64 24)
  %0 = bitcast i8* %call to %class.F*
  tail call void @_ZN1FC2Ev(%class.F* %0)
  ret i32 0
}

define void @_ZN1FC2Ev(%class.F* %this) {
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
  %call6 = tail call i32** @_ZN3ArrIPiE3getEi(%struct.Arr* %4, i32 1)
  %5 = bitcast %struct.Arr1** %f2 to %struct.Arr.0**
  %6 = load %struct.Arr.0*, %struct.Arr.0** %5
  %call8 = tail call float** @_ZN3ArrIPfE3getEi(%struct.Arr.0* %6, i32 1)
  %7 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiE3setEiPS0_(%struct.Arr* %7, i32 0, i32** %call6)
  %8 = load %struct.Arr.0*, %struct.Arr.0** %5
  tail call void @_ZN3ArrIPfE3setEiPS0_(%struct.Arr.0* %8, i32 0, float** %call8)
  %9 = load %struct.Arr.0*, %struct.Arr.0** %5
  tail call void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* %9, float** null)
  %10 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* %10, i32** null)
  %call13 = tail call i8* @_Znwm(i64 32)
  %11 = bitcast i8* %call13 to %struct.Arr*
  %12 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* %11, %struct.Arr* %12)
  tail call void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* %11, i32** null)
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

declare noalias i8* @_Znwm(i64)
declare noalias i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
declare void @__cxa_rethrow()
declare void @free(i8* nocapture)
