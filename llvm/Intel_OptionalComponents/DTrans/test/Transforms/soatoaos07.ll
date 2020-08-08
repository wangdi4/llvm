; REQUIRES: system-windows

; This test verifies that SOAToAOS is triggered when IR has specific things
; like EH etc.

; RUN: opt < %s -S -whole-program-assume -dtrans-soatoaos                     \
; RUN:          -enable-intel-advanced-opts  -mattr=+avx2                     \
; RUN:          -enable-dtrans-soatoaos -dtrans-soatoaos-size-heuristic=false \
; RUN:          -dtrans-soatoaos-ignore-classinfo=true                        \
; RUN:       | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-soatoaos              \
; RUN:          -enable-intel-advanced-opts  -mattr=+avx2                     \
; RUN:          -dtrans-soatoaos-ignore-classinfo=true                        \
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
;    int capacilty;
;    S *base;
;    int size;
;    S &get(int i) {
;      if (i >= size)
;        throw;
;      if (capacilty > 1)
;        return base[5 * i];
;      return base[i];
;    }
;    void set(int i, S val) {
;      if (i >= size)
;        throw;
;      if (capacilty > 1)
;        base[5*i] = val;
;      else
;        base[i] = val;
;    }
;    Arr(int c = 1) : capacilty(c), size(0), base(nullptr) {
;      base = (S *)malloc(capacilty * sizeof(S));
;    }
;    void realloc(int inc) {
;      if (size + inc <= capacilty)
;        return;
;
;      capacilty = size + inc;
;      S *new_base = (S *)malloc(5 * capacilty * sizeof(S));
;      for (int i = 0; i < size; ++i) {
;        new_base[5 * i] = base[i];
;      }
;      free(base);
;      base = new_base;
;    }
;    void add(const S &e) {
;      realloc(1);
;
;      if (capacilty > 1)
;        base[5 * size] = e;
;      else
;        base[size] = e;
;
;      ++size;
;    }
;    Arr(const Arr &A) {
;      capacilty = A.capacilty;
;      size = A.size;
;      if (capacilty > 1)
;        base = (S *)malloc(5 * capacilty * sizeof(S));
;      else
;        base = (S *)malloc(capacilty * sizeof(S));
;      for (int i = 0; i < size; ++i)
;        if (capacilty > 1)
;          base[5 * i] = A.base[5 * i];
;        else
;          base[i] = A.base[i];
;    }
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

; Checks that transformation is triggered.
; CHECK-DAG: %__SOADT_class.F = type { %__SOADT_AR_struct.Arr*, i64 }
; CHECK-DAG: %__SOADT_AR_struct.Arr = type { i32, %__SOADT_EL_class.F*, i32 }
; CHECK-DAG: %__SOADT_EL_class.F = type { i32*, float* }

%rtti.TypeDescriptor2 = type { i8**, i8*, [3 x i8] }
%class.F = type { %struct.Arr*, %struct.Arr.0* }
%struct.Arr = type { i32, i32**, i32 }
%struct.Arr.0 = type { i32, float**, i32 }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

@"?v1@@3HA" = dso_local global i32 20, align 4
@"?v2@@3HA" = dso_local global i32 30, align 4
@"?v3@@3MA" = dso_local global float 3.500000e+00, align 4
@"?v4@@3MA" = dso_local global float 7.500000e+00, align 4
@"??_7type_info@@6B@" = external constant i8*
@"??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { i8** @"??_7type_info@@6B@", i8* null, [3 x i8] c".H\00" }

