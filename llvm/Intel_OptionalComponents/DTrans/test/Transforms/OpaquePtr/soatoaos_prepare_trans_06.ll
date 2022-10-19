; This test verifies that SOAToAOSPrepare transformations related
; SetElem/AppendElem are done correctly.
; This test is almost same as soatoaos_prepare_trans_02.ll except
; 1. In _ZN1FC2ERKS_, %i18 is used in StoreInst, constructor and free
;    without bitcast. InvokeInst is used instead of CallInst for the
;    constructor call (i.e _ZN6RefArrIPsEC2EjbP3Mem). %bbb is the 
;    unwinddest of the invoke inst.
;
; 2. This test verifies that intel_dtrans_type is generated for alloca
;    in _ZN1FC2Ev.
;
; _ZN1FC2ERKS_: Verifies that Ctor (_ZN6RefArrIPsEC2EjbP3Mem) / AppendElem
; (_ZN7BaseArrIPsE3addEPS0_) calls are converted to CCtor/SimpleSetElem calls.
; Verifies that new CCtor and SimpleSetElem functions are created.
;
; _ZN1FC2Ev: Verifies that reverse argument promotion is done for
;  AppendElem (_ZN7BaseArrIPsE3addEPS0_).

; RUN: opt < %s -opaque-pointers -passes=dtrans-soatoaosop-prepare  -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

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

; Check IR for OP:
;
; Make sure types are created correctly.
; CHECK: %_DPRE_class.F = type { ptr, ptr, ptr, ptr }
; CHECK: %_DPRE__REP_struct.RefArr = type { i8, i32, i32, ptr, ptr }

; Cloned struct method is called from main.
; CHECK: define i32 @main()
; CHECK: tail call void @_ZN1FC2Ev
; CHECK: tail call void @_ZN1FC2ERKS_

; Make sure Ctor/AppendElem calls are replaced with CCtor/SimpleSetElem calls
; CHECK: define internal fastcc void @_ZN1FC2ERKS_(ptr "intel_dtrans_func_index"="1" %arg, ptr "intel_dtrans_func_index"="2" %arg1)
; CHECK: invoke void @_ZN7BaseArrIPsEC2EjbP3Mem{{.*}}{{.*}}
; CHECK: tail call fastcc void @_ZN7BaseArrIPsE3setEjPS0_{{.*}}
; CHECK: ret void

