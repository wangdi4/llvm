; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__bitonicSort_original(i32 addrspace(1)* nocapture, i32, i32, i32, i32) nounwind

declare i64 @get_global_id(i32)

declare void @____Vectorized_.bitonicSort_original(i32 addrspace(1)* nocapture, i32, i32, i32, i32) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__bitonicSort_separated_args(i32 addrspace(1)* nocapture %theArray, i32 %stage, i32 %passOfStage, i32 %width, i32 %direction, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = sub i32 %stage, %passOfStage
  %1 = shl i32 1, %0
  %2 = icmp eq i32 %1, 0
  %3 = select i1 %2, i32 1, i32 %1
  %4 = shl i32 %1, 1
  %5 = shl i32 1, %stage
  %6 = icmp eq i32 %5, 0
  %7 = select i1 %6, i32 1, i32 %5
  %8 = sub i32 1, %direction
  br label %SyncBB1

SyncBB1:                                          ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %9 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %10 = load i64* %9, align 8
  %11 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %12 = load i64* %11, align 8
  %13 = add i64 %10, %12
  %14 = trunc i64 %13 to i32
  %15 = urem i32 %14, %3
  %16 = udiv i32 %14, %3
  %17 = mul i32 %4, %16
  %18 = add i32 %17, %15
  %19 = add i32 %18, %1
  %20 = zext i32 %18 to i64
  %21 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %20
  %22 = load i32 addrspace(1)* %21, align 4
  %23 = zext i32 %19 to i64
  %24 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %23
  %25 = load i32 addrspace(1)* %24, align 4
  %26 = udiv i32 %14, %7
  %27 = and i32 %26, 1
  %28 = icmp eq i32 %27, 0
  %direction. = select i1 %28, i32 %direction, i32 %8
  %29 = icmp ugt i32 %22, %25
  %greater.0 = select i1 %29, i32 %22, i32 %25
  %lesser.0 = select i1 %29, i32 %25, i32 %22
  %30 = icmp eq i32 %direction., 0
  %storemerge1 = select i1 %30, i32 %greater.0, i32 %lesser.0
  %storemerge = select i1 %30, i32 %lesser.0, i32 %greater.0
  store i32 %storemerge1, i32 addrspace(1)* %21, align 4
  store i32 %storemerge, i32 addrspace(1)* %24, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %SyncBB1
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB1

SyncBB:                                           ; preds = %SyncBB1
  ret void
}

