; This test verifies that SOAToAOSPrepare transformations are
; done correctly.

; RUN: opt < %s -dtrans-soatoaos-prepare  -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes=dtrans-soatoaos-prepare  -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

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
;   void set(unsigned i, S* val) {
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
;   virtual void set(unsigned i, S* val) {
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
;    f1->set(0, pi);
;    f2->set(0, pf);
;    f3->set(0, ps);
;    f1->add(nullptr);
;    f2->add(nullptr);
;    f3->add(nullptr);
;    f1->getSize();
;    f2->getSize();
;    f3->getSize();
;    unsigned ValS = f1->getCapacity();
;    Arr<int*>* f4 = new Arr<int *>(*f1);
;    Arr<float*>* f5 = new Arr<float *>(*f2);
;    RefArr<short*>* f6 = new RefArr<short *>(ValS, true, nullptr);
;    for (unsigned i = 0; i < ValS; i++) {
;      f6->add(f3->get(i));
;    }
;    f1->getCapacity();
;    f2->getCapacity();
;    f3->getCapacity();
;    delete f1;
;    delete f2;
;    delete f3;
;  }
;
; };
; int main() {
;  F *f = new F();
; }
;

; Make sure types are created correctly.
; CHECK: %_DPRE_class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %_DPRE__REP_struct.RefArr* }
; CHECK: %_DPRE__REP_struct.RefArr = type { i8, i32, i32, i16***, %struct.Mem* }

; Cloned struct method is called from main.
; CHECK: define i32 @main()
; CHECK: tail call void @_ZN1FC2Ev{{.*}}

