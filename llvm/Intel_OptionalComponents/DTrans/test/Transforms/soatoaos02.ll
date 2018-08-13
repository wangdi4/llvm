; RUN: opt -S < %s \
; RUN: -dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos,dtrans-soatoaos-deps \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck --check-prefix=CHECK-CTOR %s
; RUN: opt -S < %s \
; RUN: -dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos,dtrans-soatoaos-deps \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck --check-prefix=CHECK-SET %s
; RUN: opt -S < %s \
; RUN: -dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos,dtrans-soatoaos-deps \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck --check-prefix=CHECK-GET %s
; RUN: opt -S < %s \
; RUN: -dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos,dtrans-soatoaos-deps \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck --check-prefix=CHECK-REALLOC %s
; RUN: opt -S < %s \
; RUN: -dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos,dtrans-soatoaos-deps \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck --check-prefix=CHECK-ADD %s

; RUN: opt -S < %s \
; RUN: -passes=dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos,dtrans-soatoaos-deps \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck --check-prefix=CHECK-CTOR %s
; RUN: opt -S < %s \
; RUN: -passes=dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos,dtrans-soatoaos-deps \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck --check-prefix=CHECK-SET %s
; RUN: opt -S < %s \
; RUN: -passes=dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos,dtrans-soatoaos-deps \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck --check-prefix=CHECK-GET %s
; RUN: opt -S < %s \
; RUN: -passes=dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos,dtrans-soatoaos-deps \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck --check-prefix=CHECK-REALLOC %s
; RUN: opt -S < %s \
; RUN: -passes=dtrans-soatoaos -enable-dtrans-soatoaos -whole-program-assume \
; RUN: -debug-only=dtrans-soatoaos,dtrans-soatoaos-deps \
; RUN: -dtrans-soatoaos-typename=noname -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN: -disable-output 2>&1 | FileCheck --check-prefix=CHECK-ADD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This test checks various approximations for side effects (store/conditional branches/etc.)
; for important primitives in SOA.
;
; extern void *malloc(int);
; extern void free(void *);
; struct Mem {
;   virtual void *allocate(int size) {return malloc(size); }
;   virtual void deallocate(void *ptr) {free(ptr);}
; };
;
; struct S1;
; struct S2;
;
; template <typename S> struct Arr {
;   Mem *mem;
;   int capacity;
;   S *base;
;   int size;
;   S get(int i) { return base[i]; }
;   void set(int i, S val) { base[i] = val; }
;   Arr(int c = 1, Mem *mem = nullptr)
;       : mem(mem), capacity(c), size(0), base(nullptr) {
;     base = (S *)mem->allocate(capacity * sizeof(S));
;   }
;
;   void realloc(int inc) {
;     if (size + inc <= capacity)
;       return;
;
;     S *new_base = (S *)mem->allocate((size + inc) * sizeof(S));
;     capacity = size + inc;
;     for (int i = 0; i < size; ++i) {
;       new_base[i] = base[i];
;     }
;     mem->deallocate(base);
;     base = new_base;
;   }
;   void add(const S &e) {
;     realloc(1);
;
;     base[size] = e;
;     ++size;
;   }
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
;   F() {
;     f1 = new Arr<int *>(10, nullptr);
;     f2 = new Arr<void *>(10, nullptr);
;     f3 = new Arr1<float *>();
;     f1->set(0, nullptr);
;     f2->get(0);
;     f1->add(nullptr);
;     f2->add(nullptr);
;   }
; };
;
; int main() { F *f = new F(); }

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Arr1 = type { %struct.Arr.base.3, [4 x i8] }
%struct.Arr.base.3 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32 }>
%struct.Arr.2 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32, [4 x i8] }>

$_ZN1FC2Ev = comdat any

$_ZN3ArrIPiEC2EiP3Mem = comdat any

$_ZN4Arr1IPfEC2Ev = comdat any

$_ZN3ArrIPiE3setEiS0_ = comdat any

$_ZN3ArrIPvE3getEi = comdat any

$_ZN3ArrIPiE3addERKS0_ = comdat any

$_ZN3ArrIPvE3addERKS0_ = comdat any

$_ZN3ArrIPiE7reallocEi = comdat any

declare noalias i8* @_Znwm(i64) #1

