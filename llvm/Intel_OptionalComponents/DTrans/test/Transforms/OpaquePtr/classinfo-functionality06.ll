; This testcase verifies that MemInitTrimDown transformation is able
; to recognize Resize(resize) member function of Arr and Arr1 classes.

; RUN: opt < %s -dtrans-meminitop-recognize-all -passes=dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop,dtrans-soatoaosopclassinfo -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" will be considered as candidate vector fields.
;
; This test verified that the following functions are recognized since
; they are in expected pattern.
; resize

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
;     // 1st pattern allowed.
;     unsigned int newMax = size + inc;
;     if (newMax <= capacity)
;        return;
;
;     if (newMax < capacity + capacity/2)
;        newMax = capacity + capacity/2;
;
;     S **newList = (S **) malloc (newMax * sizeof(S*));
;     unsigned int index = 0;
;     for (; index < size; index++)
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
;   void set(unsigned i, S* val) { }
;   S* get(unsigned i) { return nullptr; }
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
;CHECK-DAG:  Functionality of _ZN3ArrIPiE6resizeEi: Recognized as Resize

;CHECK:  Analyzing Constructor _ZN4Arr1IPfEC2EiP3Mem
;CHECK:  Passed: Constructor recognized
;CHECK-DAG:  Functionality of _ZN3ArrIPfE6resizeEi: Recognized as Resize

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
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 0
  %i0 = load i8, ptr %flag2
  store i8 %i0, ptr %flag
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 1
  %i1 = load i32, ptr %capacity3
  store i32 %i1, ptr %capacity
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %size4 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 2
  %i2 = load i32, ptr %size4
  store i32 %i2, ptr %size
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %mem = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 4
  %i4 = load i64, ptr %mem5
  store i64 %i4, ptr %mem
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias ptr @malloc(i64 %mul)
  store ptr %call, ptr %base
  tail call void @llvm.memset.p0i8.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 3
  %i7 = load ptr, ptr %base13
  %i8 = load ptr, ptr %base
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %i7, i64 %indvars.iv
  %i10 = load i64, ptr %arrayidx
  %arrayidx16 = getelementptr inbounds ptr, ptr %i8, i64 %indvars.iv
  store i64 %i10, ptr %arrayidx16
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define void @_ZN3ArrIPiE3setEiPS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !31 {
entry:
  ret void
}

define void @_ZN3ArrIPfE3setEiPS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !32 {
entry:
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !33 {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(ptr %this, i32 1)
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i0 = load ptr, ptr %base
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size
  %idxprom = zext i32 %i1 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i0, i64 %idxprom
  store ptr %val, ptr %arrayidx
  %inc = add i32 %i1, 1
  store i32 %inc, ptr %size
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !34 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(ptr %this, i32 1)
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i0 = load ptr, ptr %base
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size
  %idxprom = zext i32 %i1 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i0, i64 %idxprom
  store ptr %val, ptr %arrayidx
  %inc = add i32 %i1, 1
  store i32 %inc, ptr %size
  ret void
}

define void @_ZN3ArrIPiE6resizeEi(ptr "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !43 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i0 = load i32, ptr %size
  %add = add i32 %i0, 1
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  %i1 = load i32, ptr %capacity
  %cmp = icmp ugt i32 %add, %i1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %div = lshr i32 %i1, 1
  %add4 = add i32 %div, %i1
  %cmp5 = icmp ult i32 %add, %add4
  %spec.select = select i1 %cmp5, i32 %add4, i32 %add
  %conv = zext i32 %spec.select to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias ptr @malloc(i64 %mul)
  %cmp1330 = icmp eq i32 %i0, 0
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i3 = load ptr, ptr %base
  br i1 %cmp1330, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %i4 = load ptr, ptr %base
  %wide.trip.count = zext i32 %i0 to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %i5 = phi ptr [ %i3, %for.body.lr.ph ], [ %i4, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %i5, i64 %indvars.iv
  %i7 = load i64, ptr %arrayidx
  %arrayidx15 = getelementptr inbounds ptr, ptr %call, i64 %indvars.iv
  store i64 %i7, ptr %arrayidx15
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %if.end
  %.lcssa = phi ptr [ %i3, %if.end ], [ %i4, %for.body ]
  tail call void @free(ptr %.lcssa)
  store ptr %call, ptr %base
  store i32 %spec.select, ptr %capacity
  br label %cleanup

cleanup:                                          ; preds = %entry, %for.end
  ret void
}

define void @_ZN3ArrIPfE6resizeEi(ptr "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !42 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i0 = load i32, ptr %size
  %add = add i32 %i0, 1
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  %i1 = load i32, ptr %capacity
  %cmp = icmp ugt i32 %add, %i1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %div = lshr i32 %i1, 1
  %add4 = add i32 %div, %i1
  %cmp5 = icmp ult i32 %add, %add4
  %spec.select = select i1 %cmp5, i32 %add4, i32 %add
  %conv = zext i32 %spec.select to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias ptr @malloc(i64 %mul)
  %cmp1330 = icmp eq i32 %i0, 0
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i3 = load ptr, ptr %base
  br i1 %cmp1330, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %i4 = load ptr, ptr %base
  %wide.trip.count = zext i32 %i0 to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %i5 = phi ptr [ %i3, %for.body.lr.ph ], [ %i4, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %i5, i64 %indvars.iv
  %i7 = load i64, ptr %arrayidx
  %arrayidx15 = getelementptr inbounds ptr, ptr %call, i64 %indvars.iv
  store i64 %i7, ptr %arrayidx15
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %if.end
  %.lcssa = phi ptr [ %i3, %if.end ], [ %i4, %for.body ]
  tail call void @free(ptr %.lcssa)
  store ptr %call, ptr %base
  store i32 %spec.select, ptr %capacity
  br label %cleanup

cleanup:                                          ; preds = %entry, %for.end
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

define void @_ZN3ArrIPiED2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !38 {
entry:
  ret void
}

define void @_ZN3ArrIPfED2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !45 {
entry:
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
