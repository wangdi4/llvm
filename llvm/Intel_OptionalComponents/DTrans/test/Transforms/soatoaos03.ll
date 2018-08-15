; RUN: opt -S < %s \
; RUN: -dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck %s
; RUN: opt -S < %s \
; RUN: -passes=dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts
;
; Checks method classification and comparison of methods,
; which are going to be combined. See comments for C++ code.

; CHECK:      Checking array's method _ZN3ArrIPiEC2EiP3Mem
; CHECK-NEXT: Classification: Ctor method
; CHECK-NEXT: Checking array's method _ZN3ArrIPiE3setEiS0_
; CHECK-NEXT: Classification: Set element method
; CHECK-NEXT: Checking array's method _ZN3ArrIPiE3addERKS0_
; CHECK-NEXT: Classification: Append element method
; CHECK-NEXT: Checking array's method _ZN3ArrIPiE7reallocEi
; CHECK-NEXT: Classification: Realloc method
; CHECK-NEXT: Checking array's method _ZN3ArrIPiEC2ERKS1_
; CHECK-NEXT: Classification: CCtor method
; CHECK-NEXT: Checking array's method _ZN3ArrIPiED2Ev
; CHECK-NEXT: Classification: Dtor method
; CHECK-NEXT: Checking array's method _ZN3ArrIPvEC2EiP3Mem
; CHECK-NEXT: Classification: Ctor method
; CHECK-NEXT: Checking array's method _ZN3ArrIPvE3getEi
; CHECK-NEXT: Classification: Get pointer to element method
; CHECK-NEXT: Checking array's method _ZN3ArrIPvE3addERKS0_
; CHECK-NEXT: Classification: Append element method
; CHECK-NEXT: Checking array's method _ZN3ArrIPvE7reallocEi
; CHECK-NEXT: Classification: Realloc method
; CHECK-NEXT: Checking array's method _ZN3ArrIPvEC2ERKS1_
; CHECK-NEXT: Classification: CCtor method
; CHECK-NEXT: Checking array's method _ZN3ArrIPvED2Ev
; CHECK-NEXT: Classification: Dtor method
; CHECK: Rejecting %class.F based on dtrans-soatoaos-typename option.
;
; extern void *malloc(int);
; extern void free(void *);
; struct Mem {
;   virtual void *allocate(int size) { return malloc(size); }
;   virtual void deallocate(void *ptr) noexcept { free(ptr); }
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
;
;   MK_GetElement
;   S* get(int i) {
;     if (size > 7)
;       return base + i * i + 1;
;     return base + i;
;   }
;   MK_Set
;   void set(int i, S val) { base[i] = val; }
;   MK_Ctor, should be combined.
;   Arr(int c = 1, Mem *mem = nullptr)
;       : mem(mem), capacilty(c), size(0), base(nullptr) {
;     base = (S *)mem->allocate(capacilty * sizeof(S));
;   }
;   MK_Realloc, should be combined.
;   void realloc(int inc) {
;     if (size + inc <= capacilty)
;       return;
;
;     S *new_base = (S *)mem->allocate(5 * (size + inc) * sizeof(S));
;     capacilty = size + inc;
;     for (int i = 0; i < size; ++i) {
;       new_base[5 * i] = base[i];
;     }
;     mem->deallocate(base);
;     base = new_base;
;   }
;   MK_Append, should be combined.
;   void add(const S &e) {
;     realloc(1);
;
;     base[size] = e;
;     ++size;
;   }
;   MK_CCtor, should be combined.
;   Arr(const Arr &A) {
;     mem = A.mem;
;     capacilty = A.capacilty;
;     size = A.size;
;     base = (S *)mem->allocate(size + capacilty * sizeof(S));
;     for (int i = 0; i < size; ++i)
;       base[size + i] = A.base[i];
;   }
;   MK_Dtor, should be combined.
;   ~Arr() { mem->deallocate(base); }
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
;   F(const F &f) {
;     Arr<int *> *f1 = new Arr<int *>(*f.f1);
;     Arr<void *> *f2 = new Arr<void *>(*f.f2);
;   }
;   F() {
;     f1 = new Arr<int *>(10, nullptr);
;     f2 = new Arr<void *>(10, nullptr);
;     f3 = new Arr1<float *>();
;     f1->set(0, nullptr);
;     f2->get(0);
;     f1->add(nullptr);
;     f2->add(nullptr);
;   }
;   ~F() {
;     delete f1;
;     delete f2;
;   }
; };
;
; int main() {
;   F *f = new F();
;   F *f1 = new F(*f);
;   delete f;
;   delete f1;
; }
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Arr1 = type { %struct.Arr.base.3, [4 x i8] }
%struct.Arr.base.3 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32 }>
%struct.Arr.2 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32, [4 x i8] }>

