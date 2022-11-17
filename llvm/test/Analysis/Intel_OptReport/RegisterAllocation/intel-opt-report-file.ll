; RUN: llc -mtriple=x86_64 -mattr=fma -enable-ra-report -intel-ra-spillreport=high -intel-opt-report-file=stdout < %s | FileCheck %s
; RUN: llc -mtriple=x86_64 -mattr=fma -enable-ra-report -intel-ra-spillreport=high -intel-opt-report-file=stderr < %s 2>&1 >%tout | FileCheck %s
; RUN: llc -mtriple=x86_64 -mattr=fma -enable-ra-report -intel-ra-spillreport=high -intel-opt-report-file=%t < %s
; RUN: FileCheck %s < %t

; This test checks that the output location used by the register allocation
; report emitter is controlled by the -intel-opt-report-file option. It is
; based on llvm/test/CodeGen/X86/intel-ra-26052.ll.

; ModuleID = '1.ll'
source_filename = "1.ll"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%0 = type { <4 x float> }
%1 = type { %2 }
%2 = type { [1 x i64] }

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p3i8.p1i8.i64(i8 addrspace(3)* noalias nocapture writeonly, i8 addrspace(1)* noalias nocapture readonly, i64, i1 immarg) #0

; Function Attrs: alwaysinline convergent nounwind
declare !kernel_arg_addr_space !13 !kernel_arg_access_qual !14 !kernel_arg_type !15 !kernel_arg_base_type !15 !kernel_arg_type_qual !16 !no_barrier_path !17 !kernel_has_sub_groups !17 !max_wg_dimensions !18 !local_buffer_size !19 !barrier_buffer_size !20 !kernel_wrapper !21 !kernel_execution_length !22 !kernel_has_barrier !23 !kernel_has_global_sync !17 !private_memory_size !20 void @___ZTS28Kernel_L3_SLM_8x8_4x16_vec_1_separated_args(%0 addrspace(1)*, %1* byval(%1), %1* byval(%1), %1* byval(%1), %0 addrspace(1)*, %1* byval(%1), %1* byval(%1), %1* byval(%1), %0 addrspace(1)*, %1* byval(%1), %1* byval(%1), %1* byval(%1), %0 addrspace(3)* noalias, %1* byval(%1), %1* byval(%1), %1* byval(%1), float, float, i64, i64, i32, i8 addrspace(3)* noalias, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* noalias, i64* noalias, [4 x i64], i8* noalias, {}* noalias) local_unnamed_addr #1

define void @_ZTS28Kernel_L3_SLM_8x8_4x16_vec_1(i8* noalias %0, i64* noalias %1, {}* noalias %2) !kernel_arg_addr_space !13 !kernel_arg_access_qual !14 !kernel_arg_type !15 !kernel_arg_base_type !15 !kernel_arg_type_qual !16 !no_barrier_path !17 !kernel_has_sub_groups !17 !max_wg_dimensions !18 !local_buffer_size !19 !barrier_buffer_size !20 !kernel_execution_length !22 !kernel_has_barrier !23 !kernel_has_global_sync !17 !private_memory_size !20 !opencl.stats.InstCounter.CanVect !24 {
; CHECK-LABEL: Register allocation report for: _ZTS28Kernel_L3_SLM_8x8_4x16_vec_1
; CHECK:       FUNCTION BEGIN
; CHECK-NEXT:   {{[0-9]+}} reloads {{[0-9]+}} spills
; CHECK:        spill {{[0-9]+}} byte -- slot: {{[0-9]+}}
; CHECK:        folded reload {{[0-9]+}} byte -- slot: {{[0-9]+}}
; CHECK:        LOOP1 BEGIN at ()
; CHECK-NEXT:    {{[0-9]+}} reloads {{[0-9]+}} spills
; CHECK:         LOOP2 BEGIN at ()
; CHECK:          LOOP3 BEGIN at ()
; CHECK:           LOOP4 BEGIN at ()
; CHECK:           LOOP4 END
; CHECK:          LOOP3 END
; CHECK:          LOOP3 BEGIN at ()
; CHECK:          LOOP3 END
; CHECK:         LOOP2 END
; CHECK:        LOOP1 END
; CHECK:       FUNCTION END
  %4 = alloca [3 x i64], align 8
  %5 = alloca %1, align 8
  %6 = alloca %1, align 8
  %7 = alloca %1, align 8
  %8 = bitcast i8* %0 to %0 addrspace(1)**
  %9 = load %0 addrspace(1)*, %0 addrspace(1)** %8, align 8
  %10 = getelementptr i8, i8* %0, i64 24
  %11 = getelementptr i8, i8* %0, i64 32
  %12 = bitcast i8* %11 to %0 addrspace(1)**
  %13 = load %0 addrspace(1)*, %0 addrspace(1)** %12, align 8
  %14 = getelementptr i8, i8* %0, i64 56
  %15 = getelementptr i8, i8* %0, i64 64
  %16 = bitcast i8* %15 to %0 addrspace(1)**
  %17 = load %0 addrspace(1)*, %0 addrspace(1)** %16, align 8
  %18 = getelementptr i8, i8* %0, i64 88
  %19 = getelementptr i8, i8* %0, i64 96
  %20 = bitcast i8* %19 to i64*
  %21 = load i64, i64* %20, align 8
  %22 = alloca i8, i64 %21, align 16
  %23 = bitcast i8* %22 to %0*
  %24 = addrspacecast %0* %23 to %0 addrspace(3)*
  %25 = getelementptr i8, i8* %0, i64 136
  %26 = bitcast i8* %25 to i64*
  %27 = load i64, i64* %26, align 8
  %28 = getelementptr i8, i8* %0, i64 144
  %29 = bitcast i8* %28 to i64*
  %30 = load i64, i64* %29, align 8
  %31 = getelementptr i8, i8* %0, i64 152
  %32 = bitcast i8* %31 to i32*
  %33 = load i32, i32* %32, align 4
  %34 = getelementptr i8, i8* %0, i64 160
  %35 = bitcast i8* %34 to { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
  %36 = getelementptr i8, i8* %0, i64 216
  %37 = bitcast i8* %36 to i64*
  %38 = load i64, i64* %37, align 1
  %39 = getelementptr i8, i8* %0, i64 224
  %40 = bitcast i8* %39 to i64*
  %41 = load i64, i64* %40, align 1
  %42 = getelementptr i8, i8* %0, i64 232
  %43 = bitcast i8* %42 to i64*
  %44 = load i64, i64* %43, align 1
  %45 = mul nuw nsw i64 %38, %41
  %46 = mul nuw nsw i64 %45, %44
  %47 = mul nuw nsw i64 %46, 640
  %48 = alloca i8, i64 %47, align 128
  %49 = bitcast %1* %7 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %49)
  %50 = bitcast %1* %6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %50)
  %51 = bitcast %1* %5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %51)
  %52 = bitcast [3 x i64]* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* nonnull %52)
  %53 = bitcast i8* %18 to i64*
  %54 = getelementptr inbounds %1, %1* %5, i64 0, i32 0, i32 0, i64 0
  %55 = load i64, i64* %53, align 1
  store i64 %55, i64* %54, align 8
  %56 = bitcast i8* %14 to i64*
  %57 = getelementptr inbounds %1, %1* %6, i64 0, i32 0, i32 0, i64 0
  %58 = load i64, i64* %56, align 1
  store i64 %58, i64* %57, align 8
  %59 = bitcast i8* %10 to i64*
  %60 = getelementptr inbounds %1, %1* %7, i64 0, i32 0, i32 0, i64 0
  %61 = load i64, i64* %59, align 1
  store i64 %61, i64* %60, align 8
  %62 = getelementptr i8, i8* %0, i64 264
  %63 = bitcast i8* %62 to i64*
  %64 = load i64, i64* %63, align 1, !alias.scope !25, !noalias !28
  %65 = load i64, i64* %1, align 1, !alias.scope !32, !noalias !33
  %66 = add nsw i64 %65, 1
  %67 = icmp eq i64 %64, %66
  %68 = zext i1 %67 to i64
  %69 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %35, i64 0, i32 3, i64 %68, i64 0
  %70 = load i64, i64* %69, align 1, !alias.scope !25, !noalias !28
  %71 = getelementptr i8, i8* %0, i64 272
  %72 = bitcast i8* %71 to i64*
  %73 = load i64, i64* %72, align 1, !alias.scope !25, !noalias !28
  %74 = getelementptr i64, i64* %1, i64 1
  %75 = load i64, i64* %74, align 1, !alias.scope !32, !noalias !33
  %76 = add nsw i64 %75, 1
  %77 = icmp eq i64 %73, %76
  %78 = zext i1 %77 to i64
  %79 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %35, i64 0, i32 3, i64 %78, i64 1
  %80 = load i64, i64* %79, align 1, !alias.scope !25, !noalias !28
  %81 = getelementptr inbounds [3 x i64], [3 x i64]* %4, i64 0, i64 0
  %82 = getelementptr inbounds [3 x i64], [3 x i64]* %4, i64 0, i64 1
  store i64 0, i64* %81, align 8, !noalias !34
  store i64 0, i64* %82, align 8, !noalias !34
  %83 = trunc i64 %27 to i32
  %84 = trunc i64 %30 to i32
  %85 = icmp eq i32 %33, -1
  %86 = icmp eq i32 %83, -2147483648
  %87 = and i1 %85, %86
  %88 = icmp eq i32 %33, 0
  %89 = or i1 %87, %88
  %90 = select i1 %89, i32 1, i32 %33
  %91 = sdiv i32 %83, %90
  %92 = icmp eq i32 %84, -2147483648
  %93 = and i1 %85, %92
  %94 = or i1 %93, %88
  %95 = select i1 %94, i32 1, i32 %33
  %96 = sdiv i32 %84, %95
  %97 = trunc i64 %65 to i32
  %98 = trunc i64 %75 to i32
  %99 = select i1 %88, i32 1, i32 %33
  %100 = sdiv i32 128, %99
  %101 = mul nsw i32 %100, %97
  %102 = sdiv i32 64, %99
  %103 = shl nsw i32 %91, 1
  %104 = mul nsw i32 %91, 3
  %105 = shl nsw i32 %91, 2
  %106 = mul nsw i32 %91, 5
  %107 = mul nsw i32 %91, 6
  %108 = mul nsw i32 %91, 7
  %109 = shl i32 %96, 6
  %110 = shl i32 %98, 5
  %111 = sext i32 %102 to i64
  %112 = sext i32 %91 to i64
  %113 = sext i32 %103 to i64
  %114 = sext i32 %104 to i64
  %115 = sext i32 %105 to i64
  %116 = sext i32 %106 to i64
  %117 = sext i32 %107 to i64
  %118 = sext i32 %108 to i64
  %119 = getelementptr inbounds %0, %0 addrspace(1)* %9, i64 %61
  %120 = getelementptr inbounds %0, %0 addrspace(1)* %13, i64 %58
  %121 = getelementptr inbounds %0, %0 addrspace(1)* %17, i64 %55
  br label %126

