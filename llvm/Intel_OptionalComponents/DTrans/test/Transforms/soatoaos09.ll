; UNSUPPORTED: enable-opaque-pointers

; CMPLRLLVM-23920: This test verifies that SOAToAOS is triggered without
; compfail.

; RUN: opt < %s -S -whole-program-assume -dtrans-soatoaos                 \
; RUN:          -enable-dtrans-soatoaos                                   \
; RUN:          -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2  \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-soatoaos          \
; RUN:          -enable-dtrans-soatoaos                                   \
; RUN:          -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2  \
; RUN:  2>&1 | FileCheck %s

; Checks that transformation happens.
; CHECK-DAG: %__SOADT_class.F = type { %struct.Mem*, %__SOADT_AR_struct.Arr*, i64, i64 }
; CHECK-DAG: %__SOADT_AR_struct.Arr = type { i8, i32, i32, %__SOADT_EL_class.F*, %struct.Mem* }
; CHECK-DAG: %__SOADT_EL_class.F = type { i32**, float**, i16** }

; test.cpp: (Source program used to create thus LIT test).
;    // icx test.cpp -O1 -S -fno-inline -emit-llvm
;    // Attributess cleanup.
; #include <stdlib.h>
; #include <string.h>
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
;   unsigned getSize() { return size; }
;   unsigned getCapacity() { return capacity; }
;
;   void resize(int inc) {
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
;   ~Arr() {
;     free(base);
;   }
;
;   void add(S* val) {
;     resize(1);
;     base[size] = val;
;     size++;
;   }
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
;
;   Arr(unsigned c = 2, Mem *mem = 0)
;      : flag(false), capacity(c), size(0), base(0), mem(mem) {
;    base = (S**)malloc(capacity * sizeof(S*));
;    memset(base, 0, capacity * sizeof(S*));
;;  }
;
;   Arr(const Arr &A) :
;     flag(A.flag), capacity(A.capacity), size(A.size), base(0), mem(A.mem) {
;     base = (S**)malloc(capacity * sizeof(S*));
;     memset(base, 0, capacity * sizeof(S*));
;     for (unsigned int index = 0; index < size; index++)
;       base[index] = A.base[index];
;   }
;
; };
;
; class F {
;   Mem *mem;
; public:
;  Arr<int*>* f1;
;  Arr<float*>* f2;
;  Arr<short*>* f3;
;  void F2() {
;    f1->add(nullptr);
;    f2->add(nullptr);
;    f3->add(nullptr);
;    delete f1;
;    delete f2;
;    delete f3;
;  }
;  F() {
;    f1 = new Arr<int *>(10, mem);
;    f2 = new Arr<float *>(10, mem);
;    f3 = new Arr<short *>(10, mem);
;  }
;  F(const F &Src) {
;    unsigned ValS = Src.f3->getSize();
;    f1 = new Arr<int *>(*f1);
;    f2 = new Arr<float *>(*f2);
;    f3 = new Arr<short *>(*f3);
;  }
;
; };
; int main() {
;   F *f = new F();
;  F*ff = new F(*f);
;  f->F2();
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Arr = type { i8, i32, i32, i32***, %struct.Mem* }
%struct.Mem = type { i32 (...)** }
%struct.Arr.0 = type { i8, i32, i32, float***, %struct.Mem* }
%struct.Arr.1 = type { i8, i32, i32, i16***, %struct.Mem* }
%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.Arr.1* }