$_ZN1FC2Ev = comdat any

$_ZN1FC2ERKS_ = comdat any

$_ZN1FD2Ev = comdat any

$_ZN3ArrIPiEC2EiP3Mem = comdat any

$_ZN3ArrIPvEC2EiP3Mem = comdat any

$_ZN4Arr1IPfEC2Ev = comdat any

$_ZN3ArrIPiE3setEiS0_ = comdat any

$_ZN3ArrIPvE3getEi = comdat any

$_ZN3ArrIPiE3addERKS0_ = comdat any

$_ZN3ArrIPvE3addERKS0_ = comdat any

$_ZN3ArrIPfEC2EiP3Mem = comdat any

$_ZN3ArrIPiE7reallocEi = comdat any

$_ZN3ArrIPvE7reallocEi = comdat any

$_ZN3ArrIPiEC2ERKS1_ = comdat any

$_ZN3ArrIPvEC2ERKS1_ = comdat any

$_ZN3ArrIPiED2Ev = comdat any

$_ZN3ArrIPvED2Ev = comdat any

define i32 @main() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = call i8* @_Znwm(i64 32) #8
  %tmp = bitcast i8* %call to %class.F*
  invoke void @_ZN1FC2Ev(%class.F* %tmp)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %call1 = call i8* @_Znwm(i64 32) #8
  %tmp1 = bitcast i8* %call1 to %class.F*
  invoke void @_ZN1FC2ERKS_(%class.F* %tmp1, %class.F* dereferenceable(32) %tmp)
          to label %invoke.cont3 unwind label %lpad2

invoke.cont3:                                     ; preds = %invoke.cont
  %isnull = icmp eq %class.F* %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %invoke.cont3
  call void @_ZN1FD2Ev(%class.F* %tmp) #9
  %tmp2 = bitcast %class.F* %tmp to i8*
  call void @_ZdlPv(i8* %tmp2) #10
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %invoke.cont3
  %isnull4 = icmp eq %class.F* %tmp1, null
  br i1 %isnull4, label %delete.end6, label %delete.notnull5

delete.notnull5:                                  ; preds = %delete.end
  call void @_ZN1FD2Ev(%class.F* %tmp1) #9
  %tmp3 = bitcast %class.F* %tmp1 to i8*
  call void @_ZdlPv(i8* %tmp3) #10
  br label %delete.end6

delete.end6:                                      ; preds = %delete.notnull5, %delete.end
  ret i32 0

lpad:                                             ; preds = %entry
  %tmp4 = landingpad { i8*, i32 }
          cleanup
  %tmp5 = extractvalue { i8*, i32 } %tmp4, 0
  %tmp6 = extractvalue { i8*, i32 } %tmp4, 1
  call void @_ZdlPv(i8* %call) #10
  br label %eh.resume

lpad2:                                            ; preds = %invoke.cont
  %tmp7 = landingpad { i8*, i32 }
          cleanup
  %tmp8 = extractvalue { i8*, i32 } %tmp7, 0
  %tmp9 = extractvalue { i8*, i32 } %tmp7, 1
  call void @_ZdlPv(i8* %call1) #10
  br label %eh.resume

