; This test verifies that SOAToAOS is triggered when IR has specific things
; like EH etc.

; RUN: opt < %s -S -whole-program-assume -intel-libirc-allowed                    \
; RUN:          -passes=dtrans-soatoaosop                                    \
; RUN:          -enable-intel-advanced-opts -mattr=+avx2                     \
; RUN:          -dtrans-soatoaosop-size-heuristic=false                      \
; RUN:          2>&1 | FileCheck %s
;  // icx /GX m7.cpp -O1 -Ob0 -S -emit-llvm
;  // Attributess cleanup.
;
; m7.cpp:
;
; extern "C" {
;  extern void *malloc(int) noexcept;
;  extern void free(void *) noexcept;
;  }
;
;  template <typename S> struct Arr {
;    bool flag;
;    unsigned capacity;
;    S *base;
;    unsigned size;
;    struct Mem* mem;
;    S &get(int i) {
;     if (i >= size)
;       throw;
;     return base[i];
;    }
;    void set(int i, S val) {
;     if (i >= size)
;       throw;
;     base[i] = val;
;    }
;   Arr(unsigned c = 2, struct Mem *mem = 0)
;     : flag(false), capacity(c), size(0), base(0), mem(mem) {
;     base = (S*)malloc(capacity * sizeof(S));
;     memset(base, 0, capacity * sizeof(S));
;   }
;   void realloc(int inc) {
;     unsigned int newMax = size + inc;
;     if (newMax <= capacity)
;        return;
;     unsigned int minNewMax = (unsigned int)((double)size * 1.25);
;     if (newMax < minNewMax)
;         newMax = minNewMax;
;     S *newList = (S *) malloc (newMax * sizeof(S));
;     for (unsigned int index = 0; index < size; index++)
;        newList[index] = base[index];
;
;     free(base); //delete [] fElemList;
;     base = newList;
;     capacity = newMax;
;   }
;   void add(const S &e) {
;     realloc(1);
;     base[size] = e;
;     ++size;
;   }
;   Arr(const Arr &A) :
;     flag(A.flag), capacity(A.capacity), size(A.size), base(0), mem(A.mem) {
;     base = (S*)malloc(capacity * sizeof(S));
;     memset(base, 0, capacity * sizeof(S));
;     for (unsigned int index = 0; index < size; index++)
;       base[index] = A.base[index];
;   }
;    ~Arr() { free(base); }
;  };
;
;  class F {
;  public:
;    Arr<int *> *f1;
;    Arr<float *> *f2;
;    F(const F &f) {
;      try {
;      f1 = new Arr<int *>(*f.f1);
;      f2 = new Arr<float *>(*f.f2);
;      }
;      catch (int e) {
;        cleanup();
;        throw;
;      }
;    }
;    void put(int *a, float *b) {
;      f1->add(a);
;      f2->add(b);
;    }
;    void set1(int i, int *a) {
;      f1->set(i, a);
;    }
;    void set2(int i, float *b) {
;      f2->set(i, b);
;    }
;    int get1(int i) {
;      return *(f1->get(i));
;    }
;    float get2(int i) {
;      return *(f2->get(i));
;    }
;    F() {
;      f1 = new Arr<int *>();
;      f2 = new Arr<float *>();
;    }
;    void cleanup() {
;      delete f1;
;      delete f2;
;    }
;  };
;
;  int v1 = 20;
;  int v2 = 30;
;  float v3 = 3.5;
;  float v4 = 7.5;
;
;
;  bool check1(F *f) {
;    if (f->get1(0) != v2)
;      return false;
;    if (f->get2(0) != v4)
;      return false;
;    if (f->get1(1) != v2)
;      return false;
;    if (f->get2(1) != v4)
;      return false;
;    return true;
;  }
;
;  int main() {
;    F *f = new F();
;    f->put(&v1, &v3);
;    if (f->get1(0) != v1)
;      return -1;
;    if (f->get2(0) != v3)
;      return -1;
;    // force realloc
;    f->put(&v2, &v4);
;    if (f->get1(0) != v1)
;      return -1;
;    if (f->get2(0) != v3)
;      return -1;
;    if (f->get1(1) != v2)
;      return -1;
;    if (f->get2(1) != v4)
;      return -1;
;
;    f->set1(0, &v2);
;    f->set2(0, &v4);
;    if (!check1(f))
;      return -1;
;
;    F *f1 = new F(*f);
;    if (!check1(f1))
;      return -1;
;
;    delete f;
;    delete f1;
;    return 0;
; }

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

