; RUN: opt -enable-new-pm=0 -O2 -mtriple=x86_64-unknown-linux-gnu -mcpu=haswell -load-coalescing -load-coalescing-allow-scalars -S < %s 2>&1 | FileCheck %s --check-prefix=LC-HASWELL
; RUN: opt -enable-new-pm=0 -O2 -mtriple=x86_64-unknown-linux-gnu -mcpu=silvermont -load-coalescing -load-coalescing-allow-scalars -S < %s 2>&1 | FileCheck %s --check-prefix=LC-SILVERMONT
; RUN: opt -passes='default<O2>,load-coalescing' -mtriple=x86_64-unknown-linux-gnu -mcpu=haswell -load-coalescing-allow-scalars -S < %s 2>&1 | FileCheck %s --check-prefix=LC-HASWELL
; RUN: opt -passes='default<O2>,load-coalescing' -mtriple=x86_64-unknown-linux-gnu -mcpu=silvermont -load-coalescing-allow-scalars -S < %s 2>&1 | FileCheck %s --check-prefix=LC-SILVERMONT

@B = common dso_local global [16384 x i32] zeroinitializer, align 16
@A = common dso_local global [16384 x i32] zeroinitializer, align 16
@V1 = common dso_local global i32 0, align 4
@V2 = common dso_local global i32 0, align 4