define void @____Vectorized_.bitonicSort_separated_args(i32 addrspace(1)* nocapture %theArray, i32 %stage, i32 %passOfStage, i32 %width, i32 %direction, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
UnifiedReturnBlock:
  %temp48 = insertelement <16 x i32> undef, i32 %direction, i32 0
  %vector49 = shufflevector <16 x i32> %temp48, <16 x i32> undef, <16 x i32> zeroinitializer
  %0 = sub i32 %stage, %passOfStage
  %1 = shl i32 1, %0
  %temp13 = insertelement <16 x i32> undef, i32 %1, i32 0
  %vector14 = shufflevector <16 x i32> %temp13, <16 x i32> undef, <16 x i32> zeroinitializer
  %2 = icmp eq i32 %1, 0
  %3 = select i1 %2, i32 1, i32 %1
  %temp = insertelement <16 x i32> undef, i32 %3, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %4 = shl i32 %1, 1
  %temp11 = insertelement <16 x i32> undef, i32 %4, i32 0
  %vector12 = shufflevector <16 x i32> %temp11, <16 x i32> undef, <16 x i32> zeroinitializer
  %5 = shl i32 1, %stage
  %6 = icmp eq i32 %5, 0
  %7 = select i1 %6, i32 1, i32 %5
  %temp46 = insertelement <16 x i32> undef, i32 %7, i32 0
  %vector47 = shufflevector <16 x i32> %temp46, <16 x i32> undef, <16 x i32> zeroinitializer
  %8 = sub i32 1, %direction
  %temp50 = insertelement <16 x i32> undef, i32 %8, i32 0
  %vector51 = shufflevector <16 x i32> %temp50, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %UnifiedReturnBlock
  %CurrWI..0 = phi i64 [ 0, %UnifiedReturnBlock ], [ %"CurrWI++", %thenBB ]
  %9 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %10 = load i64* %9, align 8
  %11 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %12 = load i64* %11, align 8
  %13 = add i64 %10, %12
  %broadcast1 = insertelement <16 x i64> undef, i64 %13, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %14 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %15 = trunc <16 x i64> %14 to <16 x i32>
  %16 = urem <16 x i32> %15, %vector
  %17 = udiv <16 x i32> %15, %vector
  %18 = mul <16 x i32> %vector12, %17
  %19 = add <16 x i32> %18, %16
  %20 = add <16 x i32> %19, %vector14
  %21 = extractelement <16 x i32> %19, i32 0
  %22 = zext i32 %21 to i64
  %23 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %22
  %24 = extractelement <16 x i32> %19, i32 1
  %25 = zext i32 %24 to i64
  %26 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %25
  %27 = extractelement <16 x i32> %19, i32 2
  %28 = zext i32 %27 to i64
  %29 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %28
  %30 = extractelement <16 x i32> %19, i32 3
  %31 = zext i32 %30 to i64
  %32 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %31
  %33 = extractelement <16 x i32> %19, i32 4
  %34 = zext i32 %33 to i64
  %35 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %34
  %36 = extractelement <16 x i32> %19, i32 5
  %37 = zext i32 %36 to i64
  %38 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %37
  %39 = extractelement <16 x i32> %19, i32 6
  %40 = zext i32 %39 to i64
  %41 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %40
  %42 = extractelement <16 x i32> %19, i32 7
  %43 = zext i32 %42 to i64
  %44 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %43
  %45 = extractelement <16 x i32> %19, i32 8
  %46 = zext i32 %45 to i64
  %47 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %46
  %48 = extractelement <16 x i32> %19, i32 9
  %49 = zext i32 %48 to i64
  %50 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %49
  %51 = extractelement <16 x i32> %19, i32 10
  %52 = zext i32 %51 to i64
  %53 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %52
  %54 = extractelement <16 x i32> %19, i32 11
  %55 = zext i32 %54 to i64
  %56 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %55
  %57 = extractelement <16 x i32> %19, i32 12
  %58 = zext i32 %57 to i64
  %59 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %58
  %60 = extractelement <16 x i32> %19, i32 13
  %61 = zext i32 %60 to i64
  %62 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %61
  %63 = extractelement <16 x i32> %19, i32 14
  %64 = zext i32 %63 to i64
  %65 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %64
  %66 = extractelement <16 x i32> %19, i32 15
  %67 = zext i32 %66 to i64
  %68 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %67
  %69 = load i32 addrspace(1)* %23, align 4
  %70 = load i32 addrspace(1)* %26, align 4
  %71 = load i32 addrspace(1)* %29, align 4
  %72 = load i32 addrspace(1)* %32, align 4
  %73 = load i32 addrspace(1)* %35, align 4
  %74 = load i32 addrspace(1)* %38, align 4
  %75 = load i32 addrspace(1)* %41, align 4
  %76 = load i32 addrspace(1)* %44, align 4
  %77 = load i32 addrspace(1)* %47, align 4
  %78 = load i32 addrspace(1)* %50, align 4
  %79 = load i32 addrspace(1)* %53, align 4
  %80 = load i32 addrspace(1)* %56, align 4
  %81 = load i32 addrspace(1)* %59, align 4
  %82 = load i32 addrspace(1)* %62, align 4
  %83 = load i32 addrspace(1)* %65, align 4
  %84 = load i32 addrspace(1)* %68, align 4
  %temp.vect = insertelement <16 x i32> undef, i32 %69, i32 0
  %temp.vect53 = insertelement <16 x i32> %temp.vect, i32 %70, i32 1
  %temp.vect54 = insertelement <16 x i32> %temp.vect53, i32 %71, i32 2
  %temp.vect55 = insertelement <16 x i32> %temp.vect54, i32 %72, i32 3
  %temp.vect56 = insertelement <16 x i32> %temp.vect55, i32 %73, i32 4
  %temp.vect57 = insertelement <16 x i32> %temp.vect56, i32 %74, i32 5
  %temp.vect58 = insertelement <16 x i32> %temp.vect57, i32 %75, i32 6
  %temp.vect59 = insertelement <16 x i32> %temp.vect58, i32 %76, i32 7
  %temp.vect60 = insertelement <16 x i32> %temp.vect59, i32 %77, i32 8
  %temp.vect61 = insertelement <16 x i32> %temp.vect60, i32 %78, i32 9
  %temp.vect62 = insertelement <16 x i32> %temp.vect61, i32 %79, i32 10
  %temp.vect63 = insertelement <16 x i32> %temp.vect62, i32 %80, i32 11
  %temp.vect64 = insertelement <16 x i32> %temp.vect63, i32 %81, i32 12
  %temp.vect65 = insertelement <16 x i32> %temp.vect64, i32 %82, i32 13
  %temp.vect66 = insertelement <16 x i32> %temp.vect65, i32 %83, i32 14
  %temp.vect67 = insertelement <16 x i32> %temp.vect66, i32 %84, i32 15
  %85 = extractelement <16 x i32> %20, i32 0
  %86 = zext i32 %85 to i64
  %87 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %86
  %88 = extractelement <16 x i32> %20, i32 1
  %89 = zext i32 %88 to i64
  %90 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %89
  %91 = extractelement <16 x i32> %20, i32 2
  %92 = zext i32 %91 to i64
  %93 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %92
  %94 = extractelement <16 x i32> %20, i32 3
  %95 = zext i32 %94 to i64
  %96 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %95
  %97 = extractelement <16 x i32> %20, i32 4
  %98 = zext i32 %97 to i64
  %99 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %98
  %100 = extractelement <16 x i32> %20, i32 5
  %101 = zext i32 %100 to i64
  %102 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %101
  %103 = extractelement <16 x i32> %20, i32 6
  %104 = zext i32 %103 to i64
  %105 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %104
  %106 = extractelement <16 x i32> %20, i32 7
  %107 = zext i32 %106 to i64
  %108 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %107
  %109 = extractelement <16 x i32> %20, i32 8
  %110 = zext i32 %109 to i64
  %111 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %110
  %112 = extractelement <16 x i32> %20, i32 9
  %113 = zext i32 %112 to i64
  %114 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %113
  %115 = extractelement <16 x i32> %20, i32 10
  %116 = zext i32 %115 to i64
  %117 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %116
  %118 = extractelement <16 x i32> %20, i32 11
  %119 = zext i32 %118 to i64
  %120 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %119
  %121 = extractelement <16 x i32> %20, i32 12
  %122 = zext i32 %121 to i64
  %123 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %122
  %124 = extractelement <16 x i32> %20, i32 13
  %125 = zext i32 %124 to i64
  %126 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %125
  %127 = extractelement <16 x i32> %20, i32 14
  %128 = zext i32 %127 to i64
  %129 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %128
  %130 = extractelement <16 x i32> %20, i32 15
  %131 = zext i32 %130 to i64
  %132 = getelementptr inbounds i32 addrspace(1)* %theArray, i64 %131
  %133 = load i32 addrspace(1)* %87, align 4
  %134 = load i32 addrspace(1)* %90, align 4
  %135 = load i32 addrspace(1)* %93, align 4
  %136 = load i32 addrspace(1)* %96, align 4
  %137 = load i32 addrspace(1)* %99, align 4
  %138 = load i32 addrspace(1)* %102, align 4
  %139 = load i32 addrspace(1)* %105, align 4
  %140 = load i32 addrspace(1)* %108, align 4
  %141 = load i32 addrspace(1)* %111, align 4
  %142 = load i32 addrspace(1)* %114, align 4
  %143 = load i32 addrspace(1)* %117, align 4
  %144 = load i32 addrspace(1)* %120, align 4
  %145 = load i32 addrspace(1)* %123, align 4
  %146 = load i32 addrspace(1)* %126, align 4
  %147 = load i32 addrspace(1)* %129, align 4
  %148 = load i32 addrspace(1)* %132, align 4
  %temp.vect68 = insertelement <16 x i32> undef, i32 %133, i32 0
  %temp.vect69 = insertelement <16 x i32> %temp.vect68, i32 %134, i32 1
  %temp.vect70 = insertelement <16 x i32> %temp.vect69, i32 %135, i32 2
  %temp.vect71 = insertelement <16 x i32> %temp.vect70, i32 %136, i32 3
  %temp.vect72 = insertelement <16 x i32> %temp.vect71, i32 %137, i32 4
  %temp.vect73 = insertelement <16 x i32> %temp.vect72, i32 %138, i32 5
  %temp.vect74 = insertelement <16 x i32> %temp.vect73, i32 %139, i32 6
  %temp.vect75 = insertelement <16 x i32> %temp.vect74, i32 %140, i32 7
  %temp.vect76 = insertelement <16 x i32> %temp.vect75, i32 %141, i32 8
  %temp.vect77 = insertelement <16 x i32> %temp.vect76, i32 %142, i32 9
  %temp.vect78 = insertelement <16 x i32> %temp.vect77, i32 %143, i32 10
  %temp.vect79 = insertelement <16 x i32> %temp.vect78, i32 %144, i32 11
  %temp.vect80 = insertelement <16 x i32> %temp.vect79, i32 %145, i32 12
  %temp.vect81 = insertelement <16 x i32> %temp.vect80, i32 %146, i32 13
  %temp.vect82 = insertelement <16 x i32> %temp.vect81, i32 %147, i32 14
  %temp.vect83 = insertelement <16 x i32> %temp.vect82, i32 %148, i32 15
  %149 = udiv <16 x i32> %15, %vector47
  %150 = and <16 x i32> %149, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %151 = icmp eq <16 x i32> %150, zeroinitializer
  %direction.52 = select <16 x i1> %151, <16 x i32> %vector49, <16 x i32> %vector51
  %152 = icmp ugt <16 x i32> %temp.vect67, %temp.vect83
  %greater.084 = select <16 x i1> %152, <16 x i32> %temp.vect67, <16 x i32> %temp.vect83
  %lesser.085 = select <16 x i1> %152, <16 x i32> %temp.vect83, <16 x i32> %temp.vect67
  %153 = icmp eq <16 x i32> %direction.52, zeroinitializer
  %merge1087 = select <16 x i1> %153, <16 x i32> %lesser.085, <16 x i32> %greater.084
  %extract105 = extractelement <16 x i32> %merge1087, i32 0
  %extract106 = extractelement <16 x i32> %merge1087, i32 1
  %extract107 = extractelement <16 x i32> %merge1087, i32 2
  %extract108 = extractelement <16 x i32> %merge1087, i32 3
  %extract109 = extractelement <16 x i32> %merge1087, i32 4
  %extract110 = extractelement <16 x i32> %merge1087, i32 5
  %extract111 = extractelement <16 x i32> %merge1087, i32 6
  %extract112 = extractelement <16 x i32> %merge1087, i32 7
  %extract113 = extractelement <16 x i32> %merge1087, i32 8
  %extract114 = extractelement <16 x i32> %merge1087, i32 9
  %extract115 = extractelement <16 x i32> %merge1087, i32 10
  %extract116 = extractelement <16 x i32> %merge1087, i32 11
  %extract117 = extractelement <16 x i32> %merge1087, i32 12
  %extract118 = extractelement <16 x i32> %merge1087, i32 13
  %extract119 = extractelement <16 x i32> %merge1087, i32 14
  %extract120 = extractelement <16 x i32> %merge1087, i32 15
  %merge88 = select <16 x i1> %153, <16 x i32> %greater.084, <16 x i32> %lesser.085
  %extract89 = extractelement <16 x i32> %merge88, i32 0
  %extract90 = extractelement <16 x i32> %merge88, i32 1
  %extract91 = extractelement <16 x i32> %merge88, i32 2
  %extract92 = extractelement <16 x i32> %merge88, i32 3
  %extract93 = extractelement <16 x i32> %merge88, i32 4
  %extract94 = extractelement <16 x i32> %merge88, i32 5
  %extract95 = extractelement <16 x i32> %merge88, i32 6
  %extract96 = extractelement <16 x i32> %merge88, i32 7
  %extract97 = extractelement <16 x i32> %merge88, i32 8
  %extract98 = extractelement <16 x i32> %merge88, i32 9
  %extract99 = extractelement <16 x i32> %merge88, i32 10
  %extract100 = extractelement <16 x i32> %merge88, i32 11
  %extract101 = extractelement <16 x i32> %merge88, i32 12
  %extract102 = extractelement <16 x i32> %merge88, i32 13
  %extract103 = extractelement <16 x i32> %merge88, i32 14
  %extract104 = extractelement <16 x i32> %merge88, i32 15
  store i32 %extract89, i32 addrspace(1)* %23, align 4
  store i32 %extract90, i32 addrspace(1)* %26, align 4
  store i32 %extract91, i32 addrspace(1)* %29, align 4
  store i32 %extract92, i32 addrspace(1)* %32, align 4
  store i32 %extract93, i32 addrspace(1)* %35, align 4
  store i32 %extract94, i32 addrspace(1)* %38, align 4
  store i32 %extract95, i32 addrspace(1)* %41, align 4
  store i32 %extract96, i32 addrspace(1)* %44, align 4
  store i32 %extract97, i32 addrspace(1)* %47, align 4
  store i32 %extract98, i32 addrspace(1)* %50, align 4
  store i32 %extract99, i32 addrspace(1)* %53, align 4
  store i32 %extract100, i32 addrspace(1)* %56, align 4
  store i32 %extract101, i32 addrspace(1)* %59, align 4
  store i32 %extract102, i32 addrspace(1)* %62, align 4
  store i32 %extract103, i32 addrspace(1)* %65, align 4
  store i32 %extract104, i32 addrspace(1)* %68, align 4
  store i32 %extract105, i32 addrspace(1)* %87, align 4
  store i32 %extract106, i32 addrspace(1)* %90, align 4
  store i32 %extract107, i32 addrspace(1)* %93, align 4
  store i32 %extract108, i32 addrspace(1)* %96, align 4
  store i32 %extract109, i32 addrspace(1)* %99, align 4
  store i32 %extract110, i32 addrspace(1)* %102, align 4
  store i32 %extract111, i32 addrspace(1)* %105, align 4
  store i32 %extract112, i32 addrspace(1)* %108, align 4
  store i32 %extract113, i32 addrspace(1)* %111, align 4
  store i32 %extract114, i32 addrspace(1)* %114, align 4
  store i32 %extract115, i32 addrspace(1)* %117, align 4
  store i32 %extract116, i32 addrspace(1)* %120, align 4
  store i32 %extract117, i32 addrspace(1)* %123, align 4
  store i32 %extract118, i32 addrspace(1)* %126, align 4
  store i32 %extract119, i32 addrspace(1)* %129, align 4
  store i32 %extract120, i32 addrspace(1)* %132, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB121

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB121:                                        ; preds = %SyncBB
  ret void
}