define linkonce_odr dso_local void @_ZN3ArrIPiE6resizeEi(%struct.Arr* nocapture %this, i32 %inc) {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %i = load i32, i32* %size, align 8
  %add = add i32 %i, 1
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %i1 = load i32, i32* %capacity, align 4
  %cmp = icmp ugt i32 %add, %i1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %i to double
  %mul = fmul fast double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = call noalias i8* @malloc(i64 %mul8)
  %i2 = bitcast i8* %call to i32***
  %cmp1029 = icmp eq i32 %i, 0
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %i3 = load i32***, i32**** %base, align 8
  %wide.trip.count = zext i32 %i to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %i4 = bitcast i32**** %base to i8**
  %i5 = load i8*, i8** %i4, align 8
  call void @free(i8* %i5)
  store i8* %call, i8** %i4, align 8
  store i32 %spec.select, i32* %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32**, i32*** %i3, i64 %indvars.iv
  %i6 = bitcast i32*** %ptridx to i64*
  %i7 = load i64, i64* %i6, align 8
  %ptridx12 = getelementptr inbounds i32**, i32*** %i2, i64 %indvars.iv
  %i8 = bitcast i32*** %ptridx12 to i64*
  store i64 %i7, i64* %i8, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

declare dso_local void @free(i8* nocapture)

define linkonce_odr dso_local void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* nocapture %this, i32 %inc) {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %i = load i32, i32* %size, align 8
  %add = add i32 %i, 1
  %capacity = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  %i1 = load i32, i32* %capacity, align 4
  %cmp = icmp ugt i32 %add, %i1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %i to double
  %mul = fmul fast double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = call noalias i8* @malloc(i64 %mul8)
  %i2 = bitcast i8* %call to float***
  %cmp1029 = icmp eq i32 %i, 0
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %i3 = load float***, float**** %base, align 8
  %wide.trip.count = zext i32 %i to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %i4 = bitcast float**** %base to i8**
  %i5 = load i8*, i8** %i4, align 8
  call void @free(i8* %i5)
  store i8* %call, i8** %i4, align 8
  store i32 %spec.select, i32* %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds float**, float*** %i3, i64 %indvars.iv
  %i6 = bitcast float*** %ptridx to i64*
  %i7 = load i64, i64* %i6, align 8
  %ptridx12 = getelementptr inbounds float**, float*** %i2, i64 %indvars.iv
  %i8 = bitcast float*** %ptridx12 to i64*
  store i64 %i7, i64* %i8, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPsE6resizeEi(%struct.Arr.1* nocapture %this, i32 %inc) {
entry:
  %size = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 2
  %i = load i32, i32* %size, align 8
  %add = add i32 %i, 1
  %capacity = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 1
  %i1 = load i32, i32* %capacity, align 4
  %cmp = icmp ugt i32 %add, %i1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %i to double
  %mul = fmul fast double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = call noalias i8* @malloc(i64 %mul8)
  %i2 = bitcast i8* %call to i16***
  %cmp1029 = icmp eq i32 %i, 0
  %base = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 3
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %i3 = load i16***, i16**** %base, align 8
  %wide.trip.count = zext i32 %i to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %i4 = bitcast i16**** %base to i8**
  %i5 = load i8*, i8** %i4, align 8
  call void @free(i8* %i5)
  store i8* %call, i8** %i4, align 8
  store i32 %spec.select, i32* %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i16**, i16*** %i3, i64 %indvars.iv
  %i6 = bitcast i16*** %ptridx to i64*
  %i7 = load i64, i64* %i6, align 8
  %ptridx12 = getelementptr inbounds i16**, i16*** %i2, i64 %indvars.iv
  %i8 = bitcast i16*** %ptridx12 to i64*
  store i64 %i7, i64* %i8, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* nocapture %this, i32** nocapture %val) {
entry:
  call void @_ZN3ArrIPiE6resizeEi(%struct.Arr* %this, i32 1)
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %i = load i32***, i32**** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %i1 = load i32, i32* %size, align 8
  %idxprom = zext i32 %i1 to i64
  %ptridx = getelementptr inbounds i32**, i32*** %i, i64 %idxprom
  store i32** %val, i32*** %ptridx, align 8
  %inc = add i32 %i1, 1
  store i32 %inc, i32* %size, align 8
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* nocapture %this, float** nocapture %val) {
entry:
  call void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* %this, i32 1)
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %i = load float***, float**** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %i1 = load i32, i32* %size, align 8
  %idxprom = zext i32 %i1 to i64
  %ptridx = getelementptr inbounds float**, float*** %i, i64 %idxprom
  store float** %val, float*** %ptridx, align 8
  %inc = add i32 %i1, 1
  store i32 %inc, i32* %size, align 8
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPsE3addEPS0_(%struct.Arr.1* nocapture %this, i16** nocapture %val) {
entry:
  call void @_ZN3ArrIPsE6resizeEi(%struct.Arr.1* %this, i32 1)
  %base = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 3
  %i = load i16***, i16**** %base, align 8
  %size = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 2
  %i1 = load i32, i32* %size, align 8
  %idxprom = zext i32 %i1 to i64
  %ptridx = getelementptr inbounds i16**, i16*** %i, i64 %idxprom
  store i16** %val, i16*** %ptridx, align 8
  %inc = add i32 %i1, 1
  store i32 %inc, i32* %size, align 8
  ret void
}

define dso_local i32 @main() local_unnamed_addr personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = call noalias nonnull dereferenceable(32) i8* @_Znwm(i64 32)
  %i = bitcast i8* %call to %class.F*
  invoke void @_ZN1FC2Ev(%class.F* nonnull %i)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %call1 = call noalias nonnull dereferenceable(32) i8* @_Znwm(i64 32)
  %i1 = bitcast i8* %call1 to %class.F*
  invoke void @_ZN1FC2ERKS_(%class.F* nonnull %i1, %class.F* nonnull align 8 dereferenceable(32) %i)
          to label %invoke.cont3 unwind label %lpad2

invoke.cont3:                                     ; preds = %invoke.cont
  call void @_ZN1F2F2Ev(%class.F* nonnull %i)
  ret i32 0

lpad:                                             ; preds = %entry
  %i2 = landingpad { i8*, i32 }
          cleanup
  br label %ehcleanup

lpad2:                                            ; preds = %invoke.cont
  %i3 = landingpad { i8*, i32 }
          cleanup
  br label %ehcleanup

ehcleanup:                                        ; preds = %lpad2, %lpad
  %call1.sink = phi i8* [ %call1, %lpad2 ], [ %call, %lpad ]
  %.pn = phi { i8*, i32 } [ %i3, %lpad2 ], [ %i2, %lpad ]
  call void @_ZdlPv(i8* %call1.sink)
  resume { i8*, i32 } %.pn
}

