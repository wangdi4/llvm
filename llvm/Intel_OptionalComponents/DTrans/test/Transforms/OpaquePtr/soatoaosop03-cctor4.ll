; This is same as soatoaosop03-cctor3.ll except IR in "dummyAlloc"
; is not completely optimized.
; Test verifies that dummyAlloc is treated as dummy allocation function, which
; actually doesn't allocate any memory and just calls _CxxThrowException.

; REQUIRES: asserts

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>'  \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaosop 2>&1 | FileCheck %s

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%struct.Arr.0 = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%"class.std::bad_alloc" = type { %"class.std::exception" }
%"class.std::exception" = type { ptr, %struct.__std_exception_data }
%struct.__std_exception_data = type { ptr, i8 }
%struct.Mem = type { ptr }
%struct.MemImpl = type { ptr, i32, i32, ptr }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

; The following method should be classified as copy-ctor.
;   Arr(const Arr &A) {
;     mem = A.mem;
;     capacilty = A.capacilty;
;     size = A.size;
;     base = (S *)mem->allocate(size + capacilty * sizeof(S));
;     for (int i = 0; i < size; ++i)
;       base[size + i] = A.base[i];
;   }
; The test specifically checks for dummy allocation function.
; CHECK:      Checking array's method _ZN3ArrIPvEC2ERKS1_
; CHECK-NEXT: Classification: CCtor method
;

define void @_ZN3ArrIPvEC2ERKS1_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !17 {
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
  %conv9 = trunc i64 %add to i32
  %call = call ptr @dummyAlloc(ptr nonnull %tmp3, i32 %conv9)
  %tmp8 = bitcast ptr %call to ptr
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
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
  store ptr %tmp11, ptr %arrayidx16, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define internal "intel_dtrans_func_index"="1" ptr @dummyAlloc(ptr "intel_dtrans_func_index"="2" %this, i32 %conv4) !intel.dtrans.func.type !19 {
entry:
  %a = alloca %"class.std::bad_alloc", align 8
  %g10 = getelementptr %"class.std::bad_alloc", ptr %a, i64 0, i32 0
  %g11 = getelementptr inbounds %"class.std::exception", ptr %g10, i64 0, i32 1
  %bc0 = bitcast ptr %g11 to ptr
  %g12 = getelementptr inbounds i8, ptr %bc0, i64 8
  %bc1 = bitcast ptr %g12 to ptr
  store i64 0, ptr %bc1, align 8
  %g3 = getelementptr inbounds %"class.std::bad_alloc", ptr %a, i64 0, i32 0, i32 1, i32 0
  store ptr null, ptr %g3, align 8
  %g2 = getelementptr inbounds %"class.std::bad_alloc", ptr %a, i64 0, i32 0, i32 0
  store ptr null, ptr %g2, align 8
  %bc2 = bitcast ptr %a to ptr
  call void @_CxxThrowException(ptr %bc2, ptr null)
  unreachable
}

declare !intel.dtrans.func.type !21 void @_CxxThrowException(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2")

!intel.dtrans.types = !{!0, !4, !9, !10, !12, !14, !16}

!0 = !{!"S", %struct.Mem zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %struct.Arr.0 zeroinitializer, i32 6, !5, !3, !6, !8, !3, !6}
!5 = !{%struct.Mem zeroinitializer, i32 1}
!6 = !{!"A", i32 4, !7}
!7 = !{i8 0, i32 0}
!8 = !{i8 0, i32 2}
!9 = !{!"S", %struct.MemImpl zeroinitializer, i32 4, !5, !3, !3, !8}
!10 = !{!"S", %"class.std::bad_alloc" zeroinitializer, i32 1, !11}
!11 = !{%"class.std::exception" zeroinitializer, i32 0}
!12 = !{!"S", %"class.std::exception" zeroinitializer, i32 2, !1, !13}
!13 = !{%struct.__std_exception_data zeroinitializer, i32 0}
!14 = !{!"S", %struct.__std_exception_data zeroinitializer, i32 2, !15, !7}
!15 = !{i8 0, i32 1}
!16 = !{!"S", %eh.ThrowInfo zeroinitializer, i32 4, !3, !3, !3, !3}
!17 = distinct !{!18, !18}
!18 = !{%struct.Arr.0 zeroinitializer, i32 1}
!19 = distinct !{!15, !20}
!20 = !{%struct.MemImpl zeroinitializer, i32 1}
!21 = distinct !{!15, !22}
!22 = !{%eh.ThrowInfo zeroinitializer, i32 1}