; Regular ctor.
; Arr(int c = 1, Mem *mem = nullptr)
;     : mem(mem), capacity(c), size(0), base(nullptr) {
;   base = (S *)mem->allocate(capacity * sizeof(S));
; }
; CHECK-CTOR-LABEL: @_ZN3ArrIPiEC2EiP3Mem
define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* nocapture %this, i32 %c, %struct.Mem* %mem) unnamed_addr #2 comdat align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
; Direct store of paraeter 1 to integer field #1.
; CHECK-CTOR:      Store(Arg 1)
; CHECK-CTOR-NEXT:      (GEP(Arg 0)
; CHECK-CTOR-NEXT:           1)
; CHECK-CTOR-NEXT: store i32 %c, i32* %capacity, align 8
  store i32 %c, i32* %capacity, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
; CHECK-CTOR: %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
; Const initialization of integer field #4
; CHECK-CTOR-NEXT: Store(Const)
; CHECK-CTOR-NEXT:      (GEP(Arg 0)
; CHECK-CTOR-NEXT:           4)
; CHECK-CTOR-NEXT: store i32 0, i32* %size, align 8
  store i32 0, i32* %size, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp3 = load i32, i32* %capacilty3, align 8
  %conv = sext i32 %tmp3 to i64
  %mul = mul i64 %conv, 8
  %conv4 = trunc i64 %mul to i32
  %tmp4 = bitcast %struct.Mem* %mem to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp4, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp5 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp5(%struct.Mem* %mem, i32 %conv4)
  %base5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %ctor_base = bitcast i32*** %base5 to i8**
; Store of malloc-ed pointer to base pointer field #3.
; CHECK-CTOR:      Store(Alloc size(Func(Load(GEP(Arg 0)
; CHECK-CTOR-NEXT:                                1)))
; CHECK-CTOR-NEXT:                 (Func(Arg 2)
; CHECK-CTOR-NEXT:                      (Load(Func(Load(Arg 2))))))
; CHECK-CTOR-NEXT:      (GEP(Arg 0)
; CHECK-CTOR-NEXT:           3)
; CHECK-CTOR-NEXT: store i8* %call, i8** %ctor_base, align 8
  store i8* %call, i8** %ctor_base, align 8
  ret void
}

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #4

define void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* nocapture %this) unnamed_addr #2 comdat align 2 {
entry:
; Removed for simplicity
  ret void
}

; void set(int i, S val) { base[i] = val; }
; CHECK-SET-LABEL: @_ZN3ArrIPiE3setEiS0_
define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* nocapture readonly %this, i32 %i, i32* %set_val) #5 comdat align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp1 = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp1, i64 %idxprom
; Write parameter into array, address depends on base pointer (field #3) and parameter.
; CHECK-SET:      Store(Arg 2)
; CHECK-SET-NEXT:      (Func(Arg 1)
; CHECK-SET-NEXT:           (Load(GEP(Arg 0)
; CHECK-SET-NEXT:                     3)))
; CHECK-SET-NEXT:  store i32* %set_val, i32** %arrayidx, align 8
  store i32* %set_val, i32** %arrayidx, align 8
  ret void
}

; void add(const S &e) {
;   realloc(1);
;   base[size] = e;
;   ++size;
; }
;
; CHECK-ADD-LABEL: @_ZN3ArrIPiE3addERKS0_
define void @_ZN3ArrIPiE3addERKS0_(%struct.Arr* nocapture %this, i32** nocapture readonly dereferenceable(8) %e) #2 comdat align 2 {
entry:
; Call to method
; CHECK-ADD:      Known call (Func(Arg 0))
; CHECK-ADD-NEXT: call void @_ZN3ArrIPiE7reallocEi(%struct.Arr* %this, i32 1)
  call void @_ZN3ArrIPiE7reallocEi(%struct.Arr* %this, i32 1)
  %tmp1 = load i32*, i32** %e, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp2 = load i32**, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp3 = load i32, i32* %size, align 8
  %idxprom = sext i32 %tmp3 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp2, i64 %idxprom
; Write parameter into end of array.
; CHECK-ADD:      Store(Load(Arg 1))
; CHECK-ADD-NEXT:      (Func(Load(GEP(Arg 0)
; CHECK-ADD-NEXT:                     3))
; CHECK-ADD-NEXT:           (Load(GEP(Arg 0)
; CHECK-ADD-NEXT:                     4)))
; CHECK-ADD-NEXT: store i32* %tmp1, i32** %arrayidx, align 8
  store i32* %tmp1, i32** %arrayidx, align 8
  %size2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp4 = load i32, i32* %size2, align 8
  %new_size = add nsw i32 %tmp4, 1
