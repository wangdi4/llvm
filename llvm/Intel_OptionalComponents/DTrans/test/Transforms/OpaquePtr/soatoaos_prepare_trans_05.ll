; CMPLRLLVM-27768: This test is same as soatoaos_prepare_trans_02.ll
; except the following. This test verifies that SOAToAOSPrepare transformations
; occur when pre-header is missing for loops in _ZN6RefArrIPsED2Ev and
; _ZN1FC2ERKS_.
; 1. "for.body.lr.ph" basicblock, which is a pre-header, is removed
;   in _ZN6RefArrIPsED2Ev.
; 2. "pre" basicblock, which is a pre-header, is removed in _ZN1FC2ERKS_.
;

; This test verifies that SOAToAOSPrepare transformations related
; SetElem/AppendElem are done correctly.
;
; _ZN1FC2ERKS_: Verifies that Ctor (_ZN6RefArrIPsEC2EjbP3Mem) / AppendElem
; (_ZN7BaseArrIPsE3addEPS0_) calls are converted to CCtor/SimpleSetElem calls.
; Verifies that new CCtor and SimpleSetElem functions are created.
;
; _ZN1FC2Ev: Verifies that reverse argument promotion is done for
;  AppendElem (_ZN7BaseArrIPsE3addEPS0_).

; RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-soatoaosop-prepare  -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -passes=dtrans-soatoaosop-prepare  -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -dtrans-soatoaosop-prepare  -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck  --check-prefix=CHECK-OP %s
; RUN: opt < %s -opaque-pointers -passes=dtrans-soatoaosop-prepare  -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck  --check-prefix=CHECK-OP %s

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" fields in "F" will be considered as candidate vector
; fields for SOAToAOS transformation. "f3" will be rejected by SOAToAOS
; for the transformation because type of "f3" (i.e RefArr) is not simple
; vector class. So, SOAToAOSPrepare will consider "f3" as candidate
; and then try to convert it as simple vector class.

; Member functions of BaseArr & RefArr classes have valid definition to
; those classes as vector classes.  Definitions of member functions of
; Arr class are empty or don't make sense.

; "f3" is considered as candidate and "RefArr<short*>" is considered as
; valid vector class.

; struct Mem {
;   virtual void *allocate() = 0;
;   virtual void *deallocate() = 0;
; };

; template <typename S> struct Arr {
;   bool flag;
;   unsigned capacity;
;   unsigned size;
;   S **base;
;   Mem *mem;
;
;   unsigned getSize() { return 0; }
;   unsigned getCapacity() { return 0; }
;   void resize(int inc) { }
;   ~Arr() { free(base); }
;   void add(S* val) {
;     resize(1);
;   }
;   void set(S* val, unsigned i) {
;   }
;   S* get(unsigned i) {
;     return nullptr;
;   }
;   Arr(unsigned c = 2, Mem *mem = 0) { }
;
;   Arr(const Arr &A) { }
; };
;
; template <typename S> struct BaseArr {
;   bool flag;
;   unsigned capacity;
;   unsigned size;
;   S **base;
;   Mem *mem;
;   unsigned getSize() { return size; }
;   unsigned getCapacity() { return capacity; }
;   void resize(int inc) {
;     unsigned int newMax = size + inc;
;     if (newMax <= capacity)
;        return;
;
;     if (newMax < capacity + capacity/2)
;       newMax = capacity + capacity/2;
;
;     S **newList = (S **) malloc (newMax * sizeof(S*));
;     unsigned int index = 0;
;     for (; index < size; index++)
;        newList[index] = base[index];
;     for (; index < newMax; index++)
;        newList[index] = 0;
;
;     free(base);
;     base = newList;
;     capacity = newMax;
;   }
;
;   virtual ~BaseArr() { }
;
;   void add(S* val) {
;     resize(1);
;     base[size] = val;
;     size++;
;   }
;
;   virtual void set(S* val, unsigned i) {
;     if (i >= size)
;       throw;
;     if (flag)
;       free(base[i]);
;     base[i] = val;
;   }
;
;   S* get(unsigned i) {
;     if (i >= size)
;       throw;
;     return base[i];
;   }
;   BaseArr(unsigned c, bool adopE, Mem *mem = 0)
;      : flag(adopE), capacity(c), size(0), base(0), mem(mem) {
;      base = (S**)malloc(capacity * sizeof(S*));
;      memset(base, 0, capacity * sizeof(S*));
;   }
;   BaseArr(const BaseArr &A) { }
; };
;
; template <typename S>
; struct RefArr : public BaseArr<S> {
; public:
;   RefArr(unsigned c, bool adoptE, Mem *mem = 0);
;   ~RefArr() {
;     if (this->flag) {
;       for (unsigned int i = 0; i < this->size; i++)
;         free(this->base[i]);
;     }
;     free(this->base);
;   }
; };
; template <typename S>
;  RefArr<S>::RefArr(unsigned c, bool adoptE, Mem *mem)
;      : BaseArr<S>(c, adoptE, mem) { }
;
; class F {
;  Mem *mem;
; public:
;  Arr<int*>* f1;
;  Arr<float*>* f2;
;  RefArr<short*>* f3;
;  F() {
;    f1 = new Arr<int *>(10, nullptr);
;    f2 = new Arr<float *>(10, nullptr);
;    f3 = new RefArr<short *>(10, true, nullptr);
;    int** pi = f1->get(1);
;    float** pf = f2->get(1);
;    short** ps = f3->get(1);
;    f1->set(pi, 0);
;    f2->set(pf, 0);
;    f3->set(ps, 0);
;    f1->add(nullptr);
;    f2->add(nullptr);
;    f3->add(nullptr);
;    f1->getSize();
;    f2->getSize();
;    f3->getSize();
;    f1->getCapacity();
;    f2->getCapacity();
;    f3->getCapacity();
;    delete f1;
;    delete f2;
;    delete f3;
;  }
;  F(const F &Src) {
;    unsigned ValS = Src.f3->getSize();
;    f1 = new Arr<int *>(*f1);
;    f2 = new Arr<float *>(*f2);
;    f3 = new RefArr<short *>(ValS, true, Src.mem);
;    for (unsigned i = 0; i < ValS; i++) {
;      f3->add(Src.f3->get(i));
;    }
;  }
; };
; int main() {
;  F *f = new F();
;  F*ff = new F(*f);
; }
;

