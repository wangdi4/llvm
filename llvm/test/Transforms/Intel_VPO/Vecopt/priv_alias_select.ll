;
; Check for vectorizer does not fail on select with two private pointers in preheader.
;
; RUN: opt -passes=vplan-vec -vplan-print-after-vpentity-instrs -vplan-force-vf=4 -disable-output %s | FileCheck %s


target triple = "x86_64-pc-linux"

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define void @select_two_privates(i64 %init1, i64 %init2, ptr %ptr, i1 zeroext %pred) {
; CHECK-LABEL:  VPlan after insertion of VPEntities instructions:
; CHECK:          ptr [[VP_PRIV2:%.*]] = allocate-priv i64, OrigAlign = 8
; CHECK-NEXT:     call i64 8 ptr [[VP_PRIV2]] ptr @llvm.lifetime.start.p0
; CHECK-NEXT:     ptr [[VP_PRIV1:%.*]] = allocate-priv i64, OrigAlign = 8
; CHECK-NEXT:     call i64 8 ptr [[VP_PRIV1]] ptr @llvm.lifetime.start.p0
; CHECK-NEXT:     ptr [[VP_SELECT:%.*]] = select i1 [[PRED0:%.*]] ptr [[VP_PRIV1]] ptr [[VP_PRIV2]]

entry:
  %priv1 = alloca i64, align 8
  %priv2 = alloca i64, align 8
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16), "QUAL.OMP.PRIVATE:TYPED"(ptr %priv1, i64 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %priv2, i64 0, i32 1), "QUAL.OMP.UNIFORM"(i64 %init1), "QUAL.OMP.UNIFORM"(i64 %init2), "QUAL.OMP.UNIFORM:TYPED"(ptr %ptr, i64 0, i32 1), "QUAL.OMP.UNIFORM"(i1 %pred) ]
  br label %simd.loop.preheader

simd.loop.preheader:
  %select = select i1 %pred, ptr %priv1, ptr %priv2
  br label %simd.loop.header

simd.loop.header:
  %index = phi i64 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.latch ]
  store i64 %init1, ptr %priv1, align 8
  store i64 %init2, ptr %priv2, align 8
  %load = load i64, ptr %select, align 8
  %gep = getelementptr inbounds i64, ptr %ptr, i64 %index
  store i64 %load, ptr %gep, align 8
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

