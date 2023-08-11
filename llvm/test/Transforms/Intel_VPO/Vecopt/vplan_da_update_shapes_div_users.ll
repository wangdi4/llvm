; Test to verify that DA computes correct vector shapes for divergent users of updated VPInstructions
; that are added to the worklist.

; RUN: opt -passes=vplan-vec -vplan-dump-da -vplan-force-vf=2 -disable-output %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; Check DA results for inner.loopbypass BB. Before the fix, both "add" and "gep" VPInstructions below were
; identified as unit-strided and strided respectively.
; CHECK:       Divergent: [Shape: Random] i64 [[UPDATED_PHI:%.*]] = phi  [ i64 4242, {{BB.*}} ],  [ i64 [[INNER_IV_LIVE_OUT:%.*]], {{BB.*}} ]
; CHECK-NEXT:  Divergent: [Shape: Random] i64 [[DIV_USER_ADD:%.*]] = add i64 [[UPDATED_PHI]] i64 [[OUTER_UNIT_STRIDE:%.*]]
; CHECK-NEXT:  Divergent: [Shape: Random] ptr addrspace(1) [[GEP_USING_ADD:%.*]] = getelementptr inbounds i32, ptr addrspace(1) [[UNI_ADDRESS:.*]] i64 [[DIV_USER_ADD]]
; CHECK-NEXT:  Divergent: [Shape: Random] store i32 [[STORED_VAL:%vp.*]] ptr addrspace(1) [[GEP_USING_ADD]]

define dso_local void @foo(ptr %uni0, ptr %uni1, ptr %uni2, ptr %uni3) {
  %1 = add i64 42, 42
  br label %simd.begin.region

simd.begin.region:                                ; preds = %14
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.22 = load i64, ptr %uni2
  %load.17 = load i64, ptr %uni3
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
  %2 = sext i32 %index to i64
  %add = add nuw i64 %2, %1
  %3 = load ptr addrspace(1), ptr %uni0
  %4 = load ptr addrspace(1), ptr %uni1
  %5 = sub i64 %add, %1
  %6 = icmp ult i64 %5, %load.17
  br i1 %6, label %7, label %25

7:                                               ; preds = %simd.loop
  %8 = load ptr addrspace(1), ptr %uni1
  %9 = getelementptr inbounds i32, ptr addrspace(1) %8, i64 %5
  %10 = load i32, ptr addrspace(1) %9, align 4
  %11 = icmp eq i64 %load.22, 0
  br i1 %11, label %inner.loopbypass, label %innerloop.preheader

innerloop.preheader:                           ; preds = %7
  br label %innerloop.header

innerloop.header:                                     ; preds = %innerloop.preheader, %innerloop.header
  %outer.uni.load = phi i64 [ %21, %innerloop.header ], [ %load.22, %innerloop.preheader ]
  %innerloop.iv = phi i64 [ %20, %innerloop.header ], [ 0, %innerloop.preheader ]
  %12 = lshr i64 %outer.uni.load, 1
  %13 = add i64 %12, %innerloop.iv
  %14 = getelementptr inbounds i32, ptr addrspace(1) %3, i64 %13
  %15 = load i32, ptr addrspace(1) %14, align 4
  %16 = icmp slt i32 %15, %10
  %17 = xor i64 %12, -1
  %18 = add i64 %outer.uni.load, %17
  %19 = add i64 %13, 1
  %20 = select i1 %16, i64 %19, i64 %innerloop.iv
  %21 = select i1 %16, i64 %18, i64 %12
  %22 = icmp eq i64 %21, 0
  br i1 %22, label %inner.loopexit, label %innerloop.header

inner.loopexit: ; preds = %innerloop.header
  %.lcssa = phi i64 [ %20, %innerloop.header ]
  br label %inner.loopbypass

inner.loopbypass: ; preds = %inner.loopexit, %7
  %innerloop.iv.lcssa = phi i64 [ 4242, %7 ], [ %.lcssa, %inner.loopexit ]
  %23 = add i64 %innerloop.iv.lcssa, %5
  %24 = getelementptr inbounds i32, ptr addrspace(1) %4, i64 %23
  store i32 %10, ptr addrspace(1) %24, align 4
  br label %25

25:                                               ; preds = %inner.loopbypass, %simd.loop
  %26 = icmp ult i64 %5, %load.22
  br i1 %26, label %27, label %outer.loopexit

27:                                               ; preds = %25
  %28 = getelementptr inbounds i32, ptr addrspace(1) %3, i64 %5
  %29 = load i32, ptr addrspace(1) %28, align 4
  %factor.i.i = shl i64 %5, 1
  %30 = or i64 %factor.i.i, 1
  %31 = getelementptr inbounds i32, ptr addrspace(1) %4, i64 %30
  store i32 %29, ptr addrspace(1) %31, align 4
  br label %outer.loopexit

outer.loopexit: ; preds = %27, %25
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %outer.loopexit
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
