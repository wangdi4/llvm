; RUN: llc < %s
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__test_fn_original(i16 addrspace(1)*, i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i16 addrspace(1)* nocapture) nounwind

declare i64 @_Z13get_global_idj(i32)

define double @_Z6vload3mPU3AS1Ks(i64 %offset, i16 addrspace(1)* nocapture %ptr) nounwind {
entry:
  %res = alloca <3 x i16>, align 8
  %0 = bitcast i16 addrspace(1)* %ptr to i8*
  %mul2 = mul i64 %offset, 6
  %add.ptr = getelementptr inbounds i8, i8* %0, i64 %mul2
  %1 = bitcast <3 x i16>* %res to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %1, i8* %add.ptr, i64 6, i32 1, i1 false)
  %tmp5 = load <3 x i16>, <3 x i16>* %res, align 8
  %tmp7 = bitcast <3 x i16> %tmp5 to i48
  %tmp8 = zext i48 %tmp7 to i64
  %tmp6 = bitcast i64 %tmp8 to double
  ret double %tmp6
}

declare void @____Vectorized_.test_fn_original(i16 addrspace(1)*, i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i16 addrspace(1)* nocapture) nounwind

define i1 @allOne(i1 %pred) nounwind readnone {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %pred) nounwind readnone {
entry:
  %t = xor i1 %pred, true
  ret i1 %t
}

declare void @dummybarrier.()