; Update iteger size field
; CHECK-ADD:      Store(Func(Load(GEP(Arg 0)
; CHECK-ADD-NEXT:                     4)))
; CHECK-ADD-NEXT:      (GEP(Arg 0)
; CHECK-ADD-NEXT:           4)
; CHECK-ADD-NEXT: store i32 %new_size, i32* %size2, align 8
  store i32 %new_size, i32* %size2, align 8
  ret void
}

;   void realloc(int inc) {
;     if (size + inc <= capacity)
;       return;
;
;     S *new_base = (S *)mem->allocate((size + inc) * sizeof(S));
;     capacity = size + inc;
;     for (int i = 0; i < size; ++i) {
;       new_base[i] = base[i];
;     }
;     mem->deallocate(base);
;     base = new_base;
;   }
; CHECK-REALLOC-LABEL: @_ZN3ArrIPiE7reallocEi
define void @_ZN3ArrIPiE7reallocEi(%struct.Arr* nocapture %this, i32 %inc) #2 comdat align 2 {
entry:
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp = load i32, i32* %size, align 8
  %add = add nsw i32 %tmp, %inc
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp2 = load i32, i32* %capacity, align 8
  %cmp = icmp sle i32 %add, %tmp2
; Some function  of %inc and 1st and 4th integer fields.
; CHECK-REALLOC: %cmp = icmp sle i32 %add, %tmp2
; CHECK-REALLOC-NEXT: Func(Arg 1)
; CHECK-REALLOC-NEXT:     (Load(GEP(Arg 0)
; CHECK-REALLOC-NEXT:               1))
; CHECK-REALLOC-NEXT:     (Load(GEP(Arg 0)
; CHECK-REALLOC-NEXT:               4))
; CHECK-REALLOC-NEXT: br i1 %cmp, label %if.then, label %if.end
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp3 = load %struct.Mem*, %struct.Mem** %mem, align 8
  %size2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp4 = load i32, i32* %size2, align 8
  %add3 = add nsw i32 %tmp4, %inc
  %conv = sext i32 %add3 to i64
  %mul = mul i64 %conv, 8
  %conv4 = trunc i64 %mul to i32
  %tmp6 = bitcast %struct.Mem* %tmp3 to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp6, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp7 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp7(%struct.Mem* %tmp3, i32 %conv4)
  %tmp8 = bitcast i8* %call to i32**
  %size5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp9 = load i32, i32* %size5, align 8
  %add6 = add nsw i32 %tmp9, %inc
  %capacilty7 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  store i32 %add6, i32* %capacilty7, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc12, %for.inc ]
  %size8 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp12 = load i32, i32* %size8, align 8
  %cmp9 = icmp slt i32 %i.0, %tmp12
  br i1 %cmp9, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp13 = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp13, i64 %idxprom
; Load from base pointer
; CHECK-REALLOC:      Load(Func(Load(GEP(Arg 0)
; CHECK-REALLOC-NEXT:                    3)))
; CHECK-REALLOC-NEXT: %elem_load = load i32*, i32** %arrayidx, align 8
  %elem_load = load i32*, i32** %arrayidx, align 8
  %idxprom10 = sext i32 %i.0 to i64
  %arrayidx11 = getelementptr inbounds i32*, i32** %tmp8, i64 %idxprom10
; Copy some element from array to newly allocated memory.
; CHECK-REALLOC:  %arrayidx11 = getelementptr inbounds i32*, i32** %tmp8, i64 %idxprom10
; CHECK-REALLOC-NEXT: Store(Load(Func(Load(GEP(Arg 0)
; CHECK-REALLOC-NEXT:                          3))))
; CHECK-REALLOC-NEXT:      (Func(Alloc size(Func(Arg 1)
; CHECK-REALLOC-NEXT:                           (Load(GEP(Arg 0)
; CHECK-REALLOC-NEXT:                                     4)))
; CHECK-REALLOC-NEXT:                      (Func(Load(GEP(Arg 0)
; CHECK-REALLOC-NEXT:                                     0))
; CHECK-REALLOC-NEXT:                           (Load(Func(Load(Load(GEP(Arg 0)
; CHECK-REALLOC-NEXT:                                                   0))))))))
; CHECK-REALLOC-NEXT:  store i32* %elem_load, i32** %arrayidx11, align 8
  store i32* %elem_load, i32** %arrayidx11, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc12 = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %mem13 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp19 = load %struct.Mem*, %struct.Mem** %mem13, align 8
  %base14 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp20 = load i32**, i32*** %base14, align 8
  %tmp21 = bitcast i32** %tmp20 to i8*
  %tmp22 = bitcast %struct.Mem* %tmp19 to void (%struct.Mem*, i8*)***
  %vtable15 = load void (%struct.Mem*, i8*)**, void (%struct.Mem*, i8*)*** %tmp22, align 8
  %vfn16 = getelementptr inbounds void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vtable15, i64 1
  %tmp23 = load void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vfn16, align 8
  call void %tmp23(%struct.Mem* %tmp19, i8* %tmp21)
  %base17 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  store i32** %tmp8, i32*** %base17, align 8
  br label %return