; CHECK-DAG: %__SOADT_class.F = type { ptr, i64, ptr  }
; CHECK-DAG: %__SOADT_AR_struct.Arr = type { i8, i32, ptr, i32, ptr }
; CHECK-DAG: %__SOADT_EL_class.F = type { ptr, ptr }

%class.F = type { ptr, ptr, ptr }
%struct.Arr = type { i8, i32, ptr, i32, ptr }
%struct.Arr.0 = type { i8, i32, ptr, i32, ptr }
%struct.Mem = type { ptr }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

@"?v1@@3HA" = dso_local global i32 20, align 4
@"?v2@@3HA" = dso_local global i32 30, align 4
@"?v3@@3MA" = dso_local global float 3.500000e+00, align 4
@"?v4@@3MA" = dso_local global float 7.500000e+00, align 4

define dso_local zeroext i1 @"?check1@@YA_NPEAVF@@@Z"(ptr nocapture "intel_dtrans_func_index"="1" %f) !intel.dtrans.func.type !15 {
entry:
  %call = call i32 @"?get1@F@@QEAAHH@Z"(ptr %f, i32 0)
  %i = load i32, ptr @"?v2@@3HA", align 4
  %cmp = icmp eq i32 %call, %i
  br i1 %cmp, label %if.end, label %return

if.end:                                           ; preds = %entry
  %call1 = call float @"?get2@F@@QEAAMH@Z"(ptr %f, i32 0)
  %i1 = load float, ptr @"?v4@@3MA", align 4
  %cmp2 = fcmp une float %call1, %i1
  br i1 %cmp2, label %return, label %if.end4

if.end4:                                          ; preds = %if.end
  %call5 = call i32 @"?get1@F@@QEAAHH@Z"(ptr %f, i32 1)
  %i2 = load i32, ptr @"?v2@@3HA", align 4
  %cmp6 = icmp eq i32 %call5, %i2
  br i1 %cmp6, label %if.end8, label %return

if.end8:                                          ; preds = %if.end4
  %call9 = call float @"?get2@F@@QEAAMH@Z"(ptr %f, i32 1)
  %i3 = load float, ptr @"?v4@@3MA", align 4
  %cmp10 = fcmp oeq float %call9, %i3
  br label %return

return:                                           ; preds = %if.end8, %if.end4, %if.end, %entry
  %retval.0 = phi i1 [ false, %entry ], [ false, %if.end ], [ false, %if.end4 ], [ %cmp10, %if.end8 ]
  ret i1 %retval.0
}

define linkonce_odr dso_local i32 @"?get1@F@@QEAAHH@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i) !intel.dtrans.func.type !17 {
entry:
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 0
  %i1 = load ptr, ptr %f1, align 8
  %call = call nonnull align 8 dereferenceable(8) ptr @"?get@?$Arr@PEAH@@QEAAAEAPEAHH@Z"(ptr %i1, i32 %i)
  %i2 = load ptr, ptr %call, align 8
  %i3 = load i32, ptr %i2, align 4
  ret i32 %i3
}

define linkonce_odr dso_local float @"?get2@F@@QEAAMH@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i) !intel.dtrans.func.type !17 {
entry:
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  %i1 = load ptr, ptr %f2, align 8
  %call = call nonnull align 8 dereferenceable(8) ptr @"?get@?$Arr@PEAM@@QEAAAEAPEAMH@Z"(ptr %i1, i32 %i)
  %i2 = load ptr, ptr %call, align 8
  %i3 = load float, ptr %i2, align 4
  ret float %i3
}