eh.resume:                                        ; preds = %lpad2, %lpad
  %exn.slot.0 = phi i8* [ %tmp8, %lpad2 ], [ %tmp5, %lpad ]
  %ehselector.slot.0 = phi i32 [ %tmp9, %lpad2 ], [ %tmp6, %lpad ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0
  %lpad.val7 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { i8*, i32 } %lpad.val7
}

declare noalias i8* @_Znwm(i64) #1

define void @_ZN1FC2Ev(%class.F* nocapture %this) unnamed_addr #2 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %ref.tmp = alloca i32*, align 8
  %ref.tmp13 = alloca i8*, align 8
  %call = call i8* @_Znwm(i64 32) #8
  %tmp = bitcast i8* %call to %struct.Arr*
  invoke void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %tmp, i32 10, %struct.Mem* null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1
  store %struct.Arr* %tmp, %struct.Arr** %f1, align 8
  %call2 = call i8* @_Znwm(i64 32) #8
  %tmp1 = bitcast i8* %call2 to %struct.Arr.0*
  invoke void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* %tmp1, i32 10, %struct.Mem* null)
          to label %invoke.cont4 unwind label %lpad3

invoke.cont4:                                     ; preds = %invoke.cont
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  store %struct.Arr.0* %tmp1, %struct.Arr.0** %f2, align 8
  %call5 = call i8* @_Znwm(i64 32) #8
  %tmp2 = bitcast i8* %call5 to %struct.Arr1*
  %tmp3 = bitcast %struct.Arr1* %tmp2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %tmp3, i8 0, i64 32, i1 false)
  invoke void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* %tmp2)
          to label %invoke.cont7 unwind label %lpad6

invoke.cont7:                                     ; preds = %invoke.cont4
  %f3 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 3
  store %struct.Arr1* %tmp2, %struct.Arr1** %f3, align 8
  %f18 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1
  %tmp4 = load %struct.Arr*, %struct.Arr** %f18, align 8
  call void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* %tmp4, i32 0, i32* null)
  %f29 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  %tmp5 = load %struct.Arr.0*, %struct.Arr.0** %f29, align 8
  %call10 = call i8** @_ZN3ArrIPvE3getEi(%struct.Arr.0* %tmp5, i32 0)
  %f111 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1
  %tmp6 = load %struct.Arr*, %struct.Arr** %f111, align 8
  store i32* null, i32** %ref.tmp, align 8
  call void @_ZN3ArrIPiE3addERKS0_(%struct.Arr* %tmp6, i32** dereferenceable(8) %ref.tmp)
  %f212 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  %tmp7 = load %struct.Arr.0*, %struct.Arr.0** %f212, align 8
  store i8* null, i8** %ref.tmp13, align 8
  call void @_ZN3ArrIPvE3addERKS0_(%struct.Arr.0* %tmp7, i8** dereferenceable(8) %ref.tmp13)
  ret void

lpad:                                             ; preds = %entry
  %tmp8 = landingpad { i8*, i32 }
          cleanup
  %tmp9 = extractvalue { i8*, i32 } %tmp8, 0
  %tmp10 = extractvalue { i8*, i32 } %tmp8, 1
  call void @_ZdlPv(i8* %call) #10
  br label %eh.resume

lpad3:                                            ; preds = %invoke.cont
  %tmp11 = landingpad { i8*, i32 }
          cleanup
  %tmp12 = extractvalue { i8*, i32 } %tmp11, 0
  %tmp13 = extractvalue { i8*, i32 } %tmp11, 1
  call void @_ZdlPv(i8* %call2) #10
  br label %eh.resume

lpad6:                                            ; preds = %invoke.cont4
  %tmp14 = landingpad { i8*, i32 }
          cleanup
  %tmp15 = extractvalue { i8*, i32 } %tmp14, 0
  %tmp16 = extractvalue { i8*, i32 } %tmp14, 1
  call void @_ZdlPv(i8* %call5) #10
  br label %eh.resume