; Make sure types are created correctly.
; CHECK: %_DPRE_class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %_DPRE__REP_struct.RefArr* }
; CHECK: %_DPRE__REP_struct.RefArr = type { i8, i32, i32, i16***, %struct.Mem* }

; Cloned struct method is called from main.
; CHECK: define i32 @main()
; CHECK: tail call void @_ZN1FC2Ev{{.*}}
; CHECK: tail call void @_ZN1FC2ERKS_{{.*}}

; Make sure Ctor/AppendElem calls are replaced with CCtor/SimpleSetElem calls
; CHECK: define internal fastcc void @_ZN1FC2ERKS_{{.*}}(%_DPRE_class.F* "intel_dtrans_func_index"="1" %0, %_DPRE_class.F* "intel_dtrans_func_index"="2" %1)
; CHECK: tail call void @_ZN7BaseArrIPsEC2EjbP3Mem{{.*}}{{.*}}{{.*}}{{.*}}
; CHECK: tail call fastcc void @_ZN7BaseArrIPsE3setEjPS0_{{.*}}{{.*}}{{.*}}
; CHECK: ret void

; Make sure argument of AppendElem is demoted.
; CHECK: define internal void @_ZN1FC2Ev{{.*}}(
; CHECK: %0 = alloca i16**
; CHECK:   store i16** null, i16*** %0
; CHECK: tail call void @_ZN7BaseArrIPsE3addEPS0_{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr* %12, i16*** %0)

; Make sure new simple SetElem function is created
; CHECK: define internal void @_ZN7BaseArrIPsE3setEjPS0_{{.*}}{{.*}}{{.*}}
; CHECK:   %3 = getelementptr inbounds %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %0, i64 0, i32 3
; CHECK:  %4 = load i16***, i16**** %3, align 8
; CHECK:  %5 = zext i32 %2 to i64
; CHECK:  %6 = getelementptr inbounds i16**, i16*** %4, i64 %5
; CHECK:  store i16** %1, i16*** %6, align 8
; CHECK:   ret void

; Make sure new simple CCtor function is created
; CHECK: define internal void @_ZN7BaseArrIPsEC2EjbP3Mem{{.*}}{{.*}}{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr* nocapture "intel_dtrans_func_index"="1" %0, %_DPRE__REP_struct.RefArr* nocapture readonly "intel_dtrans_func_index"="2" %1)
; CHECK:  %2 = getelementptr %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %1, i64 0, i32 0
; CHECK:  %3 = load i8, i8* %2
; CHECK:  %flag = getelementptr inbounds %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %0, i64 0, i32 0
; CHECK:  store i8 %3, i8* %flag
; CHECK:  %4 = getelementptr %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %1, i64 0, i32 1
; CHECK:  %5 = load i32, i32* %4
; CHECK:  %capacity = getelementptr inbounds %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %0, i64 0, i32 1
; CHECK:  store i32 %5, i32* %capacity
; CHECK:  %6 = getelementptr %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %1, i64 0, i32 2
; CHECK:  %7 = load i32, i32* %6
; CHECK:  %size = getelementptr inbounds %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %0, i64 0, i32 2
; CHECK:  store i32 %7, i32* %size
; CHECK:  %base = getelementptr inbounds %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %0, i64 0, i32 3
; CHECK:  %8 = getelementptr %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %1, i64 0, i32 4
; CHECK:  %9 = load %struct.Mem*, %struct.Mem** %8
; CHECK:  %mem3 = getelementptr inbounds %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %0, i64 0, i32 4
; CHECK:  store %struct.Mem* %9, %struct.Mem** %mem3
; CHECK:  %conv = zext i32 %5 to i64
; CHECK:  %mul = shl nuw nsw i64 %conv, 3
; CHECK:  %call = tail call noalias i8* @malloc(i64 %mul)
; CHECK:  %10 = bitcast i8* %call to i16***
; CHECK:  store i16*** %10, i16**** %base
; CHECK:  tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
; CHECK:  ret void

;
; Check for OP
;
; Make sure types are created correctly.
; CHECK-OP: %_DPRE_class.F = type { ptr, ptr, ptr, ptr }
; CHECK-OP: %_DPRE__REP_struct.RefArr = type { i8, i32, i32, ptr, ptr }

; Cloned struct method is called from main.
; CHECK-OP: define i32 @main()
; CHECK-OP: tail call void @_ZN1FC2Ev
; CHECK-OP: tail call void @_ZN1FC2ERKS_

; Make sure Ctor/AppendElem calls are replaced with CCtor/SimpleSetElem calls
; CHECK-OP: define internal fastcc void @_ZN1FC2ERKS_(ptr "intel_dtrans_func_index"="1" %0, ptr "intel_dtrans_func_index"="2" %1)
; CHECK-OP: tail call void @_ZN7BaseArrIPsEC2EjbP3Mem{{.*}}{{.*}}
; CHECK-OP: tail call fastcc void @_ZN7BaseArrIPsE3setEjPS0_{{.*}}
; CHECK-OP: ret void

; Make sure argument of AppendElem is demoted.
; CHECK-OP: define linkonce_odr dso_local void @_ZN1FC2Ev(
; CHECK-OP: %0 = alloca ptr
; CHECK-OP: tail call void @_ZN3ArrIPfE3addEPS0_
; CHECK-OP: [[LD0:%[0-9]*]] = load ptr, ptr %f3
; CHECK-OP:   store ptr null, ptr %0
; CHECK-OP: tail call void @_ZN7BaseArrIPsE3addEPS0_{{.*}}(ptr [[LD0]], ptr %0)

; Make sure new simple SetElem function is created
; CHECK-OP: define linkonce_odr dso_local void @_ZN7BaseArrIPsE3setEjPS0_{{.*}}{{.*}}
; CHECK-OP:   %3 = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 3
; CHECK-OP:  %4 = load ptr, ptr %3, align 8
; CHECK-OP:  %5 = zext i32 %2 to i64
; CHECK-OP:  %6 = getelementptr inbounds ptr, ptr %4, i64 %5
; CHECK-OP:  store ptr %1, ptr %6, align 8
; CHECK-OP:   ret void

; Make sure new simple CCtor function is created
; CHECK-OP: define linkonce_odr dso_local void @_ZN7BaseArrIPsEC2EjbP3Mem{{.*}}{{.*}}{{.*}}(ptr nocapture "intel_dtrans_func_index"="1" %0, ptr nocapture readonly "intel_dtrans_func_index"="2" %1)
; CHECK-OP:  %2 = getelementptr %_DPRE__REP_struct.RefArr, ptr %1, i64 0, i32 0
; CHECK-OP:  %3 = load i8, ptr %2
; CHECK-OP:  %flag = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 0
; CHECK-OP:  store i8 %3, ptr %flag
; CHECK-OP:  %4 = getelementptr %_DPRE__REP_struct.RefArr, ptr %1, i64 0, i32 1
; CHECK-OP:  %5 = load i32, ptr %4
; CHECK-OP:  %capacity = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 1
; CHECK-OP:  store i32 %5, ptr %capacity
; CHECK-OP:  %6 = getelementptr %_DPRE__REP_struct.RefArr, ptr %1, i64 0, i32 2
; CHECK-OP:  %7 = load i32, ptr %6
; CHECK-OP:  %size = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 2
; CHECK-OP:  store i32 %7, ptr %size
; CHECK-OP:  %base = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 3
; CHECK-OP:  %8 = getelementptr %_DPRE__REP_struct.RefArr, ptr %1, i64 0, i32 4
; CHECK-OP:  %9 = load ptr, ptr %8
; CHECK-OP:  %mem3 = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 4
; CHECK-OP:  store ptr %9, ptr %mem3
; CHECK-OP:  %conv = zext i32 %5 to i64
; CHECK-OP:  %mul = shl nuw nsw i64 %conv, 3
; CHECK-OP:  %call = tail call noalias ptr @malloc(i64 %mul)
; CHECK-OP:  store ptr %call, ptr %base
; CHECK-OP:  tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
; CHECK-OP:  ret void


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.RefArr* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type { i8, i32, i32, i32***, %struct.Mem* }
%struct.Arr.0 = type { i8, i32, i32, float***, %struct.Mem* }
%struct.RefArr = type { %struct.BaseArr }
%struct.BaseArr = type { i32 (...)**, i8, i32, i32, i16***, %struct.Mem* }

$_ZTS7BaseArrIPsE = comdat any
$_ZTS6RefArrIPsE = comdat any
$_ZTV6RefArrIPsE = comdat any
$_ZTI6RefArrIPsE = comdat any
$_ZTV7BaseArrIPsE = comdat any
$_ZTI7BaseArrIPsE = comdat any

@_ZTV6RefArrIPsE = linkonce_odr dso_local unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI6RefArrIPsE to i8*), i8* bitcast (void (%struct.RefArr*)* @_ZN6RefArrIPsED2Ev to i8*), i8* bitcast (void (%struct.RefArr*)* @_ZN6RefArrIPsED0Ev to i8*), i8* bitcast (void (%struct.BaseArr*, i16**, i32)* @_ZN7BaseArrIPsE3setEjPS0_ to i8*)] }, comdat, align 8, !intel_dtrans_type !0
@_ZTS6RefArrIPsE = linkonce_odr dso_local constant [12 x i8] c"6RefArrIPsE\00", comdat, align 1
@_ZTS7BaseArrIPsE = linkonce_odr dso_local constant [13 x i8] c"7BaseArrIPsE\00", comdat, align 1
@_ZTI6RefArrIPsE = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** null, i64 2) to i8*), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @_ZTS6RefArrIPsE, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI7BaseArrIPsE to i8*) }, comdat, align 8, !intel_dtrans_type !4
@_ZTI7BaseArrIPsE = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** null, i64 2) to i8*), i8* getelementptr inbounds ([13 x i8], [13 x i8]* @_ZTS7BaseArrIPsE, i32 0, i32 0) }, comdat, align 8, !intel_dtrans_type !3
@_ZTV7BaseArrIPsE = linkonce_odr dso_local unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTI7BaseArrIPsE to i8*), i8* bitcast (void (%struct.BaseArr*)* @_ZN7BaseArrIPsED2Ev to i8*), i8* bitcast (void (%struct.BaseArr*)* @_ZN7BaseArrIPsED0Ev to i8*), i8* bitcast (void (%struct.BaseArr*, i16**, i32)* @_ZN7BaseArrIPsE3setEjPS0_ to i8*)] }, comdat, align 8, !intel_dtrans_type !0

