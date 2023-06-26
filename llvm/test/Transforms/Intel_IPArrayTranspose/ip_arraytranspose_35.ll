; RUN: opt -whole-program-assume -passes=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S < %s 2>&1 | FileCheck %s

; Check the IR for the transformation of "new" unoptimized SCEVs 

; CHECK:  bb246:  
; CHECK:  %[[IV:[A-Za-z0-9]+]] = phi i64
; CHECK:  %[[R1:[A-Za-z0-9]+]] = shl nuw nsw i64 %[[IV]], 19
; CHECK:  %[[R2:[A-Za-z0-9]+]] = add nuw i64 %[[R1]], 5140119552
; CHECK:  bb254:
; CHECK:  %[[PHI0:[A-Za-z0-9]+]] = phi i64
; CHECK:  %[[R3:[A-Za-z0-9]+]] = shl nuw nsw i64 %[[PHI0]], 11
; CHECK:  %[[R4:[A-Za-z0-9]+]] = add i64 %[[R2]], %[[R3]]
; CHECK:  bb263:
; CHECK:  %[[PHI1:[A-Za-z0-9]+]] = phi i64
; CHECK:  %[[R5:[A-Za-z0-9]+]] = shl nuw nsw i64 %[[PHI1]], 3
; CHECK:  %[[R6:[A-Za-z0-9]+]] = add i64 %[[R4]], %[[R5]]
; CHECK:  %[[SCEV0:[A-Za-z0-9]+]] = getelementptr i8, ptr %i73, i64 %[[R6]]
; CHECK:  %[[I0:[A-Za-z0-9]+]] = load i32, ptr %[[SCEV0]], align 4
; CHECK:  %[[I1:[A-Za-z0-9]+]] = or i32 %[[I0]], 2
; CHECK:  store i32 %[[I1]], ptr %[[SCEV0]], align 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #0

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local void @free(ptr allocptr nocapture noundef) local_unnamed_addr #1

; Function Attrs: nofree norecurse nounwind uwtable
define internal fastcc void @LBM_loadObstacleFile(ptr nocapture noundef %arg, ptr nocapture noundef readonly %arg1) unnamed_addr #2 {
bb:
  br label %bb2

bb2:
  %i3 = phi i64 [ 0, %bb ], [ %i29, %bb27 ]
  %i4 = shl nsw i64 %i3, 16
  br label %bb5

bb5:
  %i6 = phi i64 [ 0, %bb2 ], [ %i25, %bb23 ]
  %i7 = shl nsw i64 %i6, 8
  %i8 = add nuw nsw i64 %i7, %i4
  br label %bb9

bb9:
  %i10 = phi i64 [ 0, %bb5 ], [ %i21, %bb20 ]
  %i14 = add nuw nsw i64 %i8, %i10
  %i15 = mul nuw nsw i64 %i14, 20
  %i16 = add nuw nsw i64 %i15, 19
  %i17 = getelementptr inbounds double, ptr %arg, i64 %i16
  %i18 = load i32, ptr %i17, align 4
  %i19 = or i32 %i18, 1
  store i32 %i19, ptr %i17, align 4
  br label %bb20

bb20:
  %i21 = add nuw nsw i64 %i10, 1
  %i22 = icmp eq i64 %i21, 256
  br i1 %i22, label %bb23, label %bb9

bb23:
  %i25 = add nuw nsw i64 %i6, 1
  %i26 = icmp eq i64 %i25, 256
  br i1 %i26, label %bb27, label %bb5

bb27:
  %i29 = add nuw nsw i64 %i3, 1
  %i30 = icmp eq i64 %i29, 512
  br i1 %i30, label %bb31, label %bb2

bb31:
  ret void
}

define dso_local i32 @main(i32 noundef %arg, ptr nocapture noundef readonly %arg1) local_unnamed_addr #3 {
bb62:
  %i65 = alloca [3 x [32 x i8]], align 16
  %i69 = getelementptr inbounds [3 x [32 x i8]], ptr %i65, i64 0, i64 0, i64 0
  %i73 = call noalias dereferenceable_or_null(5410652160) ptr @malloc(i64 noundef 5410652160) #4
  %i78 = getelementptr inbounds double, ptr %i73, i64 2621440
  %i79 = call noalias dereferenceable_or_null(5410652160) ptr @malloc(i64 noundef 5410652160) #4
  %i84 = getelementptr inbounds double, ptr %i79, i64 2621440
  br label %bb85

