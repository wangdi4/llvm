; REQUIRES: system-windows

; This is similar to meminit-transform_01.ll except IR has windows
; specific things like EH, return types for ctor, cctor etc.

; This testcase verifies that MemInitTrimDown transformation is able
; to trim down capacity values that are passed to constructors of
; Arr and Arr1.

; RUN: opt < %s -dtransop-allow-typed-pointers -S -passes=dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt < %s -opaque-pointers -S -passes=dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck --check-prefix=CHECK-OP-TRANS %s

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" will be considered as candidate vector fields.
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
;
;   void resize(int inc) {
;     // 2nd pattern allowed.
;     unsigned int newMax = size + inc;
;     if (newMax <= capacity)
;        return;
;     unsigned int minNewMax = (unsigned int)((double)size * 1.25);
;     if (newMax < minNewMax)
;         newMax = minNewMax;
;     S **newList = (S **) malloc (newMax * sizeof(S*));
;     for (unsigned int index = 0; index < size; index++)
;        newList[index] = base[index];
;
;     free(base); //delete [] fElemList;
;     base = newList;
;     capacity = newMax;
;   }
;
;   void add(S* val) {
;     resize(1);
;     base[size] = val;
;     size++;
;   }
;
;   Arr(const Arr &A) :
;     flag(A.flag), capacity(A.capacity), size(A.size), base(0), mem(A.mem) {
;     base = (S**)malloc(capacity * sizeof(S*));
;     memset(base, 0, capacity * sizeof(S*));
;     for (unsigned int index = 0; index < size; index++)
;     base[index] = A.base[index];
;   }
;
;   unsigned getSize() { return size; }
;   unsigned getCapacity() { return capacity; }
;   void set(unsigned i, S* val) {
;     if (i >= size)
;       throw;
;     base[i] = val;
;   }
;   S* get(unsigned i) {
;     if (i >= size)
;       throw;
;     return base[i];
;   }
;   Arr(unsigned c = 2, Mem *mem = 0)
;      : flag(false), capacity(c), size(0), base(0), mem(mem) {
;    base = (S**)malloc(capacity * sizeof(S*));
;    memset(base, 0, capacity * sizeof(S*));
;  }
;  ~Arr() {
;    free(base);
;  }
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
;    // 4 is propagated to callee in IR.
;    f1 = new Arr<int *>(4, nullptr);
;    f2 = new Arr1<float *>(f1->getCapacity());
;    int** pi = f1->get(1);
;    float** pf = f2->get(1);
;    f1->set(0, pi);
;    f2->set(0, pf);
;    f2->add(nullptr);
;    f1->add(nullptr);
;    Arr<int*>* f3 = new Arr<int *>(*f1);
;    f1->getSize();
;    f1->getCapacity();
;    Arr1<float*>* f4 = new Arr1<float *>(4);
;    delete f1;
;    delete f2;
;  }
;
;};
;int main() {
;  F *f = new F();
;}

; capacity value (i.e 4) that is passed to constructor call of Arr vector
; in "F" function is propagated to "_ZN3ArrIPiEC2EiP3Mem". In addition to
; Store instruction that saves capacity value, memory allocation and
; memset instructions are also fixed since they use capacity values.
;
; CHECK-TRANS: define "intel_dtrans_func_index"="1" %struct.Arr* @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* "intel_dtrans_func_index"="2" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="3" %mem)
;CHECK-TRANS: store i32 1, i32* %capacity
;CHECK-TRANS-NOT: store i32 4, i32* %capacity
;CHECK-TRANS: %call = tail call noalias i8* @malloc(i64 8)
;CHECK-TRANS-NOT: %call = tail call noalias i8* @malloc(i64 %mul)
;CHECK-TRANS: tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 8, i1 false)
;CHECK-TRANS-NOT: tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
;CHECK-TRANS: ret %struct.Arr* %this

