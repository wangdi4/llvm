; RUN: opt -S < %s -dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume -debug-only=dtrans-soatoaos -dtrans-soatoaos-typename=noname -disable-output 2>&1 | FileCheck %s
; RUN: opt -S < %s -passes=dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume -debug-only=dtrans-soatoaos -dtrans-soatoaos-typename=noname -disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts

; This test check essential layout requirements and simple CFG properties. See comments inlined.

; struct Mem {
;   virtual void *allocate() = 0;
;   virtual void *deallocate() = 0;
; };
;
; struct S1;
; struct S2;
;
; template <typename S> struct Arr {
;   Mem *mem;
;   int capacilty;
;   S *base;
;   int size;
;   Parameter types are restricted.
;   S get(int i) { return base[i]; }
;   Parameter types are restricted.
;   void set(int i, S val) { base[i] = val; }

;   Only Mem is captured.

;   Arr(int c = 1, Mem *mem = nullptr)
;       : mem(mem), capacilty(c), size(0), base(nullptr) {}
;   ~Arr() {}
; };
;
; template <typename S> struct Arr1 : public Arr<S> {};
;
; class F {
;   Mem *mem;
;
; public:
;   Arr<int *> *f1;
;   Arr<void *> *f2;
;   Arr1<float *> *f3;
;   f1 and f2 are only accessed from F's methods.
;   F() {
;     f1 = new Arr<int *>(10, nullptr);
;     f2 = new Arr<void *>(10, nullptr);
;     f3 = new Arr1<float *>();
;     f1->set(0, nullptr);
;     f2->get(0);
;   }
; };
;
; int main() { F *f = new F(); }
;
; CHECK: Struct's class.F methods:
; CHECK-NEXT:_ZN1FC2Ev
; CHECK-NEXT:Fields's struct.Arr methods:
; CHECK-NEXT:_ZN3ArrIPiEC2EiP3Mem, #uses = 1
; CHECK-NEXT:_ZN3ArrIPiE3setEiS0_, #uses = 1
; CHECK-NEXT:Fields's struct.Arr.0 methods:
; CHECK-NEXT:_ZN3ArrIPvEC2EiP3Mem, #uses = 1
; CHECK-NEXT:_ZN3ArrIPvE3getEi, #uses = 1
; CHECK: Rejecting %class.F because some methods contains unknown side effect

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Arr1 = type { %struct.Arr.base.3, [4 x i8] }
%struct.Arr.base.3 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32 }>
%struct.Arr.2 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32, [4 x i8] }>

$_ZN1FC2Ev = comdat any

$_ZN3ArrIPiEC2EiP3Mem = comdat any

$_ZN3ArrIPvEC2EiP3Mem = comdat any

$_ZN4Arr1IPfEC2Ev = comdat any

$_ZN3ArrIPiE3setEiS0_ = comdat any

$_ZN3ArrIPvE3getEi = comdat any

$_ZN3ArrIPfEC2EiP3Mem = comdat any

define i32 @main() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = call i8* @_Znwm(i64 32) #7
  %0 = bitcast i8* %call to %class.F*
  invoke void @_ZN1FC2Ev(%class.F* %0)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  ret i32 0

lpad:                                             ; preds = %entry
  %1 = landingpad { i8*, i32 }
          cleanup
  %2 = extractvalue { i8*, i32 } %1, 0
  %3 = extractvalue { i8*, i32 } %1, 1
  call void @_ZdlPv(i8* %call) #8
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %2, 0
  %lpad.val1 = insertvalue { i8*, i32 } %lpad.val, i32 %3, 1
  resume { i8*, i32 } %lpad.val1
}

declare noalias i8* @_Znwm(i64) #1

define void @_ZN1FC2Ev(%class.F* nocapture %this) unnamed_addr #2 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = call i8* @_Znwm(i64 32) #7
  %0 = bitcast i8* %call to %struct.Arr*
  invoke void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %0, i32 10, %struct.Mem* null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1
  store %struct.Arr* %0, %struct.Arr** %f1, align 8
  %call2 = call i8* @_Znwm(i64 32) #7
  %1 = bitcast i8* %call2 to %struct.Arr.0*
  invoke void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* %1, i32 10, %struct.Mem* null)
          to label %invoke.cont4 unwind label %lpad3

