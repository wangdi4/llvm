; REQUIRES: system-windows
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that SOAToAOS is triggered when IR has specific things
; like EH etc.

; RUN: opt < %s -S -whole-program-assume -dtrans-soatoaos                     \
; RUN:          -enable-intel-advanced-opts  -mattr=+avx2                     \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false \
; RUN:       | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-soatoaos              \
; RUN:          -enable-intel-advanced-opts  -mattr=+avx2                     \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false \
; RUN:       | FileCheck %s

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

; CHECK-DAG: %__SOADT_class.F = type { %__SOADT_AR_struct.Arr*, i64, %struct.Mem*  }
; CHECK-DAG: %__SOADT_AR_struct.Arr = type { i8, i32, %__SOADT_EL_class.F*, i32, %struct.Mem* }
; CHECK-DAG: %__SOADT_EL_class.F = type { i32*, float* }

%rtti.TypeDescriptor2 = type { i8**, i8*, [3 x i8] }
%class.F = type { %struct.Arr*, %struct.Arr.0*, %struct.Mem* }
%struct.Arr = type { i8, i32, i32**, i32, %struct.Mem* }
%struct.Arr.0 = type { i8, i32, float**, i32, %struct.Mem* }
%struct.Mem = type { i32 (...)** }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

@"?v1@@3HA" = dso_local global i32 20, align 4
@"?v2@@3HA" = dso_local global i32 30, align 4
@"?v3@@3MA" = dso_local global float 3.500000e+00, align 4
@"?v4@@3MA" = dso_local global float 7.500000e+00, align 4
@"??_7type_info@@6B@" = external constant i8*
@"??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { i8** @"??_7type_info@@6B@", i8* null, [3 x i8] c".H\00" }

define dso_local zeroext i1 @"?check1@@YA_NPEAVF@@@Z"(%class.F* nocapture %f) {
entry:
  %call = call i32 @"?get1@F@@QEAAHH@Z"(%class.F* %f, i32 0)
  %i = load i32, i32* @"?v2@@3HA", align 4
  %cmp = icmp eq i32 %call, %i
  br i1 %cmp, label %if.end, label %return

if.end:                                           ; preds = %entry
  %call1 = call float @"?get2@F@@QEAAMH@Z"(%class.F* %f, i32 0)
  %i1 = load float, float* @"?v4@@3MA", align 4
  %cmp2 = fcmp une float %call1, %i1
  br i1 %cmp2, label %return, label %if.end4

if.end4:                                          ; preds = %if.end
  %call5 = call i32 @"?get1@F@@QEAAHH@Z"(%class.F* %f, i32 1)
  %i2 = load i32, i32* @"?v2@@3HA", align 4
  %cmp6 = icmp eq i32 %call5, %i2
  br i1 %cmp6, label %if.end8, label %return

if.end8:                                          ; preds = %if.end4
  %call9 = call float @"?get2@F@@QEAAMH@Z"(%class.F* %f, i32 1)
  %i3 = load float, float* @"?v4@@3MA", align 4
  %cmp10 = fcmp oeq float %call9, %i3
  br label %return

return:                                           ; preds = %if.end8, %if.end4, %if.end, %entry
  %retval.0 = phi i1 [ false, %entry ], [ false, %if.end ], [ false, %if.end4 ], [ %cmp10, %if.end8 ]
  ret i1 %retval.0
}

define linkonce_odr dso_local i32 @"?get1@F@@QEAAHH@Z"(%class.F* nocapture %this, i32 %i) {
entry:
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %i1 = load %struct.Arr*, %struct.Arr** %f1, align 8
  %call = call nonnull align 8 dereferenceable(8) i32** @"?get@?$Arr@PEAH@@QEAAAEAPEAHH@Z"(%struct.Arr* %i1, i32 %i)
  %i2 = load i32*, i32** %call, align 8
  %i3 = load i32, i32* %i2, align 4
  ret i32 %i3
}