; Function Attrs: noinline uwtable
define dso_local zeroext i1 @"?check1@@YA_NPEAVF@@@Z"(%class.F* nocapture %f)  {
entry:
  %call = call i32 @"?get1@F@@QEAAHH@Z"(%class.F* %f, i32 0)
  %0 = load i32, i32* @"?v2@@3HA", align 4
  %cmp = icmp eq i32 %call, %0
  br i1 %cmp, label %if.end, label %return

if.end:                                           ; preds = %entry
  %call1 = call float @"?get2@F@@QEAAMH@Z"(%class.F* %f, i32 0)
  %1 = load float, float* @"?v4@@3MA", align 4
  %cmp2 = fcmp une float %call1, %1
  br i1 %cmp2, label %return, label %if.end4

if.end4:                                          ; preds = %if.end
  %call5 = call i32 @"?get1@F@@QEAAHH@Z"(%class.F* %f, i32 1)
  %2 = load i32, i32* @"?v2@@3HA", align 4
  %cmp6 = icmp eq i32 %call5, %2
  br i1 %cmp6, label %if.end8, label %return

if.end8:                                          ; preds = %if.end4
  %call9 = call float @"?get2@F@@QEAAMH@Z"(%class.F* %f, i32 1)
  %3 = load float, float* @"?v4@@3MA", align 4
  %cmp10 = fcmp oeq float %call9, %3
  br label %return

return:                                           ; preds = %if.end8, %if.end4, %if.end, %entry
  %retval.0 = phi i1 [ false, %entry ], [ false, %if.end ], [ false, %if.end4 ], [ %cmp10, %if.end8 ]
  ret i1 %retval.0
}

; Function Attrs: noinline uwtable
define linkonce_odr dso_local i32 @"?get1@F@@QEAAHH@Z"(%class.F* nocapture %this, i32 %i) {
entry:
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %0 = load %struct.Arr*, %struct.Arr** %f1, align 8
  %call = call nonnull align 8 dereferenceable(8) i32** @"?get@?$Arr@PEAH@@QEAAAEAPEAHH@Z"(%struct.Arr* %0, i32 %i)
  %1 = load i32*, i32** %call, align 8
  %2 = load i32, i32* %1, align 4
  ret i32 %2
}

; Function Attrs: noinline uwtable
define linkonce_odr dso_local float @"?get2@F@@QEAAMH@Z"(%class.F* nocapture %this, i32 %i) {
entry:
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %0 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  %call = call nonnull align 8 dereferenceable(8) float** @"?get@?$Arr@PEAM@@QEAAAEAPEAMH@Z"(%struct.Arr.0* %0, i32 %i)
  %1 = load float*, float** %call, align 8
  %2 = load float, float* %1, align 4
  ret float %2
}

; Function Attrs: noinline norecurse uwtable
define dso_local i32 @main() local_unnamed_addr #1 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %call = call noalias nonnull dereferenceable(16) i8* @"??2@YAPEAX_K@Z"(i64 16) #8
  %0 = bitcast i8* %call to %class.F*
  %call1 = invoke %class.F* @"??0F@@QEAA@XZ"(%class.F* nonnull %0)
          to label %invoke.cont unwind label %ehcleanup

invoke.cont:                                      ; preds = %entry
  call void @"?put@F@@QEAAXPEAHPEAM@Z"(%class.F* nonnull %0, i32* nonnull @"?v1@@3HA", float* nonnull @"?v3@@3MA")
  %call2 = call i32 @"?get1@F@@QEAAHH@Z"(%class.F* nonnull %0, i32 0)
  %1 = load i32, i32* @"?v1@@3HA", align 4
  %cmp = icmp eq i32 %call2, %1
  br i1 %cmp, label %if.end, label %cleanup37

ehcleanup:                                        ; preds = %entry
  %2 = cleanuppad within none []
  call void @"??3@YAXPEAX@Z"(i8* %call) #9 [ "funclet"(token %2) ]
  cleanupret from %2 unwind label %ehcleanup38

if.end:                                           ; preds = %invoke.cont
  %call3 = call float @"?get2@F@@QEAAMH@Z"(%class.F* nonnull %0, i32 0)
  %3 = load float, float* @"?v3@@3MA", align 4
  %cmp4 = fcmp une float %call3, %3
  br i1 %cmp4, label %cleanup37, label %if.end6

if.end6:                                          ; preds = %if.end
  call void @"?put@F@@QEAAXPEAHPEAM@Z"(%class.F* nonnull %0, i32* nonnull @"?v2@@3HA", float* nonnull @"?v4@@3MA")
  %call7 = call i32 @"?get1@F@@QEAAHH@Z"(%class.F* nonnull %0, i32 0)
  %4 = load i32, i32* @"?v1@@3HA", align 4
  %cmp8 = icmp eq i32 %call7, %4
  br i1 %cmp8, label %if.end10, label %cleanup37