eh.resume:                                        ; preds = %lpad6, %lpad3, %lpad
  %exn.slot.0 = phi i8* [ %tmp15, %lpad6 ], [ %tmp12, %lpad3 ], [ %tmp9, %lpad ]
  %ehselector.slot.0 = phi i32 [ %tmp16, %lpad6 ], [ %tmp13, %lpad3 ], [ %tmp10, %lpad ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0
  %lpad.val14 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { i8*, i32 } %lpad.val14
}

declare i32 @__gxx_personality_v0(...)

declare void @_ZdlPv(i8*) #3

define void @_ZN1FC2ERKS_(%class.F* nocapture readnone %this, %class.F* nocapture readonly dereferenceable(32) %f) unnamed_addr #2 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = call i8* @_Znwm(i64 32) #8
  %tmp = bitcast i8* %call to %struct.Arr*
  %f12 = getelementptr inbounds %class.F, %class.F* %f, i32 0, i32 1
  %tmp1 = load %struct.Arr*, %struct.Arr** %f12, align 8
  invoke void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* %tmp, %struct.Arr* dereferenceable(32) %tmp1)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %call3 = call i8* @_Znwm(i64 32) #8
  %tmp2 = bitcast i8* %call3 to %struct.Arr.0*
  %f24 = getelementptr inbounds %class.F, %class.F* %f, i32 0, i32 2
  %tmp3 = load %struct.Arr.0*, %struct.Arr.0** %f24, align 8
  invoke void @_ZN3ArrIPvEC2ERKS1_(%struct.Arr.0* %tmp2, %struct.Arr.0* dereferenceable(32) %tmp3)
          to label %invoke.cont6 unwind label %lpad5

invoke.cont6:                                     ; preds = %invoke.cont
  ret void

lpad:                                             ; preds = %entry
  %tmp4 = landingpad { i8*, i32 }
          cleanup
  %tmp5 = extractvalue { i8*, i32 } %tmp4, 0
  %tmp6 = extractvalue { i8*, i32 } %tmp4, 1
  call void @_ZdlPv(i8* %call) #10
  br label %eh.resume

lpad5:                                            ; preds = %invoke.cont
  %tmp7 = landingpad { i8*, i32 }
          cleanup
  %tmp8 = extractvalue { i8*, i32 } %tmp7, 0
  %tmp9 = extractvalue { i8*, i32 } %tmp7, 1
  call void @_ZdlPv(i8* %call3) #10
  br label %eh.resume

eh.resume:                                        ; preds = %lpad5, %lpad
  %exn.slot.0 = phi i8* [ %tmp8, %lpad5 ], [ %tmp5, %lpad ]
  %ehselector.slot.0 = phi i32 [ %tmp9, %lpad5 ], [ %tmp6, %lpad ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0
  %lpad.val7 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { i8*, i32 } %lpad.val7
}

define void @_ZN1FD2Ev(%class.F* nocapture readonly %this) unnamed_addr #4 comdat align 2 {
entry:
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1
  %tmp = load %struct.Arr*, %struct.Arr** %f1, align 8
  %isnull = icmp eq %struct.Arr* %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  call void @_ZN3ArrIPiED2Ev(%struct.Arr* %tmp) #9
  %tmp1 = bitcast %struct.Arr* %tmp to i8*
  call void @_ZdlPv(i8* %tmp1) #10
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %entry
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  %tmp2 = load %struct.Arr.0*, %struct.Arr.0** %f2, align 8
  %isnull2 = icmp eq %struct.Arr.0* %tmp2, null
  br i1 %isnull2, label %delete.end4, label %delete.notnull3

delete.notnull3:                                  ; preds = %delete.end
  call void @_ZN3ArrIPvED2Ev(%struct.Arr.0* %tmp2) #9
  %tmp3 = bitcast %struct.Arr.0* %tmp2 to i8*
  call void @_ZdlPv(i8* %tmp3) #10
  br label %delete.end4

delete.end4:                                      ; preds = %delete.notnull3, %delete.end
  ret void
}

