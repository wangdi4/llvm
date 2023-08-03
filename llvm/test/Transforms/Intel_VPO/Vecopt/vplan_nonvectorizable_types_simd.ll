; REQUIRES: asserts
; RUN: opt -passes=vplan-vec -vplan-force-vf=2 -S %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"EXTENT$.btINTVL" = type { i32, i32 }
%"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Serialization of select inst with non-vectorizable operands
; Serialization code starts from here: extracts followed by selects
; CHECK: vector.body:
; CHECK:        [[TMP1:%.*]] = add nsw <2 x i32> %wide.masked.gather, <i32 1, i32 1>
; CHECK-NEXT:   [[SERIAL_EXTRACTVALUE1:%.*]] = extractelement <2 x i32> [[TMP1]], i32 1
; CHECK-NEXT:   [[SERIAL_EXTRACTVALUE0:%.*]] = extractelement <2 x i32> [[TMP1]], i32 0
; CHECK-NEXT:   [[SERIAL_INSERTVALUE1:%.*]] = insertvalue %"EXTENT$.btINTVL" undef, i32 [[SERIAL_EXTRACTVALUE0]], 0
; CHECK-NEXT:   [[SERIAL_INSERTVALUE0:%.*]] = insertvalue %"EXTENT$.btINTVL" undef, i32 [[SERIAL_EXTRACTVALUE1]], 0
; CHECK-NEXT:   [[SERIAL_INSERTVALUE3:%.*]] = insertvalue %"EXTENT$.btINTVL" [[SERIAL_INSERTVALUE1]], i32 [[SERIAL_EXTRACTVALUE0]], 1
; CHECK-NEXT:   [[SERIAL_INSERTVALUE2:%.*]] = insertvalue %"EXTENT$.btINTVL" [[SERIAL_INSERTVALUE0]], i32 [[SERIAL_EXTRACTVALUE1]], 1
; CHECK-NEXT:   [[TMP2:%.*]]  = add nsw <2 x i32> %wide.masked.gather, <i32 2, i32 2>
; CHECK-NEXT:   [[SERIAL_EXTRACTVALUE3:%.*]] = extractelement <2 x i32> [[TMP2]], i32 1
; CHECK-NEXT:   [[SERIAL_EXTRACTVALUE2:%.*]] = extractelement <2 x i32> [[TMP2]], i32 0
; CHECK-NEXT:   [[SERIAL_INSERTVALUE5:%.*]] = insertvalue %"EXTENT$.btINTVL" undef, i32 [[SERIAL_EXTRACTVALUE2]], 0
; CHECK-NEXT:   [[SERIAL_INSERTVALUE4:%.*]] = insertvalue %"EXTENT$.btINTVL" undef, i32 [[SERIAL_EXTRACTVALUE3]], 0
; CHECK-NEXT:   [[SERIAL_INSERTVALUE7:%.*]] = insertvalue %"EXTENT$.btINTVL" [[SERIAL_INSERTVALUE5]], i32 [[SERIAL_EXTRACTVALUE2]], 1
; CHECK-NEXT:   [[SERIAL_INSERTVALUE6:%.*]] = insertvalue %"EXTENT$.btINTVL" [[SERIAL_INSERTVALUE4]], i32 [[SERIAL_EXTRACTVALUE3]], 1
; CHECK-NEXT:   [[TMP3:%.*]] = icmp sgt <2 x i32> %wide.masked.gather, <i32 1, i32 1>
; CHECK-NEXT:   [[SERIAL_EXTRACTVALUE5:%.*]] = extractelement <2 x i1> [[TMP3]], i32 1
; CHECK-NEXT:   [[SERIAL_EXTRACTVALUE4:%.*]] = extractelement <2 x i1> [[TMP3]], i32 0
; CHECK-NEXT:   [[TMP4:%.*]] = select i1 [[SERIAL_EXTRACTVALUE4]], %"EXTENT$.btINTVL" [[SERIAL_INSERTVALUE3]], %"EXTENT$.btINTVL" [[SERIAL_INSERTVALUE7]]
; CHECK-NEXT:   [[TMP5:%.*]] = select i1 [[SERIAL_EXTRACTVALUE5]], %"EXTENT$.btINTVL" [[SERIAL_INSERTVALUE2]], %"EXTENT$.btINTVL" [[SERIAL_INSERTVALUE6]]
; CHECK-NEXT:   [[SERIAL_EXTRACTVALUE6:%.*]] = extractvalue %"EXTENT$.btINTVL" [[TMP4]], 0
; CHECK-NEXT:   [[TMP6:%.*]] = insertelement <2 x i32> undef, i32 [[SERIAL_EXTRACTVALUE6]], i32 0
; CHECK-NEXT:   [[SERIAL_EXTRACTVALUE7:%.*]] = extractvalue %"EXTENT$.btINTVL" [[TMP5]], 0
; CHECK-NEXT:   [[TMP7:%.*]] = insertelement <2 x i32> [[TMP6]], i32 [[SERIAL_EXTRACTVALUE7]], i32 1
; CHECK-NEXT:   [[SERIAL_EXTRACTVALUE8:%.*]] = extractvalue %"EXTENT$.btINTVL" [[TMP4]], 1
; CHECK-NEXT:   [[TMP8:%.*]] = insertelement <2 x i32> undef, i32 [[SERIAL_EXTRACTVALUE8]], i32 0
; CHECK-NEXT:   [[SERIAL_EXTRACTVALUE9:%.*]] = extractvalue %"EXTENT$.btINTVL" [[TMP5]], 1
; CHECK-NEXT:   [[TMP9:%.*]] = insertelement <2 x i32> [[TMP8]], i32 [[SERIAL_EXTRACTVALUE9]], i32 1

