; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc
;
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__MulMAdd1_original(double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__MulMAdd1_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %nIters, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %sext = shl i64 %5, 32
  %6 = ashr i64 %sext, 32
  %7 = getelementptr inbounds double addrspace(1)* %data, i64 %6
  %8 = load double addrspace(1)* %7, align 8
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB, %bb.nph
  %j.02 = phi i32 [ %147, %bb.nph ], [ 0, %SyncBB ]
  %s.01 = phi double [ %146, %bb.nph ], [ %8, %SyncBB ]
  %9 = fmul double %s.01, 3.550000e-01
  %10 = fsub double 3.750000e+00, %9
  %11 = fmul double %10, %s.01
  %12 = fmul double %11, 3.550000e-01
  %13 = fsub double 3.750000e+00, %12
  %14 = fmul double %13, %11
  %15 = fmul double %14, 3.550000e-01
  %16 = fsub double 3.750000e+00, %15
  %17 = fmul double %16, %14
  %18 = fmul double %17, 3.550000e-01
  %19 = fsub double 3.750000e+00, %18
  %20 = fmul double %19, %17
  %21 = fmul double %20, 3.550000e-01
  %22 = fsub double 3.750000e+00, %21
  %23 = fmul double %22, %20
  %24 = fmul double %23, 3.550000e-01
  %25 = fsub double 3.750000e+00, %24
  %26 = fmul double %25, %23
  %27 = fmul double %26, 3.550000e-01
  %28 = fsub double 3.750000e+00, %27
  %29 = fmul double %28, %26
  %30 = fmul double %29, 3.550000e-01
  %31 = fsub double 3.750000e+00, %30
  %32 = fmul double %31, %29
  %33 = fmul double %32, 3.550000e-01
  %34 = fsub double 3.750000e+00, %33
  %35 = fmul double %34, %32
  %36 = fmul double %35, 3.550000e-01
  %37 = fsub double 3.750000e+00, %36
  %38 = fmul double %37, %35
  %39 = fmul double %38, 3.550000e-01
  %40 = fsub double 3.750000e+00, %39
  %41 = fmul double %40, %38
  %42 = fmul double %41, 3.550000e-01
  %43 = fsub double 3.750000e+00, %42
  %44 = fmul double %43, %41
  %45 = fmul double %44, 3.550000e-01
  %46 = fsub double 3.750000e+00, %45
  %47 = fmul double %46, %44
  %48 = fmul double %47, 3.550000e-01
  %49 = fsub double 3.750000e+00, %48
  %50 = fmul double %49, %47
  %51 = fmul double %50, 3.550000e-01
  %52 = fsub double 3.750000e+00, %51
  %53 = fmul double %52, %50
  %54 = fmul double %53, 3.550000e-01
  %55 = fsub double 3.750000e+00, %54
  %56 = fmul double %55, %53
  %57 = fmul double %56, 3.550000e-01
  %58 = fsub double 3.750000e+00, %57
  %59 = fmul double %58, %56
  %60 = fmul double %59, 3.550000e-01
  %61 = fsub double 3.750000e+00, %60
  %62 = fmul double %61, %59
  %63 = fmul double %62, 3.550000e-01
  %64 = fsub double 3.750000e+00, %63
  %65 = fmul double %64, %62
  %66 = fmul double %65, 3.550000e-01
  %67 = fsub double 3.750000e+00, %66
  %68 = fmul double %67, %65
  %69 = fmul double %68, 3.550000e-01
  %70 = fsub double 3.750000e+00, %69
  %71 = fmul double %70, %68
  %72 = fmul double %71, 3.550000e-01
  %73 = fsub double 3.750000e+00, %72
  %74 = fmul double %73, %71
  %75 = fmul double %74, 3.550000e-01
  %76 = fsub double 3.750000e+00, %75
  %77 = fmul double %76, %74
  %78 = fmul double %77, 3.550000e-01
  %79 = fsub double 3.750000e+00, %78
  %80 = fmul double %79, %77
  %81 = fmul double %80, 3.550000e-01
  %82 = fsub double 3.750000e+00, %81
  %83 = fmul double %82, %80
  %84 = fmul double %83, 3.550000e-01
  %85 = fsub double 3.750000e+00, %84
  %86 = fmul double %85, %83
  %87 = fmul double %86, 3.550000e-01
  %88 = fsub double 3.750000e+00, %87
  %89 = fmul double %88, %86
  %90 = fmul double %89, 3.550000e-01
  %91 = fsub double 3.750000e+00, %90
  %92 = fmul double %91, %89
  %93 = fmul double %92, 3.550000e-01
  %94 = fsub double 3.750000e+00, %93
  %95 = fmul double %94, %92
  %96 = fmul double %95, 3.550000e-01
  %97 = fsub double 3.750000e+00, %96
  %98 = fmul double %97, %95
  %99 = fmul double %98, 3.550000e-01
  %100 = fsub double 3.750000e+00, %99
  %101 = fmul double %100, %98
  %102 = fmul double %101, 3.550000e-01
  %103 = fsub double 3.750000e+00, %102
  %104 = fmul double %103, %101
  %105 = fmul double %104, 3.550000e-01
  %106 = fsub double 3.750000e+00, %105
  %107 = fmul double %106, %104
  %108 = fmul double %107, 3.550000e-01
  %109 = fsub double 3.750000e+00, %108
  %110 = fmul double %109, %107
  %111 = fmul double %110, 3.550000e-01
  %112 = fsub double 3.750000e+00, %111
  %113 = fmul double %112, %110
  %114 = fmul double %113, 3.550000e-01
  %115 = fsub double 3.750000e+00, %114
  %116 = fmul double %115, %113
  %117 = fmul double %116, 3.550000e-01
  %118 = fsub double 3.750000e+00, %117
  %119 = fmul double %118, %116
  %120 = fmul double %119, 3.550000e-01
  %121 = fsub double 3.750000e+00, %120
  %122 = fmul double %121, %119
  %123 = fmul double %122, 3.550000e-01
  %124 = fsub double 3.750000e+00, %123
  %125 = fmul double %124, %122
  %126 = fmul double %125, 3.550000e-01
  %127 = fsub double 3.750000e+00, %126
  %128 = fmul double %127, %125
  %129 = fmul double %128, 3.550000e-01
  %130 = fsub double 3.750000e+00, %129
  %131 = fmul double %130, %128
  %132 = fmul double %131, 3.550000e-01
  %133 = fsub double 3.750000e+00, %132
  %134 = fmul double %133, %131
  %135 = fmul double %134, 3.550000e-01
  %136 = fsub double 3.750000e+00, %135
  %137 = fmul double %136, %134
  %138 = fmul double %137, 3.550000e-01
  %139 = fsub double 3.750000e+00, %138
  %140 = fmul double %139, %137
  %141 = fmul double %140, 3.550000e-01
  %142 = fsub double 3.750000e+00, %141
  %143 = fmul double %142, %140
  %144 = fmul double %143, 3.550000e-01
  %145 = fsub double 3.750000e+00, %144
  %146 = fmul double %145, %143
  %147 = add nsw i32 %j.02, 1
  %exitcond = icmp eq i32 %147, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB
  %s.0.lcssa = phi double [ %8, %SyncBB ], [ %146, %bb.nph ]
  store double %s.0.lcssa, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB3

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB3:                                          ; preds = %._crit_edge
  ret void
}

