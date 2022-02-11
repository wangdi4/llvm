; RUN: opt -passes=instcombine -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture, i64, i1) nounwind

; Verify that instcombine preserves TBAA tags when converting a memcpy into
; a scalar load and store.

%struct.test1 = type { float }

; CHECK: @test
; INTEL_CUSTOMIZATION
; Changed s/i32/float/g due to code in InstCOmbineCAlls.cpp:
; IsGoodStructMemcpy/InstCombinerImpl::GenStructFieldsCopyFromMemcpy
;
; CHECK: %[[LOAD:.*]] = load float, float* %{{.*}}, align 4, !tbaa !0
; CHECK: store float %[[LOAD:.*]], float* %{{.*}}, align 4, !tbaa !0
; END INTEL_CUSTOMIZATION
; CHECK: ret
define void @test1(%struct.test1* nocapture %a, %struct.test1* nocapture %b) {
entry:
  %0 = bitcast %struct.test1* %a to i8*
  %1 = bitcast %struct.test1* %b to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %0, i8* align 4 %1, i64 4, i1 false), !tbaa.struct !3
  ret void
}

%struct.test2 = type { i32 (i8*, i32*, double*)** }

define i32 (i8*, i32*, double*)*** @test2() {
; CHECK-LABEL: @test2(
; CHECK-NOT: memcpy
; CHECK: ret
  %tmp = alloca %struct.test2, align 8
  %tmp1 = bitcast %struct.test2* %tmp to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %tmp1, i8* align 8 undef, i64 8, i1 false), !tbaa.struct !4
  %tmp2 = getelementptr %struct.test2, %struct.test2* %tmp, i32 0, i32 0
  %tmp3 = load i32 (i8*, i32*, double*)**, i32 (i8*, i32*, double*)*** %tmp2
  ret i32 (i8*, i32*, double*)*** %tmp2
}

; INTEL
; tbaa.struct tags that point to non-scalars should not be converted to tbaa.
; They should stay in tbaa.struct form.

%struct.thing = type { [1 x %struct.thing2] }
%struct.thing2 = type { i32, i32 }

@p = dso_local global %struct.thing* null, align 8

define dso_local i64 @test3_intel(%struct.thing* %t) {
; CHECK: [[FIRST:%[0-9]+]] = load i64*{{.*}}@p{{.*}}, !tbaa !0
; CHECK: load i64, i64* [[FIRST]], align 4, !tbaa.struct !3

entry:
  %retval = alloca %struct.thing, align 4
  %t.addr = alloca %struct.thing*, align 8
  store %struct.thing* %t, %struct.thing** %t.addr, align 8, !tbaa !2
  %0 = load %struct.thing*, %struct.thing** @p, align 8, !tbaa !2
  %1 = bitcast %struct.thing* %retval to i8*
  %2 = bitcast %struct.thing* %0 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %1, i8* align 4 %2, i64 8, i1 false), !tbaa.struct !6
  %coerce.dive = getelementptr inbounds %struct.thing, %struct.thing* %retval, i32 0, i32 0
  %3 = bitcast [1 x %struct.thing2]* %coerce.dive to i64*
  %4 = load i64, i64* %3, align 4
  ret i64 %4
}

; CHECK: !0 = !{!1, !1, i64 0}
; CHECK: !1 = !{!"float", !2}

!0 = !{!"Simple C/C++ TBAA"}
!1 = !{!"omnipotent char", !0}
!2 = !{!5, !5, i64 0}
!3 = !{i64 0, i64 4, !2}
!4 = !{i64 0, i64 8, null}
!5 = !{!"float", !0}

; INTEL
; Add some non-scalar nodes in tbaa.struct form.
!6 = !{i64 0, i64 8, !7}
!7 = !{!8, !8, i64 0}
!8 = !{!"array@_ZTSA1_6thing2", !9, i64 0}
!9 = !{!"struct@_ZTS6thing2", !10, i64 0, !10, i64 4}
!10 = !{!"int", !4, i64 0}