; Function Attrs: nofree nosync nounwind uwtable
define i32 @extent_mp_foo_nd_(ptr noalias nocapture readonly dereferenceable(72) "assumed_shape" "ptrnoalias" %A) local_unnamed_addr #2 {
alloca_2:
  %"A.addr_a0$_fetch.8" = load ptr, ptr %A, align 1
  %"A.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$", ptr %A, i64 0, i32 6, i64 0, i32 1
  %"A.dim_info$.spacing$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"A.dim_info$.spacing$", i32 0)
  %"A.dim_info$.spacing$[]_fetch.9" = load i64, ptr %"A.dim_info$.spacing$[]", align 1
  %"A.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$", ptr %A, i64 0, i32 6, i64 0, i32 0
  %"A.dim_info$.extent$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"A.dim_info$.extent$", i32 0)
  %"A.dim_info$.extent$[]_fetch.10" = load i64, ptr %"A.dim_info$.extent$[]", align 1
  %0 = icmp sgt i64 %"A.dim_info$.extent$[]_fetch.10", 0
  %slct.3 = select i1 %0, i64 %"A.dim_info$.extent$[]_fetch.10", i64 0
  %"var$6" = alloca i32, i64 %slct.3, align 4
  %rel.4.not11 = icmp slt i64 %"A.dim_info$.extent$[]_fetch.10", 1
  br i1 %rel.4.not11, label %loop_exit11, label %loop_body10.preheader

loop_body10.preheader:                            ; preds = %alloca_2
  %1 = add nuw nsw i64 %"A.dim_info$.extent$[]_fetch.10", 1
  br label %loop_body10.preheader.1

loop_body10.preheader.1:                            ; preds = %loop_body10.preheader
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop_body10

loop_body10:                                      ; preds = %loop_body10.preheader.1, %loop_body10
  %"$loop_ctr.012" = phi i64 [ %add.3, %loop_body10 ], [ 1, %loop_body10.preheader.1 ]
  %"A.addr_a0$_fetch.8[]" = tail call ptr @"llvm.intel.subscript.p0s_EXTENT$.btINTVLs.i64.i64.p0s_EXTENT$.btINTVLs.i64"(i8 0, i64 1, i64 %"A.dim_info$.spacing$[]_fetch.9", ptr elementtype(%"EXTENT$.btINTVL") %"A.addr_a0$_fetch.8", i64 %"$loop_ctr.012")
  %"A.FIRST$_fetch.1.i" = load i32, ptr %"A.addr_a0$_fetch.8[]", align 1
  %add.1.i = add nsw i32 %"A.FIRST$_fetch.1.i", 1
  %"var$2_fetch.3.fca.0.insert.i" = insertvalue %"EXTENT$.btINTVL" undef, i32 %add.1.i, 0
  %"var$2_fetch.3.fca.1.insert.i" = insertvalue %"EXTENT$.btINTVL" %"var$2_fetch.3.fca.0.insert.i", i32 %add.1.i, 1
  %add.2.i = add nsw i32 %"A.FIRST$_fetch.1.i", 2
  %"var$3_fetch.5.fca.0.insert.i" = insertvalue %"EXTENT$.btINTVL" undef, i32 %add.2.i, 0
  %"var$3_fetch.5.fca.1.insert.i" = insertvalue %"EXTENT$.btINTVL" %"var$3_fetch.5.fca.0.insert.i", i32 %add.2.i, 1
  %rel.1.i = icmp sgt i32 %"A.FIRST$_fetch.1.i", 1
  %slct.1.i = select i1 %rel.1.i, %"EXTENT$.btINTVL" %"var$2_fetch.3.fca.1.insert.i", %"EXTENT$.btINTVL" %"var$3_fetch.5.fca.1.insert.i"
  %slct.1.fca.0.extract.i = extractvalue %"EXTENT$.btINTVL" %slct.1.i, 0
  %slct.1.fca.1.extract.i = extractvalue %"EXTENT$.btINTVL" %slct.1.i, 1
  %slct.1.fca.extractsum.i = add nsw i32 %slct.1.fca.0.extract.i, %slct.1.fca.1.extract.i
  %"var$6[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %"var$6", i64 %"$loop_ctr.012")
  store i32 %slct.1.fca.extractsum.i, ptr %"var$6[]", align 1
  %add.3 = add nuw nsw i64 %"$loop_ctr.012", 1
  %exitcond = icmp eq i64 %add.3, %1
  br i1 %exitcond, label %loop_exit11.loopexit.1, label %loop_body10

loop_exit11.loopexit.1:                             ; preds = %loop_body10
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  br label %loop_exit11.loopexit

loop_exit11.loopexit:                             ; preds = %loop_exit11.loopexit.1
  br label %loop_exit11

loop_exit11:                                      ; preds = %loop_exit11.loopexit, %alloca_2
  %2 = shl i64 %slct.3, 63
  %sext = ashr exact i64 %2, 63
  %slct.4 = trunc i64 %sext to i32
  ret i32 %slct.4
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @"llvm.intel.subscript.p0s_EXTENT$.btINTVLs.i64.i64.p0s_EXTENT$.btINTVLs.i64"(i8, i64, i64, ptr, i64) #3

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