define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* nocapture %this, i32 %c, %struct.Mem* %mem) unnamed_addr #2 comdat align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp = load i32, i32* %capacilty3, align 8
  %conv = sext i32 %tmp to i64
  %mul = mul i64 %conv, 8
  %conv4 = trunc i64 %mul to i32
  %tmp1 = bitcast %struct.Mem* %mem to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp1, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp2 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp2(%struct.Mem* %mem, i32 %conv4)
  %tmp3 = bitcast i8* %call to i32**
  %base5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  store i32** %tmp3, i32*** %base5, align 8
  ret void
}

define void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* nocapture %this, i32 %c, %struct.Mem* %mem) unnamed_addr #2 comdat align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  store i8** null, i8*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  %tmp = load i32, i32* %capacilty3, align 8
  %conv = sext i32 %tmp to i64
  %mul = mul i64 %conv, 8
  %conv4 = trunc i64 %mul to i32
  %tmp1 = bitcast %struct.Mem* %mem to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp1, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp2 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp2(%struct.Mem* %mem, i32 %conv4)
  %tmp3 = bitcast i8* %call to i8**
  %base5 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  store i8** %tmp3, i8*** %base5, align 8
  ret void
}

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #5

define void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* nocapture %this) unnamed_addr #2 comdat align 2 {
entry:
  %tmp = bitcast %struct.Arr1* %this to %struct.Arr.2*
  call void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* %tmp, i32 1, %struct.Mem* null)
  ret void
}

define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* nocapture readonly %this, i32 %i, i32* %val) #6 comdat align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp, i64 %idxprom
  store i32* %val, i32** %arrayidx, align 8
  ret void
}

define i8** @_ZN3ArrIPvE3getEi(%struct.Arr.0* nocapture readonly %this, i32 %i) #7 comdat align 2 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp = load i32, i32* %size, align 8
  %cmp = icmp sgt i32 %tmp, 7
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp1 = load i8**, i8*** %base, align 8
  %mul = mul nsw i32 %i, %i
  %idx.ext = sext i32 %mul to i64
  %add.ptr = getelementptr inbounds i8*, i8** %tmp1, i64 %idx.ext
  %add.ptr2 = getelementptr inbounds i8*, i8** %add.ptr, i64 1
  br label %return

if.end:                                           ; preds = %entry
  %base3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp2 = load i8**, i8*** %base3, align 8
  %idx.ext4 = sext i32 %i to i64
  %add.ptr5 = getelementptr inbounds i8*, i8** %tmp2, i64 %idx.ext4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i8** [ %add.ptr2, %if.then ], [ %add.ptr5, %if.end ]
  ret i8** %retval.0
}

define void @_ZN3ArrIPiE3addERKS0_(%struct.Arr* nocapture %this, i32** nocapture readonly dereferenceable(8) %e) #2 comdat align 2 {
entry:
  call void @_ZN3ArrIPiE7reallocEi(%struct.Arr* %this, i32 1)
  %tmp = load i32*, i32** %e, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp1 = load i32**, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp2 = load i32, i32* %size, align 8
  %idxprom = sext i32 %tmp2 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp1, i64 %idxprom
  store i32* %tmp, i32** %arrayidx, align 8
  %size2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp3 = load i32, i32* %size2, align 8
  %inc = add nsw i32 %tmp3, 1
  store i32 %inc, i32* %size2, align 8
  ret void
}

define void @_ZN3ArrIPvE3addERKS0_(%struct.Arr.0* nocapture %this, i8** nocapture readonly dereferenceable(8) %e) #2 comdat align 2 {
entry:
  call void @_ZN3ArrIPvE7reallocEi(%struct.Arr.0* %this, i32 1)
  %tmp = load i8*, i8** %e, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp1 = load i8**, i8*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp2 = load i32, i32* %size, align 8
  %idxprom = sext i32 %tmp2 to i64
  %arrayidx = getelementptr inbounds i8*, i8** %tmp1, i64 %idxprom
  store i8* %tmp, i8** %arrayidx, align 8
  %size2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp3 = load i32, i32* %size2, align 8
  %inc = add nsw i32 %tmp3, 1
  store i32 %inc, i32* %size2, align 8
  ret void
}