;CHECK-TRANS: define "intel_dtrans_func_index"="1" %class.F* @_ZN1FC2Ev(%class.F* "intel_dtrans_func_index"="2" %this)
; CHECK-OP-TRANS: define "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPiEC2EiP3Mem(ptr "intel_dtrans_func_index"="2" %this, i32 %c, ptr "intel_dtrans_func_index"="3" %mem)
;CHECK-OP-TRANS: store i32 1, ptr %capacity
;CHECK-OP-TRANS-NOT: store i32 4, ptr %capacity
;CHECK-OP-TRANS: %call = tail call noalias ptr @malloc(i64 8)
;CHECK-OP-TRANS-NOT: %call = tail call noalias ptr @malloc(i64 %mul)
;CHECK-OP-TRANS: tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 8, i1 false)
;CHECK-OP-TRANS-NOT: tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
;CHECK-OP-TRANS: ret ptr %this

;CHECK-OP-TRANS: define "intel_dtrans_func_index"="1" ptr @_ZN1FC2Ev(ptr "intel_dtrans_func_index"="2" %this)
; Non-constant capacity value that is passed to constructor of Arr1 vector
; in "F" function is not changed.
; CHECK-TRANS: tail call %struct.Arr1* @_ZN4Arr1IPfEC2EiP3Mem(%struct.Arr1* %2, i32 %cap1, %struct.Mem* null)
; CHECK-OP-TRANS: tail call ptr @_ZN4Arr1IPfEC2EiP3Mem(ptr %2, i32 %cap1, ptr null)
;
; capacity value that is passed to constructor of Arr1 vector in "F" function
; is trimmed down from 4 to 1.
;
; CHECK-TRANS: tail call %struct.Arr1* @_ZN4Arr1IPfEC2EiP3Mem(%struct.Arr1* nonnull %17, i32 1, %struct.Mem* null)
; CHECK-TRANS-NOT: tail %struct.Arr1* void @_ZN4Arr1IPfEC2EiP3Mem(%struct.Arr1* nonnull %17, i32 4, %struct.Mem* null)
; CHECK-OP-TRANS: tail call ptr @_ZN4Arr1IPfEC2EiP3Mem(ptr nonnull %17, i32 1, ptr null)
; CHECK-OP-TRANS-NOT: tail ptr void @_ZN4Arr1IPfEC2EiP3Mem(ptr nonnull %17, i32 4, ptr null)

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"
;target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
;target triple = "x86_64-unknown-linux-gnu"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type { i8, i32, i32, i32***, %struct.Mem* }
%struct.Arr1 = type { %struct.Arr.0 }
%struct.Arr.0 = type { i8, i32, i32, float***, %struct.Mem* }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

; All fields of Arr class are initialized as expected.
define "intel_dtrans_func_index"="1" %struct.Arr* @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* "intel_dtrans_func_index"="2" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="3" %mem) !intel.dtrans.func.type !26 {
entry:
  %flag = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  store i8 0, i8* %flag
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  store i32 4, i32* %capacity
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  store i32 0, i32* %size
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  store %struct.Mem* %mem, %struct.Mem** %mem2
  %conv = zext i32 4 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias i8* @malloc(i64 %mul)
  %0 = bitcast i8* %call to i32***
  call void @llvm.dbg.value(metadata i32*** %0, metadata !59, metadata !DIExpression()), !dbg !60
  store i32*** %0, i32**** %base
  tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  ret %struct.Arr* %this
}

; This is the constructor for Derived class. It just calls constructor of
; base class to initialize fields.
define "intel_dtrans_func_index"="1" %struct.Arr1* @_ZN4Arr1IPfEC2EiP3Mem(%struct.Arr1* "intel_dtrans_func_index"="2" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="3" %mem) !intel.dtrans.func.type !27 {
entry:
  %0 = getelementptr inbounds %struct.Arr1, %struct.Arr1* %this, i64 0, i32 0
  %c7 = tail call %struct.Arr.0* @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.0* %0, i32 %c, %struct.Mem* %mem)
  ret %struct.Arr1* %this
}

; This is constructor of base class of Arr1. It initializes all fields
; as expected.
define "intel_dtrans_func_index"="1" %struct.Arr.0* @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.0* "intel_dtrans_func_index"="2" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="3" %mem) !intel.dtrans.func.type !41 {
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
  ret %struct.Arr.0* %this
}