; Make sure argument of AppendElem is demoted.
; CHECK: define linkonce_odr dso_local void @_ZN1FC2Ev(
; CHECK: [[A0:%[0-9]*]] = alloca ptr, align 8, !intel_dtrans_type ![[M4:[0-9]+]]
; CHECK: tail call void @_ZN3ArrIPfE3addEPS0_
; CHECK: [[LD0:%i[0-9]*]] = load ptr, ptr %f3
; CHECK:   store ptr null, ptr %0
; CHECK: tail call void @_ZN7BaseArrIPsE3addEPS0_{{.*}}(ptr [[LD0]], ptr [[A0]])

; Make sure new simple SetElem function is created
; CHECK: define linkonce_odr dso_local void @_ZN7BaseArrIPsE3setEjPS0_.{{[0-9]+}}.{{[0-9]+}}(ptr nocapture "intel_dtrans_func_index"="1" %0, ptr "intel_dtrans_func_index"="2" %1, i32 %2) #{{[0-9]+}} !intel.dtrans.func.type ![[FMD1:[0-9]+]]
; CHECK:   %3 = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 3
; CHECK:  %4 = load ptr, ptr %3, align 8
; CHECK:  %5 = zext i32 %2 to i64
; CHECK:  %6 = getelementptr inbounds ptr, ptr %4, i64 %5
; CHECK:  store ptr %1, ptr %6, align 8
; CHECK:  ret void

; Make sure new simple CCtor function is created
; CHECK: define linkonce_odr dso_local void @_ZN7BaseArrIPsEC2EjbP3Mem.{{[0-9]+}}.{{[0-9]+}}.{{[0-9]+}}(ptr nocapture "intel_dtrans_func_index"="1" %0, ptr nocapture readonly "intel_dtrans_func_index"="2" %1) #{{[0-9]+}} !intel.dtrans.func.type ![[FMD2:[0-9]+]]
; CHECK:  %2 = getelementptr %_DPRE__REP_struct.RefArr, ptr %1, i64 0, i32 0
; CHECK:  %3 = load i8, ptr %2
; CHECK:  %flag = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 0
; CHECK:  store i8 %3, ptr %flag
; CHECK:  %4 = getelementptr %_DPRE__REP_struct.RefArr, ptr %1, i64 0, i32 1
; CHECK:  [[LD3:%[0-9]*]] = load i32, ptr %4
; CHECK:  %capacity = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 1
; CHECK:  store i32 %5, ptr %capacity
; CHECK:  [[G0:%[0-9]*]] = getelementptr %_DPRE__REP_struct.RefArr, ptr %1, i64 0, i32 2
; CHECK:  [[LD1:%[0-9]*]] = load i32, ptr [[G0]]
; CHECK:  %size = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 2
; CHECK:  store i32 [[LD1]], ptr %size
; CHECK:  %base = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 3
; CHECK:  [[G1:%[0-9]*]] = getelementptr %_DPRE__REP_struct.RefArr, ptr %1, i64 0, i32 4
; CHECK:  [[LD2:%[0-9]*]] = load ptr, ptr [[G1]]
; CHECK:  %mem3 = getelementptr inbounds %_DPRE__REP_struct.RefArr, ptr %0, i64 0, i32 4
; CHECK:  store ptr [[LD2]], ptr %mem3
; CHECK:  %conv = zext i32 [[LD3]] to i64
; CHECK:  %mul = shl nuw nsw i64 %conv, 3
; CHECK:  %call = tail call noalias ptr @malloc(i64 %mul)
; CHECK:  store ptr %call, ptr %base
; CHECK:  tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
; CHECK:  ret void

; CHECK: ![[S1:[0-9]+]] = !{%struct.Mem zeroinitializer, i32 1}
; CHECK: ![[S2:[0-9]+]] = !{%_DPRE__REP_struct.RefArr zeroinitializer, i32 1}
; CHECK: ![[M1:[0-9]+]] = !{i32 0, i32 0}
; CHECK: ![[M2:[0-9]+]] = !{i8 0, i32 0}
; CHECK: ![[M3:[0-9]+]] = !{i16 0, i32 3}
; CHECK: !{!"S", %_DPRE__REP_struct.RefArr zeroinitializer, i32 5, ![[M2]], ![[M1]], ![[M1]], ![[M3]], ![[S1]]}
; CHECK: ![[M4]] = !{i16 0, i32 2}
; CHECK: ![[FMD1]] = distinct !{![[S2]], ![[M4]]}
; CHECK: ![[FMD2]] = distinct !{![[S2]], ![[S2]]}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { ptr, ptr, ptr, ptr }
%struct.Arr = type { i8, i32, i32, ptr, ptr }
%struct.Arr.0 = type { i8, i32, i32, ptr, ptr }
%struct.RefArr = type { %struct.BaseArr }
%struct.BaseArr = type { ptr, i8, i32, i32, ptr, ptr }
%struct.Mem = type { ptr }

$_ZTV6RefArrIPsE = comdat any

$_ZTS6RefArrIPsE = comdat any

$_ZTS7BaseArrIPsE = comdat any

$_ZTI6RefArrIPsE = comdat any

$_ZTI7BaseArrIPsE = comdat any

$_ZTV7BaseArrIPsE = comdat any

@_ZTV6RefArrIPsE = linkonce_odr dso_local unnamed_addr constant { [5 x ptr] } { [5 x ptr] [ptr null, ptr @_ZTI6RefArrIPsE, ptr @_ZN6RefArrIPsED2Ev, ptr @_ZN6RefArrIPsED0Ev, ptr @_ZN7BaseArrIPsE3setEjPS0_] }, comdat, align 8, !intel_dtrans_type !0
@_ZTS6RefArrIPsE = linkonce_odr dso_local constant [12 x i8] c"6RefArrIPsE\00", comdat, align 1
@_ZTS7BaseArrIPsE = linkonce_odr dso_local constant [13 x i8] c"7BaseArrIPsE\00", comdat, align 1
@_ZTI6RefArrIPsE = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr null, i64 2), ptr @_ZTS6RefArrIPsE, ptr @_ZTI7BaseArrIPsE }, comdat, align 8, !intel_dtrans_type !3
@_ZTI7BaseArrIPsE = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr null, i64 2), ptr @_ZTS7BaseArrIPsE }, comdat, align 8, !intel_dtrans_type !4
@_ZTV7BaseArrIPsE = linkonce_odr dso_local unnamed_addr constant { [5 x ptr] } { [5 x ptr] [ptr null, ptr @_ZTI7BaseArrIPsE, ptr @_ZN7BaseArrIPsED2Ev, ptr @_ZN7BaseArrIPsED0Ev, ptr @_ZN7BaseArrIPsE3setEjPS0_] }, comdat, align 8, !intel_dtrans_type !0

