;
; Check for vectorizer does not fail on select with two private pointers in preheader.
;
; RUN: opt -enable-new-pm=0 -vplan-print-after-vpentity-instrs -vplan-vec -vplan-force-vf=4 -disable-output %s | FileCheck %s
; RUN: opt  -passes=vplan-vec -vplan-print-after-vpentity-instrs -vplan-force-vf=4 -disable-output %s | FileCheck %s


target triple = "x86_64-pc-linux"

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define void @select_two_privates(i64 %init1, i64 %init2, i64* %ptr, i1 zeroext %pred) {
; CHECK-LABEL:  VPlan after insertion of VPEntities instructions:
; CHECK:          double* [[VP_PRIV2:%.*]] = allocate-priv double*, OrigAlign = 8
; CHECK-NEXT:     i8* [[VP_PRIV2_BCAST:%.*]] = bitcast double* [[VP_PRIV2]]
; CHECK-NEXT:     call i64 8 i8* [[VP_PRIV2_BCAST]] void (i64, i8*)* @llvm.lifetime.start.p0i8
; CHECK-NEXT:     i64* [[VP_PRIV1:%.*]] = allocate-priv i64*, OrigAlign = 8
; CHECK-NEXT:     i8* [[VP_PRIV1_BCAST:%.*]] = bitcast i64* [[VP_PRIV1]]
; CHECK-NEXT:     call i64 8 i8* [[VP_PRIV1_BCAST]] void (i64, i8*)* @llvm.lifetime.start.p0i8
; CHECK-NEXT:     i64* [[VP_P2_ALIAS:%.*]] = bitcast double* [[VP_PRIV2]]
; CHECK-NEXT:     i64* [[VP_SELECT:%.*]] = select i1 [[PRED0:%.*]] i64* [[VP_PRIV1]] i64* [[VP_P2_ALIAS]]

entry:
  %priv1 = alloca i64, align 8
  %priv2 = alloca double, align 8
  %priv3 = alloca double, align 8
  %p2_alias = bitcast double* %priv2 to i64*

  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16), "QUAL.OMP.PRIVATE"(i64* %priv1), "QUAL.OMP.PRIVATE"(double* %priv2), "QUAL.OMP.UNIFORM"(i64 %init1), "QUAL.OMP.UNIFORM"(i64 %init2), "QUAL.OMP.UNIFORM"(i64* %ptr), "QUAL.OMP.UNIFORM"(i1 %pred) ]
  br label %simd.loop.preheader

simd.loop.preheader:
  %select = select i1 %pred, i64* %priv1, i64* %p2_alias
  br label %simd.loop.header

simd.loop.header:
  %index = phi i64 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.latch ]
  store i64 %init1, i64* %priv1, align 8
  store i64 %init2, i64* %p2_alias, align 8
  %load = load i64, i64* %select, align 8
  %gep = getelementptr inbounds i64, i64* %ptr, i64 %index
  store i64 %load, i64* %gep, align 8
  br label %simd.loop.latch

simd.loop.latch:
  %indvar = add nuw i64 %index, 1
  %cond = icmp ult i64 %indvar, 16
  br i1 %cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret void
}