declare void @_Z7barrierm(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define  void @test_fn(i16 addrspace(1)* %src, i32 addrspace(1)* nocapture %offsets, i32 addrspace(1)* nocapture %alignmentOffsets, i16 addrspace(1)* nocapture %results, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* byval %BaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind {
; <label>:0
  %res.i = alloca <3 x i16>, align 8
  br label %SyncBB4

SyncBB4:                                          ; preds = %0, %thenBB
  %currWI.0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %1 = getelementptr %struct.PaddedDimId, %struct.PaddedDimId* %pLocalIds, i64 %currWI.0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId, %struct.PaddedDimId* %BaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = trunc i64 %5 to i32
  %7 = sext i32 %6 to i64
  %8 = getelementptr inbounds i32, i32 addrspace(1)* %offsets, i64 %7
  %9 = load i32 addrspace(1)* %8, align 4
  %10 = zext i32 %9 to i64
  %11 = getelementptr inbounds i32, i32 addrspace(1)* %alignmentOffsets, i64 %7
  %12 = load i32 addrspace(1)* %11, align 4
  %13 = zext i32 %12 to i64
  %14 = getelementptr inbounds i16, i16 addrspace(1)* %src, i64 %13
  %15 = bitcast i16 addrspace(1)* %14 to i8*
  %mul2.i = mul i64 %10, 6
  %add.ptr.i = getelementptr inbounds i8, i8* %15, i64 %mul2.i
  %16 = bitcast <3 x i16>* %res.i to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %16, i8* %add.ptr.i, i64 6, i32 1, i1 false) nounwind
  %tmp5.i = load <3 x i16>, <3 x i16>* %res.i, align 8
  %tmp7.i = bitcast <3 x i16> %tmp5.i to i48
  %tmp2 = bitcast i48 %tmp7.i to <3 x i16>
  %17 = extractelement <3 x i16> %tmp2, i32 0
  %18 = mul nsw i32 %6, 3
  %19 = sext i32 %18 to i64
  %20 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %19
  store i16 %17, i16 addrspace(1)* %20, align 2
  %21 = extractelement <3 x i16> %tmp2, i32 1
  %22 = add nsw i32 %18, 1
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %23
  store i16 %21, i16 addrspace(1)* %24, align 2
  %25 = extractelement <3 x i16> %tmp2, i32 2
  %26 = add nsw i32 %18, 2
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %27
  store i16 %25, i16 addrspace(1)* %28, align 2
  %check.WI.iter = icmp ult i64 %currWI.0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %SyncBB4
  %"CurrWI++" = add nuw i64 %currWI.0, 1
  br label %SyncBB4

SyncBB:                                           ; preds = %SyncBB4
  ret void
}

define  void @__Vectorized_.test_fn(i16 addrspace(1)* %src, i32 addrspace(1)* nocapture %offsets, i32 addrspace(1)* nocapture %alignmentOffsets, i16 addrspace(1)* nocapture %results, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* byval %BaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind {
; <label>:0
  %res.i16 = alloca <3 x i16>, align 8
  %res.i9 = alloca <3 x i16>, align 8
  %res.i2 = alloca <3 x i16>, align 8
  %res.i = alloca <3 x i16>, align 8
  br label %SyncBB65

SyncBB65:                                         ; preds = %0, %thenBB
  %currWI.0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %1 = getelementptr %struct.PaddedDimId, %struct.PaddedDimId* %pLocalIds, i64 %currWI.0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId, %struct.PaddedDimId* %BaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %broadcast1 = insertelement <4 x i64> undef, i64 %5, i32 0
  %broadcast2 = shufflevector <4 x i64> %broadcast1, <4 x i64> undef, <4 x i32> zeroinitializer
  %6 = add <4 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3>
  %7 = trunc <4 x i64> %6 to <4 x i32>
  %8 = extractelement <4 x i32> %7, i32 0
  %9 = sext i32 %8 to i64
  %10 = getelementptr inbounds i32, i32 addrspace(1)* %offsets, i64 %9
  %ptrTypeCast = bitcast i32 addrspace(1)* %10 to <4 x i32> addrspace(1)*
  %11 = load <4 x i32>, <4 x i32> addrspace(1)* %ptrTypeCast, align 4
  %12 = zext <4 x i32> %11 to <4 x i64>
  %extract11 = extractelement <4 x i64> %12, i32 0
  %extract12 = extractelement <4 x i64> %12, i32 1
  %extract13 = extractelement <4 x i64> %12, i32 2
  %extract14 = extractelement <4 x i64> %12, i32 3
  %13 = extractelement <4 x i32> %7, i32 0
  %14 = sext i32 %13 to i64
  %15 = getelementptr inbounds i32, i32 addrspace(1)* %alignmentOffsets, i64 %14
  %ptrTypeCast6 = bitcast i32 addrspace(1)* %15 to <4 x i32> addrspace(1)*
  %16 = load <4 x i32>, <4 x i32> addrspace(1)* %ptrTypeCast6, align 4
  %17 = extractelement <4 x i32> %16, i32 0
  %18 = zext i32 %17 to i64
  %19 = getelementptr inbounds i16, i16 addrspace(1)* %src, i64 %18
  %20 = extractelement <4 x i32> %16, i32 1
  %21 = zext i32 %20 to i64
  %22 = getelementptr inbounds i16, i16 addrspace(1)* %src, i64 %21
  %23 = extractelement <4 x i32> %16, i32 2
  %24 = zext i32 %23 to i64
  %25 = getelementptr inbounds i16, i16 addrspace(1)* %src, i64 %24
  %26 = extractelement <4 x i32> %16, i32 3
  %27 = zext i32 %26 to i64
  %28 = getelementptr inbounds i16, i16 addrspace(1)* %src, i64 %27
  %29 = bitcast i16 addrspace(1)* %19 to i8*
  %mul2.i = mul i64 %extract11, 6
  %add.ptr.i = getelementptr inbounds i8, i8* %29, i64 %mul2.i
  %30 = bitcast <3 x i16>* %res.i to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %30, i8* %add.ptr.i, i64 6, i32 1, i1 false) nounwind
  %tmp5.i = load <3 x i16>, <3 x i16>* %res.i, align 8
  %tmp7.i = bitcast <3 x i16> %tmp5.i to i48
  %tmp8.i = zext i48 %tmp7.i to i64
  %tmp6.i = bitcast i64 %tmp8.i to double
  %31 = bitcast i16 addrspace(1)* %22 to i8*
  %mul2.i3 = mul i64 %extract12, 6
  %add.ptr.i4 = getelementptr inbounds i8, i8* %31, i64 %mul2.i3
  %32 = bitcast <3 x i16>* %res.i2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %32, i8* %add.ptr.i4, i64 6, i32 1, i1 false) nounwind
  %tmp5.i5 = load <3 x i16>, <3 x i16>* %res.i2, align 8
  %tmp7.i6 = bitcast <3 x i16> %tmp5.i5 to i48
  %tmp8.i7 = zext i48 %tmp7.i6 to i64
  %tmp6.i8 = bitcast i64 %tmp8.i7 to double
  %33 = bitcast i16 addrspace(1)* %25 to i8*
  %mul2.i10 = mul i64 %extract13, 6
  %add.ptr.i11 = getelementptr inbounds i8, i8* %33, i64 %mul2.i10
  %34 = bitcast <3 x i16>* %res.i9 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %34, i8* %add.ptr.i11, i64 6, i32 1, i1 false) nounwind
  %tmp5.i12 = load <3 x i16>, <3 x i16>* %res.i9, align 8
  %tmp7.i13 = bitcast <3 x i16> %tmp5.i12 to i48
  %tmp8.i14 = zext i48 %tmp7.i13 to i64
  %tmp6.i15 = bitcast i64 %tmp8.i14 to double
  %35 = bitcast i16 addrspace(1)* %28 to i8*
  %mul2.i17 = mul i64 %extract14, 6
  %add.ptr.i18 = getelementptr inbounds i8, i8* %35, i64 %mul2.i17
  %36 = bitcast <3 x i16>* %res.i16 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %36, i8* %add.ptr.i18, i64 6, i32 1, i1 false) nounwind
  %tmp5.i19 = load <3 x i16>, <3 x i16>* %res.i16, align 8
  %tmp7.i20 = bitcast <3 x i16> %tmp5.i19 to i48
  %tmp8.i21 = zext i48 %tmp7.i20 to i64
  %tmp6.i22 = bitcast i64 %tmp8.i21 to double
  %temp.vect = insertelement <4 x double> undef, double %tmp6.i, i32 0
  %temp.vect15 = insertelement <4 x double> %temp.vect, double %tmp6.i8, i32 1
  %temp.vect16 = insertelement <4 x double> %temp.vect15, double %tmp6.i15, i32 2
  %temp.vect17 = insertelement <4 x double> %temp.vect16, double %tmp6.i22, i32 3
  %tmp318 = bitcast <4 x double> %temp.vect17 to <4 x i64>
  %tmp119 = trunc <4 x i64> %tmp318 to <4 x i48>
  %extract20 = extractelement <4 x i48> %tmp119, i32 0
  %extract21 = extractelement <4 x i48> %tmp119, i32 1
  %extract22 = extractelement <4 x i48> %tmp119, i32 2
  %extract23 = extractelement <4 x i48> %tmp119, i32 3
  %37 = bitcast i48 %extract20 to <3 x i16>
  %38 = bitcast i48 %extract21 to <3 x i16>
  %39 = bitcast i48 %extract22 to <3 x i16>
  %40 = bitcast i48 %extract23 to <3 x i16>
  %extract45 = extractelement <3 x i16> %37, i32 0
  %extract46 = extractelement <3 x i16> %38, i32 0
  %extract47 = extractelement <3 x i16> %39, i32 0
  %extract48 = extractelement <3 x i16> %40, i32 0
  %extract53 = extractelement <3 x i16> %37, i32 1
  %extract54 = extractelement <3 x i16> %38, i32 1
  %extract55 = extractelement <3 x i16> %39, i32 1
  %extract56 = extractelement <3 x i16> %40, i32 1
  %extract61 = extractelement <3 x i16> %37, i32 2
  %extract62 = extractelement <3 x i16> %38, i32 2
  %extract63 = extractelement <3 x i16> %39, i32 2
  %extract64 = extractelement <3 x i16> %40, i32 2
  %41 = mul nsw <4 x i32> %7, <i32 3, i32 3, i32 3, i32 3>
  %42 = extractelement <4 x i32> %41, i32 0
  %43 = sext i32 %42 to i64
  %44 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %43
  %45 = extractelement <4 x i32> %41, i32 1
  %46 = sext i32 %45 to i64
  %47 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %46
  %48 = extractelement <4 x i32> %41, i32 2
  %49 = sext i32 %48 to i64
  %50 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %49
  %51 = extractelement <4 x i32> %41, i32 3
  %52 = sext i32 %51 to i64
  %53 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %52
  store i16 %extract45, i16 addrspace(1)* %44, align 2
  store i16 %extract46, i16 addrspace(1)* %47, align 2
  store i16 %extract47, i16 addrspace(1)* %50, align 2
  store i16 %extract48, i16 addrspace(1)* %53, align 2
  %54 = add nsw <4 x i32> %41, <i32 1, i32 1, i32 1, i32 1>
  %55 = extractelement <4 x i32> %54, i32 0
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %56
  %58 = extractelement <4 x i32> %54, i32 1
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %59
  %61 = extractelement <4 x i32> %54, i32 2
  %62 = sext i32 %61 to i64
  %63 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %62
  %64 = extractelement <4 x i32> %54, i32 3
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %65
  store i16 %extract53, i16 addrspace(1)* %57, align 2
  store i16 %extract54, i16 addrspace(1)* %60, align 2
  store i16 %extract55, i16 addrspace(1)* %63, align 2
  store i16 %extract56, i16 addrspace(1)* %66, align 2
  %67 = add nsw <4 x i32> %41, <i32 2, i32 2, i32 2, i32 2>
  %68 = extractelement <4 x i32> %67, i32 0
  %69 = sext i32 %68 to i64
  %70 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %69
  %71 = extractelement <4 x i32> %67, i32 1
  %72 = sext i32 %71 to i64
  %73 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %72
  %74 = extractelement <4 x i32> %67, i32 2
  %75 = sext i32 %74 to i64
  %76 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %75
  %77 = extractelement <4 x i32> %67, i32 3
  %78 = sext i32 %77 to i64
  %79 = getelementptr inbounds i16, i16 addrspace(1)* %results, i64 %78
  store i16 %extract61, i16 addrspace(1)* %70, align 2
  store i16 %extract62, i16 addrspace(1)* %73, align 2
  store i16 %extract63, i16 addrspace(1)* %76, align 2
  store i16 %extract64, i16 addrspace(1)* %79, align 2
  %check.WI.iter = icmp ult i64 %currWI.0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %SyncBB65
  %"CurrWI++" = add nuw i64 %currWI.0, 1
  br label %SyncBB65

SyncBB:                                           ; preds = %SyncBB65
  ret void
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture, i64, i32, i1) nounwind

declare void @SvmlThunk(i8*)

!opencl.kernels = !{!0}

!0 = !{void (i16 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i16 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @test_fn, !1, !1, !"", !"short __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, short __attribute__((address_space(1))) *", !"opencl_test_fn_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 1, i32 1, i32 1, i32 1}
!3 = !{i32 3, i32 3, i32 3, i32 3}
!4 = !{!"short*", !"uint*", !"uint*", !"short*"}
!5 = !{!"src", !"offsets", !"alignmentOffsets", !"results"}