define void @bitonicSort(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 12
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 20
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 48
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  %20 = sub i32 %4, %7
  %21 = shl i32 1, %20
  %22 = icmp eq i32 %21, 0
  %23 = select i1 %22, i32 1, i32 %21
  %24 = shl i32 %21, 1
  %25 = shl i32 1, %4
  %26 = icmp eq i32 %25, 0
  %27 = select i1 %26, i32 1, i32 %25
  %28 = sub i32 1, %10
  br label %SyncBB1.i

SyncBB1.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %29 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %30 = load i64* %29, align 8
  %31 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %32 = load i64* %31, align 8
  %33 = add i64 %30, %32
  %34 = trunc i64 %33 to i32
  %35 = urem i32 %34, %23
  %36 = udiv i32 %34, %23
  %37 = mul i32 %24, %36
  %38 = add i32 %37, %35
  %39 = add i32 %38, %21
  %40 = zext i32 %38 to i64
  %41 = getelementptr inbounds i32 addrspace(1)* %1, i64 %40
  %42 = load i32 addrspace(1)* %41, align 4
  %43 = zext i32 %39 to i64
  %44 = getelementptr inbounds i32 addrspace(1)* %1, i64 %43
  %45 = load i32 addrspace(1)* %44, align 4
  %46 = udiv i32 %34, %27
  %47 = and i32 %46, 1
  %48 = icmp eq i32 %47, 0
  %direction..i = select i1 %48, i32 %10, i32 %28
  %49 = icmp ugt i32 %42, %45
  %greater.0.i = select i1 %49, i32 %42, i32 %45
  %lesser.0.i = select i1 %49, i32 %45, i32 %42
  %50 = icmp eq i32 %direction..i, 0
  %storemerge1.i = select i1 %50, i32 %greater.0.i, i32 %lesser.0.i
  %storemerge.i = select i1 %50, i32 %lesser.0.i, i32 %greater.0.i
  store i32 %storemerge1.i, i32 addrspace(1)* %41, align 4
  store i32 %storemerge.i, i32 addrspace(1)* %44, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %__bitonicSort_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB1.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB1.i

__bitonicSort_separated_args.exit:                ; preds = %SyncBB1.i
  ret void
}