define i32 @main() {
entry:
  %call = call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %class.F*
  tail call void @_ZN1FC2Ev(%class.F* %0)
  %call1 = call i8* @_Znwm(i64 32)
  %1 = bitcast i8* %call1 to %class.F*
  tail call void @_ZN1FC2ERKS_(%class.F* %1, %class.F* %0)
  ret i32 0
}

define internal fastcc void @_ZN1FC2ERKS_(%class.F* "intel_dtrans_func_index"="1" %0, %class.F* "intel_dtrans_func_index"="2" %1) !intel.dtrans.func.type !75 {
  %3 = getelementptr inbounds %class.F, %class.F* %1, i64 0, i32 3
  %4 = bitcast %struct.RefArr** %3 to %struct.BaseArr**
  %5 = load %struct.BaseArr*, %struct.BaseArr** %4
  %6 = bitcast %struct.RefArr** %3 to %struct.BaseArr**
  %7 = bitcast %struct.RefArr** %3 to %struct.BaseArr**
  %8 = tail call fastcc i32 @_ZN7BaseArrIPsE7getSizeEv(%struct.BaseArr* %5)
  %9 = tail call i8* @_Znwm(i64 32)
  %10 = bitcast i8* %9 to %struct.Arr*
  %11 = getelementptr inbounds %class.F, %class.F* %0, i64 0, i32 1
  %12 = load %struct.Arr*, %struct.Arr** %11
  tail call fastcc void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* %10, %struct.Arr* %12)
  %13 = bitcast i8* %9 to %struct.Arr*
  store %struct.Arr* %10, %struct.Arr** %11
  %14 = tail call i8* @_Znwm(i64 32)
  %15 = bitcast i8* %14 to %struct.Arr.0*
  %16 = getelementptr inbounds %class.F, %class.F* %0, i64 0, i32 2
  %17 = load %struct.Arr.0*, %struct.Arr.0** %16
  tail call fastcc void @_ZN3ArrIPfEC2ERKS1_(%struct.Arr.0* %15, %struct.Arr.0* %17)
  %18 = bitcast %struct.Arr.0** %16 to i8**
  store %struct.Arr.0* %15, %struct.Arr.0** %16
  %19 = tail call i8* @_Znwm(i64 40)
  %20 = bitcast i8* %19 to %struct.RefArr*
  %21 = getelementptr inbounds %class.F, %class.F* %1, i64 0, i32 0
  %22 = load %struct.Mem*, %struct.Mem** %21
  tail call fastcc void @_ZN6RefArrIPsEC2EjbP3Mem(%struct.RefArr* %20, i32 %8, i1 zeroext true, %struct.Mem* %22)
  %23 = getelementptr inbounds %class.F, %class.F* %0, i64 0, i32 3
  %24 = bitcast %struct.RefArr** %23 to %struct.BaseArr**
   br label %ztt

