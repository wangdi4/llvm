;
; Test to check allzero bypass insertion for masked vector function,
; non-optimized IR (there is an empty block for the false branch, see BB4 below)
;
; REQUIRES: asserts
;
; RUN: opt -disable-output -passes=vplan-vec -vplan-print-after-all-zero-bypass -vplan-all-zero-bypass-region-threshold=10000 %s 2>&1 | FileCheck %s

; CHECK:        i1 [[VP_COND:%.*]] = fcmp une float [[VP0:%.*]] float 0.000000e+00
; CHECK-NEXT:   i1 [[VP_NOT:%.*]] = not i1 [[VP_COND]]
; CHECK-NEXT:   br [[BB4:BB[0-9]+]]
;
; CHECK:      [[BB4]]:
; CHECK-NEXT:   i1 [[VP_NPRED:%.*]] = block-predicate i1 [[VP_NOT]]
; CHECK-NEXT:   br all.zero.bypass.begin10
;
; CHECK:      all.zero.bypass.begin10:
; CHECK-NEXT:   i1 [[VP_ZCHECK:%.*]] = all-zero-check i1 [[VP_COND]]
; CHECK-NEXT:   br i1 [[VP_ZCHECK]], all.zero.bypass.end12, [[BB3:BB[0-9]+]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(readwrite) uwtable
define dso_local noundef nofpclass(nan inf) <16 x float> @_ZGVeM16vv__Z3foolPf(<16 x i64> noundef %k, <16 x ptr> noundef %pf, <16 x float> %mask) local_unnamed_addr #1 {
entry:
  %vec.k = alloca <16 x i64>, align 128
  %vec.pf = alloca <16 x ptr>, align 128
  %vec.mask = alloca <16 x float>, align 64
  %vec.retval = alloca <16 x float>, align 64
  store <16 x i64> %k, ptr %vec.k, align 128
  store <16 x ptr> %pf, ptr %vec.pf, align 128
  store <16 x float> %mask, ptr %vec.mask, align 64
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %simd.loop.latch, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.latch ]
  %mask.gep = getelementptr float, ptr %vec.mask, i32 %index
  %mask.parm = load float, ptr %mask.gep, align 4
  %mask.cond = fcmp une float %mask.parm, 0.000000e+00
  br i1 %mask.cond, label %simd.loop.then, label %simd.loop.else

simd.loop.else:                                   ; preds = %simd.loop.header
  br label %simd.loop.latch

simd.loop.then:                                   ; preds = %simd.loop.header
  %vec.k.gep = getelementptr i64, ptr %vec.k, i32 %index
  %vec.k.elem = load i64, ptr %vec.k.gep, align 8
  %vec.pf.gep = getelementptr ptr, ptr %vec.pf, i32 %index
  %vec.pf.elem = load ptr, ptr %vec.pf.gep, align 8
  %arrayidx = getelementptr inbounds float, ptr %vec.pf.elem, i64 %vec.k.elem
  %0 = load float, ptr %arrayidx, align 4
  %vec.retval.gep = getelementptr float, ptr %vec.retval, i32 %index
  store float %0, ptr %vec.retval.gep, align 4
  br label %simd.loop.latch

simd.loop.latch:                                  ; preds = %simd.loop.else, %simd.loop.then
  %indvar = add nuw nsw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region, !llvm.loop !8

simd.end.region:                                  ; preds = %simd.loop.latch
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  %vec.ret = load <16 x float>, ptr %vec.retval, align 64
  ret <16 x float> %vec.ret
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.unroll.disable"}
