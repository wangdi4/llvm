; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-mv-variable-stride -print-after=hir-mv-variable-stride  -print-before=hir-mv-variable-stride < %s 2>&1 | FileCheck %s 
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-mv-variable-stride,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; RUN: opt -opaque-pointers -hir-ssa-deconstruction -hir-temp-cleanup -hir-mv-variable-stride -print-after=hir-mv-variable-stride  -print-before=hir-mv-variable-stride < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-mv-variable-stride,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
; 
; Check MV does not happen when a variable stride is an undef.


; subroutine BCint(BC,iBC)
; 
;     integer, pointer :: p(:,:)
; 
;     integer :: nBCt(5), iBC, BC(iBC,iBC), i,j
; 
;     do i =1,5
; 
;         do j=2,10
; 
;             if(p(j,4).gt.nBCt(5)) then
; 
;                 BC(nbct(i),3:5)= p(j-1,1:3)
;                 exit
; 
;             endif
;         enddo
;     enddo
;     return
; end
 
; CHECK:Function: bcint_

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:               |   + DO i2 = 0, 8, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:               |   |   %indvars.iv.out = i2 + 2;
; CHECK:               |   |   if ((null)[4][i2 + 2] > %"bcint_$NBCT[]_fetch")
; CHECK:               |   |   {
; CHECK:               |   |      goto bb37_then;
; CHECK:               |   |   }
; CHECK:               |   + END LOOP
; CHECK:               |   
; CHECK:               |   goto bb3;
; CHECK:               |   bb37_then:
; CHECK:               |   %"bcint_$NBCT[]11_fetch" = (@"bcint_$NBCT"){{.*}}[i1];
; CHECK:               |   %int_sext23 = 4294967296 * %indvars.iv.out + -4294967296  >>  32;
; CHECK:               |
; CHECK:               |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   |   (%"bcint_$BC")[i2 + 2][%"bcint_$NBCT[]11_fetch"] = (null)[i2 + 1][%int_sext23];
; CHECK:               |   + END LOOP
; CHECK:               |   
; CHECK:               |   bb3:
; CHECK:               + END LOOP
; CHECK:         END REGION

; CHECK:Function: bcint_

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 8, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:              |   |   %indvars.iv.out = i2 + 2;
; CHECK:              |   |   if ((null)[4][i2 + 2] > %"bcint_$NBCT[]_fetch")
; CHECK:              |   |   {
; CHECK:              |   |      goto bb37_then;
; CHECK:              |   |   }
; CHECK:              |   + END LOOP
; CHECK:              |   
; CHECK:              |   goto bb3;
; CHECK:              |   bb37_then:
; CHECK:              |   %"bcint_$NBCT[]11_fetch" = (@"bcint_$NBCT"){{.*}}[i1];
; CHECK:              |   %int_sext23 = 4294967296 * %indvars.iv.out + -4294967296  >>  32;
; CHECK:              |
; CHECK:              |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:              |   |   (%"bcint_$BC")[i2 + 2][%"bcint_$NBCT[]11_fetch"] = (null)[i2 + 1][%int_sext23];
; CHECK:              |   + END LOOP
; CHECK:              |   
; CHECK:              |   bb3:
; CHECK:              + END LOOP
; CHECK:        END REGION
 
;Module Before HIR
; ModuleID = 'src.f90'
source_filename = "src.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"bcint_$NBCT" = internal unnamed_addr constant [5 x i32] zeroinitializer, align 16

; Function Attrs: nofree nounwind
define void @bcint_(i32* noalias nocapture %"bcint_$BC", i32* noalias nocapture readonly %"bcint_$IBC") local_unnamed_addr #0 {
alloca:
  %"bcint_$IBC_fetch" = load i32, i32* %"bcint_$IBC", align 4
  %int_sext9 = sext i32 %"bcint_$IBC_fetch" to i64
  %mul10 = shl nsw i64 %int_sext9, 2
  %"bcint_$P.addr_a0$_fetch[]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 undef, i64 undef, i32* elementtype(i32) null, i64 4)
  %"bcint_$NBCT[]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([5 x i32], [5 x i32]* @"bcint_$NBCT", i64 0, i64 0), i64 5)
  %"bcint_$NBCT[]_fetch" = load i32, i32* %"bcint_$NBCT[]", align 4
  br label %bb6

