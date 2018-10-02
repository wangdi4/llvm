; RUN: opt < %s -addsub-reassoc -S | FileCheck %s
; This is a test for AddSubReassoc pass to check that it kicks in for satd_16x16 like pattern.

; CHECK: [[Chain_T24_187:%.*]] = sub i32 [[l44:%.*]], [[l28:%.*]]
; CHECK-NEXT: [[Chain_T24_185:%.*]] = sub i32 [[Chain_T24_187]]
; CHECK-NEXT: [[Chain1_3:%.*]] = add i32 [[Chain_T24_185]]
; CHECK-NEXT: [[Chain_T24_174178:%.*]] = sub i32 [[l45:%.*]], [[l29:%.*]]
; CHECK-NEXT: [[Chain_T24_173177:%.*]] = sub i32 [[Chain_T24_174178]]
; CHECK-NEXT: [[Chain2_3:%.*]] = add i32 [[Chain_T24_173177]]
; CHECK-NEXT: [[Chain_T24_162166:%.*]] = sub i32 [[l46:%.*]], [[l30:%.*]]
; CHECK-NEXT: [[Chain_T24_161165:%.*]] = sub i32 [[Chain_T24_162166]]
; CHECK-NEXT: [[Chain3_3:%.*]] = add i32 [[Chain_T24_161165]]
; CHECK-NEXT: [[Chain_T24_158:%.*]] = sub i32 [[l47:%.*]], [[l31:%.*]]
; CHECK-NEXT: [[Chain_T24_156:%.*]] = sub i32 [[Chain_T24_158]]
; CHECK-NEXT: [[Chain4_3:%.*]] = add i32 [[Chain_T24_156]]
; CHECK: [[Bridge1_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK-NEXT: [[Bridge1_2:%.*]] = add i32 [[Bridge1_1]], [[Chain2_3]]
; CHECK-NEXT: [[Bridge1_3:%.*]] = add i32 [[Bridge1_2]], [[Chain1_3]]
; CHECK-NEXT: store i32 [[Bridge1_3]]
; CHECK: [[Bridge2_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK-NEXT: [[Bridge2_2:%.*]] = sub i32 [[Bridge2_1]], [[Chain2_3]]
; CHECK-NEXT: [[Bridge2_3:%.*]] = sub i32 [[Bridge2_2]], [[Chain1_3]]
; CHECK-NEXT: store i32 [[Bridge2_3]]
; CHECK: [[Bridge3_1:%.*]] = sub i32 [[Chain4_3]], [[Chain3_3]]
; CHECK-NEXT: [[Bridge3_2:%.*]] = add i32 [[Bridge3_1]], [[Chain2_3]]
; CHECK-NEXT: [[Bridge3_3:%.*]] = sub i32 [[Bridge3_2]], [[Chain1_3]]
; CHECK-NEXT: store i32 [[Bridge3_3]]
; CHECK: [[Bridge4_1:%.*]] = sub i32 [[Chain4_3]], [[Chain3_3]]
; CHECK-NEXT: [[Bridge4_2:%.*]] = sub i32 [[Bridge4_1]], [[Chain2_3]]
; CHECK-NEXT: [[Bridge4_3:%.*]] = add i32 [[Bridge4_2]], [[Chain1_3]]
; CHECK-NEXT: store i32 [[Bridge4_3]]

; Function Attrs: nounwind readonly uwtable
define dso_local i32 @x264_pixel_satd_8x8(i8* nocapture readonly %pix1, i32 %i_pix1, i8* nocapture readonly %pix2, i32 %i_pix2) #2 {
entry:
  %idx.ext.i = sext i32 %i_pix1 to i64
  %idx.ext63.i = sext i32 %i_pix2 to i64
  %alloca = alloca [8 x [4 x i32]], align 4
  br label %loop.309

loop.309:                                         ; preds = %loop.309, %entry
  %i1.i64.0 = phi i64 [ 0, %entry ], [ %nextivloop.309, %loop.309 ]
  %0 = mul i64 %i1.i64.0, %idx.ext.i
  %arrayIdx = getelementptr inbounds i8, i8* %pix1, i64 %0
  %gepload = load i8, i8* %arrayIdx, align 1, !tbaa !21
  %1 = mul i64 %i1.i64.0, %idx.ext63.i
  %arrayIdx150 = getelementptr inbounds i8, i8* %pix2, i64 %1
  %gepload151 = load i8, i8* %arrayIdx150, align 1, !tbaa !21
  %2 = add i64 %0, 4
  %arrayIdx152 = getelementptr inbounds i8, i8* %pix1, i64 %2
  %gepload153 = load i8, i8* %arrayIdx152, align 1, !tbaa !21
  %3 = add i64 %1, 4
  %arrayIdx154 = getelementptr inbounds i8, i8* %pix2, i64 %3
  %gepload155 = load i8, i8* %arrayIdx154, align 1, !tbaa !21
  %4 = add i64 %0, 1
  %arrayIdx156 = getelementptr inbounds i8, i8* %pix1, i64 %4
  %gepload157 = load i8, i8* %arrayIdx156, align 1, !tbaa !21
  %5 = add i64 %1, 1
  %arrayIdx158 = getelementptr inbounds i8, i8* %pix2, i64 %5
  %gepload159 = load i8, i8* %arrayIdx158, align 1, !tbaa !21
  %6 = add i64 %0, 5
  %arrayIdx160 = getelementptr inbounds i8, i8* %pix1, i64 %6
  %gepload161 = load i8, i8* %arrayIdx160, align 1, !tbaa !21
  %7 = add i64 %1, 5
  %arrayIdx162 = getelementptr inbounds i8, i8* %pix2, i64 %7
  %gepload163 = load i8, i8* %arrayIdx162, align 1, !tbaa !21
  %8 = add i64 %0, 2
  %arrayIdx164 = getelementptr inbounds i8, i8* %pix1, i64 %8
  %gepload165 = load i8, i8* %arrayIdx164, align 1, !tbaa !21
  %9 = add i64 %1, 2
  %arrayIdx166 = getelementptr inbounds i8, i8* %pix2, i64 %9
  %gepload167 = load i8, i8* %arrayIdx166, align 1, !tbaa !21
  %10 = add i64 %0, 6
  %arrayIdx168 = getelementptr inbounds i8, i8* %pix1, i64 %10
  %gepload169 = load i8, i8* %arrayIdx168, align 1, !tbaa !21
  %11 = add i64 %1, 6
  %arrayIdx170 = getelementptr inbounds i8, i8* %pix2, i64 %11
  %gepload171 = load i8, i8* %arrayIdx170, align 1, !tbaa !21
  %12 = add i64 %0, 3
  %arrayIdx172 = getelementptr inbounds i8, i8* %pix1, i64 %12
  %gepload173 = load i8, i8* %arrayIdx172, align 1, !tbaa !21
  %13 = add i64 %1, 3
  %arrayIdx174 = getelementptr inbounds i8, i8* %pix2, i64 %13
  %gepload175 = load i8, i8* %arrayIdx174, align 1, !tbaa !21
  %14 = add i64 %0, 7
  %arrayIdx176 = getelementptr inbounds i8, i8* %pix1, i64 %14
  %gepload177 = load i8, i8* %arrayIdx176, align 1, !tbaa !21
  %15 = add i64 %1, 7
  %arrayIdx178 = getelementptr inbounds i8, i8* %pix2, i64 %15
  %gepload179 = load i8, i8* %arrayIdx178, align 1, !tbaa !21
  %arrayIdx180 = getelementptr inbounds [8 x [4 x i32]], [8 x [4 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 0
  %16 = zext i8 %gepload173 to i32
  %17 = zext i8 %gepload165 to i32
  %18 = add nuw nsw i32 %16, %17
  %19 = zext i8 %gepload157 to i32
  %20 = add nuw nsw i32 %18, %19
  %21 = zext i8 %gepload to i32
  %22 = add nuw nsw i32 %20, %21
  %23 = zext i8 %gepload177 to i32
  %24 = shl nuw nsw i32 %23, 16
  %25 = or i32 %22, %24
  %26 = zext i8 %gepload169 to i32
  %27 = shl nuw nsw i32 %26, 16
  %28 = add nuw nsw i32 %25, %27
  %29 = zext i8 %gepload161 to i32
  %30 = shl nuw nsw i32 %29, 16
  %31 = add nuw nsw i32 %28, %30
  %32 = zext i8 %gepload153 to i32
  %33 = shl nuw nsw i32 %32, 16
  %34 = add i32 %31, %33
  %35 = zext i8 %gepload179 to i32
  %36 = shl nuw nsw i32 %35, 16
  %37 = sub i32 %34, %36
  %38 = zext i8 %gepload171 to i32
  %39 = shl nuw nsw i32 %38, 16
  %40 = sub i32 %37, %39
  %41 = zext i8 %gepload163 to i32
  %42 = shl nuw nsw i32 %41, 16
  %43 = sub i32 %40, %42
  %44 = zext i8 %gepload155 to i32
  %45 = shl nuw nsw i32 %44, 16
  %46 = sub i32 %43, %45
  %47 = zext i8 %gepload175 to i32
  %48 = sub i32 %46, %47
  %49 = zext i8 %gepload167 to i32
  %50 = sub i32 %48, %49
  %51 = zext i8 %gepload159 to i32
  %52 = sub i32 %50, %51
  %53 = zext i8 %gepload151 to i32
  %54 = sub i32 %52, %53
  store i32 %54, i32* %arrayIdx180, align 4
  %arrayIdx182 = getelementptr inbounds [8 x [4 x i32]], [8 x [4 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 2
  %55 = add nuw nsw i32 %16, %17
  %56 = sub nsw i32 %19, %55
  %57 = add nsw i32 %56, %21
  %58 = sub nsw i32 %57, %24
  %59 = sub nsw i32 %58, %27
  %60 = add nsw i32 %59, %30
  %61 = add i32 %60, %33
  %62 = add i32 %61, %36
  %63 = add i32 %62, %39
  %64 = sub i32 %63, %42
  %65 = sub i32 %64, %45
  %66 = add i32 %65, %47
  %67 = add i32 %66, %49
  %68 = sub i32 %67, %51
  %69 = sub i32 %68, %53
  store i32 %69, i32* %arrayIdx182, align 4
  %arrayIdx200 = getelementptr inbounds [8 x [4 x i32]], [8 x [4 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 1
  %70 = sub nsw i32 %17, %16
  %71 = sub nsw i32 %70, %19
  %72 = add nsw i32 %71, %21
  %73 = sub nsw i32 %72, %24
  %74 = add nsw i32 %73, %27
  %75 = sub nsw i32 %74, %30
  %76 = add i32 %75, %33
  %77 = add i32 %76, %36
  %78 = sub i32 %77, %39
  %79 = add i32 %78, %42
  %80 = sub i32 %79, %45
  %81 = add i32 %80, %47
  %82 = sub i32 %81, %49
  %83 = add i32 %82, %51
  %84 = sub i32 %83, %53
  store i32 %84, i32* %arrayIdx200, align 4
  %arrayIdx218 = getelementptr inbounds [8 x [4 x i32]], [8 x [4 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 3
  %85 = sub nsw i32 %16, %17
  %86 = sub nsw i32 %85, %19
  %87 = add nsw i32 %86, %21
  %88 = add nsw i32 %87, %24
  %89 = sub nsw i32 %88, %27
  %90 = sub nsw i32 %89, %30
  %91 = add i32 %90, %33
  %92 = sub i32 %91, %36
  %93 = add i32 %92, %39
  %94 = add i32 %93, %42
  %95 = sub i32 %94, %45
  %96 = sub i32 %95, %47
  %97 = add i32 %96, %49
  %98 = add i32 %97, %51
  %99 = sub i32 %98, %53
  store i32 %99, i32* %arrayIdx218, align 4
  %nextivloop.309 = add nuw nsw i64 %i1.i64.0, 1
  %condloop.309 = icmp ult i64 %nextivloop.309, 8
  br i1 %condloop.309, label %loop.309, label %afterloop.309

afterloop.309:                                   ; preds = %loop.309
  ret i32 0
}

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang cd5be4ea094f77df25a78b0ab0ead6e58960206c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f515ae58d78783d4a202c33471bf5e00ce0fc49b)"}
!2 = !{!3, !5, i64 56}
!3 = !{!"struct@", !4, i64 0, !4, i64 56, !4, i64 112, !4, i64 168, !8, i64 224, !4, i64 256, !4, i64 312, !4, i64 368, !9, i64 424, !11, i64 480, !4, i64 536, !13, i64 592, !14, i64 600, !14, i64 632, !16, i64 664, !17, i64 672, !9, i64 680, !11, i64 736, !9, i64 792, !11, i64 848, !18, i64 904, !20, i64 960, !20, i64 968, !20, i64 976, !20, i64 984, !20, i64 992, !20, i64 1000, !20, i64 1008, !20, i64 1016, !20, i64 1024, !20, i64 1032, !20, i64 1040, !20, i64 1048}
!4 = !{!"array@_ZTSA7_PFiPhiS_iE", !5, i64 0}
!5 = !{!"pointer@_ZTSPFiPhiS_iE", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!"array@_ZTSA4_PFiPhiS_iE", !5, i64 0}
!9 = !{!"array@_ZTSA7_PFvPhS_S_S_iPiE", !10, i64 0}
!10 = !{!"pointer@_ZTSPFvPhS_S_S_iPiE", !6, i64 0}
!11 = !{!"array@_ZTSA7_PFvPhS_S_S_S_iPiE", !12, i64 0}
!12 = !{!"pointer@_ZTSPFvPhS_S_S_S_iPiE", !6, i64 0}
!13 = !{!"pointer@_ZTSPFiPhiS_iPiE", !6, i64 0}
!14 = !{!"array@_ZTSA4_PFmPhiE", !15, i64 0}
!15 = !{!"pointer@_ZTSPFmPhiE", !6, i64 0}
!16 = !{!"pointer@_ZTSPFvPKhiS0_iPA4_iE", !6, i64 0}
!17 = !{!"pointer@_ZTSPFfPA4_iS0_iE", !6, i64 0}
!18 = !{!"array@_ZTSA7_PFiPiPtiS0_PsiiE", !19, i64 0}
!19 = !{!"pointer@_ZTSPFiPiPtiS0_PsiiE", !6, i64 0}
!20 = !{!"pointer@_ZTSPFvPhS_PiE", !6, i64 0}
!21 = !{!6, !6, i64 0}