122:                                              ; preds = %424
  %123 = phi i64 [ %425, %424 ]
  %124 = phi i64 [ %426, %424 ]
  %125 = phi i64 [ %427, %424 ]
  br label %126

126:                                              ; preds = %122, %3
  %127 = phi i64 [ 0, %3 ], [ %123, %122 ]
  %128 = phi i64 [ 0, %3 ], [ %124, %122 ]
  %129 = phi i64 [ 0, %3 ], [ %125, %122 ]
  %130 = trunc i64 %128 to i32
  %131 = trunc i64 %127 to i32
  %132 = add nsw i32 %101, %130
  %133 = getelementptr inbounds i8, i8* %48, i64 %129
  %134 = bitcast i8* %133 to i32*
  store i32 %132, i32* %134, align 4, !alias.scope !35, !noalias !36
  %135 = add nsw i32 %132, %102
  %136 = shl nsw i32 %131, 7
  %137 = add nsw i32 %136, %130
  %138 = sext i32 %137 to i64
  %139 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %138
  %140 = add nuw i64 %129, 8
  %141 = getelementptr inbounds i8, i8* %48, i64 %140
  %142 = bitcast i8* %141 to %0 addrspace(3)**
  store %0 addrspace(3)* %139, %0 addrspace(3)** %142, align 8, !alias.scope !35, !noalias !36
  %143 = add nsw i32 %137, 16
  %144 = sext i32 %143 to i64
  %145 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %144
  %146 = add nuw i64 %129, 16
  %147 = getelementptr inbounds i8, i8* %48, i64 %146
  %148 = bitcast i8* %147 to %0 addrspace(3)**
  store %0 addrspace(3)* %145, %0 addrspace(3)** %148, align 8, !alias.scope !35, !noalias !36
  %149 = add nsw i32 %137, 32
  %150 = sext i32 %149 to i64
  %151 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %150
  %152 = add nuw i64 %129, 24
  %153 = getelementptr inbounds i8, i8* %48, i64 %152
  %154 = bitcast i8* %153 to %0 addrspace(3)**
  store %0 addrspace(3)* %151, %0 addrspace(3)** %154, align 8, !alias.scope !35, !noalias !36
  %155 = add nsw i32 %137, 48
  %156 = sext i32 %155 to i64
  %157 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %156
  %158 = add nuw i64 %129, 32
  %159 = getelementptr inbounds i8, i8* %48, i64 %158
  %160 = bitcast i8* %159 to %0 addrspace(3)**
  store %0 addrspace(3)* %157, %0 addrspace(3)** %160, align 8, !alias.scope !35, !noalias !36
  %161 = add nsw i32 %137, 64
  %162 = sext i32 %161 to i64
  %163 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %162
  %164 = add nuw i64 %129, 40
  %165 = getelementptr inbounds i8, i8* %48, i64 %164
  %166 = bitcast i8* %165 to %0 addrspace(3)**
  store %0 addrspace(3)* %163, %0 addrspace(3)** %166, align 8, !alias.scope !35, !noalias !36
  %167 = add nsw i32 %137, 80
  %168 = sext i32 %167 to i64
  %169 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %168
  %170 = add nuw i64 %129, 48
  %171 = getelementptr inbounds i8, i8* %48, i64 %170
  %172 = bitcast i8* %171 to %0 addrspace(3)**
  store %0 addrspace(3)* %169, %0 addrspace(3)** %172, align 8, !alias.scope !35, !noalias !36
  %173 = add nsw i32 %137, 96
  %174 = sext i32 %173 to i64
  %175 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %174
  %176 = add nuw i64 %129, 56
  %177 = getelementptr inbounds i8, i8* %48, i64 %176
  %178 = bitcast i8* %177 to %0 addrspace(3)**
  store %0 addrspace(3)* %175, %0 addrspace(3)** %178, align 8, !alias.scope !35, !noalias !36
  %179 = add nsw i32 %137, 112
  %180 = sext i32 %179 to i64
  %181 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %180
  %182 = add nuw i64 %129, 64
  %183 = getelementptr inbounds i8, i8* %48, i64 %182
  %184 = bitcast i8* %183 to %0 addrspace(3)**
  store %0 addrspace(3)* %181, %0 addrspace(3)** %184, align 8, !alias.scope !35, !noalias !36
  %185 = sext i32 %136 to i64
  %186 = add nuw i64 %129, 72
  %187 = getelementptr inbounds i8, i8* %48, i64 %186
  %188 = bitcast i8* %187 to i64*
  store i64 %185, i64* %188, align 8, !alias.scope !35, !noalias !36
  %189 = shl i32 %131, 3
  %190 = add i32 %110, %189
  %191 = add nuw i64 %129, 80
  %192 = getelementptr inbounds i8, i8* %48, i64 %191
  %193 = bitcast i8* %192 to i32*
  store i32 %190, i32* %193, align 4, !alias.scope !35, !noalias !36
  %194 = mul i32 %91, %190
  %195 = add i32 %194, %130
  %196 = sext i32 %195 to i64
  br label %278

197:                                              ; preds = %924
  %198 = phi i64 [ %925, %924 ]
  %199 = phi i64 [ %926, %924 ]
  %200 = phi i64 [ %927, %924 ]
  %201 = phi i32 [ %932, %924 ]
  %202 = add nuw i64 %200, 632
  %203 = getelementptr inbounds i8, i8* %48, i64 %202
  %204 = bitcast i8* %203 to i32*
  %205 = load i32, i32* %204, align 4, !alias.scope !35, !noalias !36
  %206 = add nuw i64 %200, 624
  %207 = getelementptr inbounds i8, i8* %48, i64 %206
  %208 = bitcast i8* %207 to i64*
  %209 = load i64, i64* %208, align 8, !alias.scope !35, !noalias !36
  %210 = add nuw i64 %200, 608
  %211 = getelementptr inbounds i8, i8* %48, i64 %210
  %212 = bitcast i8* %211 to <4 x float>*
  %213 = load <4 x float>, <4 x float>* %212, align 16, !alias.scope !35, !noalias !36
  %214 = add nuw i64 %200, 592
  %215 = getelementptr inbounds i8, i8* %48, i64 %214
  %216 = bitcast i8* %215 to <4 x float>*
  %217 = load <4 x float>, <4 x float>* %216, align 16, !alias.scope !35, !noalias !36
  %218 = add nuw i64 %200, 576
  %219 = getelementptr inbounds i8, i8* %48, i64 %218
  %220 = bitcast i8* %219 to <4 x float>*
  %221 = load <4 x float>, <4 x float>* %220, align 16, !alias.scope !35, !noalias !36
  %222 = add nuw i64 %200, 560
  %223 = getelementptr inbounds i8, i8* %48, i64 %222
  %224 = bitcast i8* %223 to <4 x float>*
  %225 = load <4 x float>, <4 x float>* %224, align 16, !alias.scope !35, !noalias !36
  %226 = add nuw i64 %200, 544
  %227 = getelementptr inbounds i8, i8* %48, i64 %226
  %228 = bitcast i8* %227 to <4 x float>*
  %229 = load <4 x float>, <4 x float>* %228, align 16, !alias.scope !35, !noalias !36
  %230 = add nuw i64 %200, 528
  %231 = getelementptr inbounds i8, i8* %48, i64 %230
  %232 = bitcast i8* %231 to <4 x float>*
  %233 = load <4 x float>, <4 x float>* %232, align 16, !alias.scope !35, !noalias !36
  %234 = add nuw i64 %200, 512
  %235 = getelementptr inbounds i8, i8* %48, i64 %234
  %236 = bitcast i8* %235 to <4 x float>*
  %237 = load <4 x float>, <4 x float>* %236, align 16, !alias.scope !35, !noalias !36
  %238 = add nuw i64 %200, 496
  %239 = getelementptr inbounds i8, i8* %48, i64 %238
  %240 = bitcast i8* %239 to <4 x float>*
  %241 = load <4 x float>, <4 x float>* %240, align 16, !alias.scope !35, !noalias !36
  %242 = add nuw i64 %200, 480
  %243 = getelementptr inbounds i8, i8* %48, i64 %242
  %244 = bitcast i8* %243 to <4 x float>*
  %245 = load <4 x float>, <4 x float>* %244, align 16, !alias.scope !35, !noalias !36
  %246 = add nuw i64 %200, 464
  %247 = getelementptr inbounds i8, i8* %48, i64 %246
  %248 = bitcast i8* %247 to <4 x float>*
  %249 = load <4 x float>, <4 x float>* %248, align 16, !alias.scope !35, !noalias !36
  %250 = add nuw i64 %200, 448
  %251 = getelementptr inbounds i8, i8* %48, i64 %250
  %252 = bitcast i8* %251 to <4 x float>*
  %253 = load <4 x float>, <4 x float>* %252, align 16, !alias.scope !35, !noalias !36
  %254 = add nuw i64 %200, 432
  %255 = getelementptr inbounds i8, i8* %48, i64 %254
  %256 = bitcast i8* %255 to <4 x float>*
  %257 = load <4 x float>, <4 x float>* %256, align 16, !alias.scope !35, !noalias !36
  %258 = add nuw i64 %200, 416
  %259 = getelementptr inbounds i8, i8* %48, i64 %258
  %260 = bitcast i8* %259 to <4 x float>*
  %261 = load <4 x float>, <4 x float>* %260, align 16, !alias.scope !35, !noalias !36
  %262 = add nuw i64 %200, 400
  %263 = getelementptr inbounds i8, i8* %48, i64 %262
  %264 = bitcast i8* %263 to <4 x float>*
  %265 = load <4 x float>, <4 x float>* %264, align 16, !alias.scope !35, !noalias !36
  %266 = add nuw i64 %200, 384
  %267 = getelementptr inbounds i8, i8* %48, i64 %266
  %268 = bitcast i8* %267 to <4 x float>*
  %269 = load <4 x float>, <4 x float>* %268, align 16, !alias.scope !35, !noalias !36
  %270 = add nuw i64 %200, 368
  %271 = getelementptr inbounds i8, i8* %48, i64 %270
  %272 = bitcast i8* %271 to <4 x float>*
  %273 = load <4 x float>, <4 x float>* %272, align 16, !alias.scope !35, !noalias !36
  %274 = add nuw i64 %200, 364
  %275 = getelementptr inbounds i8, i8* %48, i64 %274
  %276 = bitcast i8* %275 to i32*
  %277 = load i32, i32* %276, align 4, !alias.scope !35, !noalias !36
  br label %278