define void @MulMAdd1(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 40
  %6 = bitcast i8* %5 to %struct.PaddedDimId**
  %7 = load %struct.PaddedDimId** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 48
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 64
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = icmp sgt i32 %4, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %15 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %16 = load i64* %15, align 8
  %17 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %18 = load i64* %17, align 8
  %19 = add i64 %16, %18
  %sext.i = shl i64 %19, 32
  %20 = ashr i64 %sext.i, 32
  %21 = getelementptr inbounds double addrspace(1)* %1, i64 %20
  %22 = load double addrspace(1)* %21, align 8
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB.i
  %j.02.i = phi i32 [ %161, %bb.nph.i ], [ 0, %SyncBB.i ]
  %s.01.i = phi double [ %160, %bb.nph.i ], [ %22, %SyncBB.i ]
  %23 = fmul double %s.01.i, 3.550000e-01
  %24 = fsub double 3.750000e+00, %23
  %25 = fmul double %24, %s.01.i
  %26 = fmul double %25, 3.550000e-01
  %27 = fsub double 3.750000e+00, %26
  %28 = fmul double %27, %25
  %29 = fmul double %28, 3.550000e-01
  %30 = fsub double 3.750000e+00, %29
  %31 = fmul double %30, %28
  %32 = fmul double %31, 3.550000e-01
  %33 = fsub double 3.750000e+00, %32
  %34 = fmul double %33, %31
  %35 = fmul double %34, 3.550000e-01
  %36 = fsub double 3.750000e+00, %35
  %37 = fmul double %36, %34
  %38 = fmul double %37, 3.550000e-01
  %39 = fsub double 3.750000e+00, %38
  %40 = fmul double %39, %37
  %41 = fmul double %40, 3.550000e-01
  %42 = fsub double 3.750000e+00, %41
  %43 = fmul double %42, %40
  %44 = fmul double %43, 3.550000e-01
  %45 = fsub double 3.750000e+00, %44
  %46 = fmul double %45, %43
  %47 = fmul double %46, 3.550000e-01
  %48 = fsub double 3.750000e+00, %47
  %49 = fmul double %48, %46
  %50 = fmul double %49, 3.550000e-01
  %51 = fsub double 3.750000e+00, %50
  %52 = fmul double %51, %49
  %53 = fmul double %52, 3.550000e-01
  %54 = fsub double 3.750000e+00, %53
  %55 = fmul double %54, %52
  %56 = fmul double %55, 3.550000e-01
  %57 = fsub double 3.750000e+00, %56
  %58 = fmul double %57, %55
  %59 = fmul double %58, 3.550000e-01
  %60 = fsub double 3.750000e+00, %59
  %61 = fmul double %60, %58
  %62 = fmul double %61, 3.550000e-01
  %63 = fsub double 3.750000e+00, %62
  %64 = fmul double %63, %61
  %65 = fmul double %64, 3.550000e-01
  %66 = fsub double 3.750000e+00, %65
  %67 = fmul double %66, %64
  %68 = fmul double %67, 3.550000e-01
  %69 = fsub double 3.750000e+00, %68
  %70 = fmul double %69, %67
  %71 = fmul double %70, 3.550000e-01
  %72 = fsub double 3.750000e+00, %71
  %73 = fmul double %72, %70
  %74 = fmul double %73, 3.550000e-01
  %75 = fsub double 3.750000e+00, %74
  %76 = fmul double %75, %73
  %77 = fmul double %76, 3.550000e-01
  %78 = fsub double 3.750000e+00, %77
  %79 = fmul double %78, %76
  %80 = fmul double %79, 3.550000e-01
  %81 = fsub double 3.750000e+00, %80
  %82 = fmul double %81, %79
  %83 = fmul double %82, 3.550000e-01
  %84 = fsub double 3.750000e+00, %83
  %85 = fmul double %84, %82
  %86 = fmul double %85, 3.550000e-01
  %87 = fsub double 3.750000e+00, %86
  %88 = fmul double %87, %85
  %89 = fmul double %88, 3.550000e-01
  %90 = fsub double 3.750000e+00, %89
  %91 = fmul double %90, %88
  %92 = fmul double %91, 3.550000e-01
  %93 = fsub double 3.750000e+00, %92
  %94 = fmul double %93, %91
  %95 = fmul double %94, 3.550000e-01
  %96 = fsub double 3.750000e+00, %95
  %97 = fmul double %96, %94
  %98 = fmul double %97, 3.550000e-01
  %99 = fsub double 3.750000e+00, %98
  %100 = fmul double %99, %97
  %101 = fmul double %100, 3.550000e-01
  %102 = fsub double 3.750000e+00, %101
  %103 = fmul double %102, %100
  %104 = fmul double %103, 3.550000e-01
  %105 = fsub double 3.750000e+00, %104
  %106 = fmul double %105, %103
  %107 = fmul double %106, 3.550000e-01
  %108 = fsub double 3.750000e+00, %107
  %109 = fmul double %108, %106
  %110 = fmul double %109, 3.550000e-01
  %111 = fsub double 3.750000e+00, %110
  %112 = fmul double %111, %109
  %113 = fmul double %112, 3.550000e-01
  %114 = fsub double 3.750000e+00, %113
  %115 = fmul double %114, %112
  %116 = fmul double %115, 3.550000e-01
  %117 = fsub double 3.750000e+00, %116
  %118 = fmul double %117, %115
  %119 = fmul double %118, 3.550000e-01
  %120 = fsub double 3.750000e+00, %119
  %121 = fmul double %120, %118
  %122 = fmul double %121, 3.550000e-01
  %123 = fsub double 3.750000e+00, %122
  %124 = fmul double %123, %121
  %125 = fmul double %124, 3.550000e-01
  %126 = fsub double 3.750000e+00, %125
  %127 = fmul double %126, %124
  %128 = fmul double %127, 3.550000e-01
  %129 = fsub double 3.750000e+00, %128
  %130 = fmul double %129, %127
  %131 = fmul double %130, 3.550000e-01
  %132 = fsub double 3.750000e+00, %131
  %133 = fmul double %132, %130
  %134 = fmul double %133, 3.550000e-01
  %135 = fsub double 3.750000e+00, %134
  %136 = fmul double %135, %133
  %137 = fmul double %136, 3.550000e-01
  %138 = fsub double 3.750000e+00, %137
  %139 = fmul double %138, %136
  %140 = fmul double %139, 3.550000e-01
  %141 = fsub double 3.750000e+00, %140
  %142 = fmul double %141, %139
  %143 = fmul double %142, 3.550000e-01
  %144 = fsub double 3.750000e+00, %143
  %145 = fmul double %144, %142
  %146 = fmul double %145, 3.550000e-01
  %147 = fsub double 3.750000e+00, %146
  %148 = fmul double %147, %145
  %149 = fmul double %148, 3.550000e-01
  %150 = fsub double 3.750000e+00, %149
  %151 = fmul double %150, %148
  %152 = fmul double %151, 3.550000e-01
  %153 = fsub double 3.750000e+00, %152
  %154 = fmul double %153, %151
  %155 = fmul double %154, 3.550000e-01
  %156 = fsub double 3.750000e+00, %155
  %157 = fmul double %156, %154
  %158 = fmul double %157, 3.550000e-01
  %159 = fsub double 3.750000e+00, %158
  %160 = fmul double %159, %157
  %161 = add nsw i32 %j.02.i, 1
  %exitcond.i = icmp eq i32 %161, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB.i
  %s.0.lcssa.i = phi double [ %22, %SyncBB.i ], [ %160, %bb.nph.i ]
  store double %s.0.lcssa.i, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__MulMAdd1_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__MulMAdd1_separated_args.exit:                   ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__MulMAdd1_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_MulMAdd1_locals_anchor", void (i8*)* @MulMAdd1}
!1 = metadata !{i32 0, i32 0, i32 0}
