; This test verifies that SOAToAOSPrepare transformation is able
; to detect possible candidate. It also checks the candidate fails
; functionality analysis because member functions of the class are
; not valid to make the class as vector class.

; RUN: opt < %s -passes=dtrans-soatoaosop-prepare  -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop-prepare -disable-output 2>&1 | FileCheck %s

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

; CHECK: SOAToAOSPrepare: Candidate selected for more analysis
; CHECK: Candidate struct: class.F    FieldOff: 3
; CHECK: Candidate failed after functionality analysis.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.F = type { ptr, ptr, ptr, ptr }
%struct.Arr = type { i8, i32, i32, ptr, ptr }
%struct.Arr.0 = type { i8, i32, i32, ptr, ptr }
%struct.Mem = type { ptr }
%struct.RefArr = type { %struct.BaseArr }
%struct.BaseArr = type { ptr, i8, i32, i32, ptr, ptr }

define dso_local i32 @main() {
entry:
  %call = call ptr @_Znwm(i64 32)
  tail call void @_ZN1FC2Ev(ptr %call)
  ret i32 0
}

define linkonce_odr dso_local void @_ZN1FC2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !18 {
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

define linkonce_odr dso_local void @_ZN3ArrIPiEC2EjP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !20 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2EjP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !21 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsEC2EjbP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, i1 %adoptE, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !22 {
entry:
  ret void
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPiE3getEj(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !23 {
entry:
  ret ptr null
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPfE3getEj(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !25 {
entry:
  ret ptr null
}

define linkonce_odr dso_local "intel_dtrans_func_index"="1" ptr @_ZN7BaseArrIPsE3getEj(ptr nocapture "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !27 {
entry:
  ret ptr null
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3setEjPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !30 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3setEjPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !31 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !32 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEi(ptr %this, i32 1)
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !33 {
entry:
  tail call void @_ZN3ArrIPfE6resizeEi(ptr %this, i32 1)
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE3addEPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !34 {
entry:
  tail call void @_ZN7BaseArrIPsE6resizeEi(ptr %this, i32 1)
  ret void
}

define linkonce_odr dso_local i32 @_ZN3ArrIPiE7getSizeEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !35 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN3ArrIPfE7getSizeEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !36 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN7BaseArrIPsE7getSizeEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !37 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN3ArrIPiE11getCapacityEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !38 {
bb:
  ret i32 0
}

define linkonce_odr dso_local void @_ZN3ArrIPiEC2ERKS1_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !39 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfEC2ERKS1_(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr nocapture "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !40 {
entry:
  ret void
}

define linkonce_odr dso_local i32 @_ZN3ArrIPfE11getCapacityEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !41 {
entry:
  ret i32 0
}

define linkonce_odr dso_local i32 @_ZN7BaseArrIPsE11getCapacityEv(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !42 {
entry:
  ret i32 0
}

define linkonce_odr dso_local void @_ZN3ArrIPiED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !43 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !44 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsEC2EjbP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, i1 %adopE, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !45 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !46 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN6RefArrIPsED0Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !47 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE3setEjPS0_(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !48 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsED2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !49 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsED0Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !50 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPiE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !51 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3ArrIPfE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !52 {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN7BaseArrIPsE6resizeEi(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !53 {
entry:
  ret void
}

declare !intel.dtrans.func.type !54 dso_local nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

declare !intel.dtrans.func.type !56 dso_local void @_ZdlPv(ptr "intel_dtrans_func_index"="1")

!intel.dtrans.types = !{!0, !5, !9, !12, !14, !16}

!0 = !{!"S", %class.F zeroinitializer, i32 4, !1, !2, !3, !4}
!1 = !{%struct.Mem zeroinitializer, i32 1}
!2 = !{%struct.Arr zeroinitializer, i32 1}
!3 = !{%struct.Arr.0 zeroinitializer, i32 1}
!4 = !{%struct.RefArr zeroinitializer, i32 1}
!5 = !{!"S", %struct.Mem zeroinitializer, i32 1, !6}
!6 = !{!7, i32 2}
!7 = !{!"F", i1 true, i32 0, !8}
!8 = !{i32 0, i32 0}
!9 = !{!"S", %struct.Arr zeroinitializer, i32 5, !10, !8, !8, !11, !1}
!10 = !{i8 0, i32 0}
!11 = !{i32 0, i32 3}
!12 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !10, !8, !8, !13, !1}
!13 = !{float 0.000000e+00, i32 3}
!14 = !{!"S", %struct.RefArr zeroinitializer, i32 1, !15}
!15 = !{%struct.BaseArr zeroinitializer, i32 0}
!16 = !{!"S", %struct.BaseArr zeroinitializer, i32 6, !6, !10, !8, !8, !17, !1}
!17 = !{i16 0, i32 3}
!18 = distinct !{!19}
!19 = !{%class.F zeroinitializer, i32 1}
!20 = distinct !{!2, !1}
!21 = distinct !{!3, !1}
!22 = distinct !{!4, !1}
!23 = distinct !{!24, !2}
!24 = !{i32 0, i32 2}
!25 = distinct !{!26, !3}
!26 = !{float 0.000000e+00, i32 2}
!27 = distinct !{!28, !29}
!28 = !{i16 0, i32 2}
!29 = !{%struct.BaseArr zeroinitializer, i32 1}
!30 = distinct !{!2, !24}
!31 = distinct !{!3, !26}
!32 = distinct !{!2, !24}
!33 = distinct !{!3, !26}
!34 = distinct !{!29, !28}
!35 = distinct !{!2}
!36 = distinct !{!3}
!37 = distinct !{!29}
!38 = distinct !{!2}
!39 = distinct !{!2, !2}
!40 = distinct !{!3, !3}
!41 = distinct !{!3}
!42 = distinct !{!29}
!43 = distinct !{!2}
!44 = distinct !{!3}
!45 = distinct !{!29, !1}
!46 = distinct !{!4}
!47 = distinct !{!4}
!48 = distinct !{!29, !28}
!49 = distinct !{!29}
!50 = distinct !{!29}
!51 = distinct !{!2}
!52 = distinct !{!3}
!53 = distinct !{!29}
!54 = distinct !{!55}
!55 = !{i8 0, i32 1}
!56 = distinct !{!55}
