; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-TY %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

; This test checks various approximations for side effects in copy-ctor-like function.
;   Arr(const Arr &A) {
;     mem = A.mem;
;     capacilty = A.capacilty;
;     size = A.size;
;     base = (S *)mem->allocate(size + capacilty * sizeof(S));
;     for (int i = 0; i < size; ++i)
;       base[size + i] = A.base[i];
;   }
; Check that approximations work as expected.
; CHECK-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-WF-NOT: ; Func(GEP
define void @_ZN3ArrIPvEC2ERKS1_(%struct.Arr.0*  "intel_dtrans_func_index"="1"  %this, %struct.Arr.0*  "intel_dtrans_func_index"="2" %A)  !intel.dtrans.func.type !9 {
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
  %call = call i8* @_ZN10MemManager8allocateEl(%struct.Mem* %tmp3, i64 %add)
  %tmp8 = bitcast i8* %call to i8**
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
; Store newly allocate pointer to base pointer of array.
; Additional checks are needed.
; CHECK-TY:      Store(Func(Alloc size(Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                                     4))
; CHECK-TY-NEXT:                           (Load(GEP(Arg 0)
; CHECK-TY-NEXT:                                     1)))
; CHECK-TY-NEXT:                      (Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                                     0)))))
; CHECK-TY-NEXT:      (GEP(Arg 0)
; CHECK-TY-NEXT:           3)
; CHECK-TY-NEXT: store i8** %tmp8, i8*** %base, align 8
; CHECK-OP:      Store(Func(Alloc size(Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                                     4))
; CHECK-OP-NEXT:                           (Load(GEP(Arg 0)
; CHECK-OP-NEXT:                                     1)))
; CHECK-OP-NEXT:                      (Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                                     0)))))
; CHECK-OP-NEXT:      (GEP(Arg 0)
; CHECK-OP-NEXT:           3)
; CHECK-OP-NEXT: store ptr %tmp8, ptr %base, align 8
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
; Copy element of array in argument 1 to element of array in argument 0.
; CHECK-TY:      Store(Load(Func(Load(GEP(Arg 1)
; CHECK-TY-NEXT:                          3))))
; CHECK-TY-NEXT:      (Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                     4))
; CHECK-TY-NEXT:           (Load(GEP(Arg 0)
; CHECK-TY-NEXT:                     3)))
; CHECK-TY-NEXT: store i8* %tmp11, i8** %arrayidx16, align 8
; CHECK-OP:      Store(Load(Func(Load(GEP(Arg 1)
; CHECK-OP-NEXT:                          3))))
; CHECK-OP-NEXT:      (Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                     4))
; CHECK-OP-NEXT:           (Load(GEP(Arg 0)
; CHECK-OP-NEXT:                     3)))
; CHECK-OP-NEXT: store ptr %tmp11, ptr %arrayidx16, align 8
  store i8* %tmp11, i8** %arrayidx16, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1"  i8* @_ZN10MemManager8allocateEl(%struct.Mem* "intel_dtrans_func_index"="2" nocapture readnone %this, i64 %size) align 2 !intel.dtrans.func.type !13 {
entry:
  %call = call i8* @malloc(i64 %size)
  ret i8* %call
}

declare !intel.dtrans.func.type !12 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

; XCHECK: Deps computed: 33, Queries: 53

!intel.dtrans.types = !{!0, !4}

!0 = !{!"S", %struct.Mem zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %struct.Arr.0 zeroinitializer, i32 6, !5, !3, !6, !8, !3, !6}
!5 = !{%struct.Mem zeroinitializer, i32 1}
!6 = !{!"A", i32 4, !7}
!7 = !{i8 0, i32 0}
!8 = !{i32 0, i32 2}
!9 = distinct !{!10, !10}
!10 = !{%struct.Arr.0 zeroinitializer, i32 1}
!11 = !{i8 0, i32 1}
!12 = distinct !{!11}
!13 = distinct !{!11, !5}