define linkonce_odr dso_local float @"?get2@F@@QEAAMH@Z"(%class.F* nocapture %this, i32 %i) {
entry:
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %i1 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  %call = call nonnull align 8 dereferenceable(8) float** @"?get@?$Arr@PEAM@@QEAAAEAPEAMH@Z"(%struct.Arr.0* %i1, i32 %i)
  %i2 = load float*, float** %call, align 8
  %i3 = load float, float* %i2, align 4
  ret float %i3
}

define dso_local i32 @main() local_unnamed_addr personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %call = call noalias nonnull dereferenceable(24) i8* @"??2@YAPEAX_K@Z"(i64 24)
  %i = bitcast i8* %call to %class.F*
  %call1 = invoke %class.F* @"??0F@@QEAA@XZ"(%class.F* nonnull %i)
          to label %invoke.cont unwind label %ehcleanup

invoke.cont:                                      ; preds = %entry
  call void @"?put@F@@QEAAXPEAHPEAM@Z"(%class.F* nonnull %i, i32* nonnull @"?v1@@3HA", float* nonnull @"?v3@@3MA")
  %call2 = call i32 @"?get1@F@@QEAAHH@Z"(%class.F* nonnull %i, i32 0)
  %i1 = load i32, i32* @"?v1@@3HA", align 4
  %cmp = icmp eq i32 %call2, %i1
  br i1 %cmp, label %if.end, label %cleanup37

ehcleanup:                                        ; preds = %entry
  %i2 = cleanuppad within none []
  call void @"??3@YAXPEAX@Z"(i8* %call) [ "funclet"(token %i2) ]
  cleanupret from %i2 unwind label %ehcleanup38

if.end:                                           ; preds = %invoke.cont
  %call3 = call float @"?get2@F@@QEAAMH@Z"(%class.F* nonnull %i, i32 0)
  %i3 = load float, float* @"?v3@@3MA", align 4
  %cmp4 = fcmp une float %call3, %i3
  br i1 %cmp4, label %cleanup37, label %if.end6

if.end6:                                          ; preds = %if.end
  call void @"?put@F@@QEAAXPEAHPEAM@Z"(%class.F* nonnull %i, i32* nonnull @"?v2@@3HA", float* nonnull @"?v4@@3MA")
  %call7 = call i32 @"?get1@F@@QEAAHH@Z"(%class.F* nonnull %i, i32 0)
  %i4 = load i32, i32* @"?v1@@3HA", align 4
  %cmp8 = icmp eq i32 %call7, %i4
  br i1 %cmp8, label %if.end10, label %cleanup37

if.end10:                                         ; preds = %if.end6
  %call11 = call float @"?get2@F@@QEAAMH@Z"(%class.F* nonnull %i, i32 0)
  %i5 = load float, float* @"?v3@@3MA", align 4
  %cmp12 = fcmp une float %call11, %i5
  br i1 %cmp12, label %cleanup37, label %if.end14

if.end14:                                         ; preds = %if.end10
  %call15 = call i32 @"?get1@F@@QEAAHH@Z"(%class.F* nonnull %i, i32 1)
  %i6 = load i32, i32* @"?v2@@3HA", align 4
  %cmp16 = icmp eq i32 %call15, %i6
  br i1 %cmp16, label %if.end18, label %cleanup37

if.end18:                                         ; preds = %if.end14
  %call19 = call float @"?get2@F@@QEAAMH@Z"(%class.F* nonnull %i, i32 1)
  %i7 = load float, float* @"?v4@@3MA", align 4
  %cmp20 = fcmp une float %call19, %i7
  br i1 %cmp20, label %cleanup37, label %if.end22

if.end22:                                         ; preds = %if.end18
  call void @"?set1@F@@QEAAXHPEAH@Z"(%class.F* nonnull %i, i32 0, i32* nonnull @"?v2@@3HA")
  call void @"?set2@F@@QEAAXHPEAM@Z"(%class.F* nonnull %i, i32 0, float* nonnull @"?v4@@3MA")
  %call23 = call zeroext i1 @"?check1@@YA_NPEAVF@@@Z"(%class.F* nonnull %i)
  br i1 %call23, label %if.end25, label %cleanup37

