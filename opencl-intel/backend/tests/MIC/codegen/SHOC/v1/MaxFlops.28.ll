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

declare void @__MAdd1_original(double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__MAdd1_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %nIters, 0
  br label %SyncBB3

SyncBB3:                                          ; preds = %thenBB, %FirstBB
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

bb.nph:                                           ; preds = %SyncBB3, %bb.nph
  %j.02 = phi i32 [ %149, %bb.nph ], [ 0, %SyncBB3 ]
  %s.01 = phi double [ %148, %bb.nph ], [ %8, %SyncBB3 ]
  %9 = fmul double %s.01, 9.899000e-01
  %10 = fsub double 1.000000e+01, %9
  %11 = fmul double %10, 9.899000e-01
  %12 = fsub double 1.000000e+01, %11
  %13 = fmul double %12, 9.899000e-01
  %14 = fsub double 1.000000e+01, %13
  %15 = fmul double %14, 9.899000e-01
  %16 = fsub double 1.000000e+01, %15
  %17 = fmul double %16, 9.899000e-01
  %18 = fsub double 1.000000e+01, %17
  %19 = fmul double %18, 9.899000e-01
  %20 = fsub double 1.000000e+01, %19
  %21 = fmul double %20, 9.899000e-01
  %22 = fsub double 1.000000e+01, %21
  %23 = fmul double %22, 9.899000e-01
  %24 = fsub double 1.000000e+01, %23
  %25 = fmul double %24, 9.899000e-01
  %26 = fsub double 1.000000e+01, %25
  %27 = fmul double %26, 9.899000e-01
  %28 = fsub double 1.000000e+01, %27
  %29 = fmul double %28, 9.899000e-01
  %30 = fsub double 1.000000e+01, %29
  %31 = fmul double %30, 9.899000e-01
  %32 = fsub double 1.000000e+01, %31
  %33 = fmul double %32, 9.899000e-01
  %34 = fsub double 1.000000e+01, %33
  %35 = fmul double %34, 9.899000e-01
  %36 = fsub double 1.000000e+01, %35
  %37 = fmul double %36, 9.899000e-01
  %38 = fsub double 1.000000e+01, %37
  %39 = fmul double %38, 9.899000e-01
  %40 = fsub double 1.000000e+01, %39
  %41 = fmul double %40, 9.899000e-01
  %42 = fsub double 1.000000e+01, %41
  %43 = fmul double %42, 9.899000e-01
  %44 = fsub double 1.000000e+01, %43
  %45 = fmul double %44, 9.899000e-01
  %46 = fsub double 1.000000e+01, %45
  %47 = fmul double %46, 9.899000e-01
  %48 = fsub double 1.000000e+01, %47
  %49 = fmul double %48, 9.899000e-01
  %50 = fsub double 1.000000e+01, %49
  %51 = fmul double %50, 9.899000e-01
  %52 = fsub double 1.000000e+01, %51
  %53 = fmul double %52, 9.899000e-01
  %54 = fsub double 1.000000e+01, %53
  %55 = fmul double %54, 9.899000e-01
  %56 = fsub double 1.000000e+01, %55
  %57 = fmul double %56, 9.899000e-01
  %58 = fsub double 1.000000e+01, %57
  %59 = fmul double %58, 9.899000e-01
  %60 = fsub double 1.000000e+01, %59
  %61 = fmul double %60, 9.899000e-01
  %62 = fsub double 1.000000e+01, %61
  %63 = fmul double %62, 9.899000e-01
  %64 = fsub double 1.000000e+01, %63
  %65 = fmul double %64, 9.899000e-01
  %66 = fsub double 1.000000e+01, %65
  %67 = fmul double %66, 9.899000e-01
  %68 = fsub double 1.000000e+01, %67
  %69 = fmul double %68, 9.899000e-01
  %70 = fsub double 1.000000e+01, %69
  %71 = fmul double %70, 9.899000e-01
  %72 = fsub double 1.000000e+01, %71
  %73 = fmul double %72, 9.899000e-01
  %74 = fsub double 1.000000e+01, %73
  %75 = fmul double %74, 9.899000e-01
  %76 = fsub double 1.000000e+01, %75
  %77 = fmul double %76, 9.899000e-01
  %78 = fsub double 1.000000e+01, %77
  %79 = fmul double %78, 9.899000e-01
  %80 = fsub double 1.000000e+01, %79
  %81 = fmul double %80, 9.899000e-01
  %82 = fsub double 1.000000e+01, %81
  %83 = fmul double %82, 9.899000e-01
  %84 = fsub double 1.000000e+01, %83
  %85 = fmul double %84, 9.899000e-01
  %86 = fsub double 1.000000e+01, %85
  %87 = fmul double %86, 9.899000e-01
  %88 = fsub double 1.000000e+01, %87
  %89 = fmul double %88, 9.899000e-01
  %90 = fsub double 1.000000e+01, %89
  %91 = fmul double %90, 9.899000e-01
  %92 = fsub double 1.000000e+01, %91
  %93 = fmul double %92, 9.899000e-01
  %94 = fsub double 1.000000e+01, %93
  %95 = fmul double %94, 9.899000e-01
  %96 = fsub double 1.000000e+01, %95
  %97 = fmul double %96, 9.899000e-01
  %98 = fsub double 1.000000e+01, %97
  %99 = fmul double %98, 9.899000e-01
  %100 = fsub double 1.000000e+01, %99
  %101 = fmul double %100, 9.899000e-01
  %102 = fsub double 1.000000e+01, %101
  %103 = fmul double %102, 9.899000e-01
  %104 = fsub double 1.000000e+01, %103
  %105 = fmul double %104, 9.899000e-01
  %106 = fsub double 1.000000e+01, %105
  %107 = fmul double %106, 9.899000e-01
  %108 = fsub double 1.000000e+01, %107
  %109 = fmul double %108, 9.899000e-01
  %110 = fsub double 1.000000e+01, %109
  %111 = fmul double %110, 9.899000e-01
  %112 = fsub double 1.000000e+01, %111
  %113 = fmul double %112, 9.899000e-01
  %114 = fsub double 1.000000e+01, %113
  %115 = fmul double %114, 9.899000e-01
  %116 = fsub double 1.000000e+01, %115
  %117 = fmul double %116, 9.899000e-01
  %118 = fsub double 1.000000e+01, %117
  %119 = fmul double %118, 9.899000e-01
  %120 = fsub double 1.000000e+01, %119
  %121 = fmul double %120, 9.899000e-01
  %122 = fsub double 1.000000e+01, %121
  %123 = fmul double %122, 9.899000e-01
  %124 = fsub double 1.000000e+01, %123
  %125 = fmul double %124, 9.899000e-01
  %126 = fsub double 1.000000e+01, %125
  %127 = fmul double %126, 9.899000e-01
  %128 = fsub double 1.000000e+01, %127
  %129 = fmul double %128, 9.899000e-01
  %130 = fsub double 1.000000e+01, %129
  %131 = fmul double %130, 9.899000e-01
  %132 = fsub double 1.000000e+01, %131
  %133 = fmul double %132, 9.899000e-01
  %134 = fsub double 1.000000e+01, %133
  %135 = fmul double %134, 9.899000e-01
  %136 = fsub double 1.000000e+01, %135
  %137 = fmul double %136, 9.899000e-01
  %138 = fsub double 1.000000e+01, %137
  %139 = fmul double %138, 9.899000e-01
  %140 = fsub double 1.000000e+01, %139
  %141 = fmul double %140, 9.899000e-01
  %142 = fsub double 1.000000e+01, %141
  %143 = fmul double %142, 9.899000e-01
  %144 = fsub double 1.000000e+01, %143
  %145 = fmul double %144, 9.899000e-01
  %146 = fsub double 1.000000e+01, %145
  %147 = fmul double %146, 9.899000e-01
  %148 = fsub double 1.000000e+01, %147
  %149 = add nsw i32 %j.02, 1
  %exitcond = icmp eq i32 %149, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB3
  %s.0.lcssa = phi double [ %8, %SyncBB3 ], [ %148, %bb.nph ]
  store double %s.0.lcssa, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB3

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @MAdd1(i8* %pBuffer) {
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
  br label %SyncBB3.i

SyncBB3.i:                                        ; preds = %thenBB.i, %entry
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

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB3.i
  %j.02.i = phi i32 [ %163, %bb.nph.i ], [ 0, %SyncBB3.i ]
  %s.01.i = phi double [ %162, %bb.nph.i ], [ %22, %SyncBB3.i ]
  %23 = fmul double %s.01.i, 9.899000e-01
  %24 = fsub double 1.000000e+01, %23
  %25 = fmul double %24, 9.899000e-01
  %26 = fsub double 1.000000e+01, %25
  %27 = fmul double %26, 9.899000e-01
  %28 = fsub double 1.000000e+01, %27
  %29 = fmul double %28, 9.899000e-01
  %30 = fsub double 1.000000e+01, %29
  %31 = fmul double %30, 9.899000e-01
  %32 = fsub double 1.000000e+01, %31
  %33 = fmul double %32, 9.899000e-01
  %34 = fsub double 1.000000e+01, %33
  %35 = fmul double %34, 9.899000e-01
  %36 = fsub double 1.000000e+01, %35
  %37 = fmul double %36, 9.899000e-01
  %38 = fsub double 1.000000e+01, %37
  %39 = fmul double %38, 9.899000e-01
  %40 = fsub double 1.000000e+01, %39
  %41 = fmul double %40, 9.899000e-01
  %42 = fsub double 1.000000e+01, %41
  %43 = fmul double %42, 9.899000e-01
  %44 = fsub double 1.000000e+01, %43
  %45 = fmul double %44, 9.899000e-01
  %46 = fsub double 1.000000e+01, %45
  %47 = fmul double %46, 9.899000e-01
  %48 = fsub double 1.000000e+01, %47
  %49 = fmul double %48, 9.899000e-01
  %50 = fsub double 1.000000e+01, %49
  %51 = fmul double %50, 9.899000e-01
  %52 = fsub double 1.000000e+01, %51
  %53 = fmul double %52, 9.899000e-01
  %54 = fsub double 1.000000e+01, %53
  %55 = fmul double %54, 9.899000e-01
  %56 = fsub double 1.000000e+01, %55
  %57 = fmul double %56, 9.899000e-01
  %58 = fsub double 1.000000e+01, %57
  %59 = fmul double %58, 9.899000e-01
  %60 = fsub double 1.000000e+01, %59
  %61 = fmul double %60, 9.899000e-01
  %62 = fsub double 1.000000e+01, %61
  %63 = fmul double %62, 9.899000e-01
  %64 = fsub double 1.000000e+01, %63
  %65 = fmul double %64, 9.899000e-01
  %66 = fsub double 1.000000e+01, %65
  %67 = fmul double %66, 9.899000e-01
  %68 = fsub double 1.000000e+01, %67
  %69 = fmul double %68, 9.899000e-01
  %70 = fsub double 1.000000e+01, %69
  %71 = fmul double %70, 9.899000e-01
  %72 = fsub double 1.000000e+01, %71
  %73 = fmul double %72, 9.899000e-01
  %74 = fsub double 1.000000e+01, %73
  %75 = fmul double %74, 9.899000e-01
  %76 = fsub double 1.000000e+01, %75
  %77 = fmul double %76, 9.899000e-01
  %78 = fsub double 1.000000e+01, %77
  %79 = fmul double %78, 9.899000e-01
  %80 = fsub double 1.000000e+01, %79
  %81 = fmul double %80, 9.899000e-01
  %82 = fsub double 1.000000e+01, %81
  %83 = fmul double %82, 9.899000e-01
  %84 = fsub double 1.000000e+01, %83
  %85 = fmul double %84, 9.899000e-01
  %86 = fsub double 1.000000e+01, %85
  %87 = fmul double %86, 9.899000e-01
  %88 = fsub double 1.000000e+01, %87
  %89 = fmul double %88, 9.899000e-01
  %90 = fsub double 1.000000e+01, %89
  %91 = fmul double %90, 9.899000e-01
  %92 = fsub double 1.000000e+01, %91
  %93 = fmul double %92, 9.899000e-01
  %94 = fsub double 1.000000e+01, %93
  %95 = fmul double %94, 9.899000e-01
  %96 = fsub double 1.000000e+01, %95
  %97 = fmul double %96, 9.899000e-01
  %98 = fsub double 1.000000e+01, %97
  %99 = fmul double %98, 9.899000e-01
  %100 = fsub double 1.000000e+01, %99
  %101 = fmul double %100, 9.899000e-01
  %102 = fsub double 1.000000e+01, %101
  %103 = fmul double %102, 9.899000e-01
  %104 = fsub double 1.000000e+01, %103
  %105 = fmul double %104, 9.899000e-01
  %106 = fsub double 1.000000e+01, %105
  %107 = fmul double %106, 9.899000e-01
  %108 = fsub double 1.000000e+01, %107
  %109 = fmul double %108, 9.899000e-01
  %110 = fsub double 1.000000e+01, %109
  %111 = fmul double %110, 9.899000e-01
  %112 = fsub double 1.000000e+01, %111
  %113 = fmul double %112, 9.899000e-01
  %114 = fsub double 1.000000e+01, %113
  %115 = fmul double %114, 9.899000e-01
  %116 = fsub double 1.000000e+01, %115
  %117 = fmul double %116, 9.899000e-01
  %118 = fsub double 1.000000e+01, %117
  %119 = fmul double %118, 9.899000e-01
  %120 = fsub double 1.000000e+01, %119
  %121 = fmul double %120, 9.899000e-01
  %122 = fsub double 1.000000e+01, %121
  %123 = fmul double %122, 9.899000e-01
  %124 = fsub double 1.000000e+01, %123
  %125 = fmul double %124, 9.899000e-01
  %126 = fsub double 1.000000e+01, %125
  %127 = fmul double %126, 9.899000e-01
  %128 = fsub double 1.000000e+01, %127
  %129 = fmul double %128, 9.899000e-01
  %130 = fsub double 1.000000e+01, %129
  %131 = fmul double %130, 9.899000e-01
  %132 = fsub double 1.000000e+01, %131
  %133 = fmul double %132, 9.899000e-01
  %134 = fsub double 1.000000e+01, %133
  %135 = fmul double %134, 9.899000e-01
  %136 = fsub double 1.000000e+01, %135
  %137 = fmul double %136, 9.899000e-01
  %138 = fsub double 1.000000e+01, %137
  %139 = fmul double %138, 9.899000e-01
  %140 = fsub double 1.000000e+01, %139
  %141 = fmul double %140, 9.899000e-01
  %142 = fsub double 1.000000e+01, %141
  %143 = fmul double %142, 9.899000e-01
  %144 = fsub double 1.000000e+01, %143
  %145 = fmul double %144, 9.899000e-01
  %146 = fsub double 1.000000e+01, %145
  %147 = fmul double %146, 9.899000e-01
  %148 = fsub double 1.000000e+01, %147
  %149 = fmul double %148, 9.899000e-01
  %150 = fsub double 1.000000e+01, %149
  %151 = fmul double %150, 9.899000e-01
  %152 = fsub double 1.000000e+01, %151
  %153 = fmul double %152, 9.899000e-01
  %154 = fsub double 1.000000e+01, %153
  %155 = fmul double %154, 9.899000e-01
  %156 = fsub double 1.000000e+01, %155
  %157 = fmul double %156, 9.899000e-01
  %158 = fsub double 1.000000e+01, %157
  %159 = fmul double %158, 9.899000e-01
  %160 = fsub double 1.000000e+01, %159
  %161 = fmul double %160, 9.899000e-01
  %162 = fsub double 1.000000e+01, %161
  %163 = add nsw i32 %j.02.i, 1
  %exitcond.i = icmp eq i32 %163, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB3.i
  %s.0.lcssa.i = phi double [ %22, %SyncBB3.i ], [ %162, %bb.nph.i ]
  store double %s.0.lcssa.i, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__MAdd1_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB3.i

__MAdd1_separated_args.exit:                      ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__MAdd1_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_MAdd1_locals_anchor", void (i8*)* @MAdd1}
!1 = metadata !{i32 0, i32 0, i32 0}