; Make sure original member function is not modified.
; CHECK: define void @_ZN6RefArrIPsEC2EjbP3Mem(%struct.RefArr* nocapture %this
; CHECK: %0 = getelementptr inbounds %struct.RefArr, %struct.RefArr* %this, i64 0, i32 0
; CHECK: %1 = getelementptr inbounds %struct.RefArr, %struct.RefArr* %this, i64 0, i32 0, i32 0

; Make sure field accesses are not modified in original member function.
; CHECK: define i16** @_ZN7BaseArrIPsE3getEj(%struct.BaseArr* nocapture %this
; CHECK: %size = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 3


; Checking for cloned routines starts here.

; CHECK: define void @_ZN1FC2Ev{{.*}}(%_DPRE_class.F* %this)

; Check allocation size is updated
; CHECK:  %call5 = tail call i8* @_Znwm(i64 32)
; CHECK:  bitcast i8* %call5 to %_DPRE__REP_struct.RefArr*

; Check all member function calls are replaced with cloned calls.
; CHECK: call void @_ZN7BaseArrIPsEC2EjbP3Mem{{.*}}{{.*}}
; CHECK: call i16** @_ZN7BaseArrIPsE3getEj{{.*}}{{.*}}
; CHECK: call void @_ZN7BaseArrIPsE3setEjPS0_{{.*}}{{.*}}
; CHECK: call void @_ZN7BaseArrIPsE3addEPS0_{{.*}}{{.*}}
; CHECK: call i32 @_ZN7BaseArrIPsE7getSizeEv{{.*}}{{.*}}

; Make sure Ctor wrapper is not called here
; CHECK: call void @_ZN7BaseArrIPsEC2EjbP3Mem{{.*}}{{.*}}
; CHECK-NOT: call void @_ZN6RefArrIPsEC2EjbP3Mem{{.*}}{{.*}}

; CHECK: call i32 @_ZN7BaseArrIPsE11getCapacityEv{{.*}}{{.*}}
; CHECK: call void @_ZN7BaseArrIPsE3addEPS0_{{.*}}{{.*}}

; Make sure Dtor wrapper is not called.
; CHECK: call void @_ZN6RefArrIPsED2Ev{{.*}}{{.*}}
; CHECK-NOT: call void @_ZN6RefArrIPsED0Ev{{.*}}{{.*}}


; Make sure VFtable field is not accessed in Ctor.
; CHECK: define void @_ZN6RefArrIPsEC2EjbP3Mem{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr*
; CHECK-NEXT: entry:
; CHECK-NEXT: tail call void @_ZN7BaseArrIPsEC2EjbP3Mem{{.*}}{{.*}}
; CHECK-NEXT: ret void

; Make sure GEPs are fixed correctly in cloned routines.
; CHECK: define void @_ZN7BaseArrIPsEC2EjbP3Mem{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr*
; CHECK-NEXT: entry:
; CHECK-NEXT: %flag = getelementptr inbounds %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %this, i64 0, i32 0

; Make sure GEPs are fixed correctly in cloned routines.
; CHECK: define i16** @_ZN7BaseArrIPsE3getEj{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr*
; CHECK-NEXT: entry:
; CHECK-NEXT: %size = getelementptr inbounds %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %this, i64 0, i32 2

; Make sure GEPs are fixed correctly in cloned routines.
; CHECK: define void @_ZN7BaseArrIPsE3setEjPS0_{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr
; CHECK: %base = getelementptr inbounds %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %this, i64 0, i32 3

; Checking that original calls in cloned member functions are replaced
; with cloned calls.
; CHECK: define void @_ZN7BaseArrIPsE3addEPS0_{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr
; CHECK-NEXT: entry:
; CHECK-NEXT: tail call void @_ZN7BaseArrIPsE6resizeEi{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr* %this, i32 1)

; Make sure all member functions are cloned
; CHECK: define void @_ZN7BaseArrIPsE6resizeEi{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr*
; CHECK: define i32 @_ZN7BaseArrIPsE7getSizeEv{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr*

; CHECK: define i32 @_ZN7BaseArrIPsE11getCapacityEv{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr*
; CHECK-NEXT: entry:
; CHECK-NEXT: %capacity = getelementptr inbounds %_DPRE__REP_struct.RefArr, %_DPRE__REP_struct.RefArr* %this, i64 0, i32 1

; Make sure Dtor wrapper is cloned.
; CHECK: define void @_ZN6RefArrIPsED0Ev{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr*
; CHECK-NEXT: entry:
; CHECK-NEXT: tail call void @_ZN6RefArrIPsED2Ev{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr* %this)

; CHECK: define void @_ZN6RefArrIPsED2Ev{{.*}}{{.*}}(%_DPRE__REP_struct.RefArr*



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


@_ZTV6RefArrIPsE = linkonce_odr dso_local unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI6RefArrIPsE to i8*), i8* bitcast (void (%struct.RefArr*)* @_ZN6RefArrIPsED2Ev to i8*), i8* bitcast (void (%struct.RefArr*)* @_ZN6RefArrIPsED0Ev to i8*), i8* bitcast (void (%struct.BaseArr*, i32, i16**)* @_ZN7BaseArrIPsE3setEjPS0_ to i8*)] }, comdat, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTS6RefArrIPsE = linkonce_odr dso_local constant [12 x i8] c"6RefArrIPsE\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS7BaseArrIPsE = linkonce_odr dso_local constant [13 x i8] c"7BaseArrIPsE\00", comdat, align 1
@_ZTI6RefArrIPsE = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @_ZTS6RefArrIPsE, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI7BaseArrIPsE to i8*) }, comdat, align 8
@_ZTI7BaseArrIPsE = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([13 x i8], [13 x i8]* @_ZTS7BaseArrIPsE, i32 0, i32 0) }, comdat, align 8
@_ZTV7BaseArrIPsE = linkonce_odr dso_local unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTI7BaseArrIPsE to i8*), i8* bitcast (void (%struct.BaseArr*)* @_ZN7BaseArrIPsED2Ev to i8*), i8* bitcast (void (%struct.BaseArr*)* @_ZN7BaseArrIPsED0Ev to i8*), i8* bitcast (void (%struct.BaseArr*, i32, i16**)* @_ZN7BaseArrIPsE3setEjPS0_ to i8*)] }, comdat, align 8



define i32 @main() {
entry:
  %call = call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %class.F*
  tail call void @_ZN1FC2Ev(%class.F* %0)
  ret i32 0
}

define void @_ZN1FC2Ev(%class.F* %this) {
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
  tail call void @_ZN6RefArrIPsEC2EjbP3Mem(%struct.RefArr* nonnull %4, i32 10, i1 zeroext true, %struct.Mem* null)
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
  tail call void @_ZN3ArrIPiE3setEjPS0_(%struct.Arr* %10, i32 0, i32** %call9)
  %11 = load %struct.Arr.0*, %struct.Arr.0** %f2
  tail call void @_ZN3ArrIPfE3setEjPS0_(%struct.Arr.0* %11, i32 0, float** %call11)
  %12 = load %struct.BaseArr*, %struct.BaseArr** %8
  %13 = bitcast %struct.BaseArr* %12 to void (%struct.BaseArr*, i32, i16**)***
  %vtable = load void (%struct.BaseArr*, i32, i16**)**, void (%struct.BaseArr*, i32, i16**)*** %13
  %vfn = getelementptr inbounds void (%struct.BaseArr*, i32, i16**)*, void (%struct.BaseArr*, i32, i16**)** %vtable, i64 2
  %14 = load void (%struct.BaseArr*, i32, i16**)*, void (%struct.BaseArr*, i32, i16**)** %vfn
  tail call void @_ZN7BaseArrIPsE3setEjPS0_(%struct.BaseArr* %12, i32 0, i16** %call13)
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
  %call28 = tail call i8* @_Znwm(i64 32)
  %22 = bitcast i8* %call28 to %struct.Arr*
  %23 = load %struct.Arr*, %struct.Arr** %f1
  tail call void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* nonnull %22, %struct.Arr* dereferenceable(32) %23)
  %call32 = tail call i8* @_Znwm(i64 32)
  %24 = bitcast i8* %call32 to %struct.Arr.0*
  %25 = load %struct.Arr.0*, %struct.Arr.0** %f2
  tail call void @_ZN3ArrIPfEC2ERKS1_(%struct.Arr.0* nonnull %24, %struct.Arr.0* dereferenceable(32) %25)
  %call36 = tail call i8* @_Znwm(i64 40)
  %26 = bitcast i8* %call36 to %struct.RefArr*
  tail call void @_ZN6RefArrIPsEC2EjbP3Mem(%struct.RefArr* nonnull %26, i32 %call27, i1 zeroext true, %struct.Mem* null)
  br label %for.cond.preheader

for.cond.preheader:                               ; preds = %invoke.cont7
  %cmp84 = icmp eq i32 %call27, 0
  br i1 %cmp84, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  %27 = bitcast i8* %call36 to %struct.BaseArr*
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %for.cond.preheader
  %28 = load %struct.Arr*, %struct.Arr** %f1
  %call42 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* %28)
  %29 = load %struct.Arr.0*, %struct.Arr.0** %f2
  %call44 = tail call i32 @_ZN3ArrIPfE11getCapacityEv(%struct.Arr.0* %29)
  %30 = load %struct.BaseArr*, %struct.BaseArr** %8
  %call46 = tail call i32 @_ZN7BaseArrIPsE11getCapacityEv(%struct.BaseArr* %30)
  %31 = load %struct.Arr*, %struct.Arr** %f1
  %isnull = icmp eq %struct.Arr* %31, null
  br i1 %isnull, label %delete.end, label %delete.notnull

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %i.085 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %32 = load %struct.BaseArr*, %struct.BaseArr** %8
  %call40 = tail call i16** @_ZN7BaseArrIPsE3getEj(%struct.BaseArr* %32, i32 %i.085)
  tail call void @_ZN7BaseArrIPsE3addEPS0_(%struct.BaseArr* nonnull %27, i16** %call40)
  %inc = add nuw i32 %i.085, 1
  %exitcond = icmp eq i32 %inc, %call27
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

delete.notnull:                                   ; preds = %for.cond.cleanup
  tail call void @_ZN3ArrIPiED2Ev(%struct.Arr* nonnull %31)
  %33 = getelementptr inbounds %struct.Arr, %struct.Arr* %31, i64 0, i32 0
  tail call void @_ZdlPv(i8* %33)
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %for.cond.cleanup
  %34 = load %struct.Arr.0*, %struct.Arr.0** %f2
  %isnull49 = icmp eq %struct.Arr.0* %34, null
  br i1 %isnull49, label %delete.end51, label %delete.notnull50


delete.notnull50:                                 ; preds = %delete.end
  tail call void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nonnull %34)
  %35 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %34, i64 0, i32 0
  tail call void @_ZdlPv(i8* %35)
  br label %delete.end51

delete.end51:                                     ; preds = %delete.notnull50, %delete.end
  %36 = load %struct.RefArr*, %struct.RefArr** %f3
  %isnull53 = icmp eq %struct.RefArr* %36, null
  br i1 %isnull53, label %delete.end57, label %delete.notnull54

delete.notnull54:                                 ; preds = %delete.end51
  %37 = bitcast %struct.RefArr* %36 to void (%struct.RefArr*)***
  %vtable55 = load void (%struct.RefArr*)**, void (%struct.RefArr*)*** %37
  %vfn56 = getelementptr inbounds void (%struct.RefArr*)*, void (%struct.RefArr*)** %vtable55, i64 1
  %38 = load void (%struct.RefArr*)*, void (%struct.RefArr*)** %vfn56
  tail call void @_ZN6RefArrIPsED0Ev(%struct.RefArr* nonnull %36)
  br label %delete.end57

delete.end57:                                     ; preds = %delete.notnull54, %delete.end51
  ret void
}