ztt:                                              ; preds = %2
  store %struct.RefArr* %20, %struct.RefArr** %23
  %25 = icmp eq i32 %8, 0
  br i1 %25, label %33, label %pre

pre:
  br label %26

26:                                               ; preds = %26, %pre
  %27 = phi i32 [ %31, %26 ], [ 0, %pre]
  %28 = load %struct.BaseArr*, %struct.BaseArr** %24
  %29 = load %struct.BaseArr*, %struct.BaseArr** %4
  %30 = tail call fastcc i16** @_ZN7BaseArrIPsE3getEj(%struct.BaseArr* %29, i32 %27)
  tail call fastcc void @_ZN7BaseArrIPsE3addEPS0_(%struct.BaseArr* %28, i16** %30)
  %31 = add nuw i32 %27, 1
  %32 = icmp eq i32 %31, %8
  br i1 %32, label %33, label %26

33:                                               ; preds = %26, %ztt
  ret void
}

define linkonce_odr dso_local void @_ZN1FC2Ev(%class.F* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !29 {
entry:
  %call = tail call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %struct.Arr*
  tail call void @_ZN3ArrIPiEC2EjP3Mem(%struct.Arr* %0, i32 10, %struct.Mem* null)
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %1 = bitcast %struct.Arr** %f1 to i8**
  store %struct.Arr* %0, %struct.Arr** %f1
  %call2 = tail call i8* @_Znwm(i64 32)
  %2 = bitcast i8* %call2 to %struct.Arr.0*
  tail call void @_ZN3ArrIPfEC2EjP3Mem(%struct.Arr.0* nonnull %2, i32 10, %struct.Mem* null)
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 2
  %3 = bitcast %struct.Arr.0** %f2 to i8**
  store %struct.Arr.0* %2, %struct.Arr.0** %f2
  %call5 = tail call i8* @_Znwm(i64 40)
  %4 = bitcast i8* %call5 to %struct.RefArr*
  %g1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 0
  %ld1 = load %struct.Mem*, %struct.Mem** %g1
  tail call void @_ZN6RefArrIPsEC2EjbP3Mem(%struct.RefArr* nonnull %4, i32 10, i1 zeroext true, %struct.Mem* %ld1)
  br label %invoke.cont7

invoke.cont7:                                     ; preds = %entry
  %f3 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 3
  %5 = bitcast %struct.RefArr** %f3 to i8**
  store %struct.RefArr* %4, %struct.RefArr** %f3
  %6 = load %struct.Arr*, %struct.Arr** %f1
  %call9 = tail call i32** @_ZN3ArrIPiE3getEj(%struct.Arr* %6, i32 1)
  %7 = load %struct.Arr.0*, %struct.Arr.0** %f2
  %call11 = tail call float** @_ZN3ArrIPfE3getEj(%struct.Arr.0* %7, i32 1)
  %8 = bitcast %struct.RefArr** %f3 to %struct.BaseArr**
  %9 = load %struct.BaseArr*, %struct.BaseArr** %8
  %call13 = tail call i16** @_ZN7BaseArrIPsE3getEj(%struct.BaseArr* %9, i32 1)
  %10 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiE3setEjPS0_(%struct.Arr* %10, i32** %call9, i32 0)
  %11 = load %struct.Arr.0*, %struct.Arr.0** %f2
  tail call void @_ZN3ArrIPfE3setEjPS0_(%struct.Arr.0* %11, float** %call11, i32 0)
  %12 = load %struct.BaseArr*, %struct.BaseArr** %8
  %13 = bitcast %struct.BaseArr* %12 to void (%struct.BaseArr*, i32, i16**)***
  %vtable = load void (%struct.BaseArr*, i32, i16**)**, void (%struct.BaseArr*, i32, i16**)*** %13
  %vfn = getelementptr inbounds void (%struct.BaseArr*, i32, i16**)*, void (%struct.BaseArr*, i32, i16**)** %vtable, i64 2
  %14 = load void (%struct.BaseArr*, i32, i16**)*, void (%struct.BaseArr*, i32, i16**)** %vfn
  tail call void @_ZN7BaseArrIPsE3setEjPS0_(%struct.BaseArr* %12, i16** %call13, i32 0)
  %15 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* %15, i32** null)
  %16 = load %struct.Arr.0*, %struct.Arr.0** %f2
  tail call void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* %16, float** null)
  %17 = load %struct.BaseArr*, %struct.BaseArr** %8
  tail call void @_ZN7BaseArrIPsE3addEPS0_(%struct.BaseArr* %17, i16** null)
  %18 = load %struct.Arr*, %struct.Arr** %f1
  %call21 = tail call i32 @_ZN3ArrIPiE7getSizeEv(%struct.Arr* %18)
  %19 = load %struct.Arr.0*, %struct.Arr.0** %f2
  %call23 = tail call i32 @_ZN3ArrIPfE7getSizeEv(%struct.Arr.0* %19)
  %20 = load %struct.BaseArr*, %struct.BaseArr** %8
  %call25 = tail call i32 @_ZN7BaseArrIPsE7getSizeEv(%struct.BaseArr* %20)
  %21 = load %struct.Arr*, %struct.Arr** %f1
  %call27 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* %21)
  %22 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiED2Ev(%struct.Arr* nonnull %22)
  %23 = getelementptr inbounds %struct.Arr, %struct.Arr* %22, i64 0, i32 0
  tail call void @_ZdlPv(i8* %23)
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %for.cond.cleanup
  %24 = load %struct.Arr.0*, %struct.Arr.0** %f2
  %isnull49 = icmp eq %struct.Arr.0* %24, null
  br i1 %isnull49, label %delete.end51, label %delete.notnull50