if.end10:                                         ; preds = %if.end6
  %call11 = call float @"?get2@F@@QEAAMH@Z"(%class.F* nonnull %0, i32 0)
  %5 = load float, float* @"?v3@@3MA", align 4
  %cmp12 = fcmp une float %call11, %5
  br i1 %cmp12, label %cleanup37, label %if.end14

if.end14:                                         ; preds = %if.end10
  %call15 = call i32 @"?get1@F@@QEAAHH@Z"(%class.F* nonnull %0, i32 1)
  %6 = load i32, i32* @"?v2@@3HA", align 4
  %cmp16 = icmp eq i32 %call15, %6
  br i1 %cmp16, label %if.end18, label %cleanup37

if.end18:                                         ; preds = %if.end14
  %call19 = call float @"?get2@F@@QEAAMH@Z"(%class.F* nonnull %0, i32 1)
  %7 = load float, float* @"?v4@@3MA", align 4
  %cmp20 = fcmp une float %call19, %7
  br i1 %cmp20, label %cleanup37, label %if.end22

if.end22:                                         ; preds = %if.end18
  call void @"?set1@F@@QEAAXHPEAH@Z"(%class.F* nonnull %0, i32 0, i32* nonnull @"?v2@@3HA")
  call void @"?set2@F@@QEAAXHPEAM@Z"(%class.F* nonnull %0, i32 0, float* nonnull @"?v4@@3MA")
  %call23 = call zeroext i1 @"?check1@@YA_NPEAVF@@@Z"(%class.F* nonnull %0)
  br i1 %call23, label %if.end25, label %cleanup37

if.end25:                                         ; preds = %if.end22
  %call26 = call noalias nonnull dereferenceable(16) i8* @"??2@YAPEAX_K@Z"(i64 16) #8
  %8 = bitcast i8* %call26 to %class.F*
  %call28 = invoke %class.F* @"??0F@@QEAA@AEBV0@@Z"(%class.F* nonnull %8, %class.F* nonnull align 8 dereferenceable(16) %0)
          to label %invoke.cont27 unwind label %ehcleanup29

invoke.cont27:                                    ; preds = %if.end25
  %call30 = call zeroext i1 @"?check1@@YA_NPEAVF@@@Z"(%class.F* nonnull %8)
  br i1 %call30, label %delete.notnull, label %cleanup37

ehcleanup29:                                      ; preds = %if.end25
  %9 = cleanuppad within none []
  call void @"??3@YAXPEAX@Z"(i8* %call26) #9 [ "funclet"(token %9) ]
  cleanupret from %9 unwind label %ehcleanup38

delete.notnull:                                   ; preds = %invoke.cont27
  call void @"??3@YAXPEAX@Z"(i8* %call) #9
  call void @"??3@YAXPEAX@Z"(i8* %call26) #9
  br label %cleanup37

cleanup37:                                        ; preds = %invoke.cont27, %delete.notnull, %if.end22, %if.end18, %if.end14, %if.end10, %if.end6, %if.end, %invoke.cont
  %retval.1 = phi i32 [ -1, %invoke.cont ], [ -1, %if.end ], [ -1, %if.end6 ], [ -1, %if.end10 ], [ -1, %if.end14 ], [ -1, %if.end18 ], [ -1, %if.end22 ], [ -1, %invoke.cont27 ], [ 0, %delete.notnull ]
  ret i32 %retval.1

ehcleanup38:                                      ; preds = %ehcleanup29, %ehcleanup
  %10 = cleanuppad within none []
  cleanupret from %10 unwind to caller
}

; Function Attrs: nobuiltin nofree allocsize(0)
declare dso_local noalias nonnull i8* @"??2@YAPEAX_K@Z"(i64) local_unnamed_addr #2

