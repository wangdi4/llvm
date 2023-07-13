; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

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

%struct.Arr.0 = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Mem = type { ptr }

define void @_ZN3ArrIPvEC2ERKS1_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !9 {
entry:
  %mem = getelementptr inbounds %struct.Arr.0, ptr %A, i32 0, i32 0
  %tmp = load ptr, ptr %mem, align 8
  %mem2 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 0
  store ptr %tmp, ptr %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.0, ptr %A, i32 0, i32 1
  %tmp1 = load i32, ptr %capacilty, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 1
  store i32 %tmp1, ptr %capacilty3, align 8
  %size = getelementptr inbounds %struct.Arr.0, ptr %A, i32 0, i32 4
  %tmp2 = load i32, ptr %size, align 8
  %size4 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 4
  store i32 %tmp2, ptr %size4, align 8
  %mem5 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 0
  %tmp3 = load ptr, ptr %mem5, align 8
  %size6 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 4
  %tmp4 = load i32, ptr %size6, align 8
  %conv = sext i32 %tmp4 to i64
  %capacilty7 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 1
  %tmp5 = load i32, ptr %capacilty7, align 8
  %conv8 = sext i32 %tmp5 to i64
  %mul = mul i64 %conv8, 8
  %add = add i64 %conv, %mul
  %call = call ptr @_ZN10MemManager8allocateEl(ptr %tmp3, i64 %add)
  %tmp8 = bitcast ptr %call to ptr
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
; Store newly allocate pointer to base pointer of array.
; Additional checks are needed.
; CHECK:      Store(Func(Alloc size(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                                     4))
; CHECK-NEXT:                           (Load(GEP(Arg 0)
; CHECK-NEXT:                                     1)))
; CHECK-NEXT:                      (Func(Load(GEP(Arg 0)
; CHECK-NEXT:                                     0)))))
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           3)
; CHECK-NEXT: store ptr %tmp8, ptr %base, align 8
  store ptr %tmp8, ptr %base, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %size10 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 4
  %tmp9 = load i32, ptr %size10, align 8
  %cmp = icmp slt i32 %i.0, %tmp9
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %base11 = getelementptr inbounds %struct.Arr.0, ptr %A, i32 0, i32 3
  %tmp10 = load ptr, ptr %base11, align 8
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %tmp10, i64 %idxprom
  %tmp11 = load ptr, ptr %arrayidx, align 8
  %base12 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
  %tmp12 = load ptr, ptr %base12, align 8
  %size13 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 4
  %tmp13 = load i32, ptr %size13, align 8
  %add14 = add nsw i32 %tmp13, %i.0
  %idxprom15 = sext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds ptr, ptr %tmp12, i64 %idxprom15
; Copy element of array in argument 1 to element of array in argument 0.
; CHECK:      Store(Load(Func(Load(GEP(Arg 1)
; CHECK-NEXT:                          3))))
; CHECK-NEXT:      (Func(Load(GEP(Arg 0)
; CHECK-NEXT:                     4))
; CHECK-NEXT:           (Load(GEP(Arg 0)
; CHECK-NEXT:                     3)))
; CHECK-NEXT: store ptr %tmp11, ptr %arrayidx16, align 8
  store ptr %tmp11, ptr %arrayidx16, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1" ptr @_ZN10MemManager8allocateEl(ptr nocapture readnone "intel_dtrans_func_index"="2" %this, i64 %size) align 2 !intel.dtrans.func.type !11 {
entry:
  %call = call ptr @malloc(i64 %size)
  ret ptr %call
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !13 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

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
!11 = distinct !{!12, !5}
!12 = !{i8 0, i32 1}
!13 = distinct !{!12}

; XCHECK: Deps computed: 33, Queries: 53