define void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* nocapture %this, i32 %c, %struct.Mem* %mem) unnamed_addr #2 comdat align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 3
  store float** null, float*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 1
  %tmp = load i32, i32* %capacilty3, align 8
  %conv = sext i32 %tmp to i64
  %mul = mul i64 %conv, 8
  %conv4 = trunc i64 %mul to i32
  %tmp1 = bitcast %struct.Mem* %mem to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp1, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp2 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp2(%struct.Mem* %mem, i32 %conv4)
  %tmp3 = bitcast i8* %call to float**
  %base5 = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 3
  store float** %tmp3, float*** %base5, align 8
  ret void
}

define void @_ZN3ArrIPiE7reallocEi(%struct.Arr* nocapture %this, i32 %inc) #2 comdat align 2 {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp = load i32, i32* %size, align 8
  %add = add nsw i32 %tmp, %inc
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp1 = load i32, i32* %capacilty, align 8
  %cmp = icmp sle i32 %add, %tmp1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp2 = load %struct.Mem*, %struct.Mem** %mem, align 8
  %size2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp3 = load i32, i32* %size2, align 8
  %add3 = add nsw i32 %tmp3, %inc
  %mul = mul nsw i32 5, %add3
  %conv = sext i32 %mul to i64
  %mul4 = mul i64 %conv, 8
  %conv5 = trunc i64 %mul4 to i32
  %tmp4 = bitcast %struct.Mem* %tmp2 to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp4, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp5 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp5(%struct.Mem* %tmp2, i32 %conv5)
  %tmp6 = bitcast i8* %call to i32**
  %size6 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp7 = load i32, i32* %size6, align 8
  %add7 = add nsw i32 %tmp7, %inc
  %capacilty8 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  store i32 %add7, i32* %capacilty8, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc14, %for.inc ]
  %size9 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp8 = load i32, i32* %size9, align 8
  %cmp10 = icmp slt i32 %i.0, %tmp8
  br i1 %cmp10, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp9 = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp9, i64 %idxprom
  %tmp10 = load i32*, i32** %arrayidx, align 8
  %mul11 = mul nsw i32 5, %i.0
  %idxprom12 = sext i32 %mul11 to i64
  %arrayidx13 = getelementptr inbounds i32*, i32** %tmp6, i64 %idxprom12
  store i32* %tmp10, i32** %arrayidx13, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc14 = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %mem15 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp11 = load %struct.Mem*, %struct.Mem** %mem15, align 8
  %base16 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp12 = load i32**, i32*** %base16, align 8
  %tmp13 = bitcast i32** %tmp12 to i8*
  %tmp14 = bitcast %struct.Mem* %tmp11 to void (%struct.Mem*, i8*)***
  %vtable17 = load void (%struct.Mem*, i8*)**, void (%struct.Mem*, i8*)*** %tmp14, align 8
  %vfn18 = getelementptr inbounds void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vtable17, i64 1
  %tmp15 = load void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vfn18, align 8
  call void %tmp15(%struct.Mem* %tmp11, i8* %tmp13) #9
  %base19 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  store i32** %tmp6, i32*** %base19, align 8
  br label %return

return:                                           ; preds = %for.end, %if.then
  ret void
}

