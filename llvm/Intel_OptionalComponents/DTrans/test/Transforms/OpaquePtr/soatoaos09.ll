; CMPLRLLVM-23920: This test verifies that SOAToAOS is triggered without
; compfail.

; RUN: opt < %s -S -whole-program-assume -intel-libirc-allowed \
; RUN:          -passes=dtrans-soatoaosop -enable-intel-advanced-opts     \
; RUN:          -mtriple=i686-- -mattr=+avx2                              \
; RUN:          2>&1 | FileCheck %s

; Checks that transformation happens.
; CHECK-DAG: %__SOADT_class.F = type { ptr, ptr, i64, i64 }
; CHECK-DAG: %__SOADT_AR_struct.Arr = type { i8, i32, i32, ptr, ptr }
; CHECK-DAG: %__SOADT_EL_class.F = type { ptr, ptr, ptr }
;
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

%struct.Arr = type { i8, i32, i32, ptr, ptr }
%struct.Arr.0 = type { i8, i32, i32, ptr, ptr }
%struct.Arr.1 = type { i8, i32, i32, ptr, ptr }
%class.F = type { ptr, ptr, ptr, ptr }
%struct.Mem = type { ptr }

define linkonce_odr dso_local void @_ZN3ArrIPiE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !16 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i = load i32, ptr %size, align 8
  %add = add i32 %i, 1
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  %i1 = load i32, ptr %capacity, align 4
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
  %call = call noalias ptr @malloc(i64 %mul8)
  %i2 = bitcast ptr %call to ptr
  %cmp1029 = icmp eq i32 %i, 0
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %i3 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %i4 = bitcast ptr %base to ptr
  %i5 = load ptr, ptr %i4, align 8
  call void @free(ptr %i5)
  store ptr %call, ptr %i4, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i3, i64 %indvars.iv
  %i6 = bitcast ptr %ptridx to ptr
  %i7 = load i64, ptr %i6, align 8
  %ptridx12 = getelementptr inbounds ptr, ptr %i2, i64 %indvars.iv
  %i8 = bitcast ptr %ptridx12 to ptr
  store i64 %i7, ptr %i8, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

; Function Attrs: allockind("free")
declare !intel.dtrans.func.type !17 dso_local void @free(ptr nocapture noundef "intel_dtrans_func_index"="1") #0

