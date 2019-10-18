; This testcase verifies that SOAToAOSPrepare transformation is able
; to detect possible candidate.

; RUN: opt < %s -dtrans-soatoaos-prepare  -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaos-prepare -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=dtrans-soatoaos-prepare  -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaos-prepare -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" fields in "F" will be considered as candidate vector
; fields for SOAToAOS transformation. "f3" will be rejected by SOAToAOS
; for the transformation because type of "f3" (i.e RefArr) is not simple
; vector class. So, SOAToAOSPrepare will consider "f3" as candidate
; and then try to convert it as simple vector class. Definitions of most
; of the member functions are empty or doesn't make sense.

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
;   unsigned getSize() { return 0; }
;   unsigned getCapacity() { return 0; }
;   void resize(int inc) { }
;   virtual ~BaseArr() { }
;   void add(S* val) {
;     resize(1);
;   }
;   virtual void set(unsigned i, S* val) { }
;   S* get(unsigned i) {
;     return nullptr;
;   }
;   BaseArr(unsigned c, bool adopE, Mem *mem = 0) { }
;   BaseArr(const BaseArr &A) { }
; };
;
; template <typename S>
; struct RefArr : public BaseArr<S> {
; public:
;   RefArr(unsigned c, bool adoptE, Mem *mem = 0);
;   ~RefArr() { }
; };
; template <typename S>
; RefArr<S>::RefArr(unsigned c, bool adoptE, Mem *mem) { }
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

; CHECK: SOAToAOSPrepare: Candidate found
; CHECK: Candidate struct: class.F    FieldOff: 3

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.RefArr* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type { i8, i32, i32, i32***, %struct.Mem* }
%struct.Arr.0 = type { i8, i32, i32, float***, %struct.Mem* }
%struct.RefArr = type { %struct.BaseArr }
%struct.BaseArr = type { i32 (...)**, i8, i32, i32, i16***, %struct.Mem* }

define dso_local i32 @main() {
entry:
  %call = call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %class.F*
  tail call void @_ZN1FC2Ev(%class.F* %0)
  ret i32 0
}

define linkonce_odr dso_local void @_ZN1FC2Ev(%class.F* %this) {
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
  tail call void %14(%struct.BaseArr* %12, i32 0, i16** %call13)
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
  tail call void %38(%struct.RefArr* nonnull %36)
  br label %delete.end57

delete.end57:                                     ; preds = %delete.notnull54, %delete.end51
  ret void
}


define linkonce_odr dso_local void @_ZN3ArrIPiEC2EjP3Mem(%struct.Arr*  nocapture %this, i32 %c, %struct.Mem* %mem) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2EjP3Mem(%struct.Arr.0*  nocapture %this, i32 %c, %struct.Mem* %mem) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsEC2EjbP3Mem(%struct.RefArr*  nocapture %this, i32 %c, i1 %adoptE, %struct.Mem* %mem) {
entry:
  ret void
}

define linkonce_odr dso_local i32** @_ZN3ArrIPiE3getEj(%struct.Arr* nocapture %this, i32 %i) {
entry:
  ret i32** null
}

define linkonce_odr dso_local float** @_ZN3ArrIPfE3getEj(%struct.Arr.0* nocapture %this, i32 %i) {
entry:
  ret float** null
}

define linkonce_odr dso_local i16** @_ZN7BaseArrIPsE3getEj(%struct.BaseArr* nocapture %this, i32 %i) {
entry:
  ret i16** null
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3setEjPS0_(%struct.Arr* nocapture %this, i32 %i, i32** %val) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3setEjPS0_(%struct.Arr.0* nocapture %this, i32 %i, float** %val) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3addEPS0_(%struct.Arr* nocapture %this, i32** %val) {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(%struct.Arr* %this, i32 1)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3addEPS0_(%struct.Arr.0* nocapture %this, float** %val) {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* %this, i32 1)
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE3addEPS0_(%struct.BaseArr* nocapture %this, i16** %val) {
entry:
  tail call void @_ZN7BaseArrIPsE6resizeEi(%struct.BaseArr* %this, i32 1)
  ret void
}

define linkonce_odr dso_local i32 @_ZN3ArrIPiE7getSizeEv(%struct.Arr* nocapture %this) {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN3ArrIPfE7getSizeEv(%struct.Arr.0* nocapture %this) {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN7BaseArrIPsE7getSizeEv(%struct.BaseArr* nocapture %this) {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN3ArrIPiE11getCapacityEv(%struct.Arr* nocapture %this) {
entry:
  ret i32 0
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* nocapture %this, %struct.Arr* nocapture %A) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2ERKS1_(%struct.Arr.0* nocapture %this, %struct.Arr.0* nocapture %A) {
entry:
  ret void
}

define linkonce_odr dso_local i32 @_ZN3ArrIPfE11getCapacityEv(%struct.Arr.0* nocapture %this) {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN7BaseArrIPsE11getCapacityEv(%struct.BaseArr* nocapture %this) {
entry:
  ret i32 0
}

define linkonce_odr dso_local void @_ZN3ArrIPiED2Ev(%struct.Arr* nocapture %this) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nocapture %this) {
entry:
  ret void
}


define linkonce_odr dso_local void @_ZN7BaseArrIPsEC2EjbP3Mem(%struct.BaseArr*  nocapture %this, i32 %c, i1 %adopE, %struct.Mem* %mem) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsED2Ev(%struct.RefArr* nocapture %this) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsED0Ev(%struct.RefArr* nocapture %this) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE3setEjPS0_(%struct.BaseArr* nocapture %this, i32 %i, i16** %val) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsED2Ev(%struct.BaseArr* nocapture %this) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsED0Ev(%struct.BaseArr* nocapture %this) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE6resizeEi(%struct.Arr* nocapture %this, i32 %inc) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE6resizeEi(%struct.Arr.0* nocapture %this, i32 %inc) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE6resizeEi(%struct.BaseArr* nocapture %this, i32 %inc) {
entry:
  ret void
}

declare dso_local noalias i8* @_Znwm(i64)
declare dso_local void @_ZdlPv(i8*)