278:                                              ; preds = %197, %126
  %279 = phi i64 [ %127, %126 ], [ %198, %197 ]
  %280 = phi i64 [ %128, %126 ], [ %199, %197 ]
  %281 = phi i32 [ 3, %126 ], [ %433, %197 ]
  %282 = phi i64 [ %129, %126 ], [ %200, %197 ]
  %283 = phi i64 [ %196, %126 ], [ %209, %197 ]
  %284 = phi <4 x float> [ zeroinitializer, %126 ], [ %213, %197 ]
  %285 = phi <4 x float> [ zeroinitializer, %126 ], [ %217, %197 ]
  %286 = phi <4 x float> [ zeroinitializer, %126 ], [ %221, %197 ]
  %287 = phi <4 x float> [ zeroinitializer, %126 ], [ %225, %197 ]
  %288 = phi <4 x float> [ zeroinitializer, %126 ], [ %229, %197 ]
  %289 = phi <4 x float> [ zeroinitializer, %126 ], [ %233, %197 ]
  %290 = phi <4 x float> [ zeroinitializer, %126 ], [ %237, %197 ]
  %291 = phi <4 x float> [ zeroinitializer, %126 ], [ %241, %197 ]
  %292 = phi <4 x float> [ zeroinitializer, %126 ], [ %245, %197 ]
  %293 = phi <4 x float> [ zeroinitializer, %126 ], [ %249, %197 ]
  %294 = phi <4 x float> [ zeroinitializer, %126 ], [ %253, %197 ]
  %295 = phi <4 x float> [ zeroinitializer, %126 ], [ %257, %197 ]
  %296 = phi <4 x float> [ zeroinitializer, %126 ], [ %261, %197 ]
  %297 = phi <4 x float> [ zeroinitializer, %126 ], [ %265, %197 ]
  %298 = phi <4 x float> [ zeroinitializer, %126 ], [ %269, %197 ]
  %299 = phi <4 x float> [ zeroinitializer, %126 ], [ %273, %197 ]
  %300 = phi i32 [ %132, %126 ], [ %277, %197 ]
  %301 = phi i32 [ %135, %126 ], [ %205, %197 ]
  %302 = phi i32 [ 0, %126 ], [ %201, %197 ]
  %303 = add nuw i64 %282, 360
  %304 = getelementptr inbounds i8, i8* %48, i64 %303
  %305 = bitcast i8* %304 to i32*
  store i32 %302, i32* %305, align 4, !alias.scope !35, !noalias !36
  %306 = add nuw i64 %282, 356
  %307 = getelementptr inbounds i8, i8* %48, i64 %306
  %308 = bitcast i8* %307 to i32*
  store i32 %301, i32* %308, align 4, !alias.scope !35, !noalias !36
  %309 = add nuw i64 %282, 352
  %310 = getelementptr inbounds i8, i8* %48, i64 %309
  %311 = bitcast i8* %310 to i32*
  store i32 %300, i32* %311, align 4, !alias.scope !35, !noalias !36
  %312 = add nuw i64 %282, 336
  %313 = getelementptr inbounds i8, i8* %48, i64 %312
  %314 = bitcast i8* %313 to <4 x float>*
  store <4 x float> %299, <4 x float>* %314, align 16, !alias.scope !35, !noalias !36
  %315 = add nuw i64 %282, 320
  %316 = getelementptr inbounds i8, i8* %48, i64 %315
  %317 = bitcast i8* %316 to <4 x float>*
  store <4 x float> %298, <4 x float>* %317, align 16, !alias.scope !35, !noalias !36
  %318 = add nuw i64 %282, 304
  %319 = getelementptr inbounds i8, i8* %48, i64 %318
  %320 = bitcast i8* %319 to <4 x float>*
  store <4 x float> %297, <4 x float>* %320, align 16, !alias.scope !35, !noalias !36
  %321 = add nuw i64 %282, 288
  %322 = getelementptr inbounds i8, i8* %48, i64 %321
  %323 = bitcast i8* %322 to <4 x float>*
  store <4 x float> %296, <4 x float>* %323, align 16, !alias.scope !35, !noalias !36
  %324 = add nuw i64 %282, 272
  %325 = getelementptr inbounds i8, i8* %48, i64 %324
  %326 = bitcast i8* %325 to <4 x float>*
  store <4 x float> %295, <4 x float>* %326, align 16, !alias.scope !35, !noalias !36
  %327 = add nuw i64 %282, 256
  %328 = getelementptr inbounds i8, i8* %48, i64 %327
  %329 = bitcast i8* %328 to <4 x float>*
  store <4 x float> %294, <4 x float>* %329, align 16, !alias.scope !35, !noalias !36
  %330 = add nuw i64 %282, 240
  %331 = getelementptr inbounds i8, i8* %48, i64 %330
  %332 = bitcast i8* %331 to <4 x float>*
  store <4 x float> %293, <4 x float>* %332, align 16, !alias.scope !35, !noalias !36
  %333 = add nuw i64 %282, 224
  %334 = getelementptr inbounds i8, i8* %48, i64 %333
  %335 = bitcast i8* %334 to <4 x float>*
  store <4 x float> %292, <4 x float>* %335, align 16, !alias.scope !35, !noalias !36
  %336 = add nuw i64 %282, 208
  %337 = getelementptr inbounds i8, i8* %48, i64 %336
  %338 = bitcast i8* %337 to <4 x float>*
  store <4 x float> %291, <4 x float>* %338, align 16, !alias.scope !35, !noalias !36
  %339 = add nuw i64 %282, 192
  %340 = getelementptr inbounds i8, i8* %48, i64 %339
  %341 = bitcast i8* %340 to <4 x float>*
  store <4 x float> %290, <4 x float>* %341, align 16, !alias.scope !35, !noalias !36
  %342 = add nuw i64 %282, 176
  %343 = getelementptr inbounds i8, i8* %48, i64 %342
  %344 = bitcast i8* %343 to <4 x float>*
  store <4 x float> %289, <4 x float>* %344, align 16, !alias.scope !35, !noalias !36
  %345 = add nuw i64 %282, 160
  %346 = getelementptr inbounds i8, i8* %48, i64 %345
  %347 = bitcast i8* %346 to <4 x float>*
  store <4 x float> %288, <4 x float>* %347, align 16, !alias.scope !35, !noalias !36
  %348 = add nuw i64 %282, 144
  %349 = getelementptr inbounds i8, i8* %48, i64 %348
  %350 = bitcast i8* %349 to <4 x float>*
  store <4 x float> %287, <4 x float>* %350, align 16, !alias.scope !35, !noalias !36
  %351 = add nuw i64 %282, 128
  %352 = getelementptr inbounds i8, i8* %48, i64 %351
  %353 = bitcast i8* %352 to <4 x float>*
  store <4 x float> %286, <4 x float>* %353, align 16, !alias.scope !35, !noalias !36
  %354 = add nuw i64 %282, 112
  %355 = getelementptr inbounds i8, i8* %48, i64 %354
  %356 = bitcast i8* %355 to <4 x float>*
  store <4 x float> %285, <4 x float>* %356, align 16, !alias.scope !35, !noalias !36
  %357 = add nuw i64 %282, 96
  %358 = getelementptr inbounds i8, i8* %48, i64 %357
  %359 = bitcast i8* %358 to <4 x float>*
  store <4 x float> %284, <4 x float>* %359, align 16, !alias.scope !35, !noalias !36
  %360 = add nuw i64 %282, 88
  %361 = getelementptr inbounds i8, i8* %48, i64 %360
  %362 = bitcast i8* %361 to i64*
  store i64 %283, i64* %362, align 8, !alias.scope !35, !noalias !36
  %363 = getelementptr inbounds %0, %0 addrspace(1)* %119, i64 %283
  %364 = bitcast %0 addrspace(1)* %363 to i8 addrspace(1)*
  %365 = add nuw i64 %282, 8
  %366 = getelementptr inbounds i8, i8* %48, i64 %365
  %367 = bitcast i8* %366 to i8 addrspace(3)**
  %368 = load i8 addrspace(3)*, i8 addrspace(3)** %367, align 8, !alias.scope !35, !noalias !36
  call void @llvm.memcpy.p3i8.p1i8.i64(i8 addrspace(3)* align 16 dereferenceable(16) %368, i8 addrspace(1)* align 16 dereferenceable(16) %364, i64 16, i1 false) #2, !noalias !37
  %369 = add nsw i64 %283, %112
  %370 = getelementptr inbounds %0, %0 addrspace(1)* %119, i64 %369
  %371 = bitcast %0 addrspace(1)* %370 to i8 addrspace(1)*
  %372 = add nuw i64 %282, 16
  %373 = getelementptr inbounds i8, i8* %48, i64 %372
  %374 = bitcast i8* %373 to i8 addrspace(3)**
  %375 = load i8 addrspace(3)*, i8 addrspace(3)** %374, align 8, !alias.scope !35, !noalias !36
  call void @llvm.memcpy.p3i8.p1i8.i64(i8 addrspace(3)* align 16 dereferenceable(16) %375, i8 addrspace(1)* align 16 dereferenceable(16) %371, i64 16, i1 false) #2, !noalias !37
  %376 = add nsw i64 %283, %113
  %377 = getelementptr inbounds %0, %0 addrspace(1)* %119, i64 %376
  %378 = bitcast %0 addrspace(1)* %377 to i8 addrspace(1)*
  %379 = add nuw i64 %282, 24
  %380 = getelementptr inbounds i8, i8* %48, i64 %379
  %381 = bitcast i8* %380 to i8 addrspace(3)**
  %382 = load i8 addrspace(3)*, i8 addrspace(3)** %381, align 8, !alias.scope !35, !noalias !36
  call void @llvm.memcpy.p3i8.p1i8.i64(i8 addrspace(3)* align 16 dereferenceable(16) %382, i8 addrspace(1)* align 16 dereferenceable(16) %378, i64 16, i1 false) #2, !noalias !37
  %383 = add nsw i64 %283, %114
  %384 = getelementptr inbounds %0, %0 addrspace(1)* %119, i64 %383
  %385 = bitcast %0 addrspace(1)* %384 to i8 addrspace(1)*
  %386 = add nuw i64 %282, 32
  %387 = getelementptr inbounds i8, i8* %48, i64 %386
  %388 = bitcast i8* %387 to i8 addrspace(3)**
  %389 = load i8 addrspace(3)*, i8 addrspace(3)** %388, align 8, !alias.scope !35, !noalias !36
  call void @llvm.memcpy.p3i8.p1i8.i64(i8 addrspace(3)* align 16 dereferenceable(16) %389, i8 addrspace(1)* align 16 dereferenceable(16) %385, i64 16, i1 false) #2, !noalias !37
  %390 = add nsw i64 %283, %115
  %391 = getelementptr inbounds %0, %0 addrspace(1)* %119, i64 %390
  %392 = bitcast %0 addrspace(1)* %391 to i8 addrspace(1)*
  %393 = add nuw i64 %282, 40
  %394 = getelementptr inbounds i8, i8* %48, i64 %393
  %395 = bitcast i8* %394 to i8 addrspace(3)**
  %396 = load i8 addrspace(3)*, i8 addrspace(3)** %395, align 8, !alias.scope !35, !noalias !36
  call void @llvm.memcpy.p3i8.p1i8.i64(i8 addrspace(3)* align 16 dereferenceable(16) %396, i8 addrspace(1)* align 16 dereferenceable(16) %392, i64 16, i1 false) #2, !noalias !37
  %397 = add nsw i64 %283, %116
  %398 = getelementptr inbounds %0, %0 addrspace(1)* %119, i64 %397
  %399 = bitcast %0 addrspace(1)* %398 to i8 addrspace(1)*
  %400 = add nuw i64 %282, 48
  %401 = getelementptr inbounds i8, i8* %48, i64 %400
  %402 = bitcast i8* %401 to i8 addrspace(3)**
  %403 = load i8 addrspace(3)*, i8 addrspace(3)** %402, align 8, !alias.scope !35, !noalias !36
  call void @llvm.memcpy.p3i8.p1i8.i64(i8 addrspace(3)* align 16 dereferenceable(16) %403, i8 addrspace(1)* align 16 dereferenceable(16) %399, i64 16, i1 false) #2, !noalias !37
  %404 = add nsw i64 %283, %117
  %405 = getelementptr inbounds %0, %0 addrspace(1)* %119, i64 %404
  %406 = bitcast %0 addrspace(1)* %405 to i8 addrspace(1)*
  %407 = add nuw i64 %282, 56
  %408 = getelementptr inbounds i8, i8* %48, i64 %407
  %409 = bitcast i8* %408 to i8 addrspace(3)**
  %410 = load i8 addrspace(3)*, i8 addrspace(3)** %409, align 8, !alias.scope !35, !noalias !36
  call void @llvm.memcpy.p3i8.p1i8.i64(i8 addrspace(3)* align 16 dereferenceable(16) %410, i8 addrspace(1)* align 16 dereferenceable(16) %406, i64 16, i1 false) #2, !noalias !37
  %411 = add nsw i64 %283, %118
  %412 = getelementptr inbounds %0, %0 addrspace(1)* %119, i64 %411
  %413 = bitcast %0 addrspace(1)* %412 to i8 addrspace(1)*
  %414 = add nuw i64 %282, 64
  %415 = getelementptr inbounds i8, i8* %48, i64 %414
  %416 = bitcast i8* %415 to i8 addrspace(3)**
  %417 = load i8 addrspace(3)*, i8 addrspace(3)** %416, align 8, !alias.scope !35, !noalias !36
  call void @llvm.memcpy.p3i8.p1i8.i64(i8 addrspace(3)* align 16 dereferenceable(16) %417, i8 addrspace(1)* align 16 dereferenceable(16) %413, i64 16, i1 false) #2, !noalias !37
  %418 = add nuw i64 %280, 1
  %419 = icmp ult i64 %418, %70
  br i1 %419, label %424, label %420