declare dso_local noalias nonnull i8* @_Znwm(i64) local_unnamed_addr

define linkonce_odr dso_local void @_ZN1FC2Ev(%class.F* %this) {
entry:
  %mem = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %t0 = load %struct.Mem*, %struct.Mem** %mem, align 8
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 2
  %f3 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 3
  br label %b.ctor

b.ctor:                                           ; preds = %entry
  %call = call noalias nonnull dereferenceable(32) i8* @_Znwm(i64 32)
  %t1 = bitcast i8* %call to %struct.Arr*
  call void @_ZN3ArrIPiEC2EjP3Mem(%struct.Arr* nonnull %t1, i32 10, %struct.Mem* %t0)
  store %struct.Arr* %t1, %struct.Arr** %f1, align 8
  %call2 = call noalias nonnull dereferenceable(32) i8* @_Znwm(i64 32)
  %t4 = bitcast i8* %call2 to %struct.Arr.0*
  call void @_ZN3ArrIPfEC2EjP3Mem(%struct.Arr.0* nonnull %t4, i32 10, %struct.Mem* %t0)
  store %struct.Arr.0* %t4, %struct.Arr.0** %f2, align 8
  %call6 = call noalias nonnull dereferenceable(32) i8* @_Znwm(i64 32)
  %t7 = bitcast i8* %call6 to %struct.Arr.1*
  call void @_ZN3ArrIPsEC2EjP3Mem(%struct.Arr.1* nonnull %t7, i32 10, %struct.Mem* %t0)
  store %struct.Arr.1* %t7, %struct.Arr.1** %f3, align 8
  ret void
}

define linkonce_odr dso_local void @_ZN1F2F2Ev(%class.F* %this) align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %mem = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %t0 = load %struct.Mem*, %struct.Mem** %mem, align 8
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 2
  %f3 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 3
  br label %b.append

b.append:                                         ; preds = %entry
  %a1 = load %struct.Arr*, %struct.Arr** %f1, align 8
  call void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* %a1, i32** null)
  %a2 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  call void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* %a2, float** null)
  %a3 = load %struct.Arr.1*, %struct.Arr.1** %f3, align 8
  call void @_ZN3ArrIPsE3addEPS0_(%struct.Arr.1* %a3, i16** null)
  %t9 = load %struct.Arr*, %struct.Arr** %f1, align 8
  %isnull = icmp eq %struct.Arr* %t9, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %b.append
  call void @_ZN3ArrIPiED2Ev(%struct.Arr* nonnull %t9)
  %t10 = getelementptr %struct.Arr, %struct.Arr* %t9, i64 0, i32 0
  call void @_ZdlPv(i8* %t10)
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %b.append
  %t11 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  %isnull12 = icmp eq %struct.Arr.0* %t11, null
  br i1 %isnull12, label %delete.end14, label %delete.notnull13

delete.notnull13:                                 ; preds = %delete.end
  call void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nonnull %t11)
  %t12 = getelementptr %struct.Arr.0, %struct.Arr.0* %t11, i64 0, i32 0
  call void @_ZdlPv(i8* %t12)
  br label %delete.end14

delete.end14:                                     ; preds = %delete.notnull13, %delete.end
  %t13 = load %struct.Arr.1*, %struct.Arr.1** %f3, align 8
  %isnull16 = icmp eq %struct.Arr.1* %t13, null
  br i1 %isnull16, label %delete.end18, label %delete.notnull17

