; This testcase verifies that MemInitTrimDown transformation is
; recognizing destructor and its wrapper of Arr1 class and resize
; member functions of Arr and Arr1 classes.

; RUN: opt < %s -dtrans-meminit-recognize-all -dtrans-meminittrimdown -enable-dtrans-meminittrimdown -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdown,dtrans-soatoaosclassinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtrans-meminit-recognize-all -passes=dtrans-meminittrimdown -enable-dtrans-meminittrimdown -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdown,dtrans-soatoaosclassinfo -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" will be considered as candidate vector fields.
; This test verified that the following functions are recognized.
;
; destructor & its wrapper: Two routines will be generated to handle
; "~Arr1()" virtual function.
;
; resize: IR for _ZN3ArrIPfE6resizeEi and _ZN3ArrIPiE6resizeEi definitions
; are not same even though they are same at source level.
; "size" argument of memset in _ZN3ArrIPiE6resizeEi matches with 1st pattern.
; "size" argument of memset in _ZN3ArrIPfE6resizeEi matches with 2nd pattern.
;
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
;
;   void resize(int inc) {
;     unsigned int newMax = size + inc;
;     if (newMax <= capacity)
;        return;
;
;    if (newMax < capacity + capacity/2)
;      newMax = capacity + capacity/2;
;
;     S **newList = (S **) malloc (newMax * sizeof(S*));
;    unsigned int index = 0;
;    for (; index < size; index++)
;        newList[index] = base[index];
;    for (; index < newMax; index++)
;        base[index] = 0;
;
;    free(base);
;    base = newList;
;    capacity = newMax;
;   }
;
;   virtual ~Arr() {
;   }

;   void add(S* val) {
;     resize(1);
;   }
;   void set(unsigned i, S* val) {
;   }
;   S* get(unsigned i) {
;   }
;   Arr(unsigned c = 2, Mem *mem = 0)
;      : flag(false), capacity(c), size(0), base(0), mem(mem) {
;    base = (S**)malloc(capacity * sizeof(S*));
;    memset(base, 0, capacity * sizeof(S*));
;   }
;   Arr(const Arr &A) { }
;};
;
;template <typename S>
;struct Arr1 : public Arr<S> {
; public:
;   Arr1(unsigned c = 2, Mem *mem = 0);
;   ~Arr1() {
;     free(this->base);
;   }
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
;
;

;CHECK: MemInitTrimDown transformation:
;CHECK:  Analyzing Constructor _ZN3ArrIPiEC2EiP3Mem
;CHECK:  Passed: Constructor recognized
;
;CHECK:  Functionality of _ZN3ArrIPiE6resizeEi: Recognized as Resize

;CHECK:  Analyzing Constructor _ZN4Arr1IPfEC2EiP3Mem
;CHECK:  Passed: Constructor recognized
;
;CHECK-DAG:  Functionality of _ZN3ArrIPfE6resizeEi: Recognized as Resize
;CHECK-DAG:  Functionality of _ZN4Arr1IPfED0Ev: Recognized as DestructorWrapper
;CHECK-DAG:  Functionality of _ZN4Arr1IPfED2Ev: Recognized as Destructor

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type { i32 (...)**, i8, i32, i32, i32***, %struct.Mem* }
%struct.Arr1 = type { %struct.Arr.0 }
%struct.Arr.0 = type { i32 (...)**, i8, i32, i32, float***, %struct.Mem* }

@_ZTV4Arr1IPfE = constant { [4 x i8*] } { [4 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* null to i8*), i8* bitcast (void (%struct.Arr1*)* null to i8*), i8* bitcast (void (%struct.Arr1*)* null to i8*)] }

; This is actual destructor.
define void @_ZN4Arr1IPfED2Ev(%struct.Arr1* %this) {
entry:
  %0 = getelementptr inbounds %struct.Arr1, %struct.Arr1* %this, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [4 x i8*] }, { [4 x i8*] }* @_ZTV4Arr1IPfE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %0
  %1 = getelementptr inbounds %struct.Arr1, %struct.Arr1* %this, i64 0, i32 0
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %1, i64 0, i32 4
  %2 = bitcast float**** %base to i8**
  %3 = load i8*, i8** %2
  tail call void @free(i8* %3)
  ret void
}