420:                                              ; preds = %278
  %421 = add nuw i64 %279, 1
  %422 = icmp ult i64 %421, %80
  br i1 %422, label %424, label %423

423:                                              ; preds = %420
  br label %435

424:                                              ; preds = %420, %278
  %425 = phi i64 [ %421, %420 ], [ %279, %278 ]
  %426 = phi i64 [ 0, %420 ], [ %418, %278 ]
  %427 = add nuw i64 %282, 640
  %428 = icmp eq i32 %281, 3
  br i1 %428, label %122, label %430

429:                                              ; preds = %917
  br label %430

430:                                              ; preds = %429, %424
  %431 = phi i64 [ %425, %424 ], [ 0, %429 ]
  %432 = phi i64 [ %426, %424 ], [ 0, %429 ]
  %433 = phi i32 [ %281, %424 ], [ 1, %429 ]
  %434 = phi i64 [ %427, %424 ], [ 0, %429 ]
  br label %924

435:                                              ; preds = %920, %423
  %436 = phi i64 [ %921, %920 ], [ 0, %423 ]
  %437 = phi i64 [ %922, %920 ], [ 0, %423 ]
  %438 = phi i64 [ %923, %920 ], [ 0, %423 ]
  %439 = add nuw i64 %438, 352
  %440 = getelementptr inbounds i8, i8* %48, i64 %439
  %441 = bitcast i8* %440 to i32*
  %442 = load i32, i32* %441, align 32, !alias.scope !35, !noalias !36
  %443 = add i32 %109, %442
  %444 = add nuw i64 %438, 364
  %445 = getelementptr inbounds i8, i8* %48, i64 %444
  %446 = bitcast i8* %445 to i32*
  store i32 %443, i32* %446, align 4, !alias.scope !35, !noalias !36
  %447 = or i64 %438, 96
  %448 = getelementptr inbounds i8, i8* %48, i64 %447
  %449 = bitcast i8* %448 to <4 x float>*
  %450 = load <4 x float>, <4 x float>* %449, align 32, !alias.scope !35, !noalias !36
  %451 = or i64 %438, 112
  %452 = getelementptr inbounds i8, i8* %48, i64 %451
  %453 = bitcast i8* %452 to <4 x float>*
  %454 = load <4 x float>, <4 x float>* %453, align 16, !alias.scope !35, !noalias !36
  %455 = add nuw i64 %438, 128
  %456 = getelementptr inbounds i8, i8* %48, i64 %455
  %457 = bitcast i8* %456 to <4 x float>*
  %458 = load <4 x float>, <4 x float>* %457, align 128, !alias.scope !35, !noalias !36
  %459 = add nuw i64 %438, 144
  %460 = getelementptr inbounds i8, i8* %48, i64 %459
  %461 = bitcast i8* %460 to <4 x float>*
  %462 = load <4 x float>, <4 x float>* %461, align 16, !alias.scope !35, !noalias !36
  %463 = add nuw i64 %438, 160
  %464 = getelementptr inbounds i8, i8* %48, i64 %463
  %465 = bitcast i8* %464 to <4 x float>*
  %466 = load <4 x float>, <4 x float>* %465, align 32, !alias.scope !35, !noalias !36
  %467 = add nuw i64 %438, 176
  %468 = getelementptr inbounds i8, i8* %48, i64 %467
  %469 = bitcast i8* %468 to <4 x float>*
  %470 = load <4 x float>, <4 x float>* %469, align 16, !alias.scope !35, !noalias !36
  %471 = add nuw i64 %438, 192
  %472 = getelementptr inbounds i8, i8* %48, i64 %471
  %473 = bitcast i8* %472 to <4 x float>*
  %474 = load <4 x float>, <4 x float>* %473, align 64, !alias.scope !35, !noalias !36
  %475 = add nuw i64 %438, 208
  %476 = getelementptr inbounds i8, i8* %48, i64 %475
  %477 = bitcast i8* %476 to <4 x float>*
  %478 = load <4 x float>, <4 x float>* %477, align 16, !alias.scope !35, !noalias !36
  %479 = add nuw i64 %438, 224
  %480 = getelementptr inbounds i8, i8* %48, i64 %479
  %481 = bitcast i8* %480 to <4 x float>*
  %482 = load <4 x float>, <4 x float>* %481, align 32, !alias.scope !35, !noalias !36
  %483 = add nuw i64 %438, 240
  %484 = getelementptr inbounds i8, i8* %48, i64 %483
  %485 = bitcast i8* %484 to <4 x float>*
  %486 = load <4 x float>, <4 x float>* %485, align 16, !alias.scope !35, !noalias !36
  %487 = add nuw i64 %438, 256
  %488 = getelementptr inbounds i8, i8* %48, i64 %487
  %489 = bitcast i8* %488 to <4 x float>*
  %490 = load <4 x float>, <4 x float>* %489, align 128, !alias.scope !35, !noalias !36
  %491 = add nuw i64 %438, 272
  %492 = getelementptr inbounds i8, i8* %48, i64 %491
  %493 = bitcast i8* %492 to <4 x float>*
  %494 = load <4 x float>, <4 x float>* %493, align 16, !alias.scope !35, !noalias !36
  %495 = add nuw i64 %438, 288
  %496 = getelementptr inbounds i8, i8* %48, i64 %495
  %497 = bitcast i8* %496 to <4 x float>*
  %498 = load <4 x float>, <4 x float>* %497, align 32, !alias.scope !35, !noalias !36
  %499 = add nuw i64 %438, 304
  %500 = getelementptr inbounds i8, i8* %48, i64 %499
  %501 = bitcast i8* %500 to <4 x float>*
  %502 = load <4 x float>, <4 x float>* %501, align 16, !alias.scope !35, !noalias !36
  %503 = add nuw i64 %438, 320
  %504 = getelementptr inbounds i8, i8* %48, i64 %503
  %505 = bitcast i8* %504 to <4 x float>*
  %506 = load <4 x float>, <4 x float>* %505, align 64, !alias.scope !35, !noalias !36
  %507 = add nuw i64 %438, 336
  %508 = getelementptr inbounds i8, i8* %48, i64 %507
  %509 = bitcast i8* %508 to <4 x float>*
  %510 = load <4 x float>, <4 x float>* %509, align 16, !alias.scope !35, !noalias !36
  %511 = add nuw i64 %438, 356
  %512 = getelementptr inbounds i8, i8* %48, i64 %511
  %513 = bitcast i8* %512 to i32*
  %514 = load i32, i32* %513, align 4, !alias.scope !35, !noalias !36
  %515 = or i64 %438, 72
  %516 = getelementptr inbounds i8, i8* %48, i64 %515
  %517 = bitcast i8* %516 to i64*
  %518 = add nuw i64 %438, 368
  %519 = getelementptr inbounds i8, i8* %48, i64 %518
  %520 = bitcast i8* %519 to <4 x float>*
  %521 = add nuw i64 %438, 384
  %522 = getelementptr inbounds i8, i8* %48, i64 %521
  %523 = bitcast i8* %522 to <4 x float>*
  %524 = add nuw i64 %438, 400
  %525 = getelementptr inbounds i8, i8* %48, i64 %524
  %526 = bitcast i8* %525 to <4 x float>*
  %527 = add nuw i64 %438, 416
  %528 = getelementptr inbounds i8, i8* %48, i64 %527
  %529 = bitcast i8* %528 to <4 x float>*
  %530 = add nuw i64 %438, 432
  %531 = getelementptr inbounds i8, i8* %48, i64 %530
  %532 = bitcast i8* %531 to <4 x float>*
  %533 = add nuw i64 %438, 448
  %534 = getelementptr inbounds i8, i8* %48, i64 %533
  %535 = bitcast i8* %534 to <4 x float>*
  %536 = add nuw i64 %438, 464
  %537 = getelementptr inbounds i8, i8* %48, i64 %536
  %538 = bitcast i8* %537 to <4 x float>*
  %539 = add nuw i64 %438, 480
  %540 = getelementptr inbounds i8, i8* %48, i64 %539
  %541 = bitcast i8* %540 to <4 x float>*
  %542 = add nuw i64 %438, 496
  %543 = getelementptr inbounds i8, i8* %48, i64 %542
  %544 = bitcast i8* %543 to <4 x float>*
  %545 = add nuw i64 %438, 512
  %546 = getelementptr inbounds i8, i8* %48, i64 %545
  %547 = bitcast i8* %546 to <4 x float>*
  %548 = add nuw i64 %438, 528
  %549 = getelementptr inbounds i8, i8* %48, i64 %548
  %550 = bitcast i8* %549 to <4 x float>*
  %551 = add nuw i64 %438, 544
  %552 = getelementptr inbounds i8, i8* %48, i64 %551
  %553 = bitcast i8* %552 to <4 x float>*
  %554 = add nuw i64 %438, 560
  %555 = getelementptr inbounds i8, i8* %48, i64 %554
  %556 = bitcast i8* %555 to <4 x float>*
  %557 = add nuw i64 %438, 576
  %558 = getelementptr inbounds i8, i8* %48, i64 %557
  %559 = bitcast i8* %558 to <4 x float>*
  %560 = add nuw i64 %438, 592
  %561 = getelementptr inbounds i8, i8* %48, i64 %560
  %562 = bitcast i8* %561 to <4 x float>*
  %563 = add nuw i64 %438, 608
  %564 = getelementptr inbounds i8, i8* %48, i64 %563
  %565 = bitcast i8* %564 to <4 x float>*
  br label %566