if.end25:                                         ; preds = %if.end22
  %call26 = call noalias nonnull dereferenceable(24) i8* @"??2@YAPEAX_K@Z"(i64 24)
  %i8 = bitcast i8* %call26 to %class.F*
  %call28 = invoke %class.F* @"??0F@@QEAA@AEBV0@@Z"(%class.F* nonnull %i8, %class.F* nonnull align 8 dereferenceable(24) %i)
          to label %invoke.cont27 unwind label %ehcleanup29

invoke.cont27:                                    ; preds = %if.end25
  %call30 = call zeroext i1 @"?check1@@YA_NPEAVF@@@Z"(%class.F* nonnull %i8)
  br i1 %call30, label %delete.notnull, label %cleanup37

ehcleanup29:                                      ; preds = %if.end25
  %i9 = cleanuppad within none []
  call void @"??3@YAXPEAX@Z"(i8* %call26) [ "funclet"(token %i9) ]
  cleanupret from %i9 unwind label %ehcleanup38

delete.notnull:                                   ; preds = %invoke.cont27
  call void @"??3@YAXPEAX@Z"(i8* %call)
  call void @"??3@YAXPEAX@Z"(i8* %call26)
  br label %cleanup37

cleanup37:                                        ; preds = %delete.notnull, %invoke.cont27, %if.end22, %if.end18, %if.end14, %if.end10, %if.end6, %if.end, %invoke.cont
  %retval.1 = phi i32 [ -1, %invoke.cont ], [ -1, %if.end ], [ -1, %if.end6 ], [ -1, %if.end10 ], [ -1, %if.end14 ], [ -1, %if.end18 ], [ -1, %if.end22 ], [ -1, %invoke.cont27 ], [ 0, %delete.notnull ]
  ret i32 %retval.1

ehcleanup38:                                      ; preds = %ehcleanup29, %ehcleanup
  %i10 = cleanuppad within none []
  cleanupret from %i10 unwind to caller
}

declare dso_local noalias nonnull i8* @"??2@YAPEAX_K@Z"(i64) local_unnamed_addr

define linkonce_odr dso_local %class.F* @"??0F@@QEAA@XZ"(%class.F* returned %this) personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %call = call noalias nonnull dereferenceable(32) i8* @"??2@YAPEAX_K@Z"(i64 32)
  %i = bitcast i8* %call to %struct.Arr*
  %call2 = call %struct.Arr* @"??0?$Arr@PEAH@@QEAA@H@Z"(%struct.Arr* nonnull %i, i32 1, %struct.Mem* null)
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %i1 = bitcast %struct.Arr** %f1 to i8**
  store %struct.Arr* %i, %struct.Arr** %f1, align 8
  %call3 = call noalias nonnull dereferenceable(32) i8* @"??2@YAPEAX_K@Z"(i64 32)
  %i2 = bitcast i8* %call3 to %struct.Arr.0*
  %call5 = call %struct.Arr.0* @"??0?$Arr@PEAM@@QEAA@H@Z"(%struct.Arr.0* nonnull %i2, i32 1, %struct.Mem* null)
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %i3 = bitcast %struct.Arr.0** %f2 to i8**
  store %struct.Arr.0* %i2, %struct.Arr.0** %f2, align 8
  ret %class.F* %this
}

declare dso_local i32 @__CxxFrameHandler3(...)

declare dso_local void @"??3@YAXPEAX@Z"(i8*) local_unnamed_addr

define linkonce_odr dso_local void @"?put@F@@QEAAXPEAHPEAM@Z"(%class.F* nocapture %this, i32* nocapture %a, float* nocapture %b) align 2 {
entry:
  %b.addr = alloca float*, align 8
  %a.addr = alloca i32*, align 8
  store float* %b, float** %b.addr, align 8
  store i32* %a, i32** %a.addr, align 8
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %i = load %struct.Arr*, %struct.Arr** %f1, align 8
  call void @"?add@?$Arr@PEAH@@QEAAXAEBQEAH@Z"(%struct.Arr* %i, i32** nonnull align 8 dereferenceable(8) %a.addr)
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %i1 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  call void @"?add@?$Arr@PEAM@@QEAAXAEBQEAM@Z"(%struct.Arr.0* %i1, float** nonnull align 8 dereferenceable(8) %b.addr)
  ret void
}

