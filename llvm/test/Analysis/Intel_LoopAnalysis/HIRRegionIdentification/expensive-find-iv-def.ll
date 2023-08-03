; RUN: opt < %s -xmain-opt-level=3 -hir-allow-large-integers -passes="print<hir-region-identification>" 2>&1 | FileCheck %s

; Verify that we are able to finish in reasonable time. This test was taking too
; long in findIVDefInHeader() because of redundant traceback of same instruction
; multiple times starting with %retval.1.i.i.i.


; CHECK: Region 1


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct.r123::Engine" = type { %"struct.r123::ReinterpretCtr", %struct.r123array4x32, %struct.r123array2x64, %struct.r123array2x64 }
%"struct.r123::ReinterpretCtr" = type { i8 }
%struct.r123array4x32 = type { [4 x i32] }
%struct.r123array2x64 = type { [2 x i64] }

define dso_local noundef i32 @main(i32 noundef %0, ptr nocapture noundef readnone %1) local_unnamed_addr {
entry:
  %e5 = alloca %"struct.r123::Engine", align 8
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %e5)
  %c.i = getelementptr inbounds %"struct.r123::Engine", ptr %e5, i64 0, i32 2
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %c.i, i8 0, i64 16, i1 false)
  %x.sroa.0.0..sroa_idx18.i = getelementptr inbounds %"struct.r123::Engine", ptr %e5, i64 0, i32 1, i32 0
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 4 dereferenceable(16) %x.sroa.0.0..sroa_idx18.i, i8 0, i64 16, i1 false)
  %arrayidx.i.i = getelementptr inbounds %"struct.r123::Engine", ptr %e5, i64 0, i32 3, i32 0, i64 1
  %2 = getelementptr inbounds %"struct.r123::Engine", ptr %e5, i64 0, i32 2, i32 0, i64 0
  %3 = getelementptr inbounds %"struct.r123::Engine", ptr %e5, i64 0, i32 2, i32 0, i64 1
  %ref.tmp10.sroa.0.0..sroa_idx.i.i.i = getelementptr inbounds %"struct.r123::Engine", ptr %e5, i64 0, i32 3, i32 0, i64 0
  br label %if.then.i.i

for.cond.cleanup:                                 ; preds = %_ZNSt24uniform_int_distributionIiEclIN4r1236EngineINS2_14ReinterpretCtrI13r123array2x64NS2_14Threefry4x32_RILj20EEEEEEEEEiRT_.exit
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %e5)
  ret i32 0

if.then.i.i:                                      ; preds = %_ZNSt24uniform_int_distributionIiEclIN4r1236EngineINS2_14ReinterpretCtrI13r123array2x64NS2_14Threefry4x32_RILj20EEEEEEEEEiRT_.exit, %entry
  %4 = phi i64 [ 0, %entry ], [ %.lcssa, %_ZNSt24uniform_int_distributionIiEclIN4r1236EngineINS2_14ReinterpretCtrI13r123array2x64NS2_14Threefry4x32_RILj20EEEEEEEEEiRT_.exit ]
  %i.08 = phi i32 [ 0, %entry ], [ %inc, %_ZNSt24uniform_int_distributionIiEclIN4r1236EngineINS2_14ReinterpretCtrI13r123array2x64NS2_14Threefry4x32_RILj20EEEEEEEEEiRT_.exit ]
  br label %do.body.i.i

do.body.i.i:                                      ; preds = %_ZN4r1236EngineINS_14ReinterpretCtrI13r123array2x64NS_14Threefry4x32_RILj20EEEEEEclEv.exit.i.i, %if.then.i.i
  %5 = phi i64 [ %8, %_ZN4r1236EngineINS_14ReinterpretCtrI13r123array2x64NS_14Threefry4x32_RILj20EEEEEEclEv.exit.i.i ], [ %4, %if.then.i.i ]
  %cmp8.i.i.i = icmp eq i64 %5, 0
  br i1 %cmp8.i.i.i, label %if.then9.i.i.i, label %if.end24.i.i.i

if.then9.i.i.i:                                   ; preds = %do.body.i.i
  %6 = load i64, ptr %2, align 8
  %inc.i39.i.i.i = add i64 %6, 1
  store i64 %inc.i39.i.i.i, ptr %2, align 8
  %tobool.not.i40.i.i.i = icmp eq i64 %inc.i39.i.i.i, 0
  %agg.tmp12.sroa.2.0.copyload.pre.i.i.i = load i64, ptr %3, align 8
  br i1 %tobool.not.i40.i.i.i, label %if.end17.i43.i.i.i, label %_ZN13r123array2x644incrEy.exit44.i.i.i