define dso_local void @dummyFunc(i32 %x) {
; LC-HASWELL:  [[L1:%.*]] = load <8 x i32>, <8 x i32>*
; LC-HASWELL:  [[L2:%.*]] = load <4 x i32>, <4 x i32>*

; LC-SILVERMONT:  [[L1:%.*]] = load <4 x i32>, <4 x i32>*
; LC-SILVERMONT:  [[L2:%.*]] = load <4 x i32>, <4 x i32>*
; LC-SILVERMONT:  [[L3:%.*]] = load <4 x i32>, <4 x i32>*

entry:
  %x.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4, !tbaa !0
  %0 = bitcast i32* %i to i8*
  store i32 0, i32* %i, align 4, !tbaa !0
  %1 = load i32, i32* %i, align 4, !tbaa !0
  %add = add nsw i32 %1, 0
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !4
  %3 = load i32, i32* %i, align 4, !tbaa !0
  %add1 = add nsw i32 %3, 0
  %idxprom2 = sext i32 %add1 to i64
  %arrayidx3 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom2
  store i32 %2, i32* %arrayidx3, align 4, !tbaa !4
  %4 = load i32, i32* %i, align 4, !tbaa !0
  %add4 = add nsw i32 %4, 1
  %idxprom5 = sext i32 %add4 to i64
  %arrayidx6 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom5
  %5 = load i32, i32* %arrayidx6, align 4, !tbaa !4
  %6 = load i32, i32* %i, align 4, !tbaa !0
  %add7 = add nsw i32 %6, 1
  %idxprom8 = sext i32 %add7 to i64
  %arrayidx9 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom8
  store i32 %5, i32* %arrayidx9, align 4, !tbaa !4
  %7 = load i32, i32* %i, align 4, !tbaa !0
  %add10 = add nsw i32 %7, 2
  %idxprom11 = sext i32 %add10 to i64
  %arrayidx12 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom11
  %8 = load i32, i32* %arrayidx12, align 4, !tbaa !4
  %9 = load i32, i32* %i, align 4, !tbaa !0
  %add13 = add nsw i32 %9, 2
  %idxprom14 = sext i32 %add13 to i64
  %arrayidx15 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom14
  store i32 %8, i32* %arrayidx15, align 4, !tbaa !4
  %10 = load i32, i32* %i, align 4, !tbaa !0
  %add16 = add nsw i32 %10, 3
  %idxprom17 = sext i32 %add16 to i64
  %arrayidx18 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom17
  %11 = load i32, i32* %arrayidx18, align 4, !tbaa !4
  %12 = load i32, i32* %i, align 4, !tbaa !0
  %add19 = add nsw i32 %12, 3
  %idxprom20 = sext i32 %add19 to i64
  %arrayidx21 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom20
  store i32 %11, i32* %arrayidx21, align 4, !tbaa !4
  %13 = load i32, i32* %i, align 4, !tbaa !0
  %add22 = add nsw i32 %13, 4
  %idxprom23 = sext i32 %add22 to i64
  %arrayidx24 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom23
  %14 = load i32, i32* %arrayidx24, align 4, !tbaa !4
  %15 = load i32, i32* %i, align 4, !tbaa !0
  %add25 = add nsw i32 %15, 4
  %idxprom26 = sext i32 %add25 to i64
  %arrayidx27 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom26
  store i32 %14, i32* %arrayidx27, align 4, !tbaa !4
  %16 = load i32, i32* %i, align 4, !tbaa !0
  %add28 = add nsw i32 %16, 5
  %idxprom29 = sext i32 %add28 to i64
  %arrayidx30 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom29
  %17 = load i32, i32* %arrayidx30, align 4, !tbaa !4
  %18 = load i32, i32* %i, align 4, !tbaa !0
  %add31 = add nsw i32 %18, 5
  %idxprom32 = sext i32 %add31 to i64
  %arrayidx33 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom32
  store i32 %17, i32* %arrayidx33, align 4, !tbaa !4
  %19 = load i32, i32* %i, align 4, !tbaa !0
  %add34 = add nsw i32 %19, 6
  %idxprom35 = sext i32 %add34 to i64
  %arrayidx36 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom35
  %20 = load i32, i32* %arrayidx36, align 4, !tbaa !4
  %21 = load i32, i32* %i, align 4, !tbaa !0
  %add37 = add nsw i32 %21, 6
  %idxprom38 = sext i32 %add37 to i64
  %arrayidx39 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom38
  store i32 %20, i32* %arrayidx39, align 4, !tbaa !4
  %22 = load i32, i32* %i, align 4, !tbaa !0
  %add40 = add nsw i32 %22, 7
  %idxprom41 = sext i32 %add40 to i64
  %arrayidx42 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom41
  %23 = load i32, i32* %arrayidx42, align 4, !tbaa !4
  %24 = load i32, i32* %i, align 4, !tbaa !0
  %add43 = add nsw i32 %24, 7
  %idxprom44 = sext i32 %add43 to i64
  %arrayidx45 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom44
  store i32 %23, i32* %arrayidx45, align 4, !tbaa !4
  %25 = load i32, i32* %i, align 4, !tbaa !0
  %add46 = add nsw i32 %25, 18
  %idxprom47 = sext i32 %add46 to i64
  %arrayidx48 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom47
  %26 = load i32, i32* %arrayidx48, align 4, !tbaa !4
  %27 = load i32, i32* %i, align 4, !tbaa !0
  %add49 = add nsw i32 %27, 12
  %idxprom50 = sext i32 %add49 to i64
  %arrayidx51 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom50
  store i32 %26, i32* %arrayidx51, align 4, !tbaa !4
  %28 = load i32, i32* %i, align 4, !tbaa !0
  %add52 = add nsw i32 %28, 19
  %idxprom53 = sext i32 %add52 to i64
  %arrayidx54 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom53
  %29 = load i32, i32* %arrayidx54, align 4, !tbaa !4
  %30 = load i32, i32* %i, align 4, !tbaa !0
  %add55 = add nsw i32 %30, 13
  %idxprom56 = sext i32 %add55 to i64
  %arrayidx57 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom56
  store i32 %29, i32* %arrayidx57, align 4, !tbaa !4
  %31 = load i32, i32* %i, align 4, !tbaa !0
  %add58 = add nsw i32 %31, 20
  %idxprom59 = sext i32 %add58 to i64
  %arrayidx60 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom59
  %32 = load i32, i32* %arrayidx60, align 4, !tbaa !4
  %33 = load i32, i32* %i, align 4, !tbaa !0
  %add61 = add nsw i32 %33, 14
  %idxprom62 = sext i32 %add61 to i64
  %arrayidx63 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom62
  store i32 %32, i32* %arrayidx63, align 4, !tbaa !4
  %34 = load i32, i32* %i, align 4, !tbaa !0
  %add64 = add nsw i32 %34, 21
  %idxprom65 = sext i32 %add64 to i64
  %arrayidx66 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom65
  %35 = load i32, i32* %arrayidx66, align 4, !tbaa !4
  %36 = load i32, i32* %i, align 4, !tbaa !0
  %add67 = add nsw i32 %36, 15
  %idxprom68 = sext i32 %add67 to i64
  %arrayidx69 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom68
  store i32 %35, i32* %arrayidx69, align 4, !tbaa !4
  %37 = load i32, i32* %i, align 4, !tbaa !0
  %38 = load i32, i32* %x.addr, align 4, !tbaa !0
  %add70 = add nsw i32 %37, %38
  %idxprom71 = sext i32 %add70 to i64
  %arrayidx72 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom71
  %39 = load i32, i32* %arrayidx72, align 4, !tbaa !4
  %40 = load i32, i32* %i, align 4, !tbaa !0
  %add73 = add nsw i32 %40, 29
  %idxprom74 = sext i32 %add73 to i64
  %arrayidx75 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom74
  store i32 %39, i32* %arrayidx75, align 4, !tbaa !4
  %41 = load i32, i32* %i, align 4, !tbaa !0
  %42 = load i32, i32* %x.addr, align 4, !tbaa !0
  %add76 = add nsw i32 %41, %42
  %add77 = add nsw i32 %add76, 1
  %idxprom78 = sext i32 %add77 to i64
  %arrayidx79 = getelementptr inbounds [16384 x i32], [16384 x i32]* @B, i64 0, i64 %idxprom78
  %43 = load i32, i32* %arrayidx79, align 4, !tbaa !4
  %44 = load i32, i32* %i, align 4, !tbaa !0
  %add80 = add nsw i32 %44, 30
  %idxprom81 = sext i32 %add80 to i64
  %arrayidx82 = getelementptr inbounds [16384 x i32], [16384 x i32]* @A, i64 0, i64 %idxprom81
  store i32 %43, i32* %arrayidx82, align 4, !tbaa !4
  %45 = bitcast i32* %i to i8*
  ret void
}

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!5, !1, i64 0}
!5 = !{!"array@_ZTSA16384_i", !1, i64 0}
