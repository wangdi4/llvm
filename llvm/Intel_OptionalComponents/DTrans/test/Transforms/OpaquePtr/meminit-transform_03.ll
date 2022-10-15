; This testcase verifies that MemInitTrimDown transformation is able
; to trim down capacity values that are passed to constructors of
; Arr and Arr1.

; RUN: opt < %s -opaque-pointers -S -dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -S -passes=dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

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
; CHECK: define void @_ZN3ArrIPiEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem)
; CHECK: store i32 1, ptr %capacity
; CHECK-NOT: store i32 4, ptr %capacity
; CHECK: %call = tail call noalias ptr @malloc(i64 8)
; CHECK-NOT: %call = tail call noalias ptr @malloc(i64 %mul)
; CHECK: tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 8, i1 false)
; CHECK-NOT: tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
; CHECK: ret void

; CHECK: define void @_ZN1FC2Ev(ptr "intel_dtrans_func_index"="1" %this)
; Non-constant capacity value that is passed to constructor of Arr1 vector
; in "F" function is not changed.
; CHECK: tail call void @_ZN4Arr1IPfEC2EiP3Mem(ptr %call2, i32 %cap1, ptr null)
;
; capacity value that is passed to constructor of Arr1 vector in "F" function
; is trimmed down from 4 to 1.
;
; CHECK: tail call void @_ZN4Arr1IPfEC2EiP3Mem(ptr nonnull %i16, i32 1, ptr null)
; CHECK-NOT: tail call void @_ZN4Arr1IPfEC2EiP3Mem(ptr nonnull %i16, i32 4, ptr null)


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Arr = type { i8, i32, i32, ptr, ptr }
%struct.Arr1 = type { %struct.Arr.0 }
%struct.Arr.0 = type { i8, i32, i32, ptr, ptr }
%class.F = type { ptr, ptr, ptr }
%struct.Mem = type { ptr }

define void @_ZN3ArrIPiEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !23 {
entry:
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0
  store i8 0, ptr %flag, align 1
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  store i32 4, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  store i32 0, ptr %size, align 4
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %mem2 = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2, align 8
  %conv = zext i32 4 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias ptr @malloc(i64 %mul)
  call void @llvm.dbg.value(metadata ptr %call, metadata !24, metadata !DIExpression()), !dbg !30
  store ptr %call, ptr %base, align 8
  tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  %memgep = getelementptr inbounds %struct.Mem, ptr %mem, i64 0, i32 0
  %vtable = load ptr, ptr %memgep, align 8
  %tt = tail call i1 @llvm.type.test(ptr %vtable, metadata !"typeId")
  tail call void @llvm.assume(i1 %tt)
  ret void
}

define void @_ZN4Arr1IPfEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !31 {
entry:
  %i = getelementptr inbounds %struct.Arr1, ptr %this, i64 0, i32 0
  tail call void @_ZN3ArrIPfEC2EiP3Mem(ptr %i, i32 %c, ptr %mem)
  ret void
}