define linkonce_odr dso_local void @"?set1@F@@QEAAXHPEAH@Z"(%class.F* nocapture %this, i32 %i, i32* nocapture %a) {
entry:
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %i1 = load %struct.Arr*, %struct.Arr** %f1, align 8
  call void @"?set@?$Arr@PEAH@@QEAAXHPEAH@Z"(%struct.Arr* %i1, i32 %i, i32* %a)
  ret void
}

define linkonce_odr dso_local void @"?set2@F@@QEAAXHPEAM@Z"(%class.F* nocapture %this, i32 %i, float* nocapture %b) {
entry:
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %i1 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  call void @"?set@?$Arr@PEAM@@QEAAXHPEAM@Z"(%struct.Arr.0* %i1, i32 %i, float* %b)
  ret void
}

define linkonce_odr dso_local %class.F* @"??0F@@QEAA@AEBV0@@Z"(%class.F* returned %this, %class.F* nocapture nonnull align 8 dereferenceable(24) %f) personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %call = invoke noalias nonnull dereferenceable(32) i8* @"??2@YAPEAX_K@Z"(i64 32)
          to label %invoke.cont unwind label %catch.dispatch

invoke.cont:                                      ; preds = %entry
  %i0 = bitcast i8* %call to %struct.Arr*
  %f1 = getelementptr inbounds %class.F, %class.F* %f, i64 0, i32 0
  %i1 = load %struct.Arr*, %struct.Arr** %f1, align 8
  %call3 = call %struct.Arr* @"??0?$Arr@PEAH@@QEAA@AEBU0@@Z"(%struct.Arr* nonnull %i0, %struct.Arr* nonnull align 8 dereferenceable(32) %i1)
  %f11 = getelementptr inbounds %class.F, %class.F* %f, i64 0, i32 0
  store %struct.Arr* %i0, %struct.Arr** %f11, align 8
  %call6 = invoke noalias nonnull dereferenceable(32) i8* @"??2@YAPEAX_K@Z"(i64 32)
          to label %invoke.cont5 unwind label %catch.dispatch

invoke.cont5:                                     ; preds = %invoke.cont
  %i3 = bitcast i8* %call6 to %struct.Arr.0*
  %f2 = getelementptr inbounds %class.F, %class.F* %f, i64 0, i32 1
  %i4 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  %call8 = call %struct.Arr.0* @"??0?$Arr@PEAM@@QEAA@AEBU0@@Z"(%struct.Arr.0* nonnull %i3, %struct.Arr.0* nonnull align 8 dereferenceable(32) %i4)
  %f210 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  store %struct.Arr.0* %i3, %struct.Arr.0** %f210, align 8
  ret %class.F* %this

catch.dispatch:                                   ; preds = %invoke.cont, %entry
  %i6 = catchswitch within none [label %catch] unwind to caller

catch:                                            ; preds = %catch.dispatch
  %i7 = catchpad within %i6 [%rtti.TypeDescriptor2* @"??_R0H@8", i32 0, i32* null]
  call void @"?cleanup@F@@QEAAXXZ"(%class.F* %this) [ "funclet"(token %i7) ]
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null) [ "funclet"(token %i7) ]
  unreachable
}

define linkonce_odr dso_local nonnull align 8 dereferenceable(8) i32** @"?get@?$Arr@PEAH@@QEAAAEAPEAHH@Z"(%struct.Arr* nocapture %this, i32 %i) {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %tmp = load i32, i32* %size, align 8
  %cmp = icmp ugt i32 %tmp, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null)
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2
  %tmp3 = load i32**, i32*** %base2, align 8
  %idxprom3 = zext i32 %i to i64
  %arrayidx4 = getelementptr inbounds i32*, i32** %tmp3, i64 %idxprom3
  ret i32** %arrayidx4
}

