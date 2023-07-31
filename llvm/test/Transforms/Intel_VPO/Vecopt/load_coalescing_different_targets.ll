; RUN: opt -passes='default<O2>,load-coalescing' -mtriple=x86_64-unknown-linux-gnu -mcpu=haswell -load-coalescing-allow-scalars -S < %s 2>&1 | FileCheck %s --check-prefix=LC-HASWELL
; RUN: opt -passes='default<O2>,load-coalescing' -mtriple=x86_64-unknown-linux-gnu -mcpu=silvermont -load-coalescing-allow-scalars -S < %s 2>&1 | FileCheck %s --check-prefix=LC-SILVERMONT

@B = common dso_local global [16384 x i32] zeroinitializer, align 16
@A = common dso_local global [16384 x i32] zeroinitializer, align 16
@V1 = common dso_local global i32 0, align 4
@V2 = common dso_local global i32 0, align 4

define dso_local void @dummyFunc(i32 %x) {
; LC-HASWELL:  [[L1:%.*]] = load <8 x i32>, ptr
; LC-HASWELL:  [[L2:%.*]] = load <4 x i32>, ptr

; LC-SILVERMONT:  [[L1:%.*]] = load <4 x i32>, ptr
; LC-SILVERMONT:  [[L2:%.*]] = load <4 x i32>, ptr
; LC-SILVERMONT:  [[L3:%.*]] = load <4 x i32>, ptr

entry:
  %x.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4, !tbaa !0
  store i32 0, ptr %i, align 4, !tbaa !0
  %0 = load i32, ptr %i, align 4, !tbaa !0
  %add = add nsw i32 %0, 0
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !4
  %2 = load i32, ptr %i, align 4, !tbaa !0
  %add1 = add nsw i32 %2, 0
  %idxprom2 = sext i32 %add1 to i64
  %arrayidx3 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom2
  store i32 %1, ptr %arrayidx3, align 4, !tbaa !4
  %3 = load i32, ptr %i, align 4, !tbaa !0
  %add4 = add nsw i32 %3, 1
  %idxprom5 = sext i32 %add4 to i64
  %arrayidx6 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom5
  %4 = load i32, ptr %arrayidx6, align 4, !tbaa !4
  %5 = load i32, ptr %i, align 4, !tbaa !0
  %add7 = add nsw i32 %5, 1
  %idxprom8 = sext i32 %add7 to i64
  %arrayidx9 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom8
  store i32 %4, ptr %arrayidx9, align 4, !tbaa !4
  %6 = load i32, ptr %i, align 4, !tbaa !0
  %add10 = add nsw i32 %6, 2
  %idxprom11 = sext i32 %add10 to i64
  %arrayidx12 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom11
  %7 = load i32, ptr %arrayidx12, align 4, !tbaa !4
  %8 = load i32, ptr %i, align 4, !tbaa !0
  %add13 = add nsw i32 %8, 2
  %idxprom14 = sext i32 %add13 to i64
  %arrayidx15 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom14
  store i32 %7, ptr %arrayidx15, align 4, !tbaa !4
  %9 = load i32, ptr %i, align 4, !tbaa !0
  %add16 = add nsw i32 %9, 3
  %idxprom17 = sext i32 %add16 to i64
  %arrayidx18 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom17
  %10 = load i32, ptr %arrayidx18, align 4, !tbaa !4
  %11 = load i32, ptr %i, align 4, !tbaa !0
  %add19 = add nsw i32 %11, 3
  %idxprom20 = sext i32 %add19 to i64
  %arrayidx21 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom20
  store i32 %10, ptr %arrayidx21, align 4, !tbaa !4
  %12 = load i32, ptr %i, align 4, !tbaa !0
  %add22 = add nsw i32 %12, 4
  %idxprom23 = sext i32 %add22 to i64
  %arrayidx24 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom23
  %13 = load i32, ptr %arrayidx24, align 4, !tbaa !4
  %14 = load i32, ptr %i, align 4, !tbaa !0
  %add25 = add nsw i32 %14, 4
  %idxprom26 = sext i32 %add25 to i64
  %arrayidx27 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom26
  store i32 %13, ptr %arrayidx27, align 4, !tbaa !4
  %15 = load i32, ptr %i, align 4, !tbaa !0
  %add28 = add nsw i32 %15, 5
  %idxprom29 = sext i32 %add28 to i64
  %arrayidx30 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom29
  %16 = load i32, ptr %arrayidx30, align 4, !tbaa !4
  %17 = load i32, ptr %i, align 4, !tbaa !0
  %add31 = add nsw i32 %17, 5
  %idxprom32 = sext i32 %add31 to i64
  %arrayidx33 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom32
  store i32 %16, ptr %arrayidx33, align 4, !tbaa !4
  %18 = load i32, ptr %i, align 4, !tbaa !0
  %add34 = add nsw i32 %18, 6
  %idxprom35 = sext i32 %add34 to i64
  %arrayidx36 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom35
  %19 = load i32, ptr %arrayidx36, align 4, !tbaa !4
  %20 = load i32, ptr %i, align 4, !tbaa !0
  %add37 = add nsw i32 %20, 6
  %idxprom38 = sext i32 %add37 to i64
  %arrayidx39 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom38
  store i32 %19, ptr %arrayidx39, align 4, !tbaa !4
  %21 = load i32, ptr %i, align 4, !tbaa !0
  %add40 = add nsw i32 %21, 7
  %idxprom41 = sext i32 %add40 to i64
  %arrayidx42 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom41
  %22 = load i32, ptr %arrayidx42, align 4, !tbaa !4
  %23 = load i32, ptr %i, align 4, !tbaa !0
  %add43 = add nsw i32 %23, 7
  %idxprom44 = sext i32 %add43 to i64
  %arrayidx45 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom44
  store i32 %22, ptr %arrayidx45, align 4, !tbaa !4
  %24 = load i32, ptr %i, align 4, !tbaa !0
  %add46 = add nsw i32 %24, 18
  %idxprom47 = sext i32 %add46 to i64
  %arrayidx48 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom47
  %25 = load i32, ptr %arrayidx48, align 4, !tbaa !4
  %26 = load i32, ptr %i, align 4, !tbaa !0
  %add49 = add nsw i32 %26, 12
  %idxprom50 = sext i32 %add49 to i64
  %arrayidx51 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom50
  store i32 %25, ptr %arrayidx51, align 4, !tbaa !4
  %27 = load i32, ptr %i, align 4, !tbaa !0
  %add52 = add nsw i32 %27, 19
  %idxprom53 = sext i32 %add52 to i64
  %arrayidx54 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom53
  %28 = load i32, ptr %arrayidx54, align 4, !tbaa !4
  %29 = load i32, ptr %i, align 4, !tbaa !0
  %add55 = add nsw i32 %29, 13
  %idxprom56 = sext i32 %add55 to i64
  %arrayidx57 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom56
  store i32 %28, ptr %arrayidx57, align 4, !tbaa !4
  %30 = load i32, ptr %i, align 4, !tbaa !0
  %add58 = add nsw i32 %30, 20
  %idxprom59 = sext i32 %add58 to i64
  %arrayidx60 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom59
  %31 = load i32, ptr %arrayidx60, align 4, !tbaa !4
  %32 = load i32, ptr %i, align 4, !tbaa !0
  %add61 = add nsw i32 %32, 14
  %idxprom62 = sext i32 %add61 to i64
  %arrayidx63 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom62
  store i32 %31, ptr %arrayidx63, align 4, !tbaa !4
  %33 = load i32, ptr %i, align 4, !tbaa !0
  %add64 = add nsw i32 %33, 21
  %idxprom65 = sext i32 %add64 to i64
  %arrayidx66 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom65
  %34 = load i32, ptr %arrayidx66, align 4, !tbaa !4
  %35 = load i32, ptr %i, align 4, !tbaa !0
  %add67 = add nsw i32 %35, 15
  %idxprom68 = sext i32 %add67 to i64
  %arrayidx69 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom68
  store i32 %34, ptr %arrayidx69, align 4, !tbaa !4
  %36 = load i32, ptr %i, align 4, !tbaa !0
  %37 = load i32, ptr %x.addr, align 4, !tbaa !0
  %add70 = add nsw i32 %36, %37
  %idxprom71 = sext i32 %add70 to i64
  %arrayidx72 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom71
  %38 = load i32, ptr %arrayidx72, align 4, !tbaa !4
  %39 = load i32, ptr %i, align 4, !tbaa !0
  %add73 = add nsw i32 %39, 29
  %idxprom74 = sext i32 %add73 to i64
  %arrayidx75 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom74
  store i32 %38, ptr %arrayidx75, align 4, !tbaa !4
  %40 = load i32, ptr %i, align 4, !tbaa !0
  %41 = load i32, ptr %x.addr, align 4, !tbaa !0
  %add76 = add nsw i32 %40, %41
  %add77 = add nsw i32 %add76, 1
  %idxprom78 = sext i32 %add77 to i64
  %arrayidx79 = getelementptr inbounds [16384 x i32], ptr @B, i64 0, i64 %idxprom78
  %42 = load i32, ptr %arrayidx79, align 4, !tbaa !4
  %43 = load i32, ptr %i, align 4, !tbaa !0
  %add80 = add nsw i32 %43, 30
  %idxprom81 = sext i32 %add80 to i64
  %arrayidx82 = getelementptr inbounds [16384 x i32], ptr @A, i64 0, i64 %idxprom81
  store i32 %42, ptr %arrayidx82, align 4, !tbaa !4
  ret void
}

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!5, !1, i64 0}
!5 = !{!"array@_ZTSA16384_i", !1, i64 0}