define void @_ZN3ArrIPvE7reallocEi(%struct.Arr.0* nocapture %this, i32 %inc) #2 comdat align 2 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp = load i32, i32* %size, align 8
  %add = add nsw i32 %tmp, %inc
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  %tmp1 = load i32, i32* %capacilty, align 8
  %cmp = icmp sle i32 %add, %tmp1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %mem = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  %tmp2 = load %struct.Mem*, %struct.Mem** %mem, align 8
  %size2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp3 = load i32, i32* %size2, align 8
  %add3 = add nsw i32 %tmp3, %inc
  %mul = mul nsw i32 5, %add3
  %conv = sext i32 %mul to i64
  %mul4 = mul i64 %conv, 8
  %conv5 = trunc i64 %mul4 to i32
  %tmp4 = bitcast %struct.Mem* %tmp2 to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp4, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp5 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp5(%struct.Mem* %tmp2, i32 %conv5)
  %tmp6 = bitcast i8* %call to i8**
  %size6 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp7 = load i32, i32* %size6, align 8
  %add7 = add nsw i32 %tmp7, %inc
  %capacilty8 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  store i32 %add7, i32* %capacilty8, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc14, %for.inc ]
  %size9 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp8 = load i32, i32* %size9, align 8
  %cmp10 = icmp slt i32 %i.0, %tmp8
  br i1 %cmp10, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp9 = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i8*, i8** %tmp9, i64 %idxprom
  %tmp10 = load i8*, i8** %arrayidx, align 8
  %mul11 = mul nsw i32 5, %i.0
  %idxprom12 = sext i32 %mul11 to i64
  %arrayidx13 = getelementptr inbounds i8*, i8** %tmp6, i64 %idxprom12
  store i8* %tmp10, i8** %arrayidx13, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc14 = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %mem15 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  %tmp11 = load %struct.Mem*, %struct.Mem** %mem15, align 8
  %base16 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp12 = load i8**, i8*** %base16, align 8
  %tmp13 = bitcast i8** %tmp12 to i8*
  %tmp14 = bitcast %struct.Mem* %tmp11 to void (%struct.Mem*, i8*)***
  %vtable17 = load void (%struct.Mem*, i8*)**, void (%struct.Mem*, i8*)*** %tmp14, align 8
  %vfn18 = getelementptr inbounds void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vtable17, i64 1
  %tmp15 = load void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vfn18, align 8
  call void %tmp15(%struct.Mem* %tmp11, i8* %tmp13) #9
  %base19 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  store i8** %tmp6, i8*** %base19, align 8
  br label %return

return:                                           ; preds = %for.end, %if.then
  ret void
}

define void @_ZN3ArrIPiEC2ERKS1_(%struct.Arr* nocapture %this, %struct.Arr* nocapture readonly dereferenceable(32) %A) unnamed_addr #2 comdat align 2 {
entry:
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  store %struct.Mem* %tmp, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i32 0, i32 1
  %tmp1 = load i32, i32* %capacilty, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  store i32 %tmp1, i32* %capacilty3, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i32 0, i32 4
  %tmp2 = load i32, i32* %size, align 8
  %size4 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  store i32 %tmp2, i32* %size4, align 8
  %mem5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp3 = load %struct.Mem*, %struct.Mem** %mem5, align 8
  %size6 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp4 = load i32, i32* %size6, align 8
  %conv = sext i32 %tmp4 to i64
  %capacilty7 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp5 = load i32, i32* %capacilty7, align 8
  %conv8 = sext i32 %tmp5 to i64
  %mul = mul i64 %conv8, 8
  %add = add i64 %conv, %mul
  %conv9 = trunc i64 %add to i32
  %tmp6 = bitcast %struct.Mem* %tmp3 to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp6, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp7 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp7(%struct.Mem* %tmp3, i32 %conv9)
  %tmp8 = bitcast i8* %call to i32**
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  store i32** %tmp8, i32*** %base, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %size10 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp9 = load i32, i32* %size10, align 8
  %cmp = icmp slt i32 %i.0, %tmp9
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %base11 = getelementptr inbounds %struct.Arr, %struct.Arr* %A, i32 0, i32 3
  %tmp10 = load i32**, i32*** %base11, align 8
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp10, i64 %idxprom
  %tmp11 = load i32*, i32** %arrayidx, align 8
  %base12 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp12 = load i32**, i32*** %base12, align 8
  %size13 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp13 = load i32, i32* %size13, align 8
  %add14 = add nsw i32 %tmp13, %i.0
  %idxprom15 = sext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds i32*, i32** %tmp12, i64 %idxprom15
  store i32* %tmp11, i32** %arrayidx16, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define void @_ZN3ArrIPvEC2ERKS1_(%struct.Arr.0* nocapture %this, %struct.Arr.0* nocapture readonly dereferenceable(32) %A) unnamed_addr #2 comdat align 2 {