define void @_ZN3ArrIPfEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !32 {
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

define void @_ZN3ArrIPiEC2ERKS1_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !34 {
entry:
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 0
  %i = load i8, ptr %flag2, align 1
  store i8 %i, ptr %flag, align 1
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 1
  %i1 = load i32, ptr %capacity3, align 4
  store i32 %i1, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %size4 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 2
  %i2 = load i32, ptr %size4, align 4
  store i32 %i2, ptr %size, align 4
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %mem = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 4
  %i4 = load i64, ptr %mem5, align 8
  store i64 %i4, ptr %mem, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias ptr @malloc(i64 %mul)
  store ptr %call, ptr %base, align 8
  tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 3
  %i7 = load ptr, ptr %base13, align 8
  %i8 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %i7, i64 %indvars.iv
  %i10 = load i64, ptr %arrayidx, align 8
  %arrayidx16 = getelementptr inbounds ptr, ptr %i8, i64 %indvars.iv
  store i64 %i10, ptr %arrayidx16, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define void @_ZN3ArrIPiE3setEiPS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !35 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size, align 4
  %cmp = icmp ugt i32 %i1, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i2 = load ptr, ptr %base, align 8
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i2, i64 %idxprom
  store ptr %val, ptr %arrayidx, align 8
  ret void
}

define void @_ZN3ArrIPfE3setEiPS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !37 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size, align 4
  %cmp = icmp ugt i32 %i1, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i2 = load ptr, ptr %base, align 8
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i2, i64 %idxprom
  store ptr %val, ptr %arrayidx, align 8
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !39 {
entry:
  call void @llvm.dbg.value(metadata i32 0, metadata !24, metadata !DIExpression()), !dbg !30
  tail call void @_ZN3ArrIPfE6resizeEi(ptr %this, i32 1)
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i = load ptr, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size, align 4
  %idxprom = zext i32 %i1 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i, i64 %idxprom
  store ptr %val, ptr %arrayidx, align 8
  %inc = add i32 %i1, 1
  store i32 %inc, ptr %size, align 4
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !40 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(ptr %this, i32 1)
  call void @llvm.dbg.value(metadata i32 0, metadata !24, metadata !DIExpression()), !dbg !30
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i = load ptr, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size, align 4
  %idxprom = zext i32 %i1 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i, i64 %idxprom
  store ptr %val, ptr %arrayidx, align 8
  %inc = add i32 %i1, 1
  store i32 %inc, ptr %size, align 4
  ret void
}

define void @_ZN3ArrIPiE6resizeEi(ptr "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !41 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i = load i32, ptr %size, align 4
  %add = add i32 %i, 1
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  %i1 = load i32, ptr %capacity, align 4
  %cmp = icmp ugt i32 %add, %i1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %i to double
  %mul = fmul double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = tail call noalias ptr @malloc(i64 %mul8)
  %cmp1029 = icmp eq i32 %i, 0
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i3 = load ptr, ptr %base, align 8
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %wide.trip.count = zext i32 %i to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  tail call void @free(ptr %i3)
  store ptr %call, ptr %base, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %i3, i64 %indvars.iv
  %i7 = load i64, ptr %arrayidx, align 8
  %arrayidx12 = getelementptr inbounds ptr, ptr %call, i64 %indvars.iv
  store i64 %i7, ptr %arrayidx12, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

define void @_ZN3ArrIPfE6resizeEi(ptr "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !42 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i = load i32, ptr %size, align 4
  %add = add i32 %i, 1
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  %i1 = load i32, ptr %capacity, align 4
  %cmp = icmp ugt i32 %add, %i1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %i to double
  %mul = fmul double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = tail call noalias ptr @malloc(i64 %mul8)
  %cmp1029 = icmp eq i32 %i, 0
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i3 = load ptr, ptr %base, align 8
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %wide.trip.count = zext i32 %i to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  tail call void @free(ptr %i3)
  store ptr %call, ptr %base, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %i3, i64 %indvars.iv
  %i7 = load i64, ptr %arrayidx, align 8
  %arrayidx12 = getelementptr inbounds ptr, ptr %call, i64 %indvars.iv
  store i64 %i7, ptr %arrayidx12, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

define "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPiE3getEi(ptr "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !43 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size, align 4
  %cmp = icmp ugt i32 %i1, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i2 = load ptr, ptr %base, align 8
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i2, i64 %idxprom
  %i3 = load ptr, ptr %arrayidx, align 8
  ret ptr %i3
}

define "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPfE3getEi(ptr "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !44 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size, align 8
  %cmp = icmp ugt i32 %i1, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i2 = load ptr, ptr %base, align 8
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i2, i64 %idxprom
  %i3 = load ptr, ptr %arrayidx, align 8
  ret ptr %i3
}

define void @_ZN3ArrIPiED2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !45 {
entry:
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i1 = load ptr, ptr %base, align 8
  tail call void @free(ptr %i1)
  ret void
}

define void @_ZN3ArrIPfED2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !46 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i1 = load ptr, ptr %base, align 8
  tail call void @free(ptr %i1)
  ret void
}

define i32 @_ZN3ArrIPiE7getSizeEv(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !47 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i = load i32, ptr %size, align 4
  ret i32 %i
}