declare dso_local void @_CxxThrowException(i8*, %eh.ThrowInfo*) local_unnamed_addr

define linkonce_odr dso_local nonnull align 8 dereferenceable(8) float** @"?get@?$Arr@PEAM@@QEAAAEAPEAMH@Z"(%struct.Arr.0* nocapture %this, i32 %i) {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp = load i32, i32* %size, align 8
  %cmp = icmp ugt i32 %tmp, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null)
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2
  %tmp3 = load float**, float*** %base2, align 8
  %idxprom3 = zext i32 %i to i64
  %arrayidx4 = getelementptr inbounds float*, float** %tmp3, i64 %idxprom3
  ret float** %arrayidx4
}

define linkonce_odr dso_local %struct.Arr* @"??0?$Arr@PEAH@@QEAA@H@Z"(%struct.Arr* returned %this, i32 %c, %struct.Mem* %mem) {
entry:
  %flag = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  store i8 0, i8* %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  store i32 %c, i32* %capacity, align 4
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  store i32 0, i32* %size, align 8
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias i8* @malloc(i64 %mul)
  %i = bitcast i8* %call to i32**
  store i32** %i, i32*** %base, align 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  ret %struct.Arr* %this
}

define linkonce_odr dso_local %struct.Arr.0* @"??0?$Arr@PEAM@@QEAA@H@Z"(%struct.Arr.0* returned %this, i32 %c, %struct.Mem* %mem) {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  store i8 0, i8* %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  store i32 %c, i32* %capacity, align 4
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  store float** null, float*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  store i32 0, i32* %size, align 8
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 4
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias i8* @malloc(i64 %mul)
  %i = bitcast i8* %call to float**
  store float** %i, float*** %base, align 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  ret %struct.Arr.0* %this
}

declare dso_local noalias i8* @malloc(i64) local_unnamed_addr

define linkonce_odr dso_local void @"?add@?$Arr@PEAH@@QEAAXAEBQEAH@Z"(%struct.Arr* nocapture %this, i32** nocapture nonnull align 8 dereferenceable(8) %e) {
entry:
  call void @"?realloc@?$Arr@PEAH@@QEAAXH@Z"(%struct.Arr* %this, i32 1)
  %tmp2 = load i32*, i32** %e, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2
  %tmp3 = load i32**, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp4 = load i32, i32* %size, align 8
  %idxprom = zext i32 %tmp4 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp3, i64 %idxprom
  store i32* %tmp2, i32** %arrayidx, align 8
  %inc = add nsw i32 %tmp4, 1
  store i32 %inc, i32* %size, align 8
  ret void
}

define linkonce_odr dso_local void @"?add@?$Arr@PEAM@@QEAAXAEBQEAM@Z"(%struct.Arr.0* nocapture %this, float** nocapture nonnull align 8 dereferenceable(8) %e) {
entry:
  call void @"?realloc@?$Arr@PEAM@@QEAAXH@Z"(%struct.Arr.0* %this, i32 1)
  %tmp2 = load float*, float** %e, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2
  %tmp3 = load float**, float*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp4 = load i32, i32* %size, align 8
  %idxprom = zext i32 %tmp4 to i64
  %arrayidx = getelementptr inbounds float*, float** %tmp3, i64 %idxprom
  store float* %tmp2, float** %arrayidx, align 8
  %inc = add nsw i32 %tmp4, 1
  store i32 %inc, i32* %size, align 8
  ret void
}