bb6:                                              ; preds = %bb3, %alloca
  %indvars.iv88 = phi i64 [ %indvars.iv.next89, %bb3 ], [ 1, %alloca ]
  br label %bb10

bb10:                                             ; preds = %bb76_endif, %bb6
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb76_endif ], [ 2, %bb6 ]
  %"bcint_$P.addr_a0$_fetch[][]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 undef, i64 undef, i32* elementtype(i32) %"bcint_$P.addr_a0$_fetch[]", i64 %indvars.iv)
  %"bcint_$P.addr_a0$_fetch[][]_fetch" = load i32, i32* %"bcint_$P.addr_a0$_fetch[][]", align 4
  %rel = icmp sgt i32 %"bcint_$P.addr_a0$_fetch[][]_fetch", %"bcint_$NBCT[]_fetch"
  br i1 %rel, label %bb37_then, label %bb76_endif

bb67:                                             ; preds = %bb67, %bb37_then
  %"var$7.084" = phi i64 [ 3, %bb37_then ], [ %add43, %bb67 ]
  %"var$5.083" = phi i64 [ 1, %bb37_then ], [ %add47, %bb67 ]
  %"bcint_$P.addr_a0$_fetch[]36" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 undef, i64 undef, i32* elementtype(i32) null, i64 %"var$5.083")
  %"bcint_$P.addr_a0$_fetch[]36[]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 undef, i64 undef, i32* elementtype(i32) %"bcint_$P.addr_a0$_fetch[]36", i64 %int_sext23)
  %"bcint_$P.addr_a0$_fetch[]36[]_fetch" = load i32, i32* %"bcint_$P.addr_a0$_fetch[]36[]", align 4
  %"bcint_$BC[]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %mul10, i32* elementtype(i32) %"bcint_$BC", i64 %"var$7.084")
  %"bcint_$BC[][]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %"bcint_$BC[]", i64 %int_sext13)
  store i32 %"bcint_$P.addr_a0$_fetch[]36[]_fetch", i32* %"bcint_$BC[][]", align 4
  %add43 = add nuw nsw i64 %"var$7.084", 1
  %add47 = add nuw nsw i64 %"var$5.083", 1
  %exitcond87 = icmp eq i64 %add43, 6
  br i1 %exitcond87, label %bb3.loopexit, label %bb67

bb37_then:                                        ; preds = %bb10
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %bb10 ]
  %"bcint_$NBCT[]11" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([5 x i32], [5 x i32]* @"bcint_$NBCT", i64 0, i64 0), i64 %indvars.iv88)
  %"bcint_$NBCT[]11_fetch" = load i32, i32* %"bcint_$NBCT[]11", align 4
  %int_sext13 = sext i32 %"bcint_$NBCT[]11_fetch" to i64
  %sub = shl i64 %indvars.iv.lcssa, 32
  %sext = add i64 %sub, -4294967296
  %int_sext23 = ashr exact i64 %sext, 32
  br label %bb67

bb76_endif:                                       ; preds = %bb10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %bb3.loopexit93, label %bb10

bb3.loopexit:                                     ; preds = %bb67
  br label %bb3

bb3.loopexit93:                                   ; preds = %bb76_endif
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit93, %bb3.loopexit
  %indvars.iv.next89 = add nuw nsw i64 %indvars.iv88, 1
  %exitcond90 = icmp eq i64 %indvars.iv.next89, 6
  br i1 %exitcond90, label %bb4, label %bb6

bb4:                                              ; preds = %bb3
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

attributes #0 = { nofree nounwind "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