; Function Attrs: noinline uwtable
define linkonce_odr dso_local %class.F* @"??0F@@QEAA@XZ"(%class.F* returned %this) personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %call = call noalias nonnull dereferenceable(24) i8* @"??2@YAPEAX_K@Z"(i64 24) #8
  %0 = bitcast i8* %call to %struct.Arr*
  %call2 = call %struct.Arr* @"??0?$Arr@PEAH@@QEAA@H@Z"(%struct.Arr* nonnull %0, i32 1)
  %1 = bitcast %class.F* %this to i8**
  store i8* %call, i8** %1, align 8
  %call3 = call noalias nonnull dereferenceable(24) i8* @"??2@YAPEAX_K@Z"(i64 24) #8
  %2 = bitcast i8* %call3 to %struct.Arr.0*
  %call5 = call %struct.Arr.0* @"??0?$Arr@PEAM@@QEAA@H@Z"(%struct.Arr.0* nonnull %2, i32 1)
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %3 = bitcast %struct.Arr.0** %f2 to i8**
  store i8* %call3, i8** %3, align 8
  ret %class.F* %this
}

; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...) #3

; Function Attrs: nobuiltin nounwind
declare dso_local void @"??3@YAXPEAX@Z"(i8*) local_unnamed_addr #4

; Function Attrs: noinline uwtable
define linkonce_odr dso_local void @"?put@F@@QEAAXPEAHPEAM@Z"(%class.F* nocapture %this, i32* nocapture %a, float* nocapture %b) align 2 {
entry:
  %b.addr = alloca float*, align 8
  %a.addr = alloca i32*, align 8
  store float* %b, float** %b.addr, align 8
  store i32* %a, i32** %a.addr, align 8
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %0 = load %struct.Arr*, %struct.Arr** %f1, align 8
  call void @"?add@?$Arr@PEAH@@QEAAXAEBQEAH@Z"(%struct.Arr* %0, i32** nonnull align 8 dereferenceable(8) %a.addr)
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %1 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  call void @"?add@?$Arr@PEAM@@QEAAXAEBQEAM@Z"(%struct.Arr.0* %1, float** nonnull align 8 dereferenceable(8) %b.addr)
  ret void
}

; Function Attrs: noinline uwtable
define linkonce_odr dso_local void @"?set1@F@@QEAAXHPEAH@Z"(%class.F* nocapture %this, i32 %i, i32* nocapture %a) {
entry:
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %0 = load %struct.Arr*, %struct.Arr** %f1, align 8
  call void @"?set@?$Arr@PEAH@@QEAAXHPEAH@Z"(%struct.Arr* %0, i32 %i, i32* %a)
  ret void
}

; Function Attrs: noinline uwtable
define linkonce_odr dso_local void @"?set2@F@@QEAAXHPEAM@Z"(%class.F* nocapture %this, i32 %i, float* nocapture %b) {
entry:
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %0 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  call void @"?set@?$Arr@PEAM@@QEAAXHPEAM@Z"(%struct.Arr.0* %0, i32 %i, float* %b)
  ret void
}

; Function Attrs: noinline uwtable
define linkonce_odr dso_local %class.F* @"??0F@@QEAA@AEBV0@@Z"(%class.F* returned %this, %class.F* nocapture nonnull align 8 dereferenceable(16) %f) personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %call = invoke noalias nonnull dereferenceable(24) i8* @"??2@YAPEAX_K@Z"(i64 24) #8
          to label %invoke.cont unwind label %catch.dispatch

invoke.cont:                                      ; preds = %entry
  %0 = bitcast i8* %call to %struct.Arr*
  %f1 = getelementptr inbounds %class.F, %class.F* %f, i64 0, i32 0
  %1 = load %struct.Arr*, %struct.Arr** %f1, align 8
  %call3 = call %struct.Arr* @"??0?$Arr@PEAH@@QEAA@AEBU0@@Z"(%struct.Arr* nonnull %0, %struct.Arr* nonnull align 8 dereferenceable(24) %1)
  %2 = bitcast %class.F* %this to i8**
  store i8* %call, i8** %2, align 8
  %call6 = invoke noalias nonnull dereferenceable(24) i8* @"??2@YAPEAX_K@Z"(i64 24) #8
          to label %invoke.cont5 unwind label %catch.dispatch