define i32 @main() {
entry:
  %call = call ptr @_Znwm(i64 32)
  %i = bitcast ptr %call to ptr
  tail call void @_ZN1FC2Ev(ptr %i)
  %call1 = call ptr @_Znwm(i64 32)
  %i1 = bitcast ptr %call1 to ptr
  tail call void @_ZN1FC2ERKS_(ptr %i1, ptr %i)
  ret i32 0
}

define internal fastcc void @_ZN1FC2ERKS_(ptr "intel_dtrans_func_index"="1" %arg, ptr "intel_dtrans_func_index"="2" %arg1) personality ptr null !intel.dtrans.func.type !23 {
bb:
  %i = getelementptr inbounds %class.F, ptr %arg1, i64 0, i32 3
  %i2 = bitcast ptr %i to ptr
  %i3 = load ptr, ptr %i2, align 8
  %i4 = bitcast ptr %i to ptr
  %i5 = bitcast ptr %i to ptr
  %i6 = tail call fastcc i32 @_ZN7BaseArrIPsE7getSizeEv(ptr %i3)
  %i7 = tail call ptr @_Znwm(i64 32)
  %i9 = bitcast ptr %i7 to ptr
  %i10 = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 1
  %i11 = load ptr, ptr %i10, align 8
  tail call fastcc void @_ZN3ArrIPiEC2ERKS1_(ptr %i9, ptr %i11)
  %i12 = bitcast ptr %i7 to ptr
  store ptr %i9, ptr %i10, align 8
  %i13 = tail call ptr @_Znwm(i64 32)
  %i14 = bitcast ptr %i13 to ptr
  %i15 = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 2
  %i16 = load ptr, ptr %i15, align 8
  tail call fastcc void @_ZN3ArrIPfEC2ERKS1_(ptr %i14, ptr %i16)
  %i17 = bitcast ptr %i15 to ptr
  store ptr %i14, ptr %i15, align 8
  %i18 = tail call ptr @_Znwm(i64 40)
  %i20 = getelementptr inbounds %class.F, ptr %arg1, i64 0, i32 0
  %i21 = load ptr, ptr %i20, align 8
  %i22 = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 3
  %i23 = bitcast ptr %i22 to ptr
  invoke fastcc void @_ZN6RefArrIPsEC2EjbP3Mem(ptr %i18, i32 %i6, i1 zeroext true, ptr %i21)
          to label %ztt unwind label %bbb

ztt:                                              ; preds = %bb
  store ptr %i18, ptr %i22, align 8
  %i24 = icmp eq i32 %i6, 0
  br i1 %i24, label %bb32, label %pre

pre:                                              ; preds = %ztt
  br label %bb25

bb25:                                             ; preds = %bb25, %pre
  %i26 = phi i32 [ %i30, %bb25 ], [ 0, %pre ]
  %i27 = load ptr, ptr %i23, align 8
  %i28 = load ptr, ptr %i2, align 8
  %i29 = tail call fastcc ptr @_ZN7BaseArrIPsE3getEj(ptr %i28, i32 %i26)
  tail call fastcc void @_ZN7BaseArrIPsE3addEPS0_(ptr %i27, ptr %i29)
  %i30 = add nuw i32 %i26, 1
  %i31 = icmp eq i32 %i30, %i6
  br i1 %i31, label %bb32, label %bb25

bbb:                                              ; preds = %bb
  %i8 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZdlPv(ptr noundef %i18)
  unreachable

bb32:                                             ; preds = %bb25, %ztt
  ret void
}