define dso_local i32 @main() local_unnamed_addr personality ptr @__CxxFrameHandler3 {
entry:
  %call = call noalias nonnull dereferenceable(24) ptr @"??2@YAPEAX_K@Z"(i64 24)
  %call1 = invoke ptr @"??0F@@QEAA@XZ"(ptr nonnull %call)
          to label %invoke.cont unwind label %ehcleanup

invoke.cont:                                      ; preds = %entry
  call void @"?put@F@@QEAAXPEAHPEAM@Z"(ptr nonnull %call, ptr nonnull @"?v1@@3HA", ptr nonnull @"?v3@@3MA")
  %call2 = call i32 @"?get1@F@@QEAAHH@Z"(ptr nonnull %call, i32 0)
  %i1 = load i32, ptr @"?v1@@3HA", align 4
  %cmp = icmp eq i32 %call2, %i1
  br i1 %cmp, label %if.end, label %cleanup37

ehcleanup:                                        ; preds = %entry
  %i2 = cleanuppad within none []
  call void @"??3@YAXPEAX@Z"(ptr %call) [ "funclet"(token %i2) ]
  cleanupret from %i2 unwind label %ehcleanup38

if.end:                                           ; preds = %invoke.cont
  %call3 = call float @"?get2@F@@QEAAMH@Z"(ptr nonnull %call, i32 0)
  %i3 = load float, ptr @"?v3@@3MA", align 4
  %cmp4 = fcmp une float %call3, %i3
  br i1 %cmp4, label %cleanup37, label %if.end6

if.end6:                                          ; preds = %if.end
  call void @"?put@F@@QEAAXPEAHPEAM@Z"(ptr nonnull %call, ptr nonnull @"?v2@@3HA", ptr nonnull @"?v4@@3MA")
  %call7 = call i32 @"?get1@F@@QEAAHH@Z"(ptr nonnull %call, i32 0)
  %i4 = load i32, ptr @"?v1@@3HA", align 4
  %cmp8 = icmp eq i32 %call7, %i4
  br i1 %cmp8, label %if.end10, label %cleanup37

if.end10:                                         ; preds = %if.end6
  %call11 = call float @"?get2@F@@QEAAMH@Z"(ptr nonnull %call, i32 0)
  %i5 = load float, ptr @"?v3@@3MA", align 4
  %cmp12 = fcmp une float %call11, %i5
  br i1 %cmp12, label %cleanup37, label %if.end14

if.end14:                                         ; preds = %if.end10
  %call15 = call i32 @"?get1@F@@QEAAHH@Z"(ptr nonnull %call, i32 1)
  %i6 = load i32, ptr @"?v2@@3HA", align 4
  %cmp16 = icmp eq i32 %call15, %i6
  br i1 %cmp16, label %if.end18, label %cleanup37

if.end18:                                         ; preds = %if.end14
  %call19 = call float @"?get2@F@@QEAAMH@Z"(ptr nonnull %call, i32 1)
  %i7 = load float, ptr @"?v4@@3MA", align 4
  %cmp20 = fcmp une float %call19, %i7
  br i1 %cmp20, label %cleanup37, label %if.end22

if.end22:                                         ; preds = %if.end18
  call void @"?set1@F@@QEAAXHPEAH@Z"(ptr nonnull %call, i32 0, ptr nonnull @"?v2@@3HA")
  call void @"?set2@F@@QEAAXHPEAM@Z"(ptr nonnull %call, i32 0, ptr nonnull @"?v4@@3MA")
  %call23 = call zeroext i1 @"?check1@@YA_NPEAVF@@@Z"(ptr nonnull %call)
  br i1 %call23, label %if.end25, label %cleanup37

if.end25:                                         ; preds = %if.end22
  %call26 = call noalias nonnull dereferenceable(24) ptr @"??2@YAPEAX_K@Z"(i64 24)
  %call28 = invoke ptr @"??0F@@QEAA@AEBV0@@Z"(ptr nonnull %call26, ptr nonnull align 8 dereferenceable(24) %call)
          to label %invoke.cont27 unwind label %ehcleanup29

invoke.cont27:                                    ; preds = %if.end25
  %call30 = call zeroext i1 @"?check1@@YA_NPEAVF@@@Z"(ptr nonnull %call26)
  br i1 %call30, label %delete.notnull, label %cleanup37

ehcleanup29:                                      ; preds = %if.end25
  %i9 = cleanuppad within none []
  call void @"??3@YAXPEAX@Z"(ptr %call26) [ "funclet"(token %i9) ]
  cleanupret from %i9 unwind label %ehcleanup38

delete.notnull:                                   ; preds = %invoke.cont27
  call void @"??3@YAXPEAX@Z"(ptr %call)
  call void @"??3@YAXPEAX@Z"(ptr %call26)
  br label %cleanup37

cleanup37:                                        ; preds = %delete.notnull, %invoke.cont27, %if.end22, %if.end18, %if.end14, %if.end10, %if.end6, %if.end, %invoke.cont
  %retval.1 = phi i32 [ -1, %invoke.cont ], [ -1, %if.end ], [ -1, %if.end6 ], [ -1, %if.end10 ], [ -1, %if.end14 ], [ -1, %if.end18 ], [ -1, %if.end22 ], [ -1, %invoke.cont27 ], [ 0, %delete.notnull ]
  ret i32 %retval.1

ehcleanup38:                                      ; preds = %ehcleanup29, %ehcleanup
  %i10 = cleanuppad within none []
  cleanupret from %i10 unwind to caller
}