if.end17.i43.i.i.i:                               ; preds = %if.then9.i.i.i
  %inc20.i42.i.i.i = add i64 %agg.tmp12.sroa.2.0.copyload.pre.i.i.i, 1
  store i64 %inc20.i42.i.i.i, ptr %3, align 8
  br label %_ZN13r123array2x644incrEy.exit44.i.i.i

_ZN13r123array2x644incrEy.exit44.i.i.i:           ; preds = %if.end17.i43.i.i.i, %if.then9.i.i.i
  %agg.tmp12.sroa.2.0.copyload.i.i.i = phi i64 [ %inc20.i42.i.i.i, %if.end17.i43.i.i.i ], [ %agg.tmp12.sroa.2.0.copyload.pre.i.i.i, %if.then9.i.i.i ]
  %in.sroa.5.8.extract.shift.i.i.i = lshr i64 %agg.tmp12.sroa.2.0.copyload.i.i.i, 32
  %in.sroa.5.8.extract.trunc.i.i.i = trunc i64 %in.sroa.5.8.extract.shift.i.i.i to i32
  %in.sroa.3.8.extract.trunc.i.i.i = trunc i64 %agg.tmp12.sroa.2.0.copyload.i.i.i to i32
  %in.sroa.2.0.extract.shift.i.i.i = lshr i64 %inc.i39.i.i.i, 32
  %in.sroa.2.0.extract.trunc.i.i.i = trunc i64 %in.sroa.2.0.extract.shift.i.i.i to i32
  %in.sroa.0.0.extract.trunc.i.i.i = trunc i64 %inc.i39.i.i.i to i32
  %add22.i.i.i = add i32 %in.sroa.0.0.extract.trunc.i.i.i, %in.sroa.2.0.extract.trunc.i.i.i
  %or.i.i.i.i = tail call i32 @llvm.fshl.i32(i32 %in.sroa.2.0.extract.trunc.i.i.i, i32 %in.sroa.2.0.extract.trunc.i.i.i, i32 10)
  %xor23.i.i.i = xor i32 %or.i.i.i.i, %add22.i.i.i
  %add24.i.i.i = add i32 %in.sroa.3.8.extract.trunc.i.i.i, %in.sroa.5.8.extract.trunc.i.i.i
  %or.i1.i.i.i = tail call i32 @llvm.fshl.i32(i32 %in.sroa.5.8.extract.trunc.i.i.i, i32 %in.sroa.5.8.extract.trunc.i.i.i, i32 26)
  %xor26.i.i.i = xor i32 %or.i1.i.i.i, %add24.i.i.i
  %add29.i.i.i = add i32 %xor26.i.i.i, %add22.i.i.i
  %or.i2.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor26.i.i.i, i32 %xor26.i.i.i, i32 11)
  %xor31.i.i.i = xor i32 %or.i2.i.i.i, %add29.i.i.i
  %add32.i.i.i = add i32 %add24.i.i.i, %xor23.i.i.i
  %or.i3.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor23.i.i.i, i32 %xor23.i.i.i, i32 21)
  %xor34.i.i.i = xor i32 %or.i3.i.i.i, %add32.i.i.i
  %add38.i.i.i = add i32 %add29.i.i.i, %xor34.i.i.i
  %or.i4.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor34.i.i.i, i32 %xor34.i.i.i, i32 13)
  %xor40.i.i.i = xor i32 %or.i4.i.i.i, %add38.i.i.i
  %add41.i.i.i = add i32 %xor31.i.i.i, %add32.i.i.i
  %or.i5.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor31.i.i.i, i32 %xor31.i.i.i, i32 27)
  %xor43.i.i.i = xor i32 %or.i5.i.i.i, %add41.i.i.i
  %add47.i.i.i = add i32 %xor43.i.i.i, %add38.i.i.i
  %or.i6.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor43.i.i.i, i32 %xor43.i.i.i, i32 23)
  %xor49.i.i.i = xor i32 %or.i6.i.i.i, %add47.i.i.i
  %add50.i.i.i = add i32 %add41.i.i.i, %xor40.i.i.i
  %or.i7.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor40.i.i.i, i32 %xor40.i.i.i, i32 5)
  %xor52.i.i.i = xor i32 %or.i7.i.i.i, %add50.i.i.i
  %add60.i.i.i = add i32 %xor49.i.i.i, 466688987
  %add64.i.i.i = add i32 %add47.i.i.i, %xor52.i.i.i
  %or.i8.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor52.i.i.i, i32 %xor52.i.i.i, i32 6)
  %xor66.i.i.i = xor i32 %or.i8.i.i.i, %add64.i.i.i
  %add67.i.i.i = add i32 %add50.i.i.i, %add60.i.i.i
  %or.i9.i.i.i = tail call i32 @llvm.fshl.i32(i32 %add60.i.i.i, i32 %add60.i.i.i, i32 20)
  %xor69.i.i.i = xor i32 %or.i9.i.i.i, %add67.i.i.i
  %add73.i.i.i = add i32 %xor69.i.i.i, %add64.i.i.i
  %or.i10.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor69.i.i.i, i32 %xor69.i.i.i, i32 17)
  %xor75.i.i.i = xor i32 %or.i10.i.i.i, %add73.i.i.i
  %add76.i.i.i = add i32 %add67.i.i.i, %xor66.i.i.i
  %or.i11.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor66.i.i.i, i32 %xor66.i.i.i, i32 11)
  %xor78.i.i.i = xor i32 %or.i11.i.i.i, %add76.i.i.i
  %add82.i.i.i = add i32 %add73.i.i.i, %xor78.i.i.i
  %or.i12.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor78.i.i.i, i32 %xor78.i.i.i, i32 25)
  %xor84.i.i.i = xor i32 %or.i12.i.i.i, %add82.i.i.i
  %add85.i.i.i = add i32 %xor75.i.i.i, %add76.i.i.i
  %or.i13.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor75.i.i.i, i32 %xor75.i.i.i, i32 10)
  %xor87.i.i.i = xor i32 %or.i13.i.i.i, %add85.i.i.i
  %add91.i.i.i = add i32 %xor87.i.i.i, %add82.i.i.i
  %or.i14.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor87.i.i.i, i32 %xor87.i.i.i, i32 18)
  %xor93.i.i.i = xor i32 %or.i14.i.i.i, %add91.i.i.i
  %add94.i.i.i = add i32 %add85.i.i.i, %xor84.i.i.i
  %or.i15.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor84.i.i.i, i32 %xor84.i.i.i, i32 20)
  %xor96.i.i.i = xor i32 %or.i15.i.i.i, %add94.i.i.i
  %add102.i.i.i = add i32 %add94.i.i.i, 466688986
  %add104.i.i.i = add i32 %xor93.i.i.i, 2
  %add108.i.i.i = add i32 %add91.i.i.i, %xor96.i.i.i
  %or.i16.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor96.i.i.i, i32 %xor96.i.i.i, i32 10)
  %xor110.i.i.i = xor i32 %or.i16.i.i.i, %add108.i.i.i
  %add111.i.i.i = add i32 %add102.i.i.i, %add104.i.i.i
  %or.i17.i.i.i = tail call i32 @llvm.fshl.i32(i32 %add104.i.i.i, i32 %add104.i.i.i, i32 26)
  %xor113.i.i.i = xor i32 %or.i17.i.i.i, %add111.i.i.i
  %add117.i.i.i = add i32 %xor113.i.i.i, %add108.i.i.i
  %or.i18.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor113.i.i.i, i32 %xor113.i.i.i, i32 11)
  %xor119.i.i.i = xor i32 %or.i18.i.i.i, %add117.i.i.i
  %add120.i.i.i = add i32 %add111.i.i.i, %xor110.i.i.i
  %or.i19.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor110.i.i.i, i32 %xor110.i.i.i, i32 21)
  %xor122.i.i.i = xor i32 %or.i19.i.i.i, %add120.i.i.i
  %add126.i.i.i = add i32 %add117.i.i.i, %xor122.i.i.i
  %or.i20.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor122.i.i.i, i32 %xor122.i.i.i, i32 13)
  %xor128.i.i.i = xor i32 %or.i20.i.i.i, %add126.i.i.i
  %add129.i.i.i = add i32 %xor119.i.i.i, %add120.i.i.i
  %or.i21.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor119.i.i.i, i32 %xor119.i.i.i, i32 27)
  %xor131.i.i.i = xor i32 %or.i21.i.i.i, %add129.i.i.i
  %add135.i.i.i = add i32 %xor131.i.i.i, %add126.i.i.i
  %or.i22.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor131.i.i.i, i32 %xor131.i.i.i, i32 23)
  %xor137.i.i.i = xor i32 %or.i22.i.i.i, %add135.i.i.i
  %add138.i.i.i = add i32 %add129.i.i.i, %xor128.i.i.i
  %or.i23.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor128.i.i.i, i32 %xor128.i.i.i, i32 5)
  %xor140.i.i.i = xor i32 %or.i23.i.i.i, %add138.i.i.i
  %add145.i.i.i = add i32 %xor140.i.i.i, 466688986
  %add148.i.i.i = add i32 %xor137.i.i.i, 3
  %add152.i.i.i = add i32 %add135.i.i.i, %add145.i.i.i
  %or.i24.i.i.i = tail call i32 @llvm.fshl.i32(i32 %add145.i.i.i, i32 %add145.i.i.i, i32 6)
  %xor154.i.i.i = xor i32 %or.i24.i.i.i, %add152.i.i.i
  %add155.i.i.i = add i32 %add138.i.i.i, %add148.i.i.i
  %or.i25.i.i.i = tail call i32 @llvm.fshl.i32(i32 %add148.i.i.i, i32 %add148.i.i.i, i32 20)
  %xor157.i.i.i = xor i32 %or.i25.i.i.i, %add155.i.i.i
  %add161.i.i.i = add i32 %xor157.i.i.i, %add152.i.i.i
  %or.i26.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor157.i.i.i, i32 %xor157.i.i.i, i32 17)
  %xor163.i.i.i = xor i32 %or.i26.i.i.i, %add161.i.i.i
  %add164.i.i.i = add i32 %add155.i.i.i, %xor154.i.i.i
  %or.i27.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor154.i.i.i, i32 %xor154.i.i.i, i32 11)
  %xor166.i.i.i = xor i32 %or.i27.i.i.i, %add164.i.i.i
  %add170.i.i.i = add i32 %add161.i.i.i, %xor166.i.i.i
  %or.i28.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor166.i.i.i, i32 %xor166.i.i.i, i32 25)
  %xor172.i.i.i = xor i32 %or.i28.i.i.i, %add170.i.i.i
  %add173.i.i.i = add i32 %xor163.i.i.i, %add164.i.i.i
  %or.i29.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor163.i.i.i, i32 %xor163.i.i.i, i32 10)
  %xor175.i.i.i = xor i32 %or.i29.i.i.i, %add173.i.i.i
  %add179.i.i.i = add i32 %xor175.i.i.i, %add170.i.i.i
  %or.i30.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor175.i.i.i, i32 %xor175.i.i.i, i32 18)
  %xor181.i.i.i = xor i32 %or.i30.i.i.i, %add179.i.i.i
  %add182.i.i.i = add i32 %add173.i.i.i, %xor172.i.i.i
  %or.i31.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor172.i.i.i, i32 %xor172.i.i.i, i32 20)
  %xor184.i.i.i = xor i32 %or.i31.i.i.i, %add182.i.i.i
  %add188.i.i.i = add i32 %add179.i.i.i, 466688986
  %add192.i.i.i = add i32 %xor181.i.i.i, 4
  %add196.i.i.i = add i32 %add188.i.i.i, %xor184.i.i.i
  %or.i32.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor184.i.i.i, i32 %xor184.i.i.i, i32 10)
  %xor198.i.i.i = xor i32 %or.i32.i.i.i, %add196.i.i.i
  %add199.i.i.i = add i32 %add182.i.i.i, %add192.i.i.i
  %or.i33.i.i.i = tail call i32 @llvm.fshl.i32(i32 %add192.i.i.i, i32 %add192.i.i.i, i32 26)
  %xor201.i.i.i = xor i32 %or.i33.i.i.i, %add199.i.i.i
  %add205.i.i.i = add i32 %xor201.i.i.i, %add196.i.i.i
  %or.i34.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor201.i.i.i, i32 %xor201.i.i.i, i32 11)
  %xor207.i.i.i = xor i32 %or.i34.i.i.i, %add205.i.i.i
  %add208.i.i.i = add i32 %add199.i.i.i, %xor198.i.i.i
  %or.i35.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor198.i.i.i, i32 %xor198.i.i.i, i32 21)
  %xor210.i.i.i = xor i32 %or.i35.i.i.i, %add208.i.i.i
  %add214.i.i.i = add i32 %add205.i.i.i, %xor210.i.i.i
  %or.i36.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor210.i.i.i, i32 %xor210.i.i.i, i32 13)
  %xor216.i.i.i = xor i32 %or.i36.i.i.i, %add214.i.i.i
  %add217.i.i.i = add i32 %xor207.i.i.i, %add208.i.i.i
  %or.i37.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor207.i.i.i, i32 %xor207.i.i.i, i32 27)
  %xor219.i.i.i = xor i32 %or.i37.i.i.i, %add217.i.i.i
  %or.i38.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor219.i.i.i, i32 %xor219.i.i.i, i32 23)
  %or.i39.i.i.i = tail call i32 @llvm.fshl.i32(i32 %xor216.i.i.i, i32 %xor216.i.i.i, i32 5)
  %add223.i.i.i = add i32 %xor219.i.i.i, %add214.i.i.i
  %xor225.i.i.i = xor i32 %or.i38.i.i.i, %add223.i.i.i
  %add236.i.i.i = add i32 %xor225.i.i.i, 5
  %add226.i.i.i = add i32 %add217.i.i.i, %xor216.i.i.i
  %xor228.i.i.i = xor i32 %or.i39.i.i.i, %add226.i.i.i
  %retval.sroa.2.0.insert.ext.i.i.i = zext i32 %xor228.i.i.i to i64
  %retval.sroa.2.0.insert.shift.i.i.i = shl nuw i64 %retval.sroa.2.0.insert.ext.i.i.i, 32
  %retval.sroa.0.0.insert.ext.i.i.i = zext i32 %add223.i.i.i to i64
  %retval.sroa.0.0.insert.insert.i.i.i = or i64 %retval.sroa.2.0.insert.shift.i.i.i, %retval.sroa.0.0.insert.ext.i.i.i
  %retval.sroa.5.8.insert.ext.i.i.i = zext i32 %add236.i.i.i to i64
  %retval.sroa.5.8.insert.shift.i.i.i = shl nuw i64 %retval.sroa.5.8.insert.ext.i.i.i, 32
  %retval.sroa.3.8.insert.ext.i.i.i = zext i32 %add226.i.i.i to i64
  %retval.sroa.3.8.insert.insert.i.i.i = or i64 %retval.sroa.5.8.insert.shift.i.i.i, %retval.sroa.3.8.insert.ext.i.i.i
  store i64 %retval.sroa.0.0.insert.insert.i.i.i, ptr %ref.tmp10.sroa.0.0..sroa_idx.i.i.i, align 8
  store i64 1, ptr %arrayidx.i.i, align 8
  br label %_ZN4r1236EngineINS_14ReinterpretCtrI13r123array2x64NS_14Threefry4x32_RILj20EEEEEEclEv.exit.i.i