define linkonce_odr dso_local void @"?realloc@?$Arr@PEAH@@QEAAXH@Z"(%struct.Arr* nocapture %this, i32 %inc) {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp = load i32, i32* %size, align 8
  %add = add nsw i32 %tmp, 1
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp2 = load i32, i32* %capacity, align 8
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
  %call = call noalias i8* @malloc(i64 %mul8)
  %i2 = bitcast i8* %call to i32**
  %cmp1029 = icmp eq i32 %tmp, 0
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %i3 = load i32**, i32*** %base, align 8
  %wide.trip.count = zext i32 %tmp to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %base14 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %i4 = bitcast i32*** %base14 to i8**
  %i5 = load i8*, i8** %i4, align 8
  call void @free(i8* %i5)
  store i8* %call, i8** %i4, align 8
  store i32 %spec.select, i32* %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32*, i32** %i3, i64 %indvars.iv
  %i6 = load i32*, i32** %ptridx, align 8
  %ptridx12 = getelementptr inbounds i32*, i32** %i2, i64 %indvars.iv
  store i32* %i6, i32** %ptridx12, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

declare dso_local void @free(i8* nocapture) local_unnamed_addr

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

define linkonce_odr dso_local void @"?realloc@?$Arr@PEAM@@QEAAXH@Z"(%struct.Arr.0* nocapture %this, i32 %inc) {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp = load i32, i32* %size, align 8
  %add = add nsw i32 %tmp, 1
  %capacity = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  %tmp2 = load i32, i32* %capacity, align 8
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
  %call = call noalias i8* @malloc(i64 %mul8)
  %i2 = bitcast i8* %call to float**
  %cmp1029 = icmp eq i32 %tmp, 0
  br i1 %cmp1029, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %i3 = load float**, float*** %base, align 8
  %wide.trip.count = zext i32 %tmp to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %base14 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %i4 = bitcast float*** %base14 to i8**
  %i5 = load i8*, i8** %i4, align 8
  call void @free(i8* %i5)
  store i8* %call, i8** %i4, align 8
  store i32 %spec.select, i32* %capacity, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds float*, float** %i3, i64 %indvars.iv
  %i6 = load float*, float** %ptridx, align 8
  %ptridx12 = getelementptr inbounds float*, float** %i2, i64 %indvars.iv
  store float* %i6, float** %ptridx12, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

define linkonce_odr dso_local void @"?set@?$Arr@PEAH@@QEAAXHPEAH@Z"(%struct.Arr* nocapture %this, i32 %i, i32* nocapture %val) {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp = load i32, i32* %size, align 8
  %cmp = icmp ugt i32 %tmp, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null)
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 2
  %tmp5 = load i32**, i32*** %base2, align 8
  %idxprom3 = zext i32 %i to i64
  %arrayidx4 = getelementptr inbounds i32*, i32** %tmp5, i64 %idxprom3
  store i32* %val, i32** %arrayidx4, align 8
  ret void
}

define linkonce_odr dso_local void @"?set@?$Arr@PEAM@@QEAAXHPEAM@Z"(%struct.Arr.0* nocapture %this, i32 %i, float* nocapture %val) {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp = load i32, i32* %size, align 8
  %cmp = icmp ugt i32 %tmp, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null)
  unreachable

if.end:                                           ; preds = %entry
  %base2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 2
  %tmp5 = load float**, float*** %base2, align 8
  %idxprom3 = zext i32 %i to i64
  %arrayidx4 = getelementptr inbounds float*, float** %tmp5, i64 %idxprom3
  store float* %val, float** %arrayidx4, align 8
  ret void
}