invoke.cont4:                                     ; preds = %invoke.cont
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  store %struct.Arr.0* %1, %struct.Arr.0** %f2, align 8
  %call5 = call i8* @_Znwm(i64 32) #7
  %2 = bitcast i8* %call5 to %struct.Arr1*
  %3 = bitcast %struct.Arr1* %2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %3, i8 0, i64 32, i1 false)
  invoke void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* %2)
          to label %invoke.cont7 unwind label %lpad6

invoke.cont7:                                     ; preds = %invoke.cont4
  %f3 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 3
  store %struct.Arr1* %2, %struct.Arr1** %f3, align 8
  %f18 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1
  %4 = load %struct.Arr*, %struct.Arr** %f18, align 8
  call void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* %4, i32 0, i32* null)
  %f29 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  %5 = load %struct.Arr.0*, %struct.Arr.0** %f29, align 8
  %call10 = call i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* %5, i32 0)
  ret void

lpad:                                             ; preds = %entry
  %6 = landingpad { i8*, i32 }
          cleanup
  %7 = extractvalue { i8*, i32 } %6, 0
  %8 = extractvalue { i8*, i32 } %6, 1
  call void @_ZdlPv(i8* %call) #8
  br label %eh.resume

lpad3:                                            ; preds = %invoke.cont
  %9 = landingpad { i8*, i32 }
          cleanup
  %10 = extractvalue { i8*, i32 } %9, 0
  %11 = extractvalue { i8*, i32 } %9, 1
  call void @_ZdlPv(i8* %call2) #8
  br label %eh.resume

lpad6:                                            ; preds = %invoke.cont4
  %12 = landingpad { i8*, i32 }
          cleanup
  %13 = extractvalue { i8*, i32 } %12, 0
  %14 = extractvalue { i8*, i32 } %12, 1
  call void @_ZdlPv(i8* %call5) #8
  br label %eh.resume

eh.resume:                                        ; preds = %lpad6, %lpad3, %lpad
  %exn.slot.0 = phi i8* [ %13, %lpad6 ], [ %10, %lpad3 ], [ %7, %lpad ]
  %ehselector.slot.0 = phi i32 [ %14, %lpad6 ], [ %11, %lpad3 ], [ %8, %lpad ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0
  %lpad.val11 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { i8*, i32 } %lpad.val11
}

declare i32 @__gxx_personality_v0(...)

declare void @_ZdlPv(i8*) #3

define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* nocapture %this, i32 %c, %struct.Mem* %mem) unnamed_addr #4 comdat align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  ret void
}

define void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* nocapture %this, i32 %c, %struct.Mem* %mem) unnamed_addr #4 comdat align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  store i8** null, i8*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  ret void
}

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #5

define void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* nocapture %this) unnamed_addr #4 comdat align 2 {
entry:
  %0 = bitcast %struct.Arr1* %this to %struct.Arr.2*
  call void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* %0, i32 1, %struct.Mem* null)
  ret void
}

define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* nocapture readonly %this, i32 %i, i32* %val) #4 comdat align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %0 = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32*, i32** %0, i64 %idxprom
  store i32* %val, i32** %arrayidx, align 8
  ret void
}

define i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* nocapture readonly %this, i32 %i) #6 comdat align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %0 = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i8*, i8** %0, i64 %idxprom
  %1 = load i8*, i8** %arrayidx, align 8
  ret i8* %1
}

define void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* nocapture %this, i32 %c, %struct.Mem* %mem) unnamed_addr #4 comdat align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 3
  store float** null, float*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  ret void
}

attributes #0 = { noinline norecurse uwtable }
attributes #1 = { nobuiltin }
attributes #2 = { noinline uwtable }
attributes #3 = { nobuiltin nounwind }
attributes #4 = { noinline norecurse nounwind uwtable }
attributes #5 = { argmemonly nounwind }
attributes #6 = { noinline norecurse nounwind readonly uwtable }
attributes #7 = { builtin }
attributes #8 = { builtin nounwind }