return:                                           ; preds = %for.end, %if.then
  ret void
}

; Function Attrs: noinline norecurse nounwind readonly uwtable
; CHECK-GET-LABEL: @_ZN3ArrIPvE3getEi
define i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* nocapture readonly %this, i32 %i) #6 comdat align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i8*, i8** %tmp, i64 %idxprom
; CHECK-GET:      Load(Func(Arg 1)
; CHECK-GET-NEXT:          (Load(GEP(Arg 0)
; CHECK-GET-NEXT:                    3)))
; CHECK-GET-NEXT: %get = load i8*, i8** %arrayidx, align 8
  %get = load i8*, i8** %arrayidx, align 8
; Return is represent as its operand
; CHECK-GET-NEXT: Load(Func(Arg 1)
; CHECK-GET-NEXT:          (Load(GEP(Arg 0)
; CHECK-GET-NEXT:                    3)))
; CHECK-GET-NEXT: ret i8* %get
  ret i8* %get
}

; Simplified
;   F() {
;     f1 = new Arr<int *>(10, nullptr);
;     f2 = new Arr<void *>(10, nullptr);
;     f3 = new Arr1<float *>();
;     f1->set(0, nullptr);
;     f2->get(0);
;     f1->add(nullptr);
;     f2->add(nullptr);
;   }
define void @_ZN1FC2Ev(%class.F* nocapture %this) unnamed_addr #2 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %ref.tmp = alloca i32*, align 8
  %ref.tmp13 = alloca i8*, align 8
  %call = call i8* @_Znwm(i64 32) #7
  %tmp = bitcast i8* %call to %struct.Arr*
  invoke void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %tmp, i32 10, %struct.Mem* null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1

  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  %call5 = call i8* @_Znwm(i64 32) #7
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
  %call10 = call i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* %tmp5, i32 0)
  %f111 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1
  %tmp6 = load %struct.Arr*, %struct.Arr** %f111, align 8
  store i32* null, i32** %ref.tmp, align 8
  call void @_ZN3ArrIPiE3addERKS0_(%struct.Arr* %tmp6, i32** dereferenceable(8) %ref.tmp)
  %f212 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  %tmp7 = load %struct.Arr.0*, %struct.Arr.0** %f212, align 8
  store i8* null, i8** %ref.tmp13, align 8
  ret void

lpad:                                             ; preds = %entry
  %tmp8 = landingpad { i8*, i32 }
          cleanup
  %tmp9 = extractvalue { i8*, i32 } %tmp8, 0
  %tmp10 = extractvalue { i8*, i32 } %tmp8, 1
  call void @_ZdlPv(i8* %call) #8
  br label %eh.resume

lpad3:                                            ; preds = %invoke.cont
  %tmp11 = landingpad { i8*, i32 }
          cleanup
  %tmp12 = extractvalue { i8*, i32 } %tmp11, 0
  %tmp13 = extractvalue { i8*, i32 } %tmp11, 1
  br label %eh.resume

lpad6:                                            ; preds = %invoke.cont4
  %tmp14 = landingpad { i8*, i32 }
          cleanup
  %tmp15 = extractvalue { i8*, i32 } %tmp14, 0
  %tmp16 = extractvalue { i8*, i32 } %tmp14, 1
  call void @_ZdlPv(i8* %call5) #8
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


attributes #0 = { noinline norecurse uwtable }
attributes #1 = { nobuiltin }
attributes #2 = { noinline uwtable }
attributes #3 = { nobuiltin nounwind }
attributes #4 = { argmemonly nounwind }
attributes #5 = { noinline norecurse nounwind uwtable }
attributes #6 = { noinline norecurse nounwind readonly uwtable }
attributes #7 = { builtin }
attributes #8 = { builtin nounwind }