566:                                              ; preds = %566, %435
  %567 = phi i64 [ %883, %566 ], [ 0, %435 ]
  %568 = phi <4 x float> [ %882, %566 ], [ %450, %435 ]
  %569 = phi <4 x float> [ %874, %566 ], [ %454, %435 ]
  %570 = phi <4 x float> [ %849, %566 ], [ %458, %435 ]
  %571 = phi <4 x float> [ %841, %566 ], [ %462, %435 ]
  %572 = phi <4 x float> [ %816, %566 ], [ %466, %435 ]
  %573 = phi <4 x float> [ %808, %566 ], [ %470, %435 ]
  %574 = phi <4 x float> [ %783, %566 ], [ %474, %435 ]
  %575 = phi <4 x float> [ %775, %566 ], [ %478, %435 ]
  %576 = phi <4 x float> [ %750, %566 ], [ %482, %435 ]
  %577 = phi <4 x float> [ %742, %566 ], [ %486, %435 ]
  %578 = phi <4 x float> [ %717, %566 ], [ %490, %435 ]
  %579 = phi <4 x float> [ %709, %566 ], [ %494, %435 ]
  %580 = phi <4 x float> [ %684, %566 ], [ %498, %435 ]
  %581 = phi <4 x float> [ %676, %566 ], [ %502, %435 ]
  %582 = phi <4 x float> [ %651, %566 ], [ %506, %435 ]
  %583 = phi <4 x float> [ %643, %566 ], [ %510, %435 ]
  %584 = phi i32 [ %601, %566 ], [ %442, %435 ]
  %585 = phi i32 [ %617, %566 ], [ %514, %435 ]
  %586 = sext i32 %584 to i64
  %587 = getelementptr inbounds %0, %0 addrspace(1)* %120, i64 %586, i32 0
  %588 = load <4 x float>, <4 x float> addrspace(1)* %587, align 16, !noalias !34
  %589 = add nsw i32 %584, %96
  %590 = sext i32 %589 to i64
  %591 = getelementptr inbounds %0, %0 addrspace(1)* %120, i64 %590, i32 0
  %592 = load <4 x float>, <4 x float> addrspace(1)* %591, align 16, !noalias !34
  %593 = add nsw i32 %589, %96
  %594 = sext i32 %593 to i64
  %595 = getelementptr inbounds %0, %0 addrspace(1)* %120, i64 %594, i32 0
  %596 = load <4 x float>, <4 x float> addrspace(1)* %595, align 16, !noalias !34
  %597 = add nsw i32 %593, %96
  %598 = sext i32 %597 to i64
  %599 = getelementptr inbounds %0, %0 addrspace(1)* %120, i64 %598, i32 0
  %600 = load <4 x float>, <4 x float> addrspace(1)* %599, align 16, !noalias !34
  %601 = add nsw i32 %597, %96
  %602 = sext i32 %585 to i64
  %603 = getelementptr inbounds %0, %0 addrspace(1)* %120, i64 %602, i32 0
  %604 = load <4 x float>, <4 x float> addrspace(1)* %603, align 16, !noalias !34
  %605 = add nsw i32 %585, %96
  %606 = sext i32 %605 to i64
  %607 = getelementptr inbounds %0, %0 addrspace(1)* %120, i64 %606, i32 0
  %608 = load <4 x float>, <4 x float> addrspace(1)* %607, align 16, !noalias !34
  %609 = add nsw i32 %605, %96
  %610 = sext i32 %609 to i64
  %611 = getelementptr inbounds %0, %0 addrspace(1)* %120, i64 %610, i32 0
  %612 = load <4 x float>, <4 x float> addrspace(1)* %611, align 16, !noalias !34
  %613 = add nsw i32 %609, %96
  %614 = sext i32 %613 to i64
  %615 = getelementptr inbounds %0, %0 addrspace(1)* %120, i64 %614, i32 0
  %616 = load <4 x float>, <4 x float> addrspace(1)* %615, align 16, !noalias !34
  %617 = add nsw i32 %613, %96
  %618 = load i64, i64* %517, align 8, !alias.scope !35, !noalias !36
  %619 = add nuw nsw i64 %567, %618
  %620 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %619, i32 0, i64 0
  %621 = load float, float addrspace(3)* %620, align 16, !alias.scope !38, !noalias !39
  %622 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %619, i32 0, i64 1
  %623 = load float, float addrspace(3)* %622, align 4, !alias.scope !38, !noalias !39
  %624 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %619, i32 0, i64 2
  %625 = load float, float addrspace(3)* %624, align 8, !alias.scope !38, !noalias !39
  %626 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %619, i32 0, i64 3
  %627 = load float, float addrspace(3)* %626, align 4, !alias.scope !38, !noalias !39
  %628 = insertelement <4 x float> undef, float %621, i32 0
  %629 = shufflevector <4 x float> %628, <4 x float> undef, <4 x i32> zeroinitializer
  %630 = fmul contract <4 x float> %588, %629
  %631 = fadd contract <4 x float> %630, %583
  %632 = insertelement <4 x float> undef, float %623, i32 0
  %633 = shufflevector <4 x float> %632, <4 x float> undef, <4 x i32> zeroinitializer
  %634 = fmul contract <4 x float> %592, %633
  %635 = fadd contract <4 x float> %634, %631
  %636 = insertelement <4 x float> undef, float %625, i32 0
  %637 = shufflevector <4 x float> %636, <4 x float> undef, <4 x i32> zeroinitializer
  %638 = fmul contract <4 x float> %596, %637
  %639 = fadd contract <4 x float> %638, %635
  %640 = insertelement <4 x float> undef, float %627, i32 0
  %641 = shufflevector <4 x float> %640, <4 x float> undef, <4 x i32> zeroinitializer
  %642 = fmul contract <4 x float> %600, %641
  %643 = fadd contract <4 x float> %642, %639
  %644 = fmul contract <4 x float> %604, %629
  %645 = fadd contract <4 x float> %644, %582
  %646 = fmul contract <4 x float> %608, %633
  %647 = fadd contract <4 x float> %646, %645
  %648 = fmul contract <4 x float> %612, %637
  %649 = fadd contract <4 x float> %648, %647
  %650 = fmul contract <4 x float> %616, %641
  %651 = fadd contract <4 x float> %650, %649
  %652 = add nuw nsw i64 %619, 16
  %653 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %652, i32 0, i64 0
  %654 = load float, float addrspace(3)* %653, align 16, !alias.scope !38, !noalias !39
  %655 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %652, i32 0, i64 1
  %656 = load float, float addrspace(3)* %655, align 4, !alias.scope !38, !noalias !39
  %657 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %652, i32 0, i64 2
  %658 = load float, float addrspace(3)* %657, align 8, !alias.scope !38, !noalias !39
  %659 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %652, i32 0, i64 3
  %660 = load float, float addrspace(3)* %659, align 4, !alias.scope !38, !noalias !39
  %661 = insertelement <4 x float> undef, float %654, i32 0
  %662 = shufflevector <4 x float> %661, <4 x float> undef, <4 x i32> zeroinitializer
  %663 = fmul contract <4 x float> %588, %662
  %664 = fadd contract <4 x float> %663, %581
  %665 = insertelement <4 x float> undef, float %656, i32 0
  %666 = shufflevector <4 x float> %665, <4 x float> undef, <4 x i32> zeroinitializer
  %667 = fmul contract <4 x float> %592, %666
  %668 = fadd contract <4 x float> %667, %664
  %669 = insertelement <4 x float> undef, float %658, i32 0
  %670 = shufflevector <4 x float> %669, <4 x float> undef, <4 x i32> zeroinitializer
  %671 = fmul contract <4 x float> %596, %670
  %672 = fadd contract <4 x float> %671, %668
  %673 = insertelement <4 x float> undef, float %660, i32 0
  %674 = shufflevector <4 x float> %673, <4 x float> undef, <4 x i32> zeroinitializer
  %675 = fmul contract <4 x float> %600, %674
  %676 = fadd contract <4 x float> %675, %672
  %677 = fmul contract <4 x float> %604, %662
  %678 = fadd contract <4 x float> %677, %580
  %679 = fmul contract <4 x float> %608, %666
  %680 = fadd contract <4 x float> %679, %678
  %681 = fmul contract <4 x float> %612, %670
  %682 = fadd contract <4 x float> %681, %680
  %683 = fmul contract <4 x float> %616, %674
  %684 = fadd contract <4 x float> %683, %682
  %685 = add nuw nsw i64 %619, 32
  %686 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %685, i32 0, i64 0
  %687 = load float, float addrspace(3)* %686, align 16, !alias.scope !38, !noalias !39
  %688 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %685, i32 0, i64 1
  %689 = load float, float addrspace(3)* %688, align 4, !alias.scope !38, !noalias !39
  %690 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %685, i32 0, i64 2
  %691 = load float, float addrspace(3)* %690, align 8, !alias.scope !38, !noalias !39
  %692 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %685, i32 0, i64 3
  %693 = load float, float addrspace(3)* %692, align 4, !alias.scope !38, !noalias !39
  %694 = insertelement <4 x float> undef, float %687, i32 0
  %695 = shufflevector <4 x float> %694, <4 x float> undef, <4 x i32> zeroinitializer
  %696 = fmul contract <4 x float> %588, %695
  %697 = fadd contract <4 x float> %696, %579
  %698 = insertelement <4 x float> undef, float %689, i32 0
  %699 = shufflevector <4 x float> %698, <4 x float> undef, <4 x i32> zeroinitializer
  %700 = fmul contract <4 x float> %592, %699
  %701 = fadd contract <4 x float> %700, %697
  %702 = insertelement <4 x float> undef, float %691, i32 0
  %703 = shufflevector <4 x float> %702, <4 x float> undef, <4 x i32> zeroinitializer
  %704 = fmul contract <4 x float> %596, %703
  %705 = fadd contract <4 x float> %704, %701
  %706 = insertelement <4 x float> undef, float %693, i32 0
  %707 = shufflevector <4 x float> %706, <4 x float> undef, <4 x i32> zeroinitializer
  %708 = fmul contract <4 x float> %600, %707
  %709 = fadd contract <4 x float> %708, %705
  %710 = fmul contract <4 x float> %604, %695
  %711 = fadd contract <4 x float> %710, %578
  %712 = fmul contract <4 x float> %608, %699
  %713 = fadd contract <4 x float> %712, %711
  %714 = fmul contract <4 x float> %612, %703
  %715 = fadd contract <4 x float> %714, %713
  %716 = fmul contract <4 x float> %616, %707
  %717 = fadd contract <4 x float> %716, %715
  %718 = add nuw nsw i64 %619, 48
  %719 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %718, i32 0, i64 0
  %720 = load float, float addrspace(3)* %719, align 16, !alias.scope !38, !noalias !39
  %721 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %718, i32 0, i64 1
  %722 = load float, float addrspace(3)* %721, align 4, !alias.scope !38, !noalias !39
  %723 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %718, i32 0, i64 2
  %724 = load float, float addrspace(3)* %723, align 8, !alias.scope !38, !noalias !39
  %725 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %718, i32 0, i64 3
  %726 = load float, float addrspace(3)* %725, align 4, !alias.scope !38, !noalias !39
  %727 = insertelement <4 x float> undef, float %720, i32 0
  %728 = shufflevector <4 x float> %727, <4 x float> undef, <4 x i32> zeroinitializer
  %729 = fmul contract <4 x float> %588, %728
  %730 = fadd contract <4 x float> %729, %577
  %731 = insertelement <4 x float> undef, float %722, i32 0
  %732 = shufflevector <4 x float> %731, <4 x float> undef, <4 x i32> zeroinitializer
  %733 = fmul contract <4 x float> %592, %732
  %734 = fadd contract <4 x float> %733, %730
  %735 = insertelement <4 x float> undef, float %724, i32 0
  %736 = shufflevector <4 x float> %735, <4 x float> undef, <4 x i32> zeroinitializer
  %737 = fmul contract <4 x float> %596, %736
  %738 = fadd contract <4 x float> %737, %734
  %739 = insertelement <4 x float> undef, float %726, i32 0
  %740 = shufflevector <4 x float> %739, <4 x float> undef, <4 x i32> zeroinitializer
  %741 = fmul contract <4 x float> %600, %740
  %742 = fadd contract <4 x float> %741, %738
  %743 = fmul contract <4 x float> %604, %728
  %744 = fadd contract <4 x float> %743, %576
  %745 = fmul contract <4 x float> %608, %732
  %746 = fadd contract <4 x float> %745, %744
  %747 = fmul contract <4 x float> %612, %736
  %748 = fadd contract <4 x float> %747, %746
  %749 = fmul contract <4 x float> %616, %740
  %750 = fadd contract <4 x float> %749, %748
  %751 = add nuw nsw i64 %619, 64
  %752 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %751, i32 0, i64 0
  %753 = load float, float addrspace(3)* %752, align 16, !alias.scope !38, !noalias !39
  %754 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %751, i32 0, i64 1
  %755 = load float, float addrspace(3)* %754, align 4, !alias.scope !38, !noalias !39
  %756 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %751, i32 0, i64 2
  %757 = load float, float addrspace(3)* %756, align 8, !alias.scope !38, !noalias !39
  %758 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %751, i32 0, i64 3
  %759 = load float, float addrspace(3)* %758, align 4, !alias.scope !38, !noalias !39
  %760 = insertelement <4 x float> undef, float %753, i32 0
  %761 = shufflevector <4 x float> %760, <4 x float> undef, <4 x i32> zeroinitializer
  %762 = fmul contract <4 x float> %588, %761
  %763 = fadd contract <4 x float> %762, %575
  %764 = insertelement <4 x float> undef, float %755, i32 0
  %765 = shufflevector <4 x float> %764, <4 x float> undef, <4 x i32> zeroinitializer
  %766 = fmul contract <4 x float> %592, %765
  %767 = fadd contract <4 x float> %766, %763
  %768 = insertelement <4 x float> undef, float %757, i32 0
  %769 = shufflevector <4 x float> %768, <4 x float> undef, <4 x i32> zeroinitializer
  %770 = fmul contract <4 x float> %596, %769
  %771 = fadd contract <4 x float> %770, %767
  %772 = insertelement <4 x float> undef, float %759, i32 0
  %773 = shufflevector <4 x float> %772, <4 x float> undef, <4 x i32> zeroinitializer
  %774 = fmul contract <4 x float> %600, %773
  %775 = fadd contract <4 x float> %774, %771
  %776 = fmul contract <4 x float> %604, %761
  %777 = fadd contract <4 x float> %776, %574
  %778 = fmul contract <4 x float> %608, %765
  %779 = fadd contract <4 x float> %778, %777
  %780 = fmul contract <4 x float> %612, %769
  %781 = fadd contract <4 x float> %780, %779
  %782 = fmul contract <4 x float> %616, %773
  %783 = fadd contract <4 x float> %782, %781
  %784 = add nuw nsw i64 %619, 80
  %785 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %784, i32 0, i64 0
  %786 = load float, float addrspace(3)* %785, align 16, !alias.scope !38, !noalias !39
  %787 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %784, i32 0, i64 1
  %788 = load float, float addrspace(3)* %787, align 4, !alias.scope !38, !noalias !39
  %789 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %784, i32 0, i64 2
  %790 = load float, float addrspace(3)* %789, align 8, !alias.scope !38, !noalias !39
  %791 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %784, i32 0, i64 3
  %792 = load float, float addrspace(3)* %791, align 4, !alias.scope !38, !noalias !39
  %793 = insertelement <4 x float> undef, float %786, i32 0
  %794 = shufflevector <4 x float> %793, <4 x float> undef, <4 x i32> zeroinitializer
  %795 = fmul contract <4 x float> %588, %794
  %796 = fadd contract <4 x float> %795, %573
  %797 = insertelement <4 x float> undef, float %788, i32 0
  %798 = shufflevector <4 x float> %797, <4 x float> undef, <4 x i32> zeroinitializer
  %799 = fmul contract <4 x float> %592, %798
  %800 = fadd contract <4 x float> %799, %796
  %801 = insertelement <4 x float> undef, float %790, i32 0
  %802 = shufflevector <4 x float> %801, <4 x float> undef, <4 x i32> zeroinitializer
  %803 = fmul contract <4 x float> %596, %802
  %804 = fadd contract <4 x float> %803, %800
  %805 = insertelement <4 x float> undef, float %792, i32 0
  %806 = shufflevector <4 x float> %805, <4 x float> undef, <4 x i32> zeroinitializer
  %807 = fmul contract <4 x float> %600, %806
  %808 = fadd contract <4 x float> %807, %804
  %809 = fmul contract <4 x float> %604, %794
  %810 = fadd contract <4 x float> %809, %572
  %811 = fmul contract <4 x float> %608, %798
  %812 = fadd contract <4 x float> %811, %810
  %813 = fmul contract <4 x float> %612, %802
  %814 = fadd contract <4 x float> %813, %812
  %815 = fmul contract <4 x float> %616, %806
  %816 = fadd contract <4 x float> %815, %814
  %817 = add nuw nsw i64 %619, 96
  %818 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %817, i32 0, i64 0
  %819 = load float, float addrspace(3)* %818, align 16, !alias.scope !38, !noalias !39
  %820 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %817, i32 0, i64 1
  %821 = load float, float addrspace(3)* %820, align 4, !alias.scope !38, !noalias !39
  %822 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %817, i32 0, i64 2
  %823 = load float, float addrspace(3)* %822, align 8, !alias.scope !38, !noalias !39
  %824 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %817, i32 0, i64 3
  %825 = load float, float addrspace(3)* %824, align 4, !alias.scope !38, !noalias !39
  %826 = insertelement <4 x float> undef, float %819, i32 0
  %827 = shufflevector <4 x float> %826, <4 x float> undef, <4 x i32> zeroinitializer
  %828 = fmul contract <4 x float> %588, %827
  %829 = fadd contract <4 x float> %828, %571
  %830 = insertelement <4 x float> undef, float %821, i32 0
  %831 = shufflevector <4 x float> %830, <4 x float> undef, <4 x i32> zeroinitializer
  %832 = fmul contract <4 x float> %592, %831
  %833 = fadd contract <4 x float> %832, %829
  %834 = insertelement <4 x float> undef, float %823, i32 0
  %835 = shufflevector <4 x float> %834, <4 x float> undef, <4 x i32> zeroinitializer
  %836 = fmul contract <4 x float> %596, %835
  %837 = fadd contract <4 x float> %836, %833
  %838 = insertelement <4 x float> undef, float %825, i32 0
  %839 = shufflevector <4 x float> %838, <4 x float> undef, <4 x i32> zeroinitializer
  %840 = fmul contract <4 x float> %600, %839
  %841 = fadd contract <4 x float> %840, %837
  %842 = fmul contract <4 x float> %604, %827
  %843 = fadd contract <4 x float> %842, %570
  %844 = fmul contract <4 x float> %608, %831
  %845 = fadd contract <4 x float> %844, %843
  %846 = fmul contract <4 x float> %612, %835
  %847 = fadd contract <4 x float> %846, %845
  %848 = fmul contract <4 x float> %616, %839
  %849 = fadd contract <4 x float> %848, %847
  %850 = add nuw nsw i64 %619, 112
  %851 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %850, i32 0, i64 0
  %852 = load float, float addrspace(3)* %851, align 16, !alias.scope !38, !noalias !39
  %853 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %850, i32 0, i64 1
  %854 = load float, float addrspace(3)* %853, align 4, !alias.scope !38, !noalias !39
  %855 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %850, i32 0, i64 2
  %856 = load float, float addrspace(3)* %855, align 8, !alias.scope !38, !noalias !39
  %857 = getelementptr inbounds %0, %0 addrspace(3)* %24, i64 %850, i32 0, i64 3
  %858 = load float, float addrspace(3)* %857, align 4, !alias.scope !38, !noalias !39
  %859 = insertelement <4 x float> undef, float %852, i32 0
  %860 = shufflevector <4 x float> %859, <4 x float> undef, <4 x i32> zeroinitializer
  %861 = fmul contract <4 x float> %588, %860
  %862 = fadd contract <4 x float> %861, %569
  %863 = insertelement <4 x float> undef, float %854, i32 0
  %864 = shufflevector <4 x float> %863, <4 x float> undef, <4 x i32> zeroinitializer
  %865 = fmul contract <4 x float> %592, %864
  %866 = fadd contract <4 x float> %865, %862
  %867 = insertelement <4 x float> undef, float %856, i32 0
  %868 = shufflevector <4 x float> %867, <4 x float> undef, <4 x i32> zeroinitializer
  %869 = fmul contract <4 x float> %596, %868
  %870 = fadd contract <4 x float> %869, %866
  %871 = insertelement <4 x float> undef, float %858, i32 0
  %872 = shufflevector <4 x float> %871, <4 x float> undef, <4 x i32> zeroinitializer
  %873 = fmul contract <4 x float> %600, %872
  %874 = fadd contract <4 x float> %873, %870
  %875 = fmul contract <4 x float> %604, %860
  %876 = fadd contract <4 x float> %875, %568
  %877 = fmul contract <4 x float> %608, %864
  %878 = fadd contract <4 x float> %877, %876
  %879 = fmul contract <4 x float> %612, %868
  %880 = fadd contract <4 x float> %879, %878
  %881 = fmul contract <4 x float> %616, %872
  %882 = fadd contract <4 x float> %881, %880
  %883 = add nuw nsw i64 %567, 1
  %884 = icmp eq i64 %883, 16
  br i1 %884, label %885, label %566