; This is wrapper to actual destructor.
define void @_ZN4Arr1IPfED0Ev(%struct.Arr1* %this) {
entry:
  tail call void @_ZN4Arr1IPfED2Ev(%struct.Arr1* %this)
  %0 = bitcast %struct.Arr1* %this to i8*
  tail call void @_ZdlPv(i8* %0)
  ret void
}

; _ZN3ArrIPfE6resizeEi: This is same as _ZN3ArrIPiE6resizeEi except
; size of memset matches with 2nd pattern.
define void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* %this, i32 %inc) {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %0 = load i32, i32* %size
  %add = add i32 %0, 1
  %capacity = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %1 = load i32, i32* %capacity
  %cmp = icmp ugt i32 %add, %1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %div = lshr i32 %1, 1
  %add4 = add i32 %div, %1
  %cmp5 = icmp ult i32 %add, %add4
  %spec.select = select i1 %cmp5, i32 %add4, i32 %add
  %conv = zext i32 %spec.select to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias i8* @malloc(i64 %mul)
  %2 = bitcast i8* %call to float***
  %cmp1344 = icmp eq i32 %0, 0
  br i1 %cmp1344, label %for.cond16.preheader, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 4
  %3 = load float***, float**** %base
  %wide.trip.count = zext i32 %0 to i64
  br label %for.body

shrink:                                              ; preds = %for.body
  %tr1 = trunc i64 %indvars.iv.next to i32
  br label %for.cond16.preheader

for.cond16.preheader:                             ; preds = %shrink, %if.end
  %ph1 = phi i32 [ 0, %if.end ], [ %tr1, %shrink ]
  %cmp1742 = icmp ult i32 %ph1, %spec.select
  br i1 %cmp1742, label %for.body18.preheader, label %for.end23

for.body18.preheader:                             ; preds = %for.cond16.preheader
  %4 = shl nuw nsw i32 %ph1, 3
  %scevgep = getelementptr i8, i8* %call, i32 %4
  %5 = shl nuw nsw i32 %spec.select, 3
  %6 = sub i32 %5, %4
  call void @llvm.memset.p0i8.i32(i8* align 8 %scevgep, i8 0, i32 %6, i1 false)
  br label %for.end23

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %7 = getelementptr inbounds float**, float*** %3, i64 %indvars.iv
  %8 = bitcast float*** %7 to i64*
  %9 = load i64, i64* %8
  %arrayidx15 = getelementptr inbounds float**, float*** %2, i64 %indvars.iv
  %10 = bitcast float*** %arrayidx15 to i64*
  store i64 %9, i64* %10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %shrink, label %for.body

for.end23:                                        ; preds = %for.body18.preheader, %for.cond16.preheader
  %base24 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 4
  %11 = bitcast float**** %base24 to i8**
  %12 = load i8*, i8** %11
  tail call void @free(i8* %12)
  store float*** %2, float**** %base24
  store i32 %spec.select, i32* %capacity
  br label %cleanup

cleanup:                                          ; preds = %entry, %for.end23
  ret void
}