bb85:
  %i86 = phi i64 [ -2621440, %bb62 ], [ %i126, %bb85 ]
  %i87 = getelementptr inbounds [671088640 x double], ptr %i78, i64 0, i64 %i86
  store double 0x3FD5555555555555, ptr %i87, align 8
  %i126 = add nsw i64 %i86, 20
  %i127 = icmp slt i64 %i86, 673710060
  br i1 %i127, label %bb85, label %bb174

bb174:
  %i175 = getelementptr inbounds [671088640 x double], ptr %i78, i64 0, i64 0
  call fastcc void @LBM_loadObstacleFile(ptr noundef nonnull %i175, ptr null) #5
  %i176 = getelementptr inbounds [671088640 x double], ptr %i84, i64 0, i64 0
  call fastcc void @LBM_loadObstacleFile(ptr noundef nonnull %i176, ptr null) #5
  br label %bb246

bb246:
  %i247 = phi i64 [ -2, %bb174 ], [ %i300, %bb299 ]
  %i248 = icmp eq i64 %i247, 0
  %i249 = icmp eq i64 %i247, 511
  %i250 = icmp eq i64 %i247, 1
  %i251 = icmp eq i64 %i247, 510
  %i252 = or i1 %i250, %i251
  %i253 = shl nsw i64 %i247, 16
  br label %bb254

bb254:
  %i255 = phi i64 [ 0, %bb246 ], [ %i297, %bb296 ]
  %i256 = icmp eq i64 %i255, 0
  %i257 = icmp eq i64 %i255, 255
  %i258 = trunc i64 %i255 to i32
  %i259 = add i32 %i258, -2
  %i260 = icmp ult i32 %i259, 252
  %i261 = shl nsw i64 %i255, 8
  %i262 = add nuw nsw i64 %i261, %i253
  br label %bb263

bb263:
  %i264 = phi i64 [ 0, %bb254 ], [ %i294, %bb293 ]
  %i265 = icmp eq i64 %i264, 0
  %i266 = icmp eq i64 %i264, 255
  %i267 = or i1 %i265, %i266
  %i268 = or i1 %i267, %i256
  %i269 = or i1 %i268, %i257
  %i270 = or i1 %i269, %i248
  %i271 = or i1 %i270, %i249
  br i1 %i271, label %bb293, label %bb279

bb279:
  %i280 = trunc i64 %i264 to i32
  %i281 = add i32 %i280, -2
  %i282 = icmp ult i32 %i281, 252
  %i283 = and i1 %i252, %i282
  %i284 = and i1 %i283, %i260
  br i1 %i284, label %bb285, label %bb293

bb285:
  %i286 = add nuw nsw i64 %i262, %i264
  %i287 = mul i64 %i286, 20
  %i288 = add i64 %i287, 19
  %i289 = and i64 %i288, 4294967295
  %i290 = getelementptr inbounds [671088640 x double], ptr %i78, i64 0, i64 %i289
  %i291 = load i32, ptr %i290, align 4
  %i292 = or i32 %i291, 2
  store i32 %i292, ptr %i290, align 4
  br label %bb293

bb293:
  %i294 = add nuw nsw i64 %i264, 1
  %i295 = icmp eq i64 %i294, 256
  br i1 %i295, label %bb296, label %bb263

bb296:
  %i297 = add nuw nsw i64 %i255, 1
  %i298 = icmp eq i64 %i297, 256
  br i1 %i298, label %bb299, label %bb254

bb299:
  %i300 = add nsw i64 %i247, 1
  %i301 = icmp eq i64 %i300, 514
  br i1 %i301, label %bb302, label %bb246

bb302:
  call void @free(ptr noundef nonnull %i79)
  call void @free(ptr noundef nonnull %i73)
  ret i32 0
}

attributes #0 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) }
attributes #1 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) }
attributes #2 = { nofree norecurse nounwind uwtable }
attributes #3 = { norecurse nounwind uwtable }
attributes #4 = { nounwind allocsize(0) }
attributes #5 = { nounwind }

!omp_offload.info = !{!0, !1, !2}
!llvm.ident = !{!3, !3}
!llvm.module.flags = !{!4, !5, !6, !7, !8, !9}

!0 = !{i32 0, i32 2065, i32 78645595, !"_Z19LBM_handleInOutFlow", i32 286, i32 0, i32 1, i32 0}
!1 = !{i32 0, i32 2065, i32 78645595, !"_Z19LBM_handleInOutFlow", i32 346, i32 0, i32 2, i32 0}
!2 = !{i32 0, i32 2065, i32 78645595, !"_Z24LBM_performStreamCollide", i32 169, i32 0, i32 0, i32 0}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"openmp", i32 50}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 1, !"ThinLTO", i32 0}
!8 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!9 = !{i32 1, !"LTOPostLink", i32 1}