885:                                              ; preds = %566
  %886 = phi <4 x float> [ %643, %566 ]
  %887 = phi <4 x float> [ %651, %566 ]
  %888 = phi <4 x float> [ %676, %566 ]
  %889 = phi <4 x float> [ %684, %566 ]
  %890 = phi <4 x float> [ %709, %566 ]
  %891 = phi <4 x float> [ %717, %566 ]
  %892 = phi <4 x float> [ %742, %566 ]
  %893 = phi <4 x float> [ %750, %566 ]
  %894 = phi <4 x float> [ %775, %566 ]
  %895 = phi <4 x float> [ %783, %566 ]
  %896 = phi <4 x float> [ %808, %566 ]
  %897 = phi <4 x float> [ %816, %566 ]
  %898 = phi <4 x float> [ %841, %566 ]
  %899 = phi <4 x float> [ %849, %566 ]
  %900 = phi <4 x float> [ %874, %566 ]
  %901 = phi <4 x float> [ %882, %566 ]
  store <4 x float> %886, <4 x float>* %520, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %887, <4 x float>* %523, align 128, !alias.scope !35, !noalias !36
  store <4 x float> %888, <4 x float>* %526, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %889, <4 x float>* %529, align 32, !alias.scope !35, !noalias !36
  store <4 x float> %890, <4 x float>* %532, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %891, <4 x float>* %535, align 64, !alias.scope !35, !noalias !36
  store <4 x float> %892, <4 x float>* %538, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %893, <4 x float>* %541, align 32, !alias.scope !35, !noalias !36
  store <4 x float> %894, <4 x float>* %544, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %895, <4 x float>* %547, align 128, !alias.scope !35, !noalias !36
  store <4 x float> %896, <4 x float>* %550, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %897, <4 x float>* %553, align 32, !alias.scope !35, !noalias !36
  store <4 x float> %898, <4 x float>* %556, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %899, <4 x float>* %559, align 64, !alias.scope !35, !noalias !36
  store <4 x float> %900, <4 x float>* %562, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %901, <4 x float>* %565, align 32, !alias.scope !35, !noalias !36
  %902 = or i64 %438, 88
  %903 = getelementptr inbounds i8, i8* %48, i64 %902
  %904 = bitcast i8* %903 to i64*
  %905 = load i64, i64* %904, align 8, !alias.scope !35, !noalias !36
  %906 = add i64 %905, %111
  %907 = add nuw i64 %438, 624
  %908 = getelementptr inbounds i8, i8* %48, i64 %907
  %909 = bitcast i8* %908 to i64*
  store i64 %906, i64* %909, align 16, !alias.scope !35, !noalias !36
  %910 = load i32, i32* %513, align 4, !alias.scope !35, !noalias !36
  %911 = add i32 %109, %910
  %912 = add nuw i64 %438, 632
  %913 = getelementptr inbounds i8, i8* %48, i64 %912
  %914 = bitcast i8* %913 to i32*
  store i32 %911, i32* %914, align 8, !alias.scope !35, !noalias !36
  %915 = add nuw i64 %437, 1
  %916 = icmp ult i64 %915, %70
  br i1 %916, label %920, label %917