invoke.cont5:                                     ; preds = %invoke.cont
  %3 = bitcast i8* %call6 to %struct.Arr.0*
  %f2 = getelementptr inbounds %class.F, %class.F* %f, i64 0, i32 1
  %4 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  %call8 = call %struct.Arr.0* @"??0?$Arr@PEAM@@QEAA@AEBU0@@Z"(%struct.Arr.0* nonnull %3, %struct.Arr.0* nonnull align 8 dereferenceable(24) %4)
  %f210 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %5 = bitcast %struct.Arr.0** %f210 to i8**
  store i8* %call6, i8** %5, align 8
  ret %class.F* %this

catch.dispatch:                                   ; preds = %invoke.cont, %entry
  %6 = catchswitch within none [label %catch] unwind to caller

catch:                                            ; preds = %catch.dispatch
  %7 = catchpad within %6 [%rtti.TypeDescriptor2* @"??_R0H@8", i32 0, i32* null]
  call void @"?cleanup@F@@QEAAXXZ"(%class.F* %this) [ "funclet"(token %7) ]
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null) #10 [ "funclet"(token %7) ]
  unreachable
}

; Function Attrs: noinline uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(8) i32** @"?get@?$Arr@PEAH@@QEAAAEAPEAHH@Z"(%struct.Arr* nocapture %this, i32 %i) {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size, align 8
  %cmp = icmp sgt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null) #10
  unreachable

if.end:                                           ; preds = %entry
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  %1 = load i32, i32* %capacilty, align 8
  %cmp2 = icmp sgt i32 %1, 1
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %2 = load i32**, i32*** %base, align 8
  %mul = mul nsw i32 %i, 5
  %spec.select = select i1 %cmp2, i32 %mul, i32 %i
  %idxprom.pn = sext i32 %spec.select to i64
  %retval.0 = getelementptr inbounds i32*, i32** %2, i64 %idxprom.pn
  ret i32** %retval.0
}

; Function Attrs: nofree
declare dso_local void @_CxxThrowException(i8*, %eh.ThrowInfo*) local_unnamed_addr #3

; Function Attrs: noinline uwtable
define linkonce_odr dso_local nonnull align 8 dereferenceable(8) float** @"?get@?$Arr@PEAM@@QEAAAEAPEAMH@Z"(%struct.Arr.0* nocapture %this, i32 %i) {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %0 = load i32, i32* %size, align 8
  %cmp = icmp sgt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null) #10
  unreachable