define i32 @_ZN3ArrIPiE11getCapacityEv(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !48 {
entry:
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  %i = load i32, ptr %capacity, align 4
  ret i32 %i
}

define i32 @main() {
entry:
  %call = tail call ptr @_Znwm(i64 24)
  tail call void @_ZN1FC2Ev(ptr %call)
  ret i32 0
}

define void @_ZN1FC2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !49 {
entry:
  %call = tail call ptr @_Znwm(i64 32)
  tail call void @_ZN3ArrIPiEC2EiP3Mem(ptr %call, i32 4, ptr null)
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  store ptr %call, ptr %f1, align 8
  %call2 = tail call ptr @_Znwm(i64 32)
  %cap1 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(ptr nonnull %call)
  tail call void @_ZN4Arr1IPfEC2EiP3Mem(ptr %call2, i32 %cap1, ptr null)
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
  %s = load ptr, ptr %f1, align 8
  %call18 = tail call i32 @_ZN3ArrIPiE7getSizeEv(ptr %s)
  %d = load ptr, ptr %f1, align 8
  %call20 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(ptr %d)
  %i13 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiED2Ev(ptr %i13)
  %i14 = load ptr, ptr %f2, align 8
  %i15 = getelementptr inbounds %struct.Arr1, ptr %i14, i64 0, i32 0
  tail call void @_ZN3ArrIPfED2Ev(ptr %i15)
  %i16 = tail call ptr @_Znwm(i64 32)
  tail call void @_ZN4Arr1IPfEC2EiP3Mem(ptr nonnull %i16, i32 4, ptr null)
  ret void
}

declare !intel.dtrans.func.type !51 noalias "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !53 noalias "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

declare void @__cxa_rethrow()

; Function Attrs: allockind("free")
declare !intel.dtrans.func.type !54 void @free(ptr nocapture "intel_dtrans_func_index"="1") #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata) #2

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #3

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #4

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #4 = { argmemonly nocallback nofree nounwind willreturn writeonly }

!intel.dtrans.types = !{!0, !4, !8, !11, !13}
!llvm.dbg.cu = !{!15}
!llvm.module.flags = !{!18, !19, !20}
!llvm.dbg.intel.emit_class_debug_always = !{!21}
!llvm.ident = !{!22}

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
!15 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !16, isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !17, nameTableKind: None)
!16 = !DIFile(filename: "test", directory: ".")
!17 = !{}
!18 = !{i32 2, !"Dwarf Version", i32 4}
!19 = !{i32 2, !"Debug Info Version", i32 3}
!20 = !{i32 1, !"wchar_size", i32 4}
!21 = !{!"true"}
!22 = !{!""}
!23 = distinct !{!2, !1}
!24 = !DILocalVariable(name: "na", arg: 1, scope: !25, file: !16, line: 1, type: !29)
!25 = distinct !DISubprogram(name: "na", linkageName: "na", scope: !16, file: !16, line: 1, type: !26, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !15, retainedNodes: !17)
!26 = !DISubroutineType(types: !27)
!27 = !{!28, !29}
!28 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!29 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!30 = !DILocation(line: 1, column: 1, scope: !25)
!31 = distinct !{!3, !1}
!32 = distinct !{!33, !1}
!33 = !{%struct.Arr.0 zeroinitializer, i32 1}
!34 = distinct !{!2, !2}
!35 = distinct !{!2, !36}
!36 = !{i32 0, i32 2}
!37 = distinct !{!33, !38}
!38 = !{float 0.000000e+00, i32 2}
!39 = distinct !{!33, !38}
!40 = distinct !{!2, !36}
!41 = distinct !{!2}
!42 = distinct !{!33}
!43 = distinct !{!36, !2}
!44 = distinct !{!38, !33}
!45 = distinct !{!2}
!46 = distinct !{!33}
!47 = distinct !{!2}
!48 = distinct !{!2}
!49 = distinct !{!50}
!50 = !{%class.F zeroinitializer, i32 1}
!51 = distinct !{!52}
!52 = !{i8 0, i32 1}
!53 = distinct !{!52}
!54 = distinct !{!52}