define void @__Vectorized_.bitonicSort(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 12
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 20
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 48
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  %temp48.i = insertelement <16 x i32> undef, i32 %10, i32 0
  %vector49.i = shufflevector <16 x i32> %temp48.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %20 = sub i32 %4, %7
  %21 = shl i32 1, %20
  %temp13.i = insertelement <16 x i32> undef, i32 %21, i32 0
  %vector14.i = shufflevector <16 x i32> %temp13.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %22 = icmp eq i32 %21, 0
  %23 = select i1 %22, i32 1, i32 %21
  %temp.i = insertelement <16 x i32> undef, i32 %23, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %24 = shl i32 %21, 1
  %temp11.i = insertelement <16 x i32> undef, i32 %24, i32 0
  %vector12.i = shufflevector <16 x i32> %temp11.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %25 = shl i32 1, %4
  %26 = icmp eq i32 %25, 0
  %27 = select i1 %26, i32 1, i32 %25
  %temp46.i = insertelement <16 x i32> undef, i32 %27, i32 0
  %vector47.i = shufflevector <16 x i32> %temp46.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %28 = sub i32 1, %10
  %temp50.i = insertelement <16 x i32> undef, i32 %28, i32 0
  %vector51.i = shufflevector <16 x i32> %temp50.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %29 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %30 = load i64* %29, align 8
  %31 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %32 = load i64* %31, align 8
  %33 = add i64 %30, %32
  %broadcast1.i = insertelement <16 x i64> undef, i64 %33, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %34 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %35 = trunc <16 x i64> %34 to <16 x i32>
  %36 = urem <16 x i32> %35, %vector.i
  %37 = udiv <16 x i32> %35, %vector.i
  %38 = mul <16 x i32> %vector12.i, %37
  %39 = add <16 x i32> %38, %36
  %40 = add <16 x i32> %39, %vector14.i
  %41 = extractelement <16 x i32> %39, i32 0
  %42 = zext i32 %41 to i64
  %43 = getelementptr inbounds i32 addrspace(1)* %1, i64 %42
  %44 = extractelement <16 x i32> %39, i32 1
  %45 = zext i32 %44 to i64
  %46 = getelementptr inbounds i32 addrspace(1)* %1, i64 %45
  %47 = extractelement <16 x i32> %39, i32 2
  %48 = zext i32 %47 to i64
  %49 = getelementptr inbounds i32 addrspace(1)* %1, i64 %48
  %50 = extractelement <16 x i32> %39, i32 3
  %51 = zext i32 %50 to i64
  %52 = getelementptr inbounds i32 addrspace(1)* %1, i64 %51
  %53 = extractelement <16 x i32> %39, i32 4
  %54 = zext i32 %53 to i64
  %55 = getelementptr inbounds i32 addrspace(1)* %1, i64 %54
  %56 = extractelement <16 x i32> %39, i32 5
  %57 = zext i32 %56 to i64
  %58 = getelementptr inbounds i32 addrspace(1)* %1, i64 %57
  %59 = extractelement <16 x i32> %39, i32 6
  %60 = zext i32 %59 to i64
  %61 = getelementptr inbounds i32 addrspace(1)* %1, i64 %60
  %62 = extractelement <16 x i32> %39, i32 7
  %63 = zext i32 %62 to i64
  %64 = getelementptr inbounds i32 addrspace(1)* %1, i64 %63
  %65 = extractelement <16 x i32> %39, i32 8
  %66 = zext i32 %65 to i64
  %67 = getelementptr inbounds i32 addrspace(1)* %1, i64 %66
  %68 = extractelement <16 x i32> %39, i32 9
  %69 = zext i32 %68 to i64
  %70 = getelementptr inbounds i32 addrspace(1)* %1, i64 %69
  %71 = extractelement <16 x i32> %39, i32 10
  %72 = zext i32 %71 to i64
  %73 = getelementptr inbounds i32 addrspace(1)* %1, i64 %72
  %74 = extractelement <16 x i32> %39, i32 11
  %75 = zext i32 %74 to i64
  %76 = getelementptr inbounds i32 addrspace(1)* %1, i64 %75
  %77 = extractelement <16 x i32> %39, i32 12
  %78 = zext i32 %77 to i64
  %79 = getelementptr inbounds i32 addrspace(1)* %1, i64 %78
  %80 = extractelement <16 x i32> %39, i32 13
  %81 = zext i32 %80 to i64
  %82 = getelementptr inbounds i32 addrspace(1)* %1, i64 %81
  %83 = extractelement <16 x i32> %39, i32 14
  %84 = zext i32 %83 to i64
  %85 = getelementptr inbounds i32 addrspace(1)* %1, i64 %84
  %86 = extractelement <16 x i32> %39, i32 15
  %87 = zext i32 %86 to i64
  %88 = getelementptr inbounds i32 addrspace(1)* %1, i64 %87
  %89 = load i32 addrspace(1)* %43, align 4
  %90 = load i32 addrspace(1)* %46, align 4
  %91 = load i32 addrspace(1)* %49, align 4
  %92 = load i32 addrspace(1)* %52, align 4
  %93 = load i32 addrspace(1)* %55, align 4
  %94 = load i32 addrspace(1)* %58, align 4
  %95 = load i32 addrspace(1)* %61, align 4
  %96 = load i32 addrspace(1)* %64, align 4
  %97 = load i32 addrspace(1)* %67, align 4
  %98 = load i32 addrspace(1)* %70, align 4
  %99 = load i32 addrspace(1)* %73, align 4
  %100 = load i32 addrspace(1)* %76, align 4
  %101 = load i32 addrspace(1)* %79, align 4
  %102 = load i32 addrspace(1)* %82, align 4
  %103 = load i32 addrspace(1)* %85, align 4
  %104 = load i32 addrspace(1)* %88, align 4
  %temp.vect.i = insertelement <16 x i32> undef, i32 %89, i32 0
  %temp.vect53.i = insertelement <16 x i32> %temp.vect.i, i32 %90, i32 1
  %temp.vect54.i = insertelement <16 x i32> %temp.vect53.i, i32 %91, i32 2
  %temp.vect55.i = insertelement <16 x i32> %temp.vect54.i, i32 %92, i32 3
  %temp.vect56.i = insertelement <16 x i32> %temp.vect55.i, i32 %93, i32 4
  %temp.vect57.i = insertelement <16 x i32> %temp.vect56.i, i32 %94, i32 5
  %temp.vect58.i = insertelement <16 x i32> %temp.vect57.i, i32 %95, i32 6
  %temp.vect59.i = insertelement <16 x i32> %temp.vect58.i, i32 %96, i32 7
  %temp.vect60.i = insertelement <16 x i32> %temp.vect59.i, i32 %97, i32 8
  %temp.vect61.i = insertelement <16 x i32> %temp.vect60.i, i32 %98, i32 9
  %temp.vect62.i = insertelement <16 x i32> %temp.vect61.i, i32 %99, i32 10
  %temp.vect63.i = insertelement <16 x i32> %temp.vect62.i, i32 %100, i32 11
  %temp.vect64.i = insertelement <16 x i32> %temp.vect63.i, i32 %101, i32 12
  %temp.vect65.i = insertelement <16 x i32> %temp.vect64.i, i32 %102, i32 13
  %temp.vect66.i = insertelement <16 x i32> %temp.vect65.i, i32 %103, i32 14
  %temp.vect67.i = insertelement <16 x i32> %temp.vect66.i, i32 %104, i32 15
  %105 = extractelement <16 x i32> %40, i32 0
  %106 = zext i32 %105 to i64
  %107 = getelementptr inbounds i32 addrspace(1)* %1, i64 %106
  %108 = extractelement <16 x i32> %40, i32 1
  %109 = zext i32 %108 to i64
  %110 = getelementptr inbounds i32 addrspace(1)* %1, i64 %109
  %111 = extractelement <16 x i32> %40, i32 2
  %112 = zext i32 %111 to i64
  %113 = getelementptr inbounds i32 addrspace(1)* %1, i64 %112
  %114 = extractelement <16 x i32> %40, i32 3
  %115 = zext i32 %114 to i64
  %116 = getelementptr inbounds i32 addrspace(1)* %1, i64 %115
  %117 = extractelement <16 x i32> %40, i32 4
  %118 = zext i32 %117 to i64
  %119 = getelementptr inbounds i32 addrspace(1)* %1, i64 %118
  %120 = extractelement <16 x i32> %40, i32 5
  %121 = zext i32 %120 to i64
  %122 = getelementptr inbounds i32 addrspace(1)* %1, i64 %121
  %123 = extractelement <16 x i32> %40, i32 6
  %124 = zext i32 %123 to i64
  %125 = getelementptr inbounds i32 addrspace(1)* %1, i64 %124
  %126 = extractelement <16 x i32> %40, i32 7
  %127 = zext i32 %126 to i64
  %128 = getelementptr inbounds i32 addrspace(1)* %1, i64 %127
  %129 = extractelement <16 x i32> %40, i32 8
  %130 = zext i32 %129 to i64
  %131 = getelementptr inbounds i32 addrspace(1)* %1, i64 %130
  %132 = extractelement <16 x i32> %40, i32 9
  %133 = zext i32 %132 to i64
  %134 = getelementptr inbounds i32 addrspace(1)* %1, i64 %133
  %135 = extractelement <16 x i32> %40, i32 10
  %136 = zext i32 %135 to i64
  %137 = getelementptr inbounds i32 addrspace(1)* %1, i64 %136
  %138 = extractelement <16 x i32> %40, i32 11
  %139 = zext i32 %138 to i64
  %140 = getelementptr inbounds i32 addrspace(1)* %1, i64 %139
  %141 = extractelement <16 x i32> %40, i32 12
  %142 = zext i32 %141 to i64
  %143 = getelementptr inbounds i32 addrspace(1)* %1, i64 %142
  %144 = extractelement <16 x i32> %40, i32 13
  %145 = zext i32 %144 to i64
  %146 = getelementptr inbounds i32 addrspace(1)* %1, i64 %145
  %147 = extractelement <16 x i32> %40, i32 14
  %148 = zext i32 %147 to i64
  %149 = getelementptr inbounds i32 addrspace(1)* %1, i64 %148
  %150 = extractelement <16 x i32> %40, i32 15
  %151 = zext i32 %150 to i64
  %152 = getelementptr inbounds i32 addrspace(1)* %1, i64 %151
  %153 = load i32 addrspace(1)* %107, align 4
  %154 = load i32 addrspace(1)* %110, align 4
  %155 = load i32 addrspace(1)* %113, align 4
  %156 = load i32 addrspace(1)* %116, align 4
  %157 = load i32 addrspace(1)* %119, align 4
  %158 = load i32 addrspace(1)* %122, align 4
  %159 = load i32 addrspace(1)* %125, align 4
  %160 = load i32 addrspace(1)* %128, align 4
  %161 = load i32 addrspace(1)* %131, align 4
  %162 = load i32 addrspace(1)* %134, align 4
  %163 = load i32 addrspace(1)* %137, align 4
  %164 = load i32 addrspace(1)* %140, align 4
  %165 = load i32 addrspace(1)* %143, align 4
  %166 = load i32 addrspace(1)* %146, align 4
  %167 = load i32 addrspace(1)* %149, align 4
  %168 = load i32 addrspace(1)* %152, align 4
  %temp.vect68.i = insertelement <16 x i32> undef, i32 %153, i32 0
  %temp.vect69.i = insertelement <16 x i32> %temp.vect68.i, i32 %154, i32 1
  %temp.vect70.i = insertelement <16 x i32> %temp.vect69.i, i32 %155, i32 2
  %temp.vect71.i = insertelement <16 x i32> %temp.vect70.i, i32 %156, i32 3
  %temp.vect72.i = insertelement <16 x i32> %temp.vect71.i, i32 %157, i32 4
  %temp.vect73.i = insertelement <16 x i32> %temp.vect72.i, i32 %158, i32 5
  %temp.vect74.i = insertelement <16 x i32> %temp.vect73.i, i32 %159, i32 6
  %temp.vect75.i = insertelement <16 x i32> %temp.vect74.i, i32 %160, i32 7
  %temp.vect76.i = insertelement <16 x i32> %temp.vect75.i, i32 %161, i32 8
  %temp.vect77.i = insertelement <16 x i32> %temp.vect76.i, i32 %162, i32 9
  %temp.vect78.i = insertelement <16 x i32> %temp.vect77.i, i32 %163, i32 10
  %temp.vect79.i = insertelement <16 x i32> %temp.vect78.i, i32 %164, i32 11
  %temp.vect80.i = insertelement <16 x i32> %temp.vect79.i, i32 %165, i32 12
  %temp.vect81.i = insertelement <16 x i32> %temp.vect80.i, i32 %166, i32 13
  %temp.vect82.i = insertelement <16 x i32> %temp.vect81.i, i32 %167, i32 14
  %temp.vect83.i = insertelement <16 x i32> %temp.vect82.i, i32 %168, i32 15
  %169 = udiv <16 x i32> %35, %vector47.i
  %170 = and <16 x i32> %169, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %171 = icmp eq <16 x i32> %170, zeroinitializer
  %direction.52.i = select <16 x i1> %171, <16 x i32> %vector49.i, <16 x i32> %vector51.i
  %172 = icmp ugt <16 x i32> %temp.vect67.i, %temp.vect83.i
  %greater.084.i = select <16 x i1> %172, <16 x i32> %temp.vect67.i, <16 x i32> %temp.vect83.i
  %lesser.085.i = select <16 x i1> %172, <16 x i32> %temp.vect83.i, <16 x i32> %temp.vect67.i
  %173 = icmp eq <16 x i32> %direction.52.i, zeroinitializer
  %merge1087.i = select <16 x i1> %173, <16 x i32> %lesser.085.i, <16 x i32> %greater.084.i
  %extract105.i = extractelement <16 x i32> %merge1087.i, i32 0
  %extract106.i = extractelement <16 x i32> %merge1087.i, i32 1
  %extract107.i = extractelement <16 x i32> %merge1087.i, i32 2
  %extract108.i = extractelement <16 x i32> %merge1087.i, i32 3
  %extract109.i = extractelement <16 x i32> %merge1087.i, i32 4
  %extract110.i = extractelement <16 x i32> %merge1087.i, i32 5
  %extract111.i = extractelement <16 x i32> %merge1087.i, i32 6
  %extract112.i = extractelement <16 x i32> %merge1087.i, i32 7
  %extract113.i = extractelement <16 x i32> %merge1087.i, i32 8
  %extract114.i = extractelement <16 x i32> %merge1087.i, i32 9
  %extract115.i = extractelement <16 x i32> %merge1087.i, i32 10
  %extract116.i = extractelement <16 x i32> %merge1087.i, i32 11
  %extract117.i = extractelement <16 x i32> %merge1087.i, i32 12
  %extract118.i = extractelement <16 x i32> %merge1087.i, i32 13
  %extract119.i = extractelement <16 x i32> %merge1087.i, i32 14
  %extract120.i = extractelement <16 x i32> %merge1087.i, i32 15
  %merge88.i = select <16 x i1> %173, <16 x i32> %greater.084.i, <16 x i32> %lesser.085.i
  %extract89.i = extractelement <16 x i32> %merge88.i, i32 0
  %extract90.i = extractelement <16 x i32> %merge88.i, i32 1
  %extract91.i = extractelement <16 x i32> %merge88.i, i32 2
  %extract92.i = extractelement <16 x i32> %merge88.i, i32 3
  %extract93.i = extractelement <16 x i32> %merge88.i, i32 4
  %extract94.i = extractelement <16 x i32> %merge88.i, i32 5
  %extract95.i = extractelement <16 x i32> %merge88.i, i32 6
  %extract96.i = extractelement <16 x i32> %merge88.i, i32 7
  %extract97.i = extractelement <16 x i32> %merge88.i, i32 8
  %extract98.i = extractelement <16 x i32> %merge88.i, i32 9
  %extract99.i = extractelement <16 x i32> %merge88.i, i32 10
  %extract100.i = extractelement <16 x i32> %merge88.i, i32 11
  %extract101.i = extractelement <16 x i32> %merge88.i, i32 12
  %extract102.i = extractelement <16 x i32> %merge88.i, i32 13
  %extract103.i = extractelement <16 x i32> %merge88.i, i32 14
  %extract104.i = extractelement <16 x i32> %merge88.i, i32 15
  store i32 %extract89.i, i32 addrspace(1)* %43, align 4
  store i32 %extract90.i, i32 addrspace(1)* %46, align 4
  store i32 %extract91.i, i32 addrspace(1)* %49, align 4
  store i32 %extract92.i, i32 addrspace(1)* %52, align 4
  store i32 %extract93.i, i32 addrspace(1)* %55, align 4
  store i32 %extract94.i, i32 addrspace(1)* %58, align 4
  store i32 %extract95.i, i32 addrspace(1)* %61, align 4
  store i32 %extract96.i, i32 addrspace(1)* %64, align 4
  store i32 %extract97.i, i32 addrspace(1)* %67, align 4
  store i32 %extract98.i, i32 addrspace(1)* %70, align 4
  store i32 %extract99.i, i32 addrspace(1)* %73, align 4
  store i32 %extract100.i, i32 addrspace(1)* %76, align 4
  store i32 %extract101.i, i32 addrspace(1)* %79, align 4
  store i32 %extract102.i, i32 addrspace(1)* %82, align 4
  store i32 %extract103.i, i32 addrspace(1)* %85, align 4
  store i32 %extract104.i, i32 addrspace(1)* %88, align 4
  store i32 %extract105.i, i32 addrspace(1)* %107, align 4
  store i32 %extract106.i, i32 addrspace(1)* %110, align 4
  store i32 %extract107.i, i32 addrspace(1)* %113, align 4
  store i32 %extract108.i, i32 addrspace(1)* %116, align 4
  store i32 %extract109.i, i32 addrspace(1)* %119, align 4
  store i32 %extract110.i, i32 addrspace(1)* %122, align 4
  store i32 %extract111.i, i32 addrspace(1)* %125, align 4
  store i32 %extract112.i, i32 addrspace(1)* %128, align 4
  store i32 %extract113.i, i32 addrspace(1)* %131, align 4
  store i32 %extract114.i, i32 addrspace(1)* %134, align 4
  store i32 %extract115.i, i32 addrspace(1)* %137, align 4
  store i32 %extract116.i, i32 addrspace(1)* %140, align 4
  store i32 %extract117.i, i32 addrspace(1)* %143, align 4
  store i32 %extract118.i, i32 addrspace(1)* %146, align 4
  store i32 %extract119.i, i32 addrspace(1)* %149, align 4
  store i32 %extract120.i, i32 addrspace(1)* %152, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.bitonicSort_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.bitonicSort_separated_args.exit:  ; preds = %SyncBB.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, i32, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__bitonicSort_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uint __attribute__((address_space(1))) *, uint const, uint const, uint const, uint const", metadata !"opencl_bitonicSort_locals_anchor", void (i8*)* @bitonicSort}
!1 = metadata !{i32 0, i32 0, i32 0}