define linkonce_odr dso_local void @_ZN3ArrIPfE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !19 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i = load i32, ptr %size, align 8
  %add = add i32 %i, 1
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  %i1 = load i32, ptr %capacity, align 4
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
  %call = call noalias ptr @malloc(i64 %mul8)
  %i2 = bitcast ptr %call to ptr
  %cmp1029 = icmp eq i32 %i, 0
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %i3 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %i4 = bitcast ptr %base to ptr
  %i5 = load ptr, ptr %i4, align 8
  call void @free(ptr %i5)
  store ptr %call, ptr %i4, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i3, i64 %indvars.iv
  %i6 = bitcast ptr %ptridx to ptr
  %i7 = load i64, ptr %i6, align 8
  %ptridx12 = getelementptr inbounds ptr, ptr %i2, i64 %indvars.iv
  %i8 = bitcast ptr %ptridx12 to ptr
  store i64 %i7, ptr %i8, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPsE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !20 {
entry:
  %size = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 2
  %i = load i32, ptr %size, align 8
  %add = add i32 %i, 1
  %capacity = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 1
  %i1 = load i32, ptr %capacity, align 4
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
  %call = call noalias ptr @malloc(i64 %mul8)
  %i2 = bitcast ptr %call to ptr
  %cmp1029 = icmp eq i32 %i, 0
  %base = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 3
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %i3 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %i4 = bitcast ptr %base to ptr
  %i5 = load ptr, ptr %i4, align 8
  call void @free(ptr %i5)
  store ptr %call, ptr %i4, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i3, i64 %indvars.iv
  %i6 = bitcast ptr %ptridx to ptr
  %i7 = load i64, ptr %i6, align 8
  %ptridx12 = getelementptr inbounds ptr, ptr %i2, i64 %indvars.iv
  %i8 = bitcast ptr %ptridx12 to ptr
  store i64 %i7, ptr %i8, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !21 {
entry:
  call void @_ZN3ArrIPiE6resizeEi(ptr %this, i32 1)
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i = load ptr, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size, align 8
  %idxprom = zext i32 %i1 to i64
  %ptridx = getelementptr inbounds ptr, ptr %i, i64 %idxprom
  store ptr %val, ptr %ptridx, align 8
  %inc = add i32 %i1, 1
  store i32 %inc, ptr %size, align 8
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !23 {
entry:
  call void @_ZN3ArrIPfE6resizeEi(ptr %this, i32 1)
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i = load ptr, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size, align 8
  %idxprom = zext i32 %i1 to i64
  %ptridx = getelementptr inbounds ptr, ptr %i, i64 %idxprom
  store ptr %val, ptr %ptridx, align 8
  %inc = add i32 %i1, 1
  store i32 %inc, ptr %size, align 8
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPsE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !25 {
entry:
  call void @_ZN3ArrIPsE6resizeEi(ptr %this, i32 1)
  %base = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 3
  %i = load ptr, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %size, align 8
  %idxprom = zext i32 %i1 to i64
  %ptridx = getelementptr inbounds ptr, ptr %i, i64 %idxprom
  store ptr %val, ptr %ptridx, align 8
  %inc = add i32 %i1, 1
  store i32 %inc, ptr %size, align 8
  ret void
}

define dso_local i32 @main() local_unnamed_addr personality ptr @__gxx_personality_v0 {
entry:
  %call = call noalias nonnull dereferenceable(32) ptr @_Znwm(i64 32)
  %i = bitcast ptr %call to ptr
  invoke void @_ZN1FC2Ev(ptr nonnull %i)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %call1 = call noalias nonnull dereferenceable(32) ptr @_Znwm(i64 32)
  %i1 = bitcast ptr %call1 to ptr
  invoke void @_ZN1FC2ERKS_(ptr nonnull %i1, ptr nonnull align 8 dereferenceable(32) %i)
          to label %invoke.cont3 unwind label %lpad2

invoke.cont3:                                     ; preds = %invoke.cont
  call void @_ZN1F2F2Ev(ptr nonnull %i)
  ret i32 0

lpad:                                             ; preds = %entry
  %i2 = landingpad { ptr, i32 }
          cleanup
  br label %ehcleanup

lpad2:                                            ; preds = %invoke.cont
  %i3 = landingpad { ptr, i32 }
          cleanup
  br label %ehcleanup

ehcleanup:                                        ; preds = %lpad2, %lpad
  %call1.sink = phi ptr [ %call1, %lpad2 ], [ %call, %lpad ]
  %.pn = phi { ptr, i32 } [ %i3, %lpad2 ], [ %i2, %lpad ]
  call void @_ZdlPv(ptr %call1.sink)
  resume { ptr, i32 } %.pn
}

declare !intel.dtrans.func.type !27 dso_local nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

define linkonce_odr dso_local void @_ZN1FC2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !28 {
entry:
  %mem = getelementptr inbounds %class.F, ptr %this, i64 0, i32 0
  %t0 = load ptr, ptr %mem, align 8
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 2
  %f3 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 3
  br label %b.ctor

b.ctor:                                           ; preds = %entry
  %call = call noalias nonnull dereferenceable(32) ptr @_Znwm(i64 32)
  %t1 = bitcast ptr %call to ptr
  call void @_ZN3ArrIPiEC2EjP3Mem(ptr nonnull %t1, i32 10, ptr %t0)
  store ptr %t1, ptr %f1, align 8
  %call2 = call noalias nonnull dereferenceable(32) ptr @_Znwm(i64 32)
  %t4 = bitcast ptr %call2 to ptr
  call void @_ZN3ArrIPfEC2EjP3Mem(ptr nonnull %t4, i32 10, ptr %t0)
  store ptr %t4, ptr %f2, align 8
  %call6 = call noalias nonnull dereferenceable(32) ptr @_Znwm(i64 32)
  %t7 = bitcast ptr %call6 to ptr
  call void @_ZN3ArrIPsEC2EjP3Mem(ptr nonnull %t7, i32 10, ptr %t0)
  store ptr %t7, ptr %f3, align 8
  ret void
}

define linkonce_odr dso_local void @_ZN1F2F2Ev(ptr "intel_dtrans_func_index"="1" %this) align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !30 {
entry:
  %mem = getelementptr inbounds %class.F, ptr %this, i64 0, i32 0
  %t0 = load ptr, ptr %mem, align 8
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 2
  %f3 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 3
  br label %b.append

b.append:                                         ; preds = %entry
  %a1 = load ptr, ptr %f1, align 8
  call void @_ZN3ArrIPiE3addEPS0_(ptr %a1, ptr null)
  %a2 = load ptr, ptr %f2, align 8
  call void @_ZN3ArrIPfE3addEPS0_(ptr %a2, ptr null)
  %a3 = load ptr, ptr %f3, align 8
  call void @_ZN3ArrIPsE3addEPS0_(ptr %a3, ptr null)
  %t9 = load ptr, ptr %f1, align 8
  %isnull = icmp eq ptr %t9, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %b.append
  call void @_ZN3ArrIPiED2Ev(ptr nonnull %t9)
  %t10 = getelementptr %struct.Arr, ptr %t9, i64 0, i32 0
  call void @_ZdlPv(ptr %t10)
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %b.append
  %t11 = load ptr, ptr %f2, align 8
  %isnull12 = icmp eq ptr %t11, null
  br i1 %isnull12, label %delete.end14, label %delete.notnull13

delete.notnull13:                                 ; preds = %delete.end
  call void @_ZN3ArrIPfED2Ev(ptr nonnull %t11)
  %t12 = getelementptr %struct.Arr.0, ptr %t11, i64 0, i32 0
  call void @_ZdlPv(ptr %t12)
  br label %delete.end14

delete.end14:                                     ; preds = %delete.notnull13, %delete.end
  %t13 = load ptr, ptr %f3, align 8
  %isnull16 = icmp eq ptr %t13, null
  br i1 %isnull16, label %delete.end18, label %delete.notnull17

delete.notnull17:                                 ; preds = %delete.end14
  call void @_ZN3ArrIPsED2Ev(ptr nonnull %t13)
  %t14 = getelementptr %struct.Arr.1, ptr %t13, i64 0, i32 0
  call void @_ZdlPv(ptr %t14)
  br label %delete.end18

delete.end18:                                     ; preds = %delete.notnull17, %delete.end14
  ret void
}

declare dso_local i32 @__gxx_personality_v0(...)

declare !intel.dtrans.func.type !31 dso_local void @_ZdlPv(ptr "intel_dtrans_func_index"="1")

define linkonce_odr dso_local void @_ZN1FC2ERKS_(ptr "intel_dtrans_func_index"="1" %this, ptr nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %Src) align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !32 {
entry:
  %call2 = call noalias nonnull dereferenceable(32) ptr @_Znwm(i64 32)
  %i = bitcast ptr %call2 to ptr
  %f1 = getelementptr inbounds %class.F, ptr %Src, i64 0, i32 1
  %i1 = load ptr, ptr %f1, align 8
  call void @_ZN3ArrIPiEC2ERKS1_(ptr nonnull %i, ptr nonnull align 8 dereferenceable(32) %i1)
  %f11 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  store ptr %i, ptr %f11, align 8
  %call4 = call noalias nonnull dereferenceable(32) ptr @_Znwm(i64 32)
  %i2 = bitcast ptr %call4 to ptr
  %f2 = getelementptr inbounds %class.F, ptr %Src, i64 0, i32 2
  %i3 = load ptr, ptr %f2, align 8
  call void @_ZN3ArrIPfEC2ERKS1_(ptr nonnull %i2, ptr nonnull align 8 dereferenceable(32) %i3)
  %f12 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 2
  store ptr %i2, ptr %f12, align 8
  %call8 = call noalias nonnull dereferenceable(32) ptr @_Znwm(i64 32)
  %i4 = bitcast ptr %call8 to ptr
  %f39 = getelementptr inbounds %class.F, ptr %Src, i64 0, i32 3
  %i5 = load ptr, ptr %f39, align 8
  call void @_ZN3ArrIPsEC2ERKS1_(ptr nonnull %i4, ptr nonnull align 8 dereferenceable(32) %i5)
  %f13 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 3
  store ptr %i4, ptr %f13, align 8
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2EjP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !33 {
entry:
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0
  store i8 0, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  store i32 0, ptr %size, align 8
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  store ptr null, ptr %base, align 8
  %mem2 = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  %i = bitcast ptr %call to ptr
  store ptr %i, ptr %base, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2EjP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !34 {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 0
  store i8 0, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  store i32 0, ptr %size, align 8
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  store ptr null, ptr %base, align 8
  %mem2 = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  %i = bitcast ptr %call to ptr
  store ptr %i, ptr %base, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPsEC2EjP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !35 {
entry:
  %flag = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 0
  store i8 0, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 2
  store i32 0, ptr %size, align 8
  %base = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 3
  store ptr null, ptr %base, align 8
  %mem2 = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  %i = bitcast ptr %call to ptr
  store ptr %i, ptr %base, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) align 2 !intel.dtrans.func.type !36 {
entry:
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %i = bitcast ptr %base to ptr
  %i1 = load ptr, ptr %i, align 8
  call void @free(ptr %i1)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) align 2 !intel.dtrans.func.type !37 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %i = bitcast ptr %base to ptr
  %i1 = load ptr, ptr %i, align 8
  call void @free(ptr %i1)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPsED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) align 2 !intel.dtrans.func.type !38 {
entry:
  %base = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 3
  %i = bitcast ptr %base to ptr
  %i1 = load ptr, ptr %i, align 8
  call void @free(ptr %i1)
  ret void
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !39 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) #1

define linkonce_odr dso_local i32 @_ZN3ArrIPsE7getSizeEv(ptr nocapture "intel_dtrans_func_index"="1" %this) align 2 !intel.dtrans.func.type !40 {
entry:
  %size = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 2
  %i = load i32, ptr %size, align 8
  ret i32 %i
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2ERKS1_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %A) align 2 !intel.dtrans.func.type !41 {
entry:
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 0
  %i = load i8, ptr %flag2, align 8
  store i8 %i, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 1
  %i1 = load i32, ptr %capacity3, align 4
  store i32 %i1, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %size4 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 2
  %i2 = load i32, ptr %size4, align 8
  store i32 %i2, ptr %size, align 8
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  store ptr null, ptr %base, align 8
  %mem = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 4
  %i3 = bitcast ptr %mem5 to ptr
  %i4 = load i64, ptr %i3, align 8
  %i5 = bitcast ptr %mem to ptr
  store i64 %i4, ptr %i5, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  %i6 = bitcast ptr %base to ptr
  store ptr %call, ptr %i6, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
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
  %ptridx = getelementptr inbounds ptr, ptr %i7, i64 %indvars.iv
  %i9 = bitcast ptr %ptridx to ptr
  %i10 = load i64, ptr %i9, align 8
  %ptridx16 = getelementptr inbounds ptr, ptr %i8, i64 %indvars.iv
  %i11 = bitcast ptr %ptridx16 to ptr
  store i64 %i10, ptr %i11, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2ERKS1_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %A) align 2 !intel.dtrans.func.type !42 {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 0
  %i = load i8, ptr %flag2, align 8
  store i8 %i, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 1
  %i1 = load i32, ptr %capacity3, align 4
  store i32 %i1, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %size4 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 2
  %i2 = load i32, ptr %size4, align 8
  store i32 %i2, ptr %size, align 8
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  store ptr null, ptr %base, align 8
  %mem = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 4
  %i3 = bitcast ptr %mem5 to ptr
  %i4 = load i64, ptr %i3, align 8
  %i5 = bitcast ptr %mem to ptr
  store i64 %i4, ptr %i5, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  %i6 = bitcast ptr %base to ptr
  store ptr %call, ptr %i6, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 3
  %i7 = load ptr, ptr %base13, align 8
  %i8 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i7, i64 %indvars.iv
  %i9 = bitcast ptr %ptridx to ptr
  %i10 = load i64, ptr %i9, align 8
  %ptridx16 = getelementptr inbounds ptr, ptr %i8, i64 %indvars.iv
  %i11 = bitcast ptr %ptridx16 to ptr
  store i64 %i10, ptr %i11, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define linkonce_odr dso_local void @_ZN3ArrIPsEC2ERKS1_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %A) align 2 !intel.dtrans.func.type !43 {
entry:
  %flag = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr.1, ptr %A, i64 0, i32 0
  %i = load i8, ptr %flag2, align 8
  store i8 %i, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr.1, ptr %A, i64 0, i32 1
  %i1 = load i32, ptr %capacity3, align 4
  store i32 %i1, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 2
  %size4 = getelementptr inbounds %struct.Arr.1, ptr %A, i64 0, i32 2
  %i2 = load i32, ptr %size4, align 8
  store i32 %i2, ptr %size, align 8
  %base = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 3
  store ptr null, ptr %base, align 8
  %mem = getelementptr inbounds %struct.Arr.1, ptr %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr.1, ptr %A, i64 0, i32 4
  %i3 = bitcast ptr %mem5 to ptr
  %i4 = load i64, ptr %i3, align 8
  %i5 = bitcast ptr %mem to ptr
  store i64 %i4, ptr %i5, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  %i6 = bitcast ptr %base to ptr
  store ptr %call, ptr %i6, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr.1, ptr %A, i64 0, i32 3
  %i7 = load ptr, ptr %base13, align 8
  %i8 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i7, i64 %indvars.iv
  %i9 = bitcast ptr %ptridx to ptr
  %i10 = load i64, ptr %i9, align 8
  %ptridx16 = getelementptr inbounds ptr, ptr %i8, i64 %indvars.iv
  %i11 = bitcast ptr %ptridx16 to ptr
  store i64 %i10, ptr %i11, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

attributes #0 = { allockind("free") "alloc-family"="malloc" }
attributes #1 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: write) }

!intel.dtrans.types = !{!0, !5, !9, !12, !14}

!0 = !{!"S", %class.F zeroinitializer, i32 4, !1, !2, !3, !4}
!1 = !{%struct.Mem zeroinitializer, i32 1}
!2 = !{%struct.Arr zeroinitializer, i32 1}
!3 = !{%struct.Arr.0 zeroinitializer, i32 1}
!4 = !{%struct.Arr.1 zeroinitializer, i32 1}
!5 = !{!"S", %struct.Mem zeroinitializer, i32 1, !6}
!6 = !{!7, i32 2}
!7 = !{!"F", i1 true, i32 0, !8}
!8 = !{i32 0, i32 0}
!9 = !{!"S", %struct.Arr zeroinitializer, i32 5, !10, !8, !8, !11, !1}
!10 = !{i8 0, i32 0}
!11 = !{i32 0, i32 3}
!12 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !10, !8, !8, !13, !1}
!13 = !{float 0.000000e+00, i32 3}
!14 = !{!"S", %struct.Arr.1 zeroinitializer, i32 5, !10, !8, !8, !15, !1}
!15 = !{i16 0, i32 3}
!16 = distinct !{!2}
!17 = distinct !{!18}
!18 = !{i8 0, i32 1}
!19 = distinct !{!3}
!20 = distinct !{!4}
!21 = distinct !{!2, !22}
!22 = !{i32 0, i32 2}
!23 = distinct !{!3, !24}
!24 = !{float 0.000000e+00, i32 2}
!25 = distinct !{!4, !26}
!26 = !{i16 0, i32 2}
!27 = distinct !{!18}
!28 = distinct !{!29}
!29 = !{%class.F zeroinitializer, i32 1}
!30 = distinct !{!29}
!31 = distinct !{!18}
!32 = distinct !{!29, !29}
!33 = distinct !{!2, !1}
!34 = distinct !{!3, !1}
!35 = distinct !{!4, !1}
!36 = distinct !{!2}
!37 = distinct !{!3}
!38 = distinct !{!4}
!39 = distinct !{!18}
!40 = distinct !{!4}
!41 = distinct !{!2, !2}
!42 = distinct !{!3, !3}
!43 = distinct !{!4, !4}