entry:
  %mem = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  store %struct.Mem* %tmp, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 1
  %tmp1 = load i32, i32* %capacilty, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  store i32 %tmp1, i32* %capacilty3, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 4
  %tmp2 = load i32, i32* %size, align 8
  %size4 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  store i32 %tmp2, i32* %size4, align 8
  %mem5 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  %tmp3 = load %struct.Mem*, %struct.Mem** %mem5, align 8
  %size6 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp4 = load i32, i32* %size6, align 8
  %conv = sext i32 %tmp4 to i64
  %capacilty7 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  %tmp5 = load i32, i32* %capacilty7, align 8
  %conv8 = sext i32 %tmp5 to i64
  %mul = mul i64 %conv8, 8
  %add = add i64 %conv, %mul
  %conv9 = trunc i64 %add to i32
  %tmp6 = bitcast %struct.Mem* %tmp3 to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp6, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp7 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp7(%struct.Mem* %tmp3, i32 %conv9)
  %tmp8 = bitcast i8* %call to i8**
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  store i8** %tmp8, i8*** %base, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %size10 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp9 = load i32, i32* %size10, align 8
  %cmp = icmp slt i32 %i.0, %tmp9
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %base11 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %A, i32 0, i32 3
  %tmp10 = load i8**, i8*** %base11, align 8
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i8*, i8** %tmp10, i64 %idxprom
  %tmp11 = load i8*, i8** %arrayidx, align 8
  %base12 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp12 = load i8**, i8*** %base12, align 8
  %size13 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  %tmp13 = load i32, i32* %size13, align 8
  %add14 = add nsw i32 %tmp13, %i.0
  %idxprom15 = sext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds i8*, i8** %tmp12, i64 %idxprom15
  store i8* %tmp11, i8** %arrayidx16, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define void @_ZN3ArrIPiED2Ev(%struct.Arr* nocapture readonly %this) unnamed_addr #4 comdat align 2 {
entry:
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp1 = load i32**, i32*** %base, align 8
  %tmp2 = bitcast i32** %tmp1 to i8*
  %tmp3 = bitcast %struct.Mem* %tmp to void (%struct.Mem*, i8*)***
  %vtable = load void (%struct.Mem*, i8*)**, void (%struct.Mem*, i8*)*** %tmp3, align 8
  %vfn = getelementptr inbounds void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vtable, i64 1
  %tmp4 = load void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vfn, align 8
  call void %tmp4(%struct.Mem* %tmp, i8* %tmp2) #9
  ret void
}

define void @_ZN3ArrIPvED2Ev(%struct.Arr.0* nocapture readonly %this) unnamed_addr #4 comdat align 2 {
entry:
  %mem = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp1 = load i8**, i8*** %base, align 8
  %tmp2 = bitcast i8** %tmp1 to i8*
  %tmp3 = bitcast %struct.Mem* %tmp to void (%struct.Mem*, i8*)***
  %vtable = load void (%struct.Mem*, i8*)**, void (%struct.Mem*, i8*)*** %tmp3, align 8
  %vfn = getelementptr inbounds void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vtable, i64 1
  %tmp4 = load void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vfn, align 8
  call void %tmp4(%struct.Mem* %tmp, i8* %tmp2) #9
  ret void
}

attributes #0 = { noinline norecurse uwtable }
attributes #1 = { nobuiltin }
attributes #2 = { noinline uwtable }
attributes #3 = { nobuiltin nounwind }
attributes #4 = { noinline nounwind uwtable }
attributes #5 = { argmemonly nounwind }
attributes #6 = { noinline norecurse nounwind uwtable }
attributes #7 = { noinline norecurse nounwind readonly uwtable }
attributes #8 = { builtin }
attributes #9 = { nounwind }
attributes #10 = { builtin nounwind }