define linkonce_odr dso_local %struct.Arr* @"??0?$Arr@PEAH@@QEAA@AEBU0@@Z"(%struct.Arr* returned %this, %struct.Arr* nocapture nonnull align 8 dereferenceable(32) %A) {
entry:
  %flag = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 0
  %i = load i8, i8* %flag2, align 8
  store i8 %i, i8* %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 1
  %i1 = load i32, i32* %capacity3, align 4
  store i32 %i1, i32* %capacity, align 4
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  %size4 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 3
  %i2 = load i32, i32* %size4, align 8
  store i32 %i2, i32* %size, align 8
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 4
  %i3 = load %struct.Mem*, %struct.Mem** %mem5, align 8
  store %struct.Mem* %i3, %struct.Mem** %mem, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias i8* @malloc(i64 %mul)
  %i4 = bitcast i32*** %base to i8**
  store i8* %call, i8** %i4, align 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 2
  %i5 = load i32**, i32*** %base13, align 8
  %i6 = load i32**, i32*** %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret %struct.Arr* %this

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32*, i32** %i5, i64 %indvars.iv
  %i7 = load i32*, i32** %ptridx, align 8
  %ptridx16 = getelementptr inbounds i32*, i32** %i6, i64 %indvars.iv
  store i32* %i7, i32** %ptridx16, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define linkonce_odr dso_local %struct.Arr.0* @"??0?$Arr@PEAM@@QEAA@AEBU0@@Z"(%struct.Arr.0* returned %this, %struct.Arr.0* nocapture nonnull align 8 dereferenceable(32) %A) {
entry:
  %flag = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  %flag2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 0
  %i = load i8, i8* %flag2, align 8
  store i8 %i, i8* %flag, align 8
  %capacity = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  %capacity3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 1
  %i1 = load i32, i32* %capacity3, align 4
  store i32 %i1, i32* %capacity, align 4
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  store float** null, float*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  %size4 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 3
  %i2 = load i32, i32* %size4, align 8
  store i32 %i2, i32* %size, align 8
  %mem = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 4
  %mem5 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 4
  %i3 = load %struct.Mem*, %struct.Mem** %mem5, align 8
  store %struct.Mem* %i3, %struct.Mem** %mem, align 8
  %conv = zext i32 %i1 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = call noalias i8* @malloc(i64 %mul)
  %i4 = bitcast float*** %base to i8**
  store i8* %call, i8** %i4, align 8
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  %cmp25 = icmp eq i32 %i2, 0
  br i1 %cmp25, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %base13 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 2
  %i5 = load float**, float*** %base13, align 8
  %i6 = load float**, float*** %base, align 8
  %wide.trip.count = zext i32 %i2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret %struct.Arr.0* %this

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds float*, float** %i5, i64 %indvars.iv
  %i7 = load float*, float** %ptridx, align 8
  %ptridx16 = getelementptr inbounds float*, float** %i6, i64 %indvars.iv
  store float* %i7, float** %ptridx16, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define linkonce_odr dso_local void @"?cleanup@F@@QEAAXXZ"(%class.F* nocapture %this) {
entry:
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %i = load %struct.Arr*, %struct.Arr** %f1, align 8
  %isnull = icmp eq %struct.Arr* %i, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  call void @"??1?$Arr@PEAH@@QEAA@XZ"(%struct.Arr* nonnull %i)
  %i1 = bitcast %struct.Arr* %i to i8*
  call void @"??3@YAXPEAX@Z"(i8* %i1)
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %entry
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %i2 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  %isnull2 = icmp eq %struct.Arr.0* %i2, null
  br i1 %isnull2, label %delete.end4, label %delete.notnull3

delete.notnull3:                                  ; preds = %delete.end
  call void @"??1?$Arr@PEAM@@QEAA@XZ"(%struct.Arr.0* nonnull %i2)
  %i3 = bitcast %struct.Arr.0* %i2 to i8*
  call void @"??3@YAXPEAX@Z"(i8* %i3)
  br label %delete.end4

delete.end4:                                      ; preds = %delete.notnull3, %delete.end
  ret void
}

define linkonce_odr dso_local void @"??1?$Arr@PEAH@@QEAA@XZ"(%struct.Arr* nocapture %this) {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %i = bitcast i32*** %base to i8**
  %i1 = load i8*, i8** %i, align 8
  call void @free(i8* %i1)
  ret void
}

define linkonce_odr dso_local void @"??1?$Arr@PEAM@@QEAA@XZ"(%struct.Arr.0* nocapture %this) {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %i = bitcast float*** %base to i8**
  %i1 = load i8*, i8** %i, align 8
  call void @free(i8* %i1)
  ret void
}

attributes #0 = { argmemonly nofree nosync nounwind willreturn writeonly }