define linkonce_odr dso_local void @_ZN1FC2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !25 {
entry:
  %call = tail call ptr @_Znwm(i64 32)
  %i = bitcast ptr %call to ptr
  tail call void @_ZN3ArrIPiEC2EjP3Mem(ptr %i, i32 10, ptr null)
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  %i1 = bitcast ptr %f1 to ptr
  store ptr %i, ptr %f1, align 8
  %call2 = tail call ptr @_Znwm(i64 32)
  %i2 = bitcast ptr %call2 to ptr
  tail call void @_ZN3ArrIPfEC2EjP3Mem(ptr nonnull %i2, i32 10, ptr null)
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 2
  %i3 = bitcast ptr %f2 to ptr
  store ptr %i2, ptr %f2, align 8
  %call5 = tail call ptr @_Znwm(i64 40)
  %i4 = bitcast ptr %call5 to ptr
  %g1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 0
  %ld1 = load ptr, ptr %g1, align 8
  tail call void @_ZN6RefArrIPsEC2EjbP3Mem(ptr nonnull %i4, i32 10, i1 zeroext true, ptr %ld1)
  br label %invoke.cont7

invoke.cont7:                                     ; preds = %entry
  %f3 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 3
  %i5 = bitcast ptr %f3 to ptr
  store ptr %i4, ptr %f3, align 8
  %i6 = load ptr, ptr %f1, align 8
  %call9 = tail call ptr @_ZN3ArrIPiE3getEj(ptr %i6, i32 1)
  %i7 = load ptr, ptr %f2, align 8
  %call11 = tail call ptr @_ZN3ArrIPfE3getEj(ptr %i7, i32 1)
  %i8 = bitcast ptr %f3 to ptr
  %i9 = load ptr, ptr %i8, align 8
  %call13 = tail call ptr @_ZN7BaseArrIPsE3getEj(ptr %i9, i32 1)
  %i10 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiE3setEjPS0_(ptr %i10, ptr %call9, i32 0)
  %i11 = load ptr, ptr %f2, align 8
  tail call void @_ZN3ArrIPfE3setEjPS0_(ptr %i11, ptr %call11, i32 0)
  %i12 = load ptr, ptr %i8, align 8
  %i13 = bitcast ptr %i12 to ptr
  %vtable = load ptr, ptr %i13, align 8
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %i14 = load ptr, ptr %vfn, align 8
  tail call void @_ZN7BaseArrIPsE3setEjPS0_(ptr %i12, ptr %call13, i32 0)
  %i15 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiE3addEPS0_(ptr %i15, ptr null)
  %i16 = load ptr, ptr %f2, align 8
  tail call void @_ZN3ArrIPfE3addEPS0_(ptr %i16, ptr null)
  %i17 = load ptr, ptr %i8, align 8
  tail call void @_ZN7BaseArrIPsE3addEPS0_(ptr %i17, ptr null)
  %i18 = load ptr, ptr %f1, align 8
  %call21 = tail call i32 @_ZN3ArrIPiE7getSizeEv(ptr %i18)
  %i19 = load ptr, ptr %f2, align 8
  %call23 = tail call i32 @_ZN3ArrIPfE7getSizeEv(ptr %i19)
  %i20 = load ptr, ptr %i8, align 8
  %call25 = tail call i32 @_ZN7BaseArrIPsE7getSizeEv(ptr %i20)
  %i21 = load ptr, ptr %f1, align 8
  %call27 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(ptr %i21)
  %i22 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiED2Ev(ptr nonnull %i22)
  %i23 = getelementptr inbounds %struct.Arr, ptr %i22, i64 0, i32 0
  tail call void @_ZdlPv(ptr %i23)
  br label %delete.end

delete.end:                                       ; preds = %invoke.cont7
  %i24 = load ptr, ptr %f2, align 8
  %isnull49 = icmp eq ptr %i24, null
  br i1 %isnull49, label %delete.end51, label %delete.notnull50

delete.notnull50:                                 ; preds = %delete.end
  tail call void @_ZN3ArrIPfED2Ev(ptr nonnull %i24)
  %i25 = getelementptr inbounds %struct.Arr.0, ptr %i24, i64 0, i32 0
  tail call void @_ZdlPv(ptr %i25)
  br label %delete.end51

delete.end51:                                     ; preds = %delete.notnull50, %delete.end
  %i26 = load ptr, ptr %f3, align 8
  %isnull53 = icmp eq ptr %i26, null
  br i1 %isnull53, label %delete.end57, label %delete.notnull54

delete.notnull54:                                 ; preds = %delete.end51
  %i27 = bitcast ptr %i26 to ptr
  %vtable55 = load ptr, ptr %i27, align 8
  %vfn56 = getelementptr inbounds ptr, ptr %vtable55, i64 1
  %i28 = load ptr, ptr %vfn56, align 8
  tail call void @_ZN6RefArrIPsED0Ev(ptr nonnull %i26)
  br label %delete.end57

delete.end57:                                     ; preds = %delete.notnull54, %delete.end51
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2EjP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !26 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2EjP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !27 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsEC2EjbP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, i1 %adoptE, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !28 {
entry:
  %i = getelementptr inbounds %struct.RefArr, ptr %this, i64 0, i32 0
  tail call void @_ZN7BaseArrIPsEC2EjbP3Mem(ptr %i, i32 %c, i1 zeroext %adoptE, ptr %mem)
  %i1 = getelementptr inbounds %struct.RefArr, ptr %this, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTV6RefArrIPsE, i64 0, inrange i32 0, i64 2), ptr %i1, align 8
  ret void
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPiE3getEj(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !29 {
entry:
  ret ptr null
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPfE3getEj(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !31 {
entry:
  ret ptr null
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN7BaseArrIPsE3getEj(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !33 {
entry:
  %size = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 3
  %i1 = load i32, ptr %size, align 4
  %cmp = icmp ugt i32 %i1, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %base = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 4
  %i2 = load ptr, ptr %base, align 8
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i2, i64 %idxprom
  %i3 = load ptr, ptr %arrayidx, align 8
  ret ptr %i3
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3setEjPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val, i32 %i) !intel.dtrans.func.type !36 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3setEjPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val, i32 %i) !intel.dtrans.func.type !37 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !38 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(ptr %this, i32 1)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !39 {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(ptr %this, i32 1)
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !40 {
entry:
  tail call void @_ZN7BaseArrIPsE6resizeEi(ptr %this, i32 1)
  %base = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 4
  %i = load ptr, ptr %base, align 8
  %size = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 3
  %i1 = load i32, ptr %size, align 4
  %idxprom = zext i32 %i1 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i, i64 %idxprom
  store ptr %val, ptr %arrayidx, align 8
  %inc = add i32 %i1, 1
  store i32 %inc, ptr %size, align 4
  ret void
}

define linkonce_odr dso_local i32 @_ZN3ArrIPiE7getSizeEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !41 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN3ArrIPfE7getSizeEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !42 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN7BaseArrIPsE7getSizeEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !43 {
entry:
  %size = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 3
  %i = load i32, ptr %size, align 4
  ret i32 %i
}

define linkonce_odr dso_local i32 @_ZN3ArrIPiE11getCapacityEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !44 {
entry:
  ret i32 0
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2ERKS1_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !45 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2ERKS1_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !46 {
entry:
  ret void
}

define linkonce_odr dso_local i32 @_ZN3ArrIPfE11getCapacityEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !47 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN7BaseArrIPsE11getCapacityEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !48 {
entry:
  %capacity = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 2
  %i = load i32, ptr %capacity, align 4
  ret i32 %i
}

define linkonce_odr dso_local void @_ZN3ArrIPiED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !49 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !50 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsEC2EjbP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, i1 %adopE, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !51 {
entry:
  %i = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 0
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTV7BaseArrIPsE, i64 0, inrange i32 0, i64 2), ptr %i, align 8
  %flag = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 1
  store i8 1, ptr %flag, align 1
  %capacity = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 2
  store i32 %c, ptr %capacity, align 4
  %size = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 3
  store i32 0, ptr %size, align 4
  %base = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 4
  %mem3 = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 5
  store ptr %mem, ptr %mem3, align 8
  %conv = zext i32 %c to i64
  %mul = shl nuw nsw i64 %conv, 3
  %call = tail call noalias ptr @malloc(i64 %mul)
  %i1 = bitcast ptr %call to ptr
  store ptr %i1, ptr %base, align 8
  tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !52 {
entry:
  %i = getelementptr inbounds %struct.RefArr, ptr %this, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTV6RefArrIPsE, i64 0, inrange i32 0, i64 2), ptr %i, align 8
  %i1 = getelementptr inbounds %struct.RefArr, ptr %this, i64 0, i32 0
  %flag = getelementptr inbounds %struct.BaseArr, ptr %i1, i64 0, i32 1
  %i2 = load i8, ptr %flag, align 1
  %tobool = icmp eq i8 %i2, 0
  br i1 %tobool, label %if.end, label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %size = getelementptr inbounds %struct.BaseArr, ptr %i1, i64 0, i32 3
  %i3 = load i32, ptr %size, align 4
  %cmp6 = icmp eq i32 %i3, 0
  br i1 %cmp6, label %if.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  %base = getelementptr inbounds %struct.BaseArr, ptr %i1, i64 0, i32 4
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %i4 = load ptr, ptr %base, align 8
  %arrayidx = getelementptr inbounds ptr, ptr %i4, i64 %indvars.iv
  %i5 = bitcast ptr %arrayidx to ptr
  %i6 = load ptr, ptr %i5, align 8
  tail call void @free(ptr %i6)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %i7 = load i32, ptr %size, align 4
  %i8 = zext i32 %i7 to i64
  %cmp = icmp ult i64 %indvars.iv.next, %i8
  br i1 %cmp, label %for.body, label %if.end

if.end:                                           ; preds = %for.body, %for.cond.preheader, %entry
  %base2 = getelementptr inbounds %struct.BaseArr, ptr %i1, i64 0, i32 4
  %i9 = bitcast ptr %base2 to ptr
  %i10 = load ptr, ptr %i9, align 8
  tail call void @free(ptr %i10)
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsED0Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !53 {
entry:
  tail call void @_ZN6RefArrIPsED2Ev(ptr %this)
  %i = bitcast ptr %this to ptr
  tail call void @_ZdlPv(ptr %i)
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE3setEjPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val, i32 %i) !intel.dtrans.func.type !54 {
entry:
  %size = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 3
  %i1 = load i32, ptr %size, align 4
  %cmp = icmp ugt i32 %i1, %i
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @__cxa_rethrow()
  unreachable

if.end:                                           ; preds = %entry
  %flag = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 1
  %i2 = load i8, ptr %flag, align 8
  %tobool = icmp eq i8 %i2, 0
  br i1 %tobool, label %if.end3, label %if.then2

if.then2:                                         ; preds = %if.end
  %base = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 4
  %i3 = load ptr, ptr %base, align 8
  %idxprom = zext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i3, i64 %idxprom
  %i4 = bitcast ptr %arrayidx to ptr
  %i5 = load ptr, ptr %i4, align 8
  tail call void @free(ptr %i5)
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  %base4 = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 4
  %i6 = load ptr, ptr %base4, align 8
  %idxprom5 = zext i32 %i to i64
  %arrayidx6 = getelementptr inbounds ptr, ptr %i6, i64 %idxprom5
  store ptr %val, ptr %arrayidx6, align 8
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !55 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsED0Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !56 {
entry:
  tail call void @_ZN7BaseArrIPsED2Ev(ptr %this)
  %i = bitcast ptr %this to ptr
  tail call void @_ZdlPv(ptr %i)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !57 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !58 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !59 {
entry:
  %size = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 3
  %i = load i32, ptr %size, align 4
  %add = add i32 %i, 1
  %capacity = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 2
  %i1 = load i32, ptr %capacity, align 4
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
  %i2 = bitcast ptr %call to ptr
  %cmp1345 = icmp eq i32 %i, 0
  br i1 %cmp1345, label %for.cond17.preheader, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %base = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 4
  %i3 = load ptr, ptr %base, align 8
  %wide.trip.count = zext i32 %i to i64
  br label %for.body

for.cond17.preheader:                             ; preds = %for.body, %if.end
  %cmp1843 = icmp ult i32 %i, %spec.select
  br i1 %cmp1843, label %for.body19.preheader, label %for.end24

for.body19.preheader:                             ; preds = %for.cond17.preheader
  %i4 = zext i32 %i to i64
  %i5 = shl nuw nsw i64 %i4, 3
  %scevgep = getelementptr i8, ptr %call, i64 %i5
  %i6 = xor i32 %i, -1
  %i7 = add i32 %spec.select, %i6
  %i8 = zext i32 %i7 to i64
  %i9 = shl nuw nsw i64 %i8, 3
  %i10 = add nuw nsw i64 %i9, 8
  call void @llvm.memset.p0.i64(ptr nonnull align 8 dereferenceable(1) %scevgep, i8 0, i64 %i10, i1 false)
  br label %for.end24

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %i3, i64 %indvars.iv
  %i11 = bitcast ptr %arrayidx to ptr
  %i12 = load i64, ptr %i11, align 8
  %arrayidx15 = getelementptr inbounds ptr, ptr %i2, i64 %indvars.iv
  %i13 = bitcast ptr %arrayidx15 to ptr
  store i64 %i12, ptr %i13, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond17.preheader, label %for.body

for.end24:                                        ; preds = %for.body19.preheader, %for.cond17.preheader
  %base25 = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 4
  %i14 = bitcast ptr %base25 to ptr
  %i15 = load ptr, ptr %i14, align 8
  tail call void @free(ptr %i15)
  store ptr %i2, ptr %base25, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

cleanup:                                          ; preds = %for.end24, %entry
  ret void
}

declare !intel.dtrans.func.type !60 dso_local nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

declare !intel.dtrans.func.type !61 dso_local void @_ZdlPv(ptr "intel_dtrans_func_index"="1")

declare dso_local void @__cxa_rethrow()

; Function Attrs: allockind("free")
declare !intel.dtrans.func.type !62 dso_local void @free(ptr "intel_dtrans_func_index"="1") #0

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !63 dso_local noalias "intel_dtrans_func_index"="1" ptr @malloc(i64) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare !intel.dtrans.func.type !63 i1 @llvm.type.test(ptr, metadata) #2

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #3

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #4

attributes #0 = { allockind("free") "alloc-family"="malloc" }
attributes #1 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #4 = { argmemonly nocallback nofree nounwind willreturn writeonly }

!intel.dtrans.types = !{!5, !10, !14, !17, !19, !21}

!0 = !{!"L", i32 1, !1}
!1 = !{!"A", i32 5, !2}
!2 = !{i8 0, i32 1}
!3 = !{!"L", i32 3, !2, !2, !2}
!4 = !{!"L", i32 2, !2, !2}
!5 = !{!"S", %class.F zeroinitializer, i32 4, !6, !7, !8, !9}
!6 = !{%struct.Mem zeroinitializer, i32 1}
!7 = !{%struct.Arr zeroinitializer, i32 1}
!8 = !{%struct.Arr.0 zeroinitializer, i32 1}
!9 = !{%struct.RefArr zeroinitializer, i32 1}
!10 = !{!"S", %struct.Mem zeroinitializer, i32 1, !11}
!11 = !{!12, i32 2}
!12 = !{!"F", i1 true, i32 0, !13}
!13 = !{i32 0, i32 0}
!14 = !{!"S", %struct.Arr zeroinitializer, i32 5, !15, !13, !13, !16, !6}
!15 = !{i8 0, i32 0}
!16 = !{i32 0, i32 3}
!17 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !15, !13, !13, !18, !6}
!18 = !{float 0.000000e+00, i32 3}
!19 = !{!"S", %struct.RefArr zeroinitializer, i32 1, !20}
!20 = !{%struct.BaseArr zeroinitializer, i32 0}
!21 = !{!"S", %struct.BaseArr zeroinitializer, i32 6, !11, !15, !13, !13, !22, !6}
!22 = !{i16 0, i32 3}
!23 = distinct !{!24, !24}
!24 = !{%class.F zeroinitializer, i32 1}
!25 = distinct !{!24}
!26 = distinct !{!7, !6}
!27 = distinct !{!8, !6}
!28 = distinct !{!9, !6}
!29 = distinct !{!30, !7}
!30 = !{i32 0, i32 2}
!31 = distinct !{!32, !8}
!32 = !{float 0.000000e+00, i32 2}
!33 = distinct !{!34, !35}
!34 = !{i16 0, i32 2}
!35 = !{%struct.BaseArr zeroinitializer, i32 1}
!36 = distinct !{!7, !30}
!37 = distinct !{!8, !32}
!38 = distinct !{!7, !30}
!39 = distinct !{!8, !32}
!40 = distinct !{!35, !34}
!41 = distinct !{!7}
!42 = distinct !{!8}
!43 = distinct !{!35}
!44 = distinct !{!7}
!45 = distinct !{!7, !7}
!46 = distinct !{!8, !8}
!47 = distinct !{!8}
!48 = distinct !{!35}
!49 = distinct !{!7}
!50 = distinct !{!8}
!51 = distinct !{!35, !6}
!52 = distinct !{!9}
!53 = distinct !{!9}
!54 = distinct !{!35, !34}
!55 = distinct !{!35}
!56 = distinct !{!35}
!57 = distinct !{!7}
!58 = distinct !{!8}
!59 = distinct !{!35}
!60 = distinct !{!2}
!61 = distinct !{!2}
!62 = distinct !{!2}
!63 = distinct !{!2}