delete.notnull17:                                 ; preds = %delete.end14
  call void @_ZN3ArrIPsED2Ev(%struct.Arr.1* nonnull %t13)
  %t14 = getelementptr %struct.Arr.1, %struct.Arr.1* %t13, i64 0, i32 0
  call void @_ZdlPv(i8* %t14)
  br label %delete.end18

delete.end18:                                     ; preds = %delete.notnull17, %delete.end14
  ret void
}

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local void @_ZdlPv(i8*) local_unnamed_addr

define linkonce_odr dso_local void @_ZN1FC2ERKS_(%class.F* %this, %class.F* nonnull align 8 dereferenceable(32) %Src) align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call2 = call noalias nonnull dereferenceable(32) i8* @_Znwm(i64 32)
  %i = bitcast i8* %call2 to %struct.Arr*
  %f1 = getelementptr inbounds %class.F, %class.F* %Src, i64 0, i32 1
  %i1 = load %struct.Arr*, %struct.Arr** %f1, align 8
  call void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* nonnull %i, %struct.Arr* nonnull align 8 dereferenceable(32) %i1)
  %f11 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  store %struct.Arr* %i, %struct.Arr** %f11, align 8
  %call4 = call noalias nonnull dereferenceable(32) i8* @_Znwm(i64 32)
  %i2 = bitcast i8* %call4 to %struct.Arr.0*
  %f2 = getelementptr inbounds %class.F, %class.F* %Src, i64 0, i32 2
  %i3 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  call void @_ZN3ArrIPfEC2ERKS1_(%struct.Arr.0* nonnull %i2, %struct.Arr.0* nonnull align 8 dereferenceable(32) %i3)
  %f12 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 2
  store %struct.Arr.0* %i2, %struct.Arr.0** %f12, align 8
  %call8 = call noalias nonnull dereferenceable(32) i8* @_Znwm(i64 32)
  %i4 = bitcast i8* %call8 to %struct.Arr.1*
  %f39 = getelementptr inbounds %class.F, %class.F* %Src, i64 0, i32 3
  %i5 = load %struct.Arr.1*, %struct.Arr.1** %f39, align 8
  call void @_ZN3ArrIPsEC2ERKS1_(%struct.Arr.1* nonnull %i4, %struct.Arr.1* nonnull align 8 dereferenceable(32) %i5)
  %f13 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 3
  store %struct.Arr.1* %i4, %struct.Arr.1** %f13, align 8
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2EjP3Mem(%struct.Arr* nocapture %this, i32 %c, %struct.Mem* %mem) align 2 {
entry:
  %flag = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  store i8 0, i8* %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  store i32 %c, i32* %capacity, align 4
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  store i32 0, i32* %size, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  store i32*** null, i32**** %base, align 8
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias i8* @malloc(i64 %mul)
  %i = bitcast i8* %call to i32***
  store i32*** %i, i32**** %base, align 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2EjP3Mem(%struct.Arr.0* nocapture %this, i32 %c, %struct.Mem* %mem) align 2 {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  store i8 0, i8* %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  store i32 %c, i32* %capacity, align 4
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  store i32 0, i32* %size, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  store float*** null, float**** %base, align 8
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 4
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias i8* @malloc(i64 %mul)
  %i = bitcast i8* %call to float***
  store float*** %i, float**** %base, align 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPsEC2EjP3Mem(%struct.Arr.1* nocapture %this, i32 %c, %struct.Mem* %mem) align 2 {
entry:
  %flag = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 0
  store i8 0, i8* %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 1
  store i32 %c, i32* %capacity, align 4
  %size = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 2
  store i32 0, i32* %size, align 8
  %base = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 3
  store i16*** null, i16**** %base, align 8
  %mem2 = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 4
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias i8* @malloc(i64 %mul)
  %i = bitcast i8* %call to i16***
  store i16*** %i, i16**** %base, align 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiED2Ev(%struct.Arr* nocapture %this) align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %i = bitcast i32**** %base to i8**
  %i1 = load i8*, i8** %i, align 8
  call void @free(i8* %i1)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nocapture %this) align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %i = bitcast float**** %base to i8**
  %i1 = load i8*, i8** %i, align 8
  call void @free(i8* %i1)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPsED2Ev(%struct.Arr.1* nocapture %this) align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 3
  %i = bitcast i16**** %base to i8**
  %i1 = load i8*, i8** %i, align 8
  call void @free(i8* %i1)
  ret void
}

declare dso_local noalias i8* @malloc(i64) local_unnamed_addr

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

define linkonce_odr dso_local i32 @_ZN3ArrIPsE7getSizeEv(%struct.Arr.1* nocapture %this) align 2 {
entry:
  %size = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 2
  %i = load i32, i32* %size, align 8
  ret i32 %i
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* nocapture %this, %struct.Arr* nocapture nonnull align 8 dereferenceable(32) %A) align 2 {
entry:
  %flag = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 0
  %i = load i8, i8* %flag2, align 8
  store i8 %i, i8* %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 1
  %i1 = load i32, i32* %capacity3, align 4
  store i32 %i1, i32* %capacity, align 4
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %size4 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 2
  %i2 = load i32, i32* %size4, align 8
  store i32 %i2, i32* %size, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  store i32*** null, i32**** %base, align 8
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 4
  %i3 = bitcast %struct.Mem** %mem5 to i64*
  %i4 = load i64, i64* %i3, align 8
  %i5 = bitcast %struct.Mem** %mem to i64*
  store i64 %i4, i64* %i5, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias i8* @malloc(i64 %mul)
  %i6 = bitcast i32**** %base to i8**
  store i8* %call, i8** %i6, align 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 3
  %i7 = load i32***, i32**** %base13, align 8
  %i8 = load i32***, i32**** %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32**, i32*** %i7, i64 %indvars.iv
  %i9 = bitcast i32*** %ptridx to i64*
  %i10 = load i64, i64* %i9, align 8
  %ptridx16 = getelementptr inbounds i32**, i32*** %i8, i64 %indvars.iv
  %i11 = bitcast i32*** %ptridx16 to i64*
  store i64 %i10, i64* %i11, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2ERKS1_(%struct.Arr.0* nocapture %this, %struct.Arr.0* nocapture nonnull align 8 dereferenceable(32) %A) align 2 {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 0
  %i = load i8, i8* %flag2, align 8
  store i8 %i, i8* %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 1
  %i1 = load i32, i32* %capacity3, align 4
  store i32 %i1, i32* %capacity, align 4
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %size4 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 2
  %i2 = load i32, i32* %size4, align 8
  store i32 %i2, i32* %size, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  store float*** null, float**** %base, align 8
  %mem = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 4
  %i3 = bitcast %struct.Mem** %mem5 to i64*
  %i4 = load i64, i64* %i3, align 8
  %i5 = bitcast %struct.Mem** %mem to i64*
  store i64 %i4, i64* %i5, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias i8* @malloc(i64 %mul)
  %i6 = bitcast float**** %base to i8**
  store i8* %call, i8** %i6, align 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 3
  %i7 = load float***, float**** %base13, align 8
  %i8 = load float***, float**** %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds float**, float*** %i7, i64 %indvars.iv
  %i9 = bitcast float*** %ptridx to i64*
  %i10 = load i64, i64* %i9, align 8
  %ptridx16 = getelementptr inbounds float**, float*** %i8, i64 %indvars.iv
  %i11 = bitcast float*** %ptridx16 to i64*
  store i64 %i10, i64* %i11, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define linkonce_odr dso_local void @_ZN3ArrIPsEC2ERKS1_(%struct.Arr.1* nocapture %this, %struct.Arr.1* nocapture nonnull align 8 dereferenceable(32) %A) align 2 {
entry:
  %flag = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %A, i64 0, i32 0
  %i = load i8, i8* %flag2, align 8
  store i8 %i, i8* %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %A, i64 0, i32 1
  %i1 = load i32, i32* %capacity3, align 4
  store i32 %i1, i32* %capacity, align 4
  %size = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 2
  %size4 = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %A, i64 0, i32 2
  %i2 = load i32, i32* %size4, align 8
  store i32 %i2, i32* %size, align 8
  %base = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 3
  store i16*** null, i16**** %base, align 8
  %mem = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %A, i64 0, i32 4
  %i3 = bitcast %struct.Mem** %mem5 to i64*
  %i4 = load i64, i64* %i3, align 8
  %i5 = bitcast %struct.Mem** %mem to i64*
  store i64 %i4, i64* %i5, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias i8* @malloc(i64 %mul)
  %i6 = bitcast i16**** %base to i8**
  store i8* %call, i8** %i6, align 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr.1, %struct.Arr.1* %A, i64 0, i32 3
  %i7 = load i16***, i16**** %base13, align 8
  %i8 = load i16***, i16**** %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i16**, i16*** %i7, i64 %indvars.iv
  %i9 = bitcast i16*** %ptridx to i64*
  %i10 = load i64, i64* %i9, align 8
  %ptridx16 = getelementptr inbounds i16**, i16*** %i8, i64 %indvars.iv
  %i11 = bitcast i16*** %ptridx16 to i64*
  store i64 %i10, i64* %i11, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { argmemonly nounwind willreturn writeonly }