917:                                              ; preds = %885
  %918 = add nuw i64 %436, 1
  %919 = icmp ult i64 %918, %80
  br i1 %919, label %920, label %429

920:                                              ; preds = %917, %885
  %921 = phi i64 [ %918, %917 ], [ %436, %885 ]
  %922 = phi i64 [ 0, %917 ], [ %915, %885 ]
  %923 = add nuw i64 %438, 640
  br label %435

924:                                              ; preds = %1060, %430
  %925 = phi i64 [ %1061, %1060 ], [ %431, %430 ]
  %926 = phi i64 [ %1062, %1060 ], [ %432, %430 ]
  %927 = phi i64 [ %1063, %1060 ], [ %434, %430 ]
  %928 = add nuw i64 %927, 360
  %929 = getelementptr inbounds i8, i8* %48, i64 %928
  %930 = bitcast i8* %929 to i32*
  %931 = load i32, i32* %930, align 4, !alias.scope !35, !noalias !36
  %932 = add nsw i32 %931, %102
  %933 = icmp slt i32 %932, %91
  br i1 %933, label %197, label %934

934:                                              ; preds = %924
  %935 = add nuw i64 %927, 80
  %936 = getelementptr inbounds i8, i8* %48, i64 %935
  %937 = bitcast i8* %936 to i32*
  %938 = load i32, i32* %937, align 4, !alias.scope !35, !noalias !36
  %939 = mul nsw i32 %938, %96
  %940 = getelementptr inbounds i8, i8* %48, i64 %927
  %941 = bitcast i8* %940 to i32*
  %942 = load i32, i32* %941, align 4, !alias.scope !35, !noalias !36
  %943 = add nsw i32 %939, %942
  %944 = add nsw i32 %943, %102
  %945 = sext i32 %943 to i64
  %946 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %945, i32 0
  %947 = add nuw i64 %927, 368
  %948 = getelementptr inbounds i8, i8* %48, i64 %947
  %949 = bitcast i8* %948 to <4 x float>*
  %950 = load <4 x float>, <4 x float>* %949, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %950, <4 x float> addrspace(1)* %946, align 16, !noalias !34
  %951 = add nsw i32 %943, %96
  %952 = sext i32 %951 to i64
  %953 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %952, i32 0
  %954 = add nuw i64 %927, 400
  %955 = getelementptr inbounds i8, i8* %48, i64 %954
  %956 = bitcast i8* %955 to <4 x float>*
  %957 = load <4 x float>, <4 x float>* %956, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %957, <4 x float> addrspace(1)* %953, align 16, !noalias !34
  %958 = add nsw i32 %951, %96
  %959 = sext i32 %958 to i64
  %960 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %959, i32 0
  %961 = add nuw i64 %927, 432
  %962 = getelementptr inbounds i8, i8* %48, i64 %961
  %963 = bitcast i8* %962 to <4 x float>*
  %964 = load <4 x float>, <4 x float>* %963, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %964, <4 x float> addrspace(1)* %960, align 16, !noalias !34
  %965 = add nsw i32 %958, %96
  %966 = sext i32 %965 to i64
  %967 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %966, i32 0
  %968 = add nuw i64 %927, 464
  %969 = getelementptr inbounds i8, i8* %48, i64 %968
  %970 = bitcast i8* %969 to <4 x float>*
  %971 = load <4 x float>, <4 x float>* %970, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %971, <4 x float> addrspace(1)* %967, align 16, !noalias !34
  %972 = add nsw i32 %965, %96
  %973 = sext i32 %972 to i64
  %974 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %973, i32 0
  %975 = add nuw i64 %927, 496
  %976 = getelementptr inbounds i8, i8* %48, i64 %975
  %977 = bitcast i8* %976 to <4 x float>*
  %978 = load <4 x float>, <4 x float>* %977, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %978, <4 x float> addrspace(1)* %974, align 16, !noalias !34
  %979 = add nsw i32 %972, %96
  %980 = sext i32 %979 to i64
  %981 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %980, i32 0
  %982 = add nuw i64 %927, 528
  %983 = getelementptr inbounds i8, i8* %48, i64 %982
  %984 = bitcast i8* %983 to <4 x float>*
  %985 = load <4 x float>, <4 x float>* %984, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %985, <4 x float> addrspace(1)* %981, align 16, !noalias !34
  %986 = add nsw i32 %979, %96
  %987 = sext i32 %986 to i64
  %988 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %987, i32 0
  %989 = add nuw i64 %927, 560
  %990 = getelementptr inbounds i8, i8* %48, i64 %989
  %991 = bitcast i8* %990 to <4 x float>*
  %992 = load <4 x float>, <4 x float>* %991, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %992, <4 x float> addrspace(1)* %988, align 16, !noalias !34
  %993 = add nsw i32 %986, %96
  %994 = sext i32 %993 to i64
  %995 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %994, i32 0
  %996 = add nuw i64 %927, 592
  %997 = getelementptr inbounds i8, i8* %48, i64 %996
  %998 = bitcast i8* %997 to <4 x float>*
  %999 = load <4 x float>, <4 x float>* %998, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %999, <4 x float> addrspace(1)* %995, align 16, !noalias !34
  %1000 = sext i32 %944 to i64
  %1001 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %1000, i32 0
  %1002 = add nuw i64 %927, 384
  %1003 = getelementptr inbounds i8, i8* %48, i64 %1002
  %1004 = bitcast i8* %1003 to <4 x float>*
  %1005 = load <4 x float>, <4 x float>* %1004, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %1005, <4 x float> addrspace(1)* %1001, align 16, !noalias !34
  %1006 = add nsw i32 %944, %96
  %1007 = sext i32 %1006 to i64
  %1008 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %1007, i32 0
  %1009 = add nuw i64 %927, 416
  %1010 = getelementptr inbounds i8, i8* %48, i64 %1009
  %1011 = bitcast i8* %1010 to <4 x float>*
  %1012 = load <4 x float>, <4 x float>* %1011, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %1012, <4 x float> addrspace(1)* %1008, align 16, !noalias !34
  %1013 = add nsw i32 %1006, %96
  %1014 = sext i32 %1013 to i64
  %1015 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %1014, i32 0
  %1016 = add nuw i64 %927, 448
  %1017 = getelementptr inbounds i8, i8* %48, i64 %1016
  %1018 = bitcast i8* %1017 to <4 x float>*
  %1019 = load <4 x float>, <4 x float>* %1018, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %1019, <4 x float> addrspace(1)* %1015, align 16, !noalias !34
  %1020 = add nsw i32 %1013, %96
  %1021 = sext i32 %1020 to i64
  %1022 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %1021, i32 0
  %1023 = add nuw i64 %927, 480
  %1024 = getelementptr inbounds i8, i8* %48, i64 %1023
  %1025 = bitcast i8* %1024 to <4 x float>*
  %1026 = load <4 x float>, <4 x float>* %1025, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %1026, <4 x float> addrspace(1)* %1022, align 16, !noalias !34
  %1027 = add nsw i32 %1020, %96
  %1028 = sext i32 %1027 to i64
  %1029 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %1028, i32 0
  %1030 = add nuw i64 %927, 512
  %1031 = getelementptr inbounds i8, i8* %48, i64 %1030
  %1032 = bitcast i8* %1031 to <4 x float>*
  %1033 = load <4 x float>, <4 x float>* %1032, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %1033, <4 x float> addrspace(1)* %1029, align 16, !noalias !34
  %1034 = add nsw i32 %1027, %96
  %1035 = sext i32 %1034 to i64
  %1036 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %1035, i32 0
  %1037 = add nuw i64 %927, 544
  %1038 = getelementptr inbounds i8, i8* %48, i64 %1037
  %1039 = bitcast i8* %1038 to <4 x float>*
  %1040 = load <4 x float>, <4 x float>* %1039, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %1040, <4 x float> addrspace(1)* %1036, align 16, !noalias !34
  %1041 = add nsw i32 %1034, %96
  %1042 = sext i32 %1041 to i64
  %1043 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %1042, i32 0
  %1044 = add nuw i64 %927, 576
  %1045 = getelementptr inbounds i8, i8* %48, i64 %1044
  %1046 = bitcast i8* %1045 to <4 x float>*
  %1047 = load <4 x float>, <4 x float>* %1046, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %1047, <4 x float> addrspace(1)* %1043, align 16, !noalias !34
  %1048 = add nsw i32 %1041, %96
  %1049 = sext i32 %1048 to i64
  %1050 = getelementptr inbounds %0, %0 addrspace(1)* %121, i64 %1049, i32 0
  %1051 = add nuw i64 %927, 608
  %1052 = getelementptr inbounds i8, i8* %48, i64 %1051
  %1053 = bitcast i8* %1052 to <4 x float>*
  %1054 = load <4 x float>, <4 x float>* %1053, align 16, !alias.scope !35, !noalias !36
  store <4 x float> %1054, <4 x float> addrspace(1)* %1050, align 16, !noalias !34
  %1055 = add nuw i64 %926, 1
  %1056 = icmp ult i64 %1055, %70
  br i1 %1056, label %1060, label %1057