declare !intel.dtrans.func.type !18 dso_local noalias nonnull "intel_dtrans_func_index"="1" ptr @"??2@YAPEAX_K@Z"(i64) local_unnamed_addr

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @"??0F@@QEAA@XZ"(ptr returned "intel_dtrans_func_index"="2" %this) personality ptr @__CxxFrameHandler3 !intel.dtrans.func.type !20 {
entry:
  %call = call noalias nonnull dereferenceable(32) ptr @"??2@YAPEAX_K@Z"(i64 32)
  %call2 = call ptr @"??0?$Arr@PEAH@@QEAA@H@Z"(ptr nonnull %call, i32 1, ptr null)
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 0
  store ptr %call, ptr %f1, align 8
  %call3 = call noalias nonnull dereferenceable(32) ptr @"??2@YAPEAX_K@Z"(i64 32)
  %call5 = call ptr @"??0?$Arr@PEAM@@QEAA@H@Z"(ptr nonnull %call3, i32 1, ptr null)
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  store ptr %call3, ptr %f2, align 8
  ret ptr %this
}

declare dso_local i32 @__CxxFrameHandler3(...)

declare !intel.dtrans.func.type !21 dso_local void @"??3@YAXPEAX@Z"(ptr "intel_dtrans_func_index"="1") local_unnamed_addr

define linkonce_odr dso_local void @"?put@F@@QEAAXPEAHPEAM@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture "intel_dtrans_func_index"="2" %a, ptr nocapture "intel_dtrans_func_index"="3" %b) align 2 !intel.dtrans.func.type !22 {
entry:
  %b.addr = alloca ptr, align 8, !intel_dtrans_type !24
  %a.addr = alloca ptr, align 8, !intel_dtrans_type !23
  store ptr %b, ptr %b.addr, align 8
  store ptr %a, ptr %a.addr, align 8
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 0
  %i = load ptr, ptr %f1, align 8
  call void @"?add@?$Arr@PEAH@@QEAAXAEBQEAH@Z"(ptr %i, ptr nonnull align 8 dereferenceable(8) %a.addr)
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  %i1 = load ptr, ptr %f2, align 8
  call void @"?add@?$Arr@PEAM@@QEAAXAEBQEAM@Z"(ptr %i1, ptr nonnull align 8 dereferenceable(8) %b.addr)
  ret void
}