define void @_ZN3ArrIPiEC2EjP3Mem(%struct.Arr*  nocapture %this, i32 %c, %struct.Mem* %mem) {
entry:
  ret void
}

define void @_ZN3ArrIPfEC2EjP3Mem(%struct.Arr.0*  nocapture %this, i32 %c, %struct.Mem* %mem) {
entry:
  ret void
}

define void @_ZN6RefArrIPsEC2EjbP3Mem(%struct.RefArr*  nocapture %this, i32 %c, i1 %adoptE, %struct.Mem* %mem) {
entry:
  %0 = getelementptr inbounds %struct.RefArr, %struct.RefArr* %this, i64 0, i32 0
  tail call void @_ZN7BaseArrIPsEC2EjbP3Mem(%struct.BaseArr* %0, i32 %c, i1 zeroext %adoptE, %struct.Mem* %mem)
  %1 = getelementptr inbounds %struct.RefArr, %struct.RefArr* %this, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTV6RefArrIPsE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %1
  ret void
}

define i32** @_ZN3ArrIPiE3getEj(%struct.Arr* nocapture %this, i32 %i) {
entry:
  ret i32** null
}

define float** @_ZN3ArrIPfE3getEj(%struct.Arr.0* nocapture %this, i32 %i) {
entry:
  ret float** null
}