if.end24.i.i.i:                                   ; preds = %do.body.i.i
  %dec.i.i.i = add i64 %5, -1
  store i64 %dec.i.i.i, ptr %arrayidx.i.i, align 8
  %arrayidx.i46.i.i.i = getelementptr inbounds %"struct.r123::Engine", ptr %e5, i64 0, i32 3, i32 0, i64 %dec.i.i.i
  %7 = load i64, ptr %arrayidx.i46.i.i.i, align 8
  br label %_ZN4r1236EngineINS_14ReinterpretCtrI13r123array2x64NS_14Threefry4x32_RILj20EEEEEEclEv.exit.i.i

_ZN4r1236EngineINS_14ReinterpretCtrI13r123array2x64NS_14Threefry4x32_RILj20EEEEEEclEv.exit.i.i: ; preds = %if.end24.i.i.i, %_ZN13r123array2x644incrEy.exit44.i.i.i
  %8 = phi i64 [ 1, %_ZN13r123array2x644incrEy.exit44.i.i.i ], [ %dec.i.i.i, %if.end24.i.i.i ]
  %retval.1.i.i.i = phi i64 [ %retval.sroa.3.8.insert.insert.i.i.i, %_ZN13r123array2x644incrEy.exit44.i.i.i ], [ %7, %if.end24.i.i.i ]
  %cmp9.not.i.i = icmp ult i64 %retval.1.i.i.i, -4
  br i1 %cmp9.not.i.i, label %_ZNSt24uniform_int_distributionIiEclIN4r1236EngineINS2_14ReinterpretCtrI13r123array2x64NS2_14Threefry4x32_RILj20EEEEEEEEEiRT_.exit, label %do.body.i.i

_ZNSt24uniform_int_distributionIiEclIN4r1236EngineINS2_14ReinterpretCtrI13r123array2x64NS2_14Threefry4x32_RILj20EEEEEEEEEiRT_.exit: ; preds = %_ZN4r1236EngineINS_14ReinterpretCtrI13r123array2x64NS_14Threefry4x32_RILj20EEEEEEclEv.exit.i.i
  %.lcssa = phi i64 [ %8, %_ZN4r1236EngineINS_14ReinterpretCtrI13r123array2x64NS_14Threefry4x32_RILj20EEEEEEclEv.exit.i.i ]
  %inc = add nuw nsw i32 %i.08, 1
  %exitcond.not = icmp eq i32 %inc, 10000
  br i1 %exitcond.not, label %for.cond.cleanup, label %if.then.i.i
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.fshl.i32(i32, i32, i32) #2

attributes #0 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #1 = { argmemonly nocallback nofree nounwind willreturn writeonly }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