; _ZN3ArrIPiE6resizeEi: The remainder loop, which resets base[index]
; to zero, in "resize" function is converted to memset. Size of memset
; matches with 1st pattern.
;
define void @_ZN3ArrIPiE6resizeEi(%struct.Arr* %this, i32 %inc) {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %0 = load i32, i32* %size
  %add = add i32 %0, 1
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %1 = load i32, i32* %capacity
  %cmp = icmp ugt i32 %add, %1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %div = lshr i32 %1, 1
  %add4 = add i32 %div, %1
  %cmp5 = icmp ult i32 %add, %add4
  %spec.select = select i1 %cmp5, i32 %add4, i32 %add
  %conv = zext i32 %spec.select to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias i8* @malloc(i64 %mul)
  %2 = bitcast i8* %call to i32***
  %cmp1344 = icmp eq i32 %0, 0
  br i1 %cmp1344, label %for.cond16.preheader, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  %3 = load i32***, i32**** %base
  %wide.trip.count = zext i32 %0 to i64
  br label %for.body

for.cond16.preheader:                             ; preds = %for.body, %if.end
  %cmp1742 = icmp ult i32 %0, %spec.select
  br i1 %cmp1742, label %for.body18.preheader, label %for.end23

for.body18.preheader:                             ; preds = %for.cond16.preheader
  %4 = zext i32 %0 to i64
  %5 = shl nuw nsw i64 %4, 3
  %scevgep = getelementptr i8, i8* %call, i64 %5
  %6 = xor i32 %0, -1
  %7 = add i32 %spec.select, %6
  %8 = zext i32 %7 to i64
  %9 = shl nuw nsw i64 %8, 3
  %10 = add nuw nsw i64 %9, 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %scevgep, i8 0, i64 %10, i1 false)
  br label %for.end23

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32**, i32*** %3, i64 %indvars.iv
  %11 = bitcast i32*** %arrayidx to i64*
  %12 = load i64, i64* %11
  %arrayidx15 = getelementptr inbounds i32**, i32*** %2, i64 %indvars.iv
  %13 = bitcast i32*** %arrayidx15 to i64*
  store i64 %12, i64* %13
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond16.preheader, label %for.body

for.end23:                                        ; preds = %for.body18.preheader, %for.cond16.preheader
  %base24 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  %14 = bitcast i32**** %base24 to i8**
  %15 = load i8*, i8** %14
  tail call void @free(i8* %15)
  store i32*** %2, i32**** %base24
  store i32 %spec.select, i32* %capacity
  br label %cleanup

cleanup:                                          ; preds = %entry, %for.end23
  ret void
}

; All fields of Arr class are initialized as expected.
define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %this, i32 %c, %struct.Mem* %mem) {
entry:
  %flag = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  store i8 0, i8* %flag
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  store i32 %c, i32* %capacity
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  store i32 0, i32* %size
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 5
  store %struct.Mem* %mem, %struct.Mem** %mem2
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias i8* @malloc(i64 %mul)
  %0 = bitcast i8* %call to i32***
  store i32*** %0, i32**** %base
  tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

; This is constructor of base class of Arr1. It initialize all fields
; as expected.
define void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.0* %this, i32 %c, %struct.Mem* %mem) {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  store i8 0, i8* %flag
  %capacity = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  store i32 %c, i32* %capacity
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  store i32 0, i32* %size
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 4
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 5
  store %struct.Mem* %mem, %struct.Mem** %mem2
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias i8* @malloc(i64 %mul)
  %0 = bitcast i8* %call to float***
  store float*** %0, float**** %base
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

define void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* %this, %struct.Arr* %A) {
entry:
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
  tail call void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* %this, i32 1)
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* %this, i32** %val) {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(%struct.Arr* %this, i32 1)
  ret void
}
define i32** @_ZN3ArrIPiE3getEi(%struct.Arr* %this, i32 %i) {
entry:
  ret i32** null
}

define float** @_ZN3ArrIPfE3getEi(%struct.Arr.0* %this, i32 %i) {
entry:
  ret float** null
}

define void @_ZN3ArrIPiED2Ev(%struct.Arr* %this) {
entry:
  ret void
}

define i32 @_ZN3ArrIPiE7getSizeEv(%struct.Arr* %this) {
entry:
  ret i32 0
}

define i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* %this) {
entry:
  ret i32 0

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
  tail call void @_ZN4Arr1IPfED0Ev(%struct.Arr1* %14)
  ret void
}

declare noalias i8* @_Znwm(i64)
declare noalias i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
declare void @llvm.memset.p0i8.i32(i8* nocapture writeonly, i8, i32, i1 immarg)
declare void @__cxa_rethrow()
declare void @free(i8* nocapture)
declare void @_ZdlPv(i8* nocapture)
