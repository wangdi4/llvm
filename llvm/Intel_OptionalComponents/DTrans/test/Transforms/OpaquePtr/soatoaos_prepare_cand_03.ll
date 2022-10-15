; This test verifies that SOAToAOSPrepare transformation is able
; to detect possible candidate. It also verifies that functionality
; of member functions are analyzed to prove that the candidate is
; vector class.

; This is same as soatoaos_prepare_cand_01.ll except this test supports
; only opaque pointers and all bitcasts are removed.

; RUN: opt < %s -opaque-pointers -dtrans-soatoaosop-prepare  -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop-prepare -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes=dtrans-soatoaosop-prepare  -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop-prepare -disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts

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

; CHECK: SOAToAOSPrepare: Candidate selected for more analysis
; CHECK:   Candidate struct: class.F    FieldOff: 3
; CHECK:   Candidate Passed Analysis.
; CHECK:    Candidate: class.F
; CHECK:    Member functions for 3 candidate field:

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

define dso_local i32 @main() {
entry:
  %call = call ptr @_Znwm(i64 32)
  tail call void @_ZN1FC2Ev(ptr %call)
  ret i32 0
}

define linkonce_odr dso_local void @_ZN1FC2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !23 {
entry:
  %call = tail call ptr @_Znwm(i64 32)
  tail call void @_ZN3ArrIPiEC2EjP3Mem(ptr %call, i32 10, ptr null)
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  store ptr %call, ptr %f1, align 8
  %call2 = tail call ptr @_Znwm(i64 32)
  tail call void @_ZN3ArrIPfEC2EjP3Mem(ptr nonnull %call2, i32 10, ptr null)
  %f2 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 2
  store ptr %call2, ptr %f2, align 8
  %call5 = tail call ptr @_Znwm(i64 40)
  tail call void @_ZN6RefArrIPsEC2EjbP3Mem(ptr nonnull %call5, i32 10, i1 zeroext true, ptr null)
  br label %invoke.cont7

invoke.cont7:                                     ; preds = %entry
  %f3 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 3
  store ptr %call5, ptr %f3, align 8
  %i6 = load ptr, ptr %f1, align 8
  %call9 = tail call ptr @_ZN3ArrIPiE3getEj(ptr %i6, i32 1)
  %i7 = load ptr, ptr %f2, align 8
  %call11 = tail call ptr @_ZN3ArrIPfE3getEj(ptr %i7, i32 1)
  %i9 = load ptr, ptr %f3, align 8
  %call13 = tail call ptr @_ZN7BaseArrIPsE3getEj(ptr %i9, i32 1)
  %i10 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiE3setEjPS0_(ptr %i10, i32 0, ptr %call9)
  %i11 = load ptr, ptr %f2, align 8
  tail call void @_ZN3ArrIPfE3setEjPS0_(ptr %i11, i32 0, ptr %call11)
  %i12 = load ptr, ptr %f3, align 8
  %vtable = load ptr, ptr %i12, align 8
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %i14 = load ptr, ptr %vfn, align 8
  tail call void @_ZN7BaseArrIPsE3setEjPS0_(ptr %i12, i32 0, ptr %call13)
  %i15 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiE3addEPS0_(ptr %i15, ptr null)
  %i16 = load ptr, ptr %f2, align 8
  tail call void @_ZN3ArrIPfE3addEPS0_(ptr %i16, ptr null)
  %i17 = load ptr, ptr %f3, align 8
  tail call void @_ZN7BaseArrIPsE3addEPS0_(ptr %i17, ptr null)
  %i18 = load ptr, ptr %f1, align 8
  %call21 = tail call i32 @_ZN3ArrIPiE7getSizeEv(ptr %i18)
  %i19 = load ptr, ptr %f2, align 8
  %call23 = tail call i32 @_ZN3ArrIPfE7getSizeEv(ptr %i19)
  %i20 = load ptr, ptr %f3, align 8
  %call25 = tail call i32 @_ZN7BaseArrIPsE7getSizeEv(ptr %i20)
  %i21 = load ptr, ptr %f1, align 8
  %call27 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(ptr %i21)
  %call28 = tail call ptr @_Znwm(i64 32)
  %i23 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiEC2ERKS1_(ptr nonnull %call28, ptr dereferenceable(32) %i23)
  %call32 = tail call ptr @_Znwm(i64 32)
  %i25 = load ptr, ptr %f2, align 8
  tail call void @_ZN3ArrIPfEC2ERKS1_(ptr nonnull %call32, ptr dereferenceable(32) %i25)
  %call36 = tail call ptr @_Znwm(i64 40)
  tail call void @_ZN6RefArrIPsEC2EjbP3Mem(ptr nonnull %call36, i32 %call27, i1 zeroext true, ptr null)
  br label %for.cond.preheader

for.cond.preheader:                               ; preds = %invoke.cont7
  %cmp84 = icmp eq i32 %call27, 0
  br i1 %cmp84, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %for.cond.preheader
  %i28 = load ptr, ptr %f1, align 8
  %call42 = tail call i32 @_ZN3ArrIPiE11getCapacityEv(ptr %i28)
  %i29 = load ptr, ptr %f2, align 8
  %call44 = tail call i32 @_ZN3ArrIPfE11getCapacityEv(ptr %i29)
  %i30 = load ptr, ptr %f3, align 8
  %call46 = tail call i32 @_ZN7BaseArrIPsE11getCapacityEv(ptr %i30)
  %i31 = load ptr, ptr %f1, align 8
  %isnull = icmp eq ptr %i31, null
  br i1 %isnull, label %delete.end, label %delete.notnull

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %i.085 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %i32 = load ptr, ptr %f3, align 8
  %call40 = tail call ptr @_ZN7BaseArrIPsE3getEj(ptr %i32, i32 %i.085)
  tail call void @_ZN7BaseArrIPsE3addEPS0_(ptr nonnull %call36, ptr %call40)
  %inc = add nuw i32 %i.085, 1
  %exitcond = icmp eq i32 %inc, %call27
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

delete.notnull:                                   ; preds = %for.cond.cleanup
  tail call void @_ZN3ArrIPiED2Ev(ptr nonnull %i31)
  %i33 = getelementptr inbounds %struct.Arr, ptr %i31, i64 0, i32 0
  tail call void @_ZdlPv(ptr %i33)
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %for.cond.cleanup
  %i34 = load ptr, ptr %f2, align 8
  %isnull49 = icmp eq ptr %i34, null
  br i1 %isnull49, label %delete.end51, label %delete.notnull50

delete.notnull50:                                 ; preds = %delete.end
  tail call void @_ZN3ArrIPfED2Ev(ptr nonnull %i34)
  %i35 = getelementptr inbounds %struct.Arr.0, ptr %i34, i64 0, i32 0
  tail call void @_ZdlPv(ptr %i35)
  br label %delete.end51

delete.end51:                                     ; preds = %delete.notnull50, %delete.end
  %i36 = load ptr, ptr %f3, align 8
  %isnull53 = icmp eq ptr %i36, null
  br i1 %isnull53, label %delete.end57, label %delete.notnull54

delete.notnull54:                                 ; preds = %delete.end51
  %vtable55 = load ptr, ptr %i36, align 8
  %vfn56 = getelementptr inbounds ptr, ptr %vtable55, i64 1
  %i38 = load ptr, ptr %vfn56, align 8
  tail call void @_ZN6RefArrIPsED0Ev(ptr nonnull %i36)
  br label %delete.end57

delete.end57:                                     ; preds = %delete.notnull54, %delete.end51
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2EjP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !25 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2EjP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !26 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsEC2EjbP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, i1 %adoptE, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !27 {
entry:
  %i = getelementptr inbounds %struct.RefArr, ptr %this, i64 0, i32 0
  tail call void @_ZN7BaseArrIPsEC2EjbP3Mem(ptr %i, i32 %c, i1 zeroext %adoptE, ptr %mem)
  %i1 = getelementptr inbounds %struct.RefArr, ptr %this, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTV6RefArrIPsE, i64 0, inrange i32 0, i64 2), ptr %i1, align 8
  ret void
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPiE3getEj(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !28 {
entry:
  ret ptr null
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPfE3getEj(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !30 {
entry:
  ret ptr null
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN7BaseArrIPsE3getEj(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !32 {
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

define linkonce_odr dso_local void @_ZN3ArrIPiE3setEjPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !35 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3setEjPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !36 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !37 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(ptr %this, i32 1)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !38 {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(ptr %this, i32 1)
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !39 {
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

define linkonce_odr dso_local i32 @_ZN3ArrIPiE7getSizeEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !40 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN3ArrIPfE7getSizeEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !41 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN7BaseArrIPsE7getSizeEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !42 {
entry:
  %size = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 3
  %i = load i32, ptr %size, align 4
  ret i32 %i
}

define linkonce_odr dso_local i32 @_ZN3ArrIPiE11getCapacityEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !43 {
entry:
  ret i32 0
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2ERKS1_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !44 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2ERKS1_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !45 {
entry:
  ret void
}

define linkonce_odr dso_local i32 @_ZN3ArrIPfE11getCapacityEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !46 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN7BaseArrIPsE11getCapacityEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !47 {
entry:
  %capacity = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 2
  %i = load i32, ptr %capacity, align 4
  ret i32 %i
}

define linkonce_odr dso_local void @_ZN3ArrIPiED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !48 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !49 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsEC2EjbP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, i1 %adopE, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !50 {
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
  store ptr %call, ptr %base, align 8
  tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !51 {
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
  %i6 = load ptr, ptr %arrayidx, align 8
  tail call void @free(ptr %i6)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %i7 = load i32, ptr %size, align 4
  %i8 = zext i32 %i7 to i64
  %cmp = icmp ult i64 %indvars.iv.next, %i8
  br i1 %cmp, label %for.body, label %if.end

if.end:                                           ; preds = %for.body, %for.cond.preheader, %entry
  %base2 = getelementptr inbounds %struct.BaseArr, ptr %i1, i64 0, i32 4
  %i10 = load ptr, ptr %base2, align 8
  tail call void @free(ptr %i10)
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsED0Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !52 {
entry:
  tail call void @_ZN6RefArrIPsED2Ev(ptr %this)
  tail call void @_ZdlPv(ptr %this)
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE3setEjPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !53 {
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
  %i5 = load ptr, ptr %arrayidx, align 8
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

define linkonce_odr dso_local void @_ZN7BaseArrIPsED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !54 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsED0Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !55 {
entry:
  tail call void @_ZN7BaseArrIPsED2Ev(ptr %this)
  tail call void @_ZdlPv(ptr %this)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !56 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !57 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !58 {
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
  %i12 = load i64, ptr %arrayidx, align 8
  %arrayidx15 = getelementptr inbounds ptr, ptr %call, i64 %indvars.iv
  store i64 %i12, ptr %arrayidx15, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond17.preheader, label %for.body

for.end24:                                        ; preds = %for.body19.preheader, %for.cond17.preheader
  %base25 = getelementptr inbounds %struct.BaseArr, ptr %this, i64 0, i32 4
  %i15 = load ptr, ptr %base25, align 8
  tail call void @free(ptr %i15)
  store ptr %call, ptr %base25, align 8
  store i32 %spec.select, ptr %capacity, align 4
  br label %cleanup

cleanup:                                          ; preds = %for.end24, %entry
  ret void
}

declare !intel.dtrans.func.type !59 dso_local nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

declare !intel.dtrans.func.type !60 dso_local void @_ZdlPv(ptr "intel_dtrans_func_index"="1")

declare dso_local void @__cxa_rethrow()

; Function Attrs: allockind("free")
declare !intel.dtrans.func.type !61 dso_local void @free(ptr "intel_dtrans_func_index"="1") #0

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !62 dso_local noalias "intel_dtrans_func_index"="1" ptr @malloc(i64) #1

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

attributes #0 = { allockind("free") "alloc-family"="malloc" }
attributes #1 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #2 = { argmemonly nocallback nofree nounwind willreturn writeonly }

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
!23 = distinct !{!24}
!24 = !{%class.F zeroinitializer, i32 1}
!25 = distinct !{!7, !6}
!26 = distinct !{!8, !6}
!27 = distinct !{!9, !6}
!28 = distinct !{!29, !7}
!29 = !{i32 0, i32 2}
!30 = distinct !{!31, !8}
!31 = !{float 0.000000e+00, i32 2}
!32 = distinct !{!33, !34}
!33 = !{i16 0, i32 2}
!34 = !{%struct.BaseArr zeroinitializer, i32 1}
!35 = distinct !{!7, !29}
!36 = distinct !{!8, !31}
!37 = distinct !{!7, !29}
!38 = distinct !{!8, !31}
!39 = distinct !{!34, !33}
!40 = distinct !{!7}
!41 = distinct !{!8}
!42 = distinct !{!34}
!43 = distinct !{!7}
!44 = distinct !{!7, !7}
!45 = distinct !{!8, !8}
!46 = distinct !{!8}
!47 = distinct !{!34}
!48 = distinct !{!7}
!49 = distinct !{!8}
!50 = distinct !{!34, !6}
!51 = distinct !{!9}
!52 = distinct !{!9}
!53 = distinct !{!34, !33}
!54 = distinct !{!34}
!55 = distinct !{!34}
!56 = distinct !{!7}
!57 = distinct !{!8}
!58 = distinct !{!34}
!59 = distinct !{!2}
!60 = distinct !{!2}
!61 = distinct !{!2}
!62 = distinct !{!2}