define "intel_dtrans_func_index"="1" %struct.Arr* @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* "intel_dtrans_func_index"="2" %this, %struct.Arr* "intel_dtrans_func_index"="3" %A) !intel.dtrans.func.type !35 {
entry:
  %flag = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 0
  %0 = load i8, i8* %flag2
  store i8 %0, i8* %flag
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 1
  %1 = load i32, i32* %capacity3
  store i32 %1, i32* %capacity
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %size4 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 2
  %2 = load i32, i32* %size4
  store i32 %2, i32* %size
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 4
  %3 = bitcast %struct.Mem** %mem5 to i64*
  %4 = load i64, i64* %3
  %5 = bitcast %struct.Mem** %mem to i64*
  store i64 %4, i64* %5
  %conv = zext i32 %1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias i8* @malloc(i64 %mul)
  %6 = bitcast i32**** %base to i8**
  store i8* %call, i8** %6
  tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 3
  %7 = load i32***, i32**** %base13
  %8 = load i32***, i32**** %base
  %wide.trip.count = zext i32 %2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret %struct.Arr* %this

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32**, i32*** %7, i64 %indvars.iv
  %9 = bitcast i32*** %arrayidx to i64*
  %10 = load i64, i64* %9
  %arrayidx16 = getelementptr inbounds i32**, i32*** %8, i64 %indvars.iv
  %11 = bitcast i32*** %arrayidx16 to i64*
  store i64 %10, i64* %11
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define void @_ZN3ArrIPiE3setEiPS0_(%struct.Arr* "intel_dtrans_func_index"="1" %this, i32 %i, i32** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !31 {
entry:
  %ai = alloca i64, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  %cmp = icmp ugt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %bc1 = bitcast i64* %ai to i8*
  call void @_CxxThrowException(i8* nonnull %bc1, %eh.ThrowInfo* null)
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %1 = load i32***, i32**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds i32**, i32*** %1, i64 %idxprom
  store i32** %val, i32*** %arrayidx
  ret void
}

define void @_ZN3ArrIPfE3setEiPS0_(%struct.Arr.0* "intel_dtrans_func_index"="1" %this, i32 %i, float** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !32 {
entry:
  %ai = alloca i64, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  %cmp = icmp ugt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %bc1 = bitcast i64* %ai to i8*
  call void @_CxxThrowException(i8* nonnull %bc1, %eh.ThrowInfo* null)
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %1 = load float***, float**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds float**, float*** %1, i64 %idxprom
  store float** %val, float*** %arrayidx
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* "intel_dtrans_func_index"="1" %this, float** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !33 {
entry:
  call void @llvm.dbg.value(metadata i32 0, metadata !59, metadata !DIExpression()), !dbg !60
  tail call void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* %this, i32 1)
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %0 = load float***, float**** %base
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %1 = load i32, i32* %size
  %idxprom = zext i32 %1 to i64
  %arrayidx = getelementptr inbounds float**, float*** %0, i64 %idxprom
  store float** %val, float*** %arrayidx
  %inc = add i32 %1, 1
  store i32 %inc, i32* %size
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* "intel_dtrans_func_index"="1" %this, i32** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !34 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(%struct.Arr* %this, i32 1)
  call void @llvm.dbg.value(metadata i32 0, metadata !59, metadata !DIExpression()), !dbg !60
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %0 = load i32***, i32**** %base
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %1 = load i32, i32* %size
  %idxprom = zext i32 %1 to i64
  %arrayidx = getelementptr inbounds i32**, i32*** %0, i64 %idxprom
  store i32** %val, i32*** %arrayidx
  %inc = add i32 %1, 1
  store i32 %inc, i32* %size
  ret void
}

define void @_ZN3ArrIPiE6resizeEi(%struct.Arr* "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !43 {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  %add = add i32 %0, 1
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %1 = load i32, i32* %capacity
  %cmp = icmp ugt i32 %add, %1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %0 to double
  %mul = fmul double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = tail call noalias i8* @malloc(i64 %mul8)
  %2 = bitcast i8* %call to i32***
  %cmp1029 = icmp eq i32 %0, 0
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %3 = load i32***, i32**** %base
  %wide.trip.count = zext i32 %0 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %4 = bitcast i32**** %base to i8**
  %5 = load i8*, i8** %4
  tail call void @free(i8* %5)
  store i8* %call, i8** %4
  store i32 %spec.select, i32* %capacity
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32**, i32*** %3, i64 %indvars.iv
  %6 = bitcast i32*** %arrayidx to i64*
  %7 = load i64, i64* %6
  %arrayidx12 = getelementptr inbounds i32**, i32*** %2, i64 %indvars.iv
  %8 = bitcast i32*** %arrayidx12 to i64*
  store i64 %7, i64* %8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %entry, %for.cond.cleanup
  ret void
}

define void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !42 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  %add = add i32 %0, 1
  %capacity = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  %1 = load i32, i32* %capacity
  %cmp = icmp ugt i32 %add, %1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %0 to double
  %mul = fmul double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = tail call noalias i8* @malloc(i64 %mul8)
  %2 = bitcast i8* %call to float***
  %cmp1029 = icmp eq i32 %0, 0
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %3 = load float***, float**** %base
  %wide.trip.count = zext i32 %0 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %4 = bitcast float**** %base to i8**
  %5 = load i8*, i8** %4
  tail call void @free(i8* %5)
  store i8* %call, i8** %4
  store i32 %spec.select, i32* %capacity
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float**, float*** %3, i64 %indvars.iv
  %6 = bitcast float*** %arrayidx to i64*
  %7 = load i64, i64* %6
  %arrayidx12 = getelementptr inbounds float**, float*** %2, i64 %indvars.iv
  %8 = bitcast float*** %arrayidx12 to i64*
  store i64 %7, i64* %8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %entry, %for.cond.cleanup
  ret void
}

define "intel_dtrans_func_index"="1" i32** @_ZN3ArrIPiE3getEi(%struct.Arr* "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !28 {
entry:
  %ai = alloca i64, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  %cmp = icmp ugt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %bc1 = bitcast i64* %ai to i8*
  call void @_CxxThrowException(i8* nonnull %bc1, %eh.ThrowInfo* null)
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %1 = load i32***, i32**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds i32**, i32*** %1, i64 %idxprom
  %2 = load i32**, i32*** %arrayidx
  ret i32** %2
}

define "intel_dtrans_func_index"="1" float** @_ZN3ArrIPfE3getEi(%struct.Arr.0* "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !29 {
entry:
  %ai = alloca i64, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %0 = load i32, i32* %size, align 8
  %cmp = icmp ugt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %bc1 = bitcast i64* %ai to i8*
  call void @_CxxThrowException(i8* nonnull %bc1, %eh.ThrowInfo* null)
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %1 = load float***, float**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds float**, float*** %1, i64 %idxprom
  %2 = load float**, float*** %arrayidx
  ret float** %2
}

define void @_ZN3ArrIPiED2Ev(%struct.Arr* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !38 {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %0 = bitcast i32**** %base to i8**
  %1 = load i8*, i8** %0
  tail call void @free(i8* %1)
  ret void
}

define void @_ZN3ArrIPfED2Ev(%struct.Arr.0* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !45 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %0 = bitcast float**** %base to i8**
  %1 = load i8*, i8** %0
  tail call void @free(i8* %1)
  ret void
}

define i32 @_ZN3ArrIPiE7getSizeEv(%struct.Arr* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !36 {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size
  ret i32 %0
}

define i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !37 {
entry:
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %0 = load i32, i32* %capacity
  ret i32 %0
}

define i32 @main() {
entry:
  %call = tail call i8* @_Znwm(i64 24)
  %0 = bitcast i8* %call to %class.F*
  %cc1 = tail call %class.F* @_ZN1FC2Ev(%class.F* %0)
  ret i32 0
}

define "intel_dtrans_func_index"="1" %class.F* @_ZN1FC2Ev(%class.F* "intel_dtrans_func_index"="2" %this) !intel.dtrans.func.type !22 {
entry:
  %call = tail call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %struct.Arr*
  %c1 = tail call %struct.Arr* @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %0, i32 4, %struct.Mem* null)
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %1 = bitcast %struct.Arr** %f1 to i8**
  store %struct.Arr* %0, %struct.Arr** %f1
  %call2 = tail call i8* @_Znwm(i64 32)
  %2 = bitcast i8* %call2 to %struct.Arr1*
  %cap1 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* nonnull %0)
  %c2 = tail call %struct.Arr1* @_ZN4Arr1IPfEC2EiP3Mem(%struct.Arr1* %2, i32 %cap1, %struct.Mem* null)
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
  %c3 = tail call %struct.Arr* @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* %11, %struct.Arr* %12)
  %s = load %struct.Arr*, %struct.Arr** %f1, align 8
  %call18 = tail call i32 @_ZN3ArrIPiE7getSizeEv(%struct.Arr* %s)
  %d = load %struct.Arr*, %struct.Arr** %f1, align 8
  %call20 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* %d)
  %13 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiED2Ev(%struct.Arr* %13)
  %14 = load %struct.Arr1*, %struct.Arr1** %f2
  %15 = getelementptr inbounds %struct.Arr1, %struct.Arr1* %14, i64 0, i32 0
  tail call void @_ZN3ArrIPfED2Ev(%struct.Arr.0* %15)
  %16 = tail call i8* @_Znwm(i64 32) #8
  %17 = bitcast i8* %16 to %struct.Arr1*
  %d5 = tail call %struct.Arr1* @_ZN4Arr1IPfEC2EiP3Mem(%struct.Arr1* nonnull %17, i32 4, %struct.Mem* null)

  ret %class.F* %this
}