if.end:                                           ; preds = %entry
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  %1 = load i32, i32* %capacilty, align 8
  %cmp2 = icmp sgt i32 %1, 1
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  %2 = load float**, float*** %base, align 8
  %mul = mul nsw i32 %i, 5
  %spec.select = select i1 %cmp2, i32 %mul, i32 %i
  %idxprom.pn = sext i32 %spec.select to i64
  %retval.0 = getelementptr inbounds float*, float** %2, i64 %idxprom.pn
  ret float** %retval.0
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local %struct.Arr* @"??0?$Arr@PEAH@@QEAA@H@Z"(%struct.Arr* returned %this, i32 %c) {
entry:
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  store i32 0, i32* %size, align 8
  %mul = shl i32 %c, 3
  %call = call i8* @malloc(i32 %mul) #11
  %0 = bitcast i32*** %base to i8**
  store i8* %call, i8** %0, align 8
  ret %struct.Arr* %this
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local %struct.Arr.0* @"??0?$Arr@PEAM@@QEAA@H@Z"(%struct.Arr.0* returned %this, i32 %c) {
entry:
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  store float** null, float*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  store i32 0, i32* %size, align 8
  %mul = shl i32 %c, 3
  %call = call i8* @malloc(i32 %mul) #11
  %0 = bitcast float*** %base to i8**
  store i8* %call, i8** %0, align 8
  ret %struct.Arr.0* %this
}

; Function Attrs: nofree nounwind
declare dso_local noalias i8* @malloc(i32) local_unnamed_addr #6

; Function Attrs: noinline uwtable
define linkonce_odr dso_local void @"?add@?$Arr@PEAH@@QEAAXAEBQEAH@Z"(%struct.Arr* nocapture %this, i32** nocapture nonnull align 8 dereferenceable(8) %e) {
entry:
  call void @"?realloc@?$Arr@PEAH@@QEAAXH@Z"(%struct.Arr* %this, i32 1)
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  %0 = load i32, i32* %capacilty, align 8
  %cmp = icmp sgt i32 %0, 1
  %1 = bitcast i32** %e to i64*
  %2 = load i64, i64* %1, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %3 = load i32**, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %4 = load i32, i32* %size, align 8
  %mul = mul nsw i32 %4, 5
  %5 = select i1 %cmp, i32 %mul, i32 %4
  %idxprom4 = sext i32 %5 to i64
  %ptridx5 = getelementptr inbounds i32*, i32** %3, i64 %idxprom4
  %6 = bitcast i32** %ptridx5 to i64*
  store i64 %2, i64* %6, align 8
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %size, align 8
  ret void
}

; Function Attrs: noinline uwtable
define linkonce_odr dso_local void @"?add@?$Arr@PEAM@@QEAAXAEBQEAM@Z"(%struct.Arr.0* nocapture %this, float** nocapture nonnull align 8 dereferenceable(8) %e) {
entry:
  call void @"?realloc@?$Arr@PEAM@@QEAAXH@Z"(%struct.Arr.0* %this, i32 1)
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  %0 = load i32, i32* %capacilty, align 8
  %cmp = icmp sgt i32 %0, 1
  %1 = bitcast float** %e to i64*
  %2 = load i64, i64* %1, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  %3 = load float**, float*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %4 = load i32, i32* %size, align 8
  %mul = mul nsw i32 %4, 5
  %5 = select i1 %cmp, i32 %mul, i32 %4
  %idxprom4 = sext i32 %5 to i64
  %ptridx5 = getelementptr inbounds float*, float** %3, i64 %idxprom4
  %6 = bitcast float** %ptridx5 to i64*
  store i64 %2, i64* %6, align 8
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %size, align 8
  ret void
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local void @"?realloc@?$Arr@PEAH@@QEAAXH@Z"(%struct.Arr* nocapture %this, i32 %inc) {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size, align 8
  %add = add nsw i32 %0, %inc
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  %1 = load i32, i32* %capacilty, align 8
  %cmp = icmp sgt i32 %add, %1
  br i1 %cmp, label %if.end, label %return

if.end:                                           ; preds = %entry
  store i32 %add, i32* %capacilty, align 8
  %mul6 = mul i32 %add, 40
  %call = call i8* @malloc(i32 %mul6) #11
  %2 = bitcast i8* %call to i32**
  %cmp923 = icmp sgt i32 %0, 0
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  br i1 %cmp923, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %if.end
  %3 = load i32**, i32*** %base, align 8
  %wide.trip.count = sext i32 %0 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %4 = bitcast i32*** %base to i8**
  %5 = load i8*, i8** %4, align 8
  call void @free(i8* %5) #11
  store i8* %call, i8** %4, align 8
  br label %return

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32*, i32** %3, i64 %indvars.iv
  %6 = bitcast i32** %ptridx to i64*
  %7 = load i64, i64* %6, align 8
  %8 = mul nuw nsw i64 %indvars.iv, 5
  %ptridx12 = getelementptr inbounds i32*, i32** %2, i64 %8
  %9 = bitcast i32** %ptridx12 to i64*
  store i64 %7, i64* %9, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

return:                                           ; preds = %entry, %for.cond.cleanup
  ret void
}

; Function Attrs: nounwind
declare dso_local void @free(i8* nocapture) local_unnamed_addr #7

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local void @"?realloc@?$Arr@PEAM@@QEAAXH@Z"(%struct.Arr.0* nocapture %this, i32 %inc) {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %0 = load i32, i32* %size, align 8
  %add = add nsw i32 %0, %inc
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  %1 = load i32, i32* %capacilty, align 8
  %cmp = icmp sgt i32 %add, %1
  br i1 %cmp, label %if.end, label %return

if.end:                                           ; preds = %entry
  store i32 %add, i32* %capacilty, align 8
  %mul6 = mul i32 %add, 40
  %call = call i8* @malloc(i32 %mul6) #11
  %2 = bitcast i8* %call to float**
  %cmp923 = icmp sgt i32 %0, 0
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  br i1 %cmp923, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %if.end
  %3 = load float**, float*** %base, align 8
  %wide.trip.count = sext i32 %0 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %4 = bitcast float*** %base to i8**
  %5 = load i8*, i8** %4, align 8
  call void @free(i8* %5) #11
  store i8* %call, i8** %4, align 8
  br label %return

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds float*, float** %3, i64 %indvars.iv
  %6 = bitcast float** %ptridx to i64*
  %7 = load i64, i64* %6, align 8
  %8 = mul nuw nsw i64 %indvars.iv, 5
  %ptridx12 = getelementptr inbounds float*, float** %2, i64 %8
  %9 = bitcast float** %ptridx12 to i64*
  store i64 %7, i64* %9, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

return:                                           ; preds = %entry, %for.cond.cleanup
  ret void
}

; Function Attrs: noinline uwtable
define linkonce_odr dso_local void @"?set@?$Arr@PEAH@@QEAAXHPEAH@Z"(%struct.Arr* nocapture %this, i32 %i, i32* nocapture %val) {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  %0 = load i32, i32* %size, align 8
  %cmp = icmp sgt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null) #10
  unreachable

if.end:                                           ; preds = %entry
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  %1 = load i32, i32* %capacilty, align 8
  %cmp2 = icmp sgt i32 %1, 1
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %2 = load i32**, i32*** %base, align 8
  %mul = mul nsw i32 %i, 5
  %3 = select i1 %cmp2, i32 %mul, i32 %i
  %idxprom5 = sext i32 %3 to i64
  %ptridx6 = getelementptr inbounds i32*, i32** %2, i64 %idxprom5
  store i32* %val, i32** %ptridx6, align 8
  ret void
}

; Function Attrs: noinline uwtable
define linkonce_odr dso_local void @"?set@?$Arr@PEAM@@QEAAXHPEAM@Z"(%struct.Arr.0* nocapture %this, i32 %i, float* nocapture %val) {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  %0 = load i32, i32* %size, align 8
  %cmp = icmp sgt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @_CxxThrowException(i8* null, %eh.ThrowInfo* null) #10
  unreachable

if.end:                                           ; preds = %entry
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  %1 = load i32, i32* %capacilty, align 8
  %cmp2 = icmp sgt i32 %1, 1
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  %2 = load float**, float*** %base, align 8
  %mul = mul nsw i32 %i, 5
  %3 = select i1 %cmp2, i32 %mul, i32 %i
  %idxprom5 = sext i32 %3 to i64
  %ptridx6 = getelementptr inbounds float*, float** %2, i64 %idxprom5
  store float* %val, float** %ptridx6, align 8
  ret void
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local %struct.Arr* @"??0?$Arr@PEAH@@QEAA@AEBU0@@Z"(%struct.Arr* returned %this, %struct.Arr* nocapture nonnull align 8 dereferenceable(24) %A) {
entry:
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 0
  %0 = load i32, i32* %capacilty, align 8
  %capacilty2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  store i32 %0, i32* %capacilty2, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 2
  %1 = load i32, i32* %size, align 8
  %size3 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 2
  store i32 %1, i32* %size3, align 8
  %cmp = icmp sgt i32 %0, 1
  %mul6 = mul i32 %0, 40
  %mul10 = shl i32 %0, 3
  %2 = select i1 %cmp, i32 %mul6, i32 %mul10
  %call12 = call i8* @malloc(i32 %2) #11
  %base13 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %3 = bitcast i32*** %base13 to i8**
  store i8* %call12, i8** %3, align 8
  %cmp1540 = icmp sgt i32 %1, 0
  br i1 %cmp1540, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %base19 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i64 0, i32 1
  %4 = load i32**, i32*** %base19, align 8
  %wide.trip.count = sext i32 %1 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret %struct.Arr* %this

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %5 = mul nuw nsw i64 %indvars.iv, 5
  %6 = select i1 %cmp, i64 %5, i64 %indvars.iv
  %ptridx = getelementptr inbounds i32*, i32** %4, i64 %6
  %7 = bitcast i32** %ptridx to i64*
  %8 = load i64, i64* %7, align 8
  %9 = load i32**, i32*** %base13, align 8
  %ptridx24 = getelementptr inbounds i32*, i32** %9, i64 %6
  %10 = bitcast i32** %ptridx24 to i64*
  store i64 %8, i64* %10, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local %struct.Arr.0* @"??0?$Arr@PEAM@@QEAA@AEBU0@@Z"(%struct.Arr.0* returned %this, %struct.Arr.0* nocapture nonnull align 8 dereferenceable(24) %A) {
entry:
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 0
  %0 = load i32, i32* %capacilty, align 8
  %capacilty2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  store i32 %0, i32* %capacilty2, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 2
  %1 = load i32, i32* %size, align 8
  %size3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 2
  store i32 %1, i32* %size3, align 8
  %cmp = icmp sgt i32 %0, 1
  %mul6 = mul i32 %0, 40
  %mul10 = shl i32 %0, 3
  %2 = select i1 %cmp, i32 %mul6, i32 %mul10
  %call12 = call i8* @malloc(i32 %2) #11
  %base13 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  %3 = bitcast float*** %base13 to i8**
  store i8* %call12, i8** %3, align 8
  %cmp1540 = icmp sgt i32 %1, 0
  br i1 %cmp1540, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %base19 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i64 0, i32 1
  %4 = load float**, float*** %base19, align 8
  %wide.trip.count = sext i32 %1 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret %struct.Arr.0* %this

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %5 = mul nuw nsw i64 %indvars.iv, 5
  %6 = select i1 %cmp, i64 %5, i64 %indvars.iv
  %ptridx = getelementptr inbounds float*, float** %4, i64 %6
  %7 = bitcast float** %ptridx to i64*
  %8 = load i64, i64* %7, align 8
  %9 = load float**, float*** %base13, align 8
  %ptridx24 = getelementptr inbounds float*, float** %9, i64 %6
  %10 = bitcast float** %ptridx24 to i64*
  store i64 %8, i64* %10, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local void @"?cleanup@F@@QEAAXXZ"(%class.F* nocapture %this) {
entry:
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %0 = load %struct.Arr*, %struct.Arr** %f1, align 8
  %isnull = icmp eq %struct.Arr* %0, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  call void @"??1?$Arr@PEAH@@QEAA@XZ"(%struct.Arr* nonnull %0) #11
  %1 = bitcast %struct.Arr* %0 to i8*
  call void @"??3@YAXPEAX@Z"(i8* %1) #9
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %entry
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %2 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  %isnull2 = icmp eq %struct.Arr.0* %2, null
  br i1 %isnull2, label %delete.end4, label %delete.notnull3

delete.notnull3:                                  ; preds = %delete.end
  call void @"??1?$Arr@PEAM@@QEAA@XZ"(%struct.Arr.0* nonnull %2) #11
  %3 = bitcast %struct.Arr.0* %2 to i8*
  call void @"??3@YAXPEAX@Z"(i8* %3) #9
  br label %delete.end4

delete.end4:                                      ; preds = %delete.notnull3, %delete.end
  ret void
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local void @"??1?$Arr@PEAH@@QEAA@XZ"(%struct.Arr* nocapture %this) {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  %0 = bitcast i32*** %base to i8**
  %1 = load i8*, i8** %0, align 8
  call void @free(i8* %1) #11
  ret void
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local void @"??1?$Arr@PEAM@@QEAA@XZ"(%struct.Arr.0* nocapture %this) {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  %0 = bitcast float*** %base to i8**
  %1 = load i8*, i8** %0, align 8
  call void @free(i8* %1) #11
  ret void
}