delete.notnull50:                                 ; preds = %delete.end
  tail call void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nonnull %24)
  %25 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %24, i64 0, i32 0
  tail call void @_ZdlPv(i8* %25)
  br label %delete.end51

delete.end51:                                     ; preds = %delete.notnull50, %delete.end
  %26 = load %struct.RefArr*, %struct.RefArr** %f3
  %isnull53 = icmp eq %struct.RefArr* %26, null
  br i1 %isnull53, label %delete.end57, label %delete.notnull54

delete.notnull54:                                 ; preds = %delete.end51
  %27 = bitcast %struct.RefArr* %26 to void (%struct.RefArr*)***
  %vtable55 = load void (%struct.RefArr*)**, void (%struct.RefArr*)*** %27
  %vfn56 = getelementptr inbounds void (%struct.RefArr*)*, void (%struct.RefArr*)** %vtable55, i64 1
  %28 = load void (%struct.RefArr*)*, void (%struct.RefArr*)** %vfn56
  tail call void @_ZN6RefArrIPsED0Ev(%struct.RefArr* nonnull %26)
  br label %delete.end57

delete.end57:                                     ; preds = %delete.notnull54, %delete.end51
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2EjP3Mem(%struct.Arr* nocapture "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !40 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2EjP3Mem(%struct.Arr.0* "intel_dtrans_func_index"="1" nocapture %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !41 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsEC2EjbP3Mem(%struct.RefArr*  nocapture "intel_dtrans_func_index"="1" %this, i32 %c, i1 %adoptE, %struct.Mem* "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !42 {
entry:
  %0 = getelementptr inbounds %struct.RefArr, %struct.RefArr* %this, i64 0, i32 0
  tail call void @_ZN7BaseArrIPsEC2EjbP3Mem(%struct.BaseArr* %0, i32 %c, i1 zeroext %adoptE, %struct.Mem* %mem)
  %1 = getelementptr inbounds %struct.RefArr, %struct.RefArr* %this, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTV6RefArrIPsE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %1
  ret void
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" i32** @_ZN3ArrIPiE3getEj(%struct.Arr* nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !43 {
entry:
  ret i32** null
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" float** @_ZN3ArrIPfE3getEj(%struct.Arr.0* nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !44 {
entry:
  ret float** null
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" i16** @_ZN7BaseArrIPsE3getEj(%struct.BaseArr* nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !45 {
entry:
  %size = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 3
  %0 = load i32, i32* %size
  %cmp = icmp ugt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow() #12
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 4
  %1 = load i16***, i16**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds i16**, i16*** %1, i64 %idxprom
  %2 = load i16**, i16*** %arrayidx
  ret i16** %2
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3setEjPS0_(%struct.Arr* nocapture "intel_dtrans_func_index"="1" %this, i32** "intel_dtrans_func_index"="2" %val, i32 %i) !intel.dtrans.func.type !46 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3setEjPS0_(%struct.Arr.0* nocapture  "intel_dtrans_func_index"="1" %this, float** "intel_dtrans_func_index"="2" %val, i32 %i) !intel.dtrans.func.type !47 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* nocapture "intel_dtrans_func_index"="1" %this, i32** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !48 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(%struct.Arr* %this, i32 1)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* nocapture "intel_dtrans_func_index"="1" %this, float** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !49 {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* %this, i32 1)
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE3addEPS0_(%struct.BaseArr* nocapture "intel_dtrans_func_index"="1" %this, i16** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !50 {
entry:
  tail call void @_ZN7BaseArrIPsE6resizeEi(%struct.BaseArr* %this, i32 1)
  %base = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 4
  %0 = load i16***, i16**** %base
  %size = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 3
  %1 = load i32, i32* %size
  %idxprom = zext i32 %1 to i64
  %arrayidx = getelementptr inbounds i16**, i16*** %0, i64 %idxprom
  store i16** %val, i16*** %arrayidx
  %inc = add i32 %1, 1
  store i32 %inc, i32* %size
  ret void
}

define linkonce_odr dso_local i32 @_ZN3ArrIPiE7getSizeEv(%struct.Arr* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !51 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN3ArrIPfE7getSizeEv(%struct.Arr.0* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !52 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN7BaseArrIPsE7getSizeEv(%struct.BaseArr* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !53 {
entry:
  %size = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 3
  %0 = load i32, i32* %size
  ret i32 %0
}

define linkonce_odr dso_local i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !54 {
entry:
  ret i32 0
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* nocapture "intel_dtrans_func_index"="1" %this, %struct.Arr* nocapture "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !55 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2ERKS1_(%struct.Arr.0* nocapture "intel_dtrans_func_index"="1" %this, %struct.Arr.0* "intel_dtrans_func_index"="2" nocapture %A) !intel.dtrans.func.type !56 {
entry:
  ret void
}

define linkonce_odr dso_local i32 @_ZN3ArrIPfE11getCapacityEv(%struct.Arr.0* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !57 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN7BaseArrIPsE11getCapacityEv(%struct.BaseArr* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !58 {
entry:
  %capacity = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 2
  %0 = load i32, i32* %capacity
  ret i32 %0
}

define linkonce_odr dso_local void @_ZN3ArrIPiED2Ev(%struct.Arr* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !59 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !60 {
entry:
  ret void
}


define linkonce_odr dso_local void @_ZN7BaseArrIPsEC2EjbP3Mem(%struct.BaseArr*  nocapture "intel_dtrans_func_index"="1" %this, i32 %c, i1 %adopE, %struct.Mem* "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !61 {
entry:
  %0 = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTV7BaseArrIPsE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %0
  %flag = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 1
  store i8 1, i8* %flag
  %capacity = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 2
  store i32 %c, i32* %capacity
  %size = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 3
  store i32 0, i32* %size
  %base = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 4
  %mem3 = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 5
  store %struct.Mem* %mem, %struct.Mem** %mem3
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias i8* @malloc(i64 %mul)
  %1 = bitcast i8* %call to i16***
  store i16*** %1, i16**** %base
  tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsED2Ev(%struct.RefArr* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !62 {
entry:
  %0 = getelementptr inbounds %struct.RefArr, %struct.RefArr* %this, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTV6RefArrIPsE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %0
  %1 = getelementptr inbounds %struct.RefArr, %struct.RefArr* %this, i64 0, i32 0
  %flag = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %1, i64 0, i32 1
  %2 = load i8, i8* %flag
  %tobool = icmp eq i8 %2, 0
  br i1 %tobool, label %if.end, label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %size = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %1, i64 0, i32 3
  %3 = load i32, i32* %size
  %cmp6 = icmp eq i32 %3, 0
  br i1 %cmp6, label %if.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  %base = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %1, i64 0, i32 4
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %4 = load i16***, i16**** %base
  %arrayidx = getelementptr inbounds i16**, i16*** %4, i64 %indvars.iv
  %5 = bitcast i16*** %arrayidx to i8**
  %6 = load i8*, i8** %5
  tail call void @free(i8* %6)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %7 = load i32, i32* %size
  %8 = zext i32 %7 to i64
  %cmp = icmp ult i64 %indvars.iv.next, %8
  br i1 %cmp, label %for.body, label %if.end

if.end:                                           ; preds = %for.body, %for.cond.preheader, %entry
  %base2 = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %1, i64 0, i32 4
  %9 = bitcast i16**** %base2 to i8**
  %10 = load i8*, i8** %9
  tail call void @free(i8* %10)
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsED0Ev(%struct.RefArr* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !64 {
entry:
  tail call void @_ZN6RefArrIPsED2Ev(%struct.RefArr* %this)
  %0 = bitcast %struct.RefArr* %this to i8*
  tail call void @_ZdlPv(i8* %0)
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE3setEjPS0_(%struct.BaseArr* nocapture "intel_dtrans_func_index"="1" %this, i16** "intel_dtrans_func_index"="2" %val, i32 %i) !intel.dtrans.func.type !65 {
entry:
  %size = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 3
  %0 = load i32, i32* %size
  %cmp = icmp ugt i32 %0, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow() #12
  unreachable

if.end:                                           ; preds = %entry
  %flag = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 1
  %1 = load i8, i8* %flag, align 8
  %tobool = icmp eq i8 %1, 0
  br i1 %tobool, label %if.end3, label %if.then2

if.then2:                                         ; preds = %if.end
  %base = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 4
  %2 = load i16***, i16**** %base
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds i16**, i16*** %2, i64 %idxprom
  %3 = bitcast i16*** %arrayidx to i8**
  %4 = load i8*, i8** %3
  tail call void @free(i8* %4)
  br label %if.end3

if.end3:                                          ; preds = %if.end, %if.then2
  %base4 = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 4
  %5 = load i16***, i16**** %base4
  %idxprom5 = zext i32 %i to i64
  %arrayidx6 = getelementptr inbounds i16**, i16*** %5, i64 %idxprom5
  store i16** %val, i16*** %arrayidx6
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsED2Ev(%struct.BaseArr* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !67 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsED0Ev(%struct.BaseArr* nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !68 {
entry:
  tail call void @_ZN7BaseArrIPsED2Ev(%struct.BaseArr* %this)
  %0 = bitcast %struct.BaseArr* %this to i8*
  tail call void @_ZdlPv(i8* %0)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE6resizeEi(%struct.Arr* nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !70 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !71 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE6resizeEi(%struct.BaseArr* nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !72 {
entry:
  %size = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 3
  %0 = load i32, i32* %size
  %add = add i32 %0, 1
  %capacity = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 2
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
  %2 = bitcast i8* %call to i16***
  %cmp1345 = icmp eq i32 %0, 0
  br i1 %cmp1345, label %for.cond17.preheader, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %base = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 4
  %3 = load i16***, i16**** %base
  %wide.trip.count = zext i32 %0 to i64
  br label %for.body

for.cond17.preheader:                             ; preds = %for.body, %if.end
  %cmp1843 = icmp ult i32 %0, %spec.select
  br i1 %cmp1843, label %for.body19.preheader, label %for.end24

for.body19.preheader:                             ; preds = %for.cond17.preheader
  %4 = zext i32 %0 to i64
  %5 = shl nuw nsw i64 %4, 3
  %scevgep = getelementptr i8, i8* %call, i64 %5
  %6 = xor i32 %0, -1
  %7 = add i32 %spec.select, %6
  %8 = zext i32 %7 to i64
  %9 = shl nuw nsw i64 %8, 3
  %10 = add nuw nsw i64 %9, 8
  call void @llvm.memset.p0i8.i64(i8* nonnull align 8 dereferenceable(1) %scevgep, i8 0, i64 %10, i1 false)
  br label %for.end24

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i16**, i16*** %3, i64 %indvars.iv
  %11 = bitcast i16*** %arrayidx to i64*
  %12 = load i64, i64* %11
  %arrayidx15 = getelementptr inbounds i16**, i16*** %2, i64 %indvars.iv
  %13 = bitcast i16*** %arrayidx15 to i64*
  store i64 %12, i64* %13
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond17.preheader, label %for.body

for.end24:                                        ; preds = %for.body19.preheader, %for.cond17.preheader
  %base25 = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 4
  %14 = bitcast i16**** %base25 to i8**
  %15 = load i8*, i8** %14
  tail call void @free(i8* %15)
  store i16*** %2, i16**** %base25
  store i32 %spec.select, i32* %capacity
  br label %cleanup

cleanup:                                          ; preds = %entry, %for.end24
  ret void
}

declare !intel.dtrans.func.type !28 dso_local nonnull "intel_dtrans_func_index"="1" i8* @_Znwm(i64)
declare !intel.dtrans.func.type !39 dso_local void @_ZdlPv(i8* "intel_dtrans_func_index"="1")
declare dso_local void @__cxa_rethrow()
declare !intel.dtrans.func.type !69 dso_local void @free(i8* "intel_dtrans_func_index"="1") #1
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
declare !intel.dtrans.func.type !66 dso_local noalias "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !66 i1 @llvm.type.test(i8* "intel_dtrans_func_index"="1", metadata)
declare void @llvm.assume(i1)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!8, !13, !17, !20, !22, !24}

!0 = !{!"L", i32 1, !1}
!1 = !{!"A", i32 5, !2}
!2 = !{i8 0, i32 1}
!3 = !{!"L", i32 2, !2, !2}
!4 = !{!"L", i32 3, !2, !2, !2}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"uwtable", i32 1}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{!"S", %class.F zeroinitializer, i32 4, !9, !10, !11, !12}
!9 = !{%struct.Mem zeroinitializer, i32 1}
!10 = !{%struct.Arr zeroinitializer, i32 1}
!11 = !{%struct.Arr.0 zeroinitializer, i32 1}
!12 = !{%struct.RefArr zeroinitializer, i32 1}
!13 = !{!"S", %struct.Mem zeroinitializer, i32 1, !14}
!14 = !{!15, i32 2}
!15 = !{!"F", i1 true, i32 0, !16}
!16 = !{i32 0, i32 0}
!17 = !{!"S", %struct.Arr zeroinitializer, i32 5, !18, !16, !16, !19, !9}
!18 = !{i8 0, i32 0}
!19 = !{i32 0, i32 3}
!20 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !18, !16, !16, !21, !9}
!21 = !{float 0.000000e+00, i32 3}
!22 = !{!"S", %struct.RefArr zeroinitializer, i32 1, !23}
!23 = !{%struct.BaseArr zeroinitializer, i32 0}
!24 = !{!"S", %struct.BaseArr zeroinitializer, i32 6, !14, !18, !16, !16, !25, !9}
!25 = !{i16 0, i32 3}
!26 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!27 = !{%class.F zeroinitializer, i32 1}
!28 = distinct !{!2}
!29 = distinct !{!27}
!30 = !{i32 0, i32 2}
!31 = !{float 0.000000e+00, i32 2}
!32 = !{i16 0, i32 2}
!33 = !{!"F", i1 false, i32 3, !34, !35, !16, !32}
!34 = !{!"void", i32 0}
!35 = !{%struct.BaseArr zeroinitializer, i32 1}
!36 = distinct !{!36, !37}
!37 = !{!"llvm.loop.mustprogress"}
!38 = !{!"F", i1 false, i32 1, !34, !12}
!39 = distinct !{!2}
!40 = distinct !{!10, !9}
!41 = distinct !{!11, !9}
!42 = distinct !{!12, !9}
!43 = distinct !{!30, !10}
!44 = distinct !{!31, !11}
!45 = distinct !{!32, !35}
!46 = distinct !{!10, !30}
!47 = distinct !{!11, !31}
!48 = distinct !{!10, !30}
!49 = distinct !{!11, !31}
!50 = distinct !{!35, !32}
!51 = distinct !{!10}
!52 = distinct !{!11}
!53 = distinct !{!35}
!54 = distinct !{!10}
!55 = distinct !{!10, !10}
!56 = distinct !{!11, !11}
!57 = distinct !{!11}
!58 = distinct !{!35}
!59 = distinct !{!10}
!60 = distinct !{!11}
!61 = distinct !{!35, !9}
!62 = distinct !{!12}
!63 = distinct !{!63, !37}
!64 = distinct !{!12}
!65 = distinct !{!35, !32}
!66 = distinct !{!2}
!67 = distinct !{!35}
!68 = distinct !{!35}
!69 = distinct !{!2}
!70 = distinct !{!10}
!71 = distinct !{!11}
!72 = distinct !{!35}
!73 = distinct !{!73, !37}
!74 = distinct !{!74, !37}
!75 = distinct !{!27, !27}