define linkonce_odr dso_local void @"?set1@F@@QEAAXHPEAH@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i, ptr nocapture "intel_dtrans_func_index"="2" %a) !intel.dtrans.func.type !25 {
entry:
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 0
  %i1 = load ptr, ptr %f1, align 8
  call void @"?set@?$Arr@PEAH@@QEAAXHPEAH@Z"(ptr %i1, i32 %i, ptr %a)
  ret void
}

define linkonce_odr dso_local void @"?set2@F@@QEAAXHPEAM@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i, ptr nocapture "intel_dtrans_func_index"="2" %b) !intel.dtrans.func.type !26 {
entry:
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  %i1 = load ptr, ptr %f2, align 8
  call void @"?set@?$Arr@PEAM@@QEAAXHPEAM@Z"(ptr %i1, i32 %i, ptr %b)
  ret void
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @"??0F@@QEAA@AEBV0@@Z"(ptr returned "intel_dtrans_func_index"="2" %this, ptr nocapture nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="3" %f) personality ptr @__CxxFrameHandler3 !intel.dtrans.func.type !27 {
entry:
  %call = invoke noalias nonnull dereferenceable(32) ptr @"??2@YAPEAX_K@Z"(i64 32)
          to label %invoke.cont unwind label %catch.dispatch

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, ptr %f, i64 0, i32 0
  %i1 = load ptr, ptr %f1, align 8
  %call3 = call ptr @"??0?$Arr@PEAH@@QEAA@AEBU0@@Z"(ptr nonnull %call, ptr nonnull align 8 dereferenceable(32) %i1)
  %f11 = getelementptr inbounds %class.F, ptr %f, i64 0, i32 0
  store ptr %call, ptr %f11, align 8
  %call6 = invoke noalias nonnull dereferenceable(32) ptr @"??2@YAPEAX_K@Z"(i64 32)
          to label %invoke.cont5 unwind label %catch.dispatch

invoke.cont5:                                     ; preds = %invoke.cont
  %f2 = getelementptr inbounds %class.F, ptr %f, i64 0, i32 1
  %i4 = load ptr, ptr %f2, align 8
  %call8 = call ptr @"??0?$Arr@PEAM@@QEAA@AEBU0@@Z"(ptr nonnull %call6, ptr nonnull align 8 dereferenceable(32) %i4)
  %f210 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  store ptr %call6, ptr %f210, align 8
  ret ptr %this

catch.dispatch:                                   ; preds = %invoke.cont, %entry
  %i6 = catchswitch within none [label %catch] unwind to caller

catch:                                            ; preds = %catch.dispatch
  %i7 = catchpad within %i6 [ptr null, i32 0, ptr null]
  call void @"?cleanup@F@@QEAAXXZ"(ptr %this) [ "funclet"(token %i7) ]
  call void @_CxxThrowException(ptr null, ptr null) [ "funclet"(token %i7) ]
  unreachable
}

define linkonce_odr dso_local nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @"?get@?$Arr@PEAH@@QEAAAEAPEAHH@Z"(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !28 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %tmp = load i32, ptr %size, align 8
  %cmp = icmp ugt i32 %tmp, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(ptr null, ptr null)
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 2
  %tmp3 = load ptr, ptr %base2, align 8
  %idxprom3 = zext i32 %i to i64
  %arrayidx4 = getelementptr inbounds ptr, ptr %tmp3, i64 %idxprom3
  ret ptr %arrayidx4
}