declare !intel.dtrans.func.type !20 noalias "intel_dtrans_func_index"="1" i8* @_Znwm(i64)
declare !intel.dtrans.func.type !40 noalias "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
declare !intel.dtrans.func.type !70 void @_CxxThrowException(i8* "intel_dtrans_func_index"="1", %eh.ThrowInfo* "intel_dtrans_func_index"="2")
declare !intel.dtrans.func.type !44 void @free(i8* "intel_dtrans_func_index"="1" nocapture) #1
declare void @llvm.dbg.value(metadata, metadata, metadata)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!3, !7, !11, !14, !16, !64}
!llvm.dbg.cu = !{!46}
!llvm.module.flags = !{!49, !50, !51}
!llvm.dbg.intel.emit_class_debug_always = !{!52}
!llvm.ident = !{!53}

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
!22 = distinct !{!19, !19}
!23 = !{i32 0, i32 2}
!24 = !{float 0.000000e+00, i32 2}
!25 = distinct !{!21}
!26 = distinct !{!5, !5, !4}
!27 = distinct !{!6, !6, !4}
!28 = distinct !{!23, !5}
!29 = distinct !{!24, !30}
!30 = !{%struct.Arr.0 zeroinitializer, i32 1}
!31 = distinct !{!5, !23}
!32 = distinct !{!30, !24}
!33 = distinct !{!30, !24}
!34 = distinct !{!5, !23}
!35 = distinct !{!5, !5, !5}
!36 = distinct !{!5}
!37 = distinct !{!5}
!38 = distinct !{!5}
!39 = distinct !{!6}
!40 = distinct !{!21}
!41 = distinct !{!30, !30, !4}
!42 = distinct !{!30}
!43 = distinct !{!5}
!44 = distinct !{!21}
!45 = distinct !{!30}
!64 = !{!"S", %eh.ThrowInfo zeroinitializer, i32 4, !10, !10, !10, !10}
!65 = !{%eh.ThrowInfo zeroinitializer, i32 1}
!70 = distinct !{!21, !65}

!46 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !47, producer: "", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !48, nameTableKind: None)
!47 = !DIFile(filename: "test", directory: ".")
!48 = !{}
!49 = !{i32 2, !"Dwarf Version", i32 4}
!50 = !{i32 2, !"Debug Info Version", i32 3}
!51 = !{i32 1, !"wchar_size", i32 4}
!52 = !{!"true"}
!53 = !{!""}
!54 = distinct !DISubprogram(name: "na", linkageName: "na", scope: !47, file: !47, line: 1, type: !55, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !46, retainedNodes: !48)
; int(void*) type.
!55 = !DISubroutineType(types: !56)
!56 = !{!57, !58}
!57 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!58 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!59 = !DILocalVariable(name: "na", arg: 1, scope: !54, file: !47, line: 1, type: !58)
!60 = !DILocation(line: 1, column: 1, scope: !54)
!61 = !DILocation(line: 1, column: 1, scope: !54)
!62 = !DILocation(line: 1, column: 1, scope: !54)