define i16** @_ZN7BaseArrIPsE3getEj(%struct.BaseArr* nocapture %this, i32 %i) {
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

define void @_ZN3ArrIPiE3setEjPS0_(%struct.Arr* nocapture %this, i32 %i, i32** %val) {
entry:
  ret void
}

define void @_ZN3ArrIPfE3setEjPS0_(%struct.Arr.0* nocapture %this, i32 %i, float** %val) {
entry:
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* nocapture %this, i32** %val) {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(%struct.Arr* %this, i32 1)
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* nocapture %this, float** %val) {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* %this, i32 1)
  ret void
}

define void @_ZN7BaseArrIPsE3addEPS0_(%struct.BaseArr* nocapture %this, i16** %val) {
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

define i32 @_ZN3ArrIPiE7getSizeEv(%struct.Arr* nocapture %this) {
entry:
  ret i32 0
}

define i32 @_ZN3ArrIPfE7getSizeEv(%struct.Arr.0* nocapture %this) {
entry:
  ret i32 0
}

define i32 @_ZN7BaseArrIPsE7getSizeEv(%struct.BaseArr* nocapture %this) {
entry:
  %size = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 3
  %0 = load i32, i32* %size
  ret i32 %0
}

define i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* nocapture %this) {
entry:
  ret i32 0
}

define void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* nocapture %this, %struct.Arr* nocapture %A) {
entry:
  ret void
}

define void @_ZN3ArrIPfEC2ERKS1_(%struct.Arr.0* nocapture %this, %struct.Arr.0* nocapture %A) {
entry:
  ret void
}

define i32 @_ZN3ArrIPfE11getCapacityEv(%struct.Arr.0* nocapture %this) {
entry:
  ret i32 0
}

define i32 @_ZN7BaseArrIPsE11getCapacityEv(%struct.BaseArr* nocapture %this) {
entry:
  %capacity = getelementptr inbounds %struct.BaseArr, %struct.BaseArr* %this, i64 0, i32 2
  %0 = load i32, i32* %capacity
  ret i32 %0
}

define void @_ZN3ArrIPiED2Ev(%struct.Arr* nocapture %this) {
entry:
  ret void
}

define void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nocapture %this) {
entry:
  ret void
}


define void @_ZN7BaseArrIPsEC2EjbP3Mem(%struct.BaseArr*  nocapture %this, i32 %c, i1 %adopE, %struct.Mem* %mem) {
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

define void @_ZN6RefArrIPsED2Ev(%struct.RefArr* nocapture %this) {
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

define void @_ZN6RefArrIPsED0Ev(%struct.RefArr* nocapture %this) {
entry:
  tail call void @_ZN6RefArrIPsED2Ev(%struct.RefArr* %this)
  %0 = bitcast %struct.RefArr* %this to i8*
  tail call void @_ZdlPv(i8* %0)
  ret void
}

define void @_ZN7BaseArrIPsE3setEjPS0_(%struct.BaseArr* nocapture %this, i32 %i, i16** %val) {
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

define void @_ZN7BaseArrIPsED2Ev(%struct.BaseArr* nocapture %this) {
entry:
  ret void
}

define void @_ZN7BaseArrIPsED0Ev(%struct.BaseArr* nocapture %this) {
entry:
  tail call void @_ZN7BaseArrIPsED2Ev(%struct.BaseArr* %this)
  %0 = bitcast %struct.BaseArr* %this to i8*
  tail call void @_ZdlPv(i8* %0)
  ret void
}

define void @_ZN3ArrIPiE6resizeEi(%struct.Arr* nocapture %this, i32 %inc) {
entry:
  ret void
}

define void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* nocapture %this, i32 %inc) {
entry:
  ret void
}

define void @_ZN7BaseArrIPsE6resizeEi(%struct.BaseArr* nocapture %this, i32 %inc) {
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

declare dso_local noalias i8* @_Znwm(i64)
declare dso_local void @_ZdlPv(i8*)
declare dso_local void @__cxa_rethrow()
declare dso_local void @free(i8* nocapture)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
declare dso_local noalias i8* @malloc(i64)