declare !intel.dtrans.func.type !29 dso_local void @_CxxThrowException(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2") local_unnamed_addr

define linkonce_odr dso_local nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="1" ptr @"?get@?$Arr@PEAM@@QEAAAEAPEAMH@Z"(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !31 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
  %tmp = load i32, ptr %size, align 8
  %cmp = icmp ugt i32 %tmp, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(ptr null, ptr null)
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 2
  %tmp3 = load ptr, ptr %base2, align 8
  %idxprom3 = zext i32 %i to i64
  %arrayidx4 = getelementptr inbounds ptr, ptr %tmp3, i64 %idxprom3
  ret ptr %arrayidx4
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @"??0?$Arr@PEAH@@QEAA@H@Z"(ptr returned "intel_dtrans_func_index"="2" %this, i32 %c, ptr "intel_dtrans_func_index"="3" %mem) !intel.dtrans.func.type !32 {
entry:
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0
  store i8 0, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity, align 4
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  store i32 0, ptr %size, align 8
  %mem2 = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  store ptr %call, ptr %base, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret ptr %this
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @"??0?$Arr@PEAM@@QEAA@H@Z"(ptr returned "intel_dtrans_func_index"="2" %this, i32 %c, ptr "intel_dtrans_func_index"="3" %mem) !intel.dtrans.func.type !33 {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 0
  store i8 0, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacity, align 4
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  store i32 0, ptr %size, align 8
  %mem2 = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 4
  store ptr %mem, ptr %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  store ptr %call, ptr %base, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret ptr %this
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !34 dso_local noalias "intel_dtrans_func_index"="1" ptr @malloc(i64) local_unnamed_addr #0

define linkonce_odr dso_local void @"?add@?$Arr@PEAH@@QEAAXAEBQEAH@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %e) !intel.dtrans.func.type !35 {
entry:
  call void @"?realloc@?$Arr@PEAH@@QEAAXH@Z"(ptr %this, i32 1)
  %tmp2 = load ptr, ptr %e, align 8
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 2
  %tmp3 = load ptr, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  %tmp4 = load i32, ptr %size, align 8
  %idxprom = zext i32 %tmp4 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %tmp3, i64 %idxprom
  store ptr %tmp2, ptr %arrayidx, align 8
  %inc = add nsw i32 %tmp4, 1
  store i32 %inc, ptr %size, align 8
  ret void
}

define linkonce_odr dso_local void @"?add@?$Arr@PEAM@@QEAAXAEBQEAM@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %e) !intel.dtrans.func.type !36 {
entry:
  call void @"?realloc@?$Arr@PEAM@@QEAAXH@Z"(ptr %this, i32 1)
  %tmp2 = load ptr, ptr %e, align 8
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 2
  %tmp3 = load ptr, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
  %tmp4 = load i32, ptr %size, align 8
  %idxprom = zext i32 %tmp4 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %tmp3, i64 %idxprom
  store ptr %tmp2, ptr %arrayidx, align 8
  %inc = add nsw i32 %tmp4, 1
  store i32 %inc, ptr %size, align 8
  ret void
}

define linkonce_odr dso_local void @"?realloc@?$Arr@PEAH@@QEAAXH@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !37 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  %tmp = load i32, ptr %size, align 8
  %add = add nsw i32 %tmp, 1
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 1
  %tmp2 = load i32, ptr %capacity, align 8
  %cmp = icmp ugt i32 %add, %tmp2
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %tmp to double
  %mul = fmul fast double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = call noalias ptr @malloc(i64 %mul8)
  %cmp1029 = icmp eq i32 %tmp, 0
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i3 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %tmp to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %base14 = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i5 = load ptr, ptr %base14, align 8
  call void @free(ptr %i5)
  store ptr %call, ptr %base14, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i3, i64 %indvars.iv
  %i6 = load ptr, ptr %ptridx, align 8
  %ptridx12 = getelementptr inbounds ptr, ptr %call, i64 %indvars.iv
  store ptr %i6, ptr %ptridx12, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

; Function Attrs: allockind("free")
declare !intel.dtrans.func.type !38 dso_local void @free(ptr nocapture "intel_dtrans_func_index"="1") local_unnamed_addr #1

define linkonce_odr dso_local void @"?realloc@?$Arr@PEAM@@QEAAXH@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !39 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
  %tmp = load i32, ptr %size, align 8
  %add = add nsw i32 %tmp, 1
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 1
  %tmp2 = load i32, ptr %capacity, align 8
  %cmp = icmp ugt i32 %add, %tmp2
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %tmp to double
  %mul = fmul fast double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %call = call noalias ptr @malloc(i64 %mul8)
  %cmp1029 = icmp eq i32 %tmp, 0
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i3 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %tmp to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %base14 = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i5 = load ptr, ptr %base14, align 8
  call void @free(ptr %i5)
  store ptr %call, ptr %base14, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i3, i64 %indvars.iv
  %i6 = load ptr, ptr %ptridx, align 8
  %ptridx12 = getelementptr inbounds ptr, ptr %call, i64 %indvars.iv
  store ptr %i6, ptr %ptridx12, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

define linkonce_odr dso_local void @"?set@?$Arr@PEAH@@QEAAXHPEAH@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i, ptr nocapture "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !40 {
entry:
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  %tmp = load i32, ptr %size, align 8
  %cmp = icmp ugt i32 %tmp, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(ptr null, ptr null)
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 2
  %tmp5 = load ptr, ptr %base2, align 8
  %idxprom3 = zext i32 %i to i64
  %arrayidx4 = getelementptr inbounds ptr, ptr %tmp5, i64 %idxprom3
  store ptr %val, ptr %arrayidx4, align 8
  ret void
}

define linkonce_odr dso_local void @"?set@?$Arr@PEAM@@QEAAXHPEAM@Z"(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i, ptr nocapture "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !41 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
  %tmp = load i32, ptr %size, align 8
  %cmp = icmp ugt i32 %tmp, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(ptr null, ptr null)
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 2
  %tmp5 = load ptr, ptr %base2, align 8
  %idxprom3 = zext i32 %i to i64
  %arrayidx4 = getelementptr inbounds ptr, ptr %tmp5, i64 %idxprom3
  store ptr %val, ptr %arrayidx4, align 8
  ret void
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @"??0?$Arr@PEAH@@QEAA@AEBU0@@Z"(ptr returned "intel_dtrans_func_index"="2" %this, ptr nocapture nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="3" %A) !intel.dtrans.func.type !42 {
entry:
  %flag = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 0
  %i = load i8, ptr %flag2, align 8
  store i8 %i, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 1
  %i1 = load i32, ptr %capacity3, align 4
  store i32 %i1, ptr %capacity, align 4
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  %size4 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 3
  %i2 = load i32, ptr %size4, align 8
  store i32 %i2, ptr %size, align 8
  %mem = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 4
  %i3 = load ptr, ptr %mem5, align 8
  store ptr %i3, ptr %mem, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  store ptr %call, ptr %base, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr, ptr %A, i64 0, i32 2
  %i5 = load ptr, ptr %base13, align 8
  %i6 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret ptr %this

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i5, i64 %indvars.iv
  %i7 = load ptr, ptr %ptridx, align 8
  %ptridx16 = getelementptr inbounds ptr, ptr %i6, i64 %indvars.iv
  store ptr %i7, ptr %ptridx16, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @"??0?$Arr@PEAM@@QEAA@AEBU0@@Z"(ptr returned "intel_dtrans_func_index"="2" %this, ptr nocapture nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="3" %A) !intel.dtrans.func.type !43 {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 0
  %i = load i8, ptr %flag2, align 8
  store i8 %i, ptr %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 1
  %i1 = load i32, ptr %capacity3, align 4
  store i32 %i1, ptr %capacity, align 4
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  %size4 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 3
  %i2 = load i32, ptr %size4, align 8
  store i32 %i2, ptr %size, align 8
  %mem = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 4
  %i3 = load ptr, ptr %mem5, align 8
  store ptr %i3, ptr %mem, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias ptr @malloc(i64 %mul)
  store ptr %call, ptr %base, align 8
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr.0, ptr %A, i64 0, i32 2
  %i5 = load ptr, ptr %base13, align 8
  %i6 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret ptr %this

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds ptr, ptr %i5, i64 %indvars.iv
  %i7 = load ptr, ptr %ptridx, align 8
  %ptridx16 = getelementptr inbounds ptr, ptr %i6, i64 %indvars.iv
  store ptr %i7, ptr %ptridx16, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define linkonce_odr dso_local void @"?cleanup@F@@QEAAXXZ"(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !44 {
entry:
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 0
  %i = load ptr, ptr %f1, align 8
  %isnull = icmp eq ptr %i, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  call void @"??1?$Arr@PEAH@@QEAA@XZ"(ptr nonnull %i)
  call void @"??3@YAXPEAX@Z"(ptr %i)
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %entry
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  %i2 = load ptr, ptr %f2, align 8
  %isnull2 = icmp eq ptr %i2, null
  br i1 %isnull2, label %delete.end4, label %delete.notnull3

delete.notnull3:                                  ; preds = %delete.end
  call void @"??1?$Arr@PEAM@@QEAA@XZ"(ptr nonnull %i2)
  call void @"??3@YAXPEAX@Z"(ptr %i2)
  br label %delete.end4

delete.end4:                                      ; preds = %delete.notnull3, %delete.end
  ret void
}

define linkonce_odr dso_local void @"??1?$Arr@PEAH@@QEAA@XZ"(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !45 {
entry:
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 2
  %i1 = load ptr, ptr %base, align 8
  call void @free(ptr %i1)
  ret void
}

define linkonce_odr dso_local void @"??1?$Arr@PEAM@@QEAA@XZ"(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !46 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 2
  %i1 = load ptr, ptr %base, align 8
  call void @free(ptr %i1)
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: write) }

!intel.dtrans.types = !{!0, !4, !8, !12, !14}

!0 = !{!"S", %class.F zeroinitializer, i32 3, !1, !2, !3}
!1 = !{%struct.Arr zeroinitializer, i32 1}
!2 = !{%struct.Arr.0 zeroinitializer, i32 1}
!3 = !{%struct.Mem zeroinitializer, i32 1}
!4 = !{!"S", %struct.Arr zeroinitializer, i32 5, !5, !6, !7, !6, !3}
!5 = !{i8 0, i32 0}
!6 = !{i32 0, i32 0}
!7 = !{i32 0, i32 2}
!8 = !{!"S", %struct.Mem zeroinitializer, i32 1, !9}
!9 = !{!10, i32 2}
!10 = !{!"F", i1 true, i32 0, !11}
!11 = !{!"A", i32 4, !5}
!12 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !5, !6, !13, !6, !3}
!13 = !{float 0.000000e+00, i32 2}
!14 = !{!"S", %eh.ThrowInfo zeroinitializer, i32 4, !6, !6, !6, !6}
!15 = distinct !{!16}
!16 = !{%class.F zeroinitializer, i32 1}
!17 = distinct !{!16}
!18 = distinct !{!19}
!19 = !{i8 0, i32 1}
!20 = distinct !{!16, !16}
!21 = distinct !{!19}
!22 = distinct !{!16, !23, !24}
!23 = !{i32 0, i32 1}
!24 = !{float 0.000000e+00, i32 1}
!25 = distinct !{!16, !23}
!26 = distinct !{!16, !24}
!27 = distinct !{!16, !16, !16}
!28 = distinct !{!7, !1}
!29 = distinct !{!19, !30}
!30 = !{%eh.ThrowInfo zeroinitializer, i32 1}
!31 = distinct !{!13, !2}
!32 = distinct !{!1, !1, !3}
!33 = distinct !{!2, !2, !3}
!34 = distinct !{!19}
!35 = distinct !{!1, !7}
!36 = distinct !{!2, !13}
!37 = distinct !{!1}
!38 = distinct !{!19}
!39 = distinct !{!2}
!40 = distinct !{!1, !23}
!41 = distinct !{!2, !24}
!42 = distinct !{!1, !1, !1}
!43 = distinct !{!2, !2, !2}
!44 = distinct !{!16}
!45 = distinct !{!1}
!46 = distinct !{!2}