1057:                                             ; preds = %934
  %1058 = add nuw i64 %925, 1
  %1059 = icmp ult i64 %1058, %80
  br i1 %1059, label %1060, label %1064

1060:                                             ; preds = %1057, %934
  %1061 = phi i64 [ %1058, %1057 ], [ %925, %934 ]
  %1062 = phi i64 [ 0, %1057 ], [ %1055, %934 ]
  %1063 = add nuw i64 %927, 640
  br label %924

1064:                                             ; preds = %1057
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %49)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %50)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %51)
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %52)
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
attributes #1 = { alwaysinline convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #2 = { nounwind }

!spirv.MemoryModel = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!spirv.Generator = !{!5}
!opencl.stats.InstCounter.CanVect = !{!6}
!opencl.kernels = !{!7}
!opencl.stat.type = !{!8}
!opencl.stat.exec_time = !{!9}
!opencl.stat.run_time_version = !{!10}
!opencl.stat.workload_name = !{!11}
!opencl.stat.module_name = !{!12}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 2}
!3 = !{i32 1, i32 0}
!4 = !{}
!5 = !{i16 6, i16 14}
!6 = !{!"Code is vectorizable"}
!7 = !{void (%0 addrspace(1)*, %1*, %1*, %1*, %0 addrspace(1)*, %1*, %1*, %1*, %0 addrspace(1)*, %1*, %1*, %1*, %0 addrspace(3)*, %1*, %1*, %1*, float, float, i64, i64, i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @___ZTS28Kernel_L3_SLM_8x8_4x16_vec_1_separated_args}
!8 = !{!""}
!9 = !{!"2021-02-07 16:44:13"}
!10 = !{!"2021.11.2.0"}
!11 = !{!"sgemm-intel.run"}
!12 = !{!"sgemm-intel.run1"}
!13 = !{i32 1, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0, i32 3, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!14 = !{!"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!15 = !{!"class._ZTSN2cl4sycl3vecIfLi4EEE.cl::sycl::vec*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl3vecIfLi4EEE.cl::sycl::vec*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl3vecIfLi4EEE.cl::sycl::vec*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl3vecIfLi4EEE.cl::sycl::vec*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"float", !"float", !"long", !"long", !"int"}
!16 = !{!"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !"", !""}
!17 = !{i1 false}
!18 = !{i32 2}
!19 = !{i32 0}
!20 = !{i32 640}
!21 = !{void (i8*, i64*, {}*)* @_ZTS28Kernel_L3_SLM_8x8_4x16_vec_1}
!22 = !{i32 26159}
!23 = !{i1 true}
!24 = !{i32 1}
!25 = !{!26}
!26 = distinct !{!26, !27, !"___ZTS28Kernel_L3_SLM_8x8_4x16_vec_1_separated_args: %pWorkDim"}
!27 = distinct !{!27, !"___ZTS28Kernel_L3_SLM_8x8_4x16_vec_1_separated_args"}
!28 = !{!29, !30, !31}
!29 = distinct !{!29, !27, !"___ZTS28Kernel_L3_SLM_8x8_4x16_vec_1_separated_args: %_arg_14"}
!30 = distinct !{!30, !27, !"___ZTS28Kernel_L3_SLM_8x8_4x16_vec_1_separated_args: %pWGId"}
!31 = distinct !{!31, !27, !"___ZTS28Kernel_L3_SLM_8x8_4x16_vec_1_separated_args: %pSpecialBuf"}
!32 = !{!30}
!33 = !{!29, !26, !31}
!34 = !{!29, !26, !30, !31}
!35 = !{!31}
!36 = !{!29, !26, !30}
!37 = !{!26, !30}
!38 = !{!29}
!39 = !{!26, !30, !31}
