; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlPhong.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_PHONG_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_PHONG_parameters = appending global [344 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint const, unsigned char __attribute__((address_space(1))) *, uint const, uint const\00", section "llvm.metadata" ; <[344 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, i8 addrspace(1)*, i32, i32)* @PHONG to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_PHONG_locals to i8*), i8* getelementptr inbounds ([344 x i8]* @opencl_PHONG_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @PHONG(float addrspace(1)* %output, float addrspace(1)* %norm_org, float addrspace(1)* %ldir_org, float addrspace(1)* %v_pnt_org, float addrspace(1)* %ambient, float addrspace(1)* %diffuse, i32 %m_num_total_f, i8 addrspace(1)* %outRgb, i32 %m_phongThread, i32 %m_phongConst) nounwind {
entry:
  %output.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=4]
  %norm_org.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=4]
  %ldir_org.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=4]
  %v_pnt_org.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=4]
  %ambient.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=4]
  %diffuse.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=4]
  %m_num_total_f.addr = alloca i32, align 4       ; <i32*> [#uses=2]
  %outRgb.addr = alloca i8 addrspace(1)*, align 4 ; <i8 addrspace(1)**> [#uses=4]
  %m_phongThread.addr = alloca i32, align 4       ; <i32*> [#uses=2]
  %m_phongConst.addr = alloca i32, align 4        ; <i32*> [#uses=2]
  %globalIdx = alloca i32, align 4                ; <i32*> [#uses=2]
  %groupIdx = alloca i32, align 4                 ; <i32*> [#uses=1]
  %localIdx = alloca i32, align 4                 ; <i32*> [#uses=1]
  %localSize = alloca i32, align 4                ; <i32*> [#uses=1]
  %num_pnt = alloca i32, align 4                  ; <i32*> [#uses=2]
  %chuckSize = alloca i32, align 4                ; <i32*> [#uses=3]
  %_ldir = alloca [3 x float], align 4            ; <[3 x float]*> [#uses=6]
  %_v_pnt = alloca [3 x float], align 4           ; <[3 x float]*> [#uses=6]
  %specular = alloca float, align 4               ; <float*> [#uses=2]
  %phongconst = alloca i64, align 8               ; <i64*> [#uses=2]
  %_ldir_v4 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %_v_pnt_v4 = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=2]
  %.compoundliteral56 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %_norm_v4 = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %ambient_v4 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=2]
  %.compoundliteral73 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %diffuse_v4 = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=2]
  %.compoundliteral89 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %f255_v4 = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=3]
  %offset = alloca i32, align 4                   ; <i32*> [#uses=5]
  %indexPosition = alloca i32, align 4            ; <i32*> [#uses=12]
  %.compoundliteral115 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=4]
  %j = alloca i32, align 4                        ; <i32*> [#uses=0]
  %costheta = alloca float, align 4               ; <float*> [#uses=2]
  %cosalpha = alloca float, align 4               ; <float*> [#uses=3]
  %specularN = alloca float, align 4              ; <float*> [#uses=4]
  %costheta_v4 = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=5]
  %dt_v4 = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %R_v4 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  %intensity_v4 = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=7]
  %specularN_v4 = alloca <4 x float>, align 16    ; <<4 x float>*> [#uses=2]
  %tmp_v4 = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=4]
  store float addrspace(1)* %output, float addrspace(1)** %output.addr
  store float addrspace(1)* %norm_org, float addrspace(1)** %norm_org.addr
  store float addrspace(1)* %ldir_org, float addrspace(1)** %ldir_org.addr
  store float addrspace(1)* %v_pnt_org, float addrspace(1)** %v_pnt_org.addr
  store float addrspace(1)* %ambient, float addrspace(1)** %ambient.addr
  store float addrspace(1)* %diffuse, float addrspace(1)** %diffuse.addr
  store i32 %m_num_total_f, i32* %m_num_total_f.addr
  store i8 addrspace(1)* %outRgb, i8 addrspace(1)** %outRgb.addr
  store i32 %m_phongThread, i32* %m_phongThread.addr
  store i32 %m_phongConst, i32* %m_phongConst.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %globalIdx
  %call1 = call i32 @get_group_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call1, i32* %groupIdx
  %call2 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call2, i32* %localIdx
  %call3 = call i32 @get_local_size(i32 0)        ; <i32> [#uses=1]
  store i32 %call3, i32* %localSize
  %tmp = load i32* %m_num_total_f.addr            ; <i32> [#uses=1]
  %div = udiv i32 %tmp, 3                         ; <i32> [#uses=1]
  store i32 %div, i32* %num_pnt
  %tmp5 = load i32* %num_pnt                      ; <i32> [#uses=1]
  %tmp6 = load i32* %m_phongThread.addr           ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp6                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp6         ; <i32> [#uses=1]
  %div7 = udiv i32 %tmp5, %sel                    ; <i32> [#uses=1]
  store i32 %div7, i32* %chuckSize
  %tmp9 = load float addrspace(1)** %ldir_org.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp9, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp10 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  %arraydecay = getelementptr inbounds [3 x float]* %_ldir, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx11 = getelementptr inbounds float* %arraydecay, i32 0 ; <float*> [#uses=1]
  store float %tmp10, float* %arrayidx11
  %tmp12 = load float addrspace(1)** %ldir_org.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx13 = getelementptr inbounds float addrspace(1)* %tmp12, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp14 = load float addrspace(1)* %arrayidx13   ; <float> [#uses=1]
  %arraydecay15 = getelementptr inbounds [3 x float]* %_ldir, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx16 = getelementptr inbounds float* %arraydecay15, i32 1 ; <float*> [#uses=1]
  store float %tmp14, float* %arrayidx16
  %tmp17 = load float addrspace(1)** %ldir_org.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx18 = getelementptr inbounds float addrspace(1)* %tmp17, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp19 = load float addrspace(1)* %arrayidx18   ; <float> [#uses=1]
  %arraydecay20 = getelementptr inbounds [3 x float]* %_ldir, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx21 = getelementptr inbounds float* %arraydecay20, i32 2 ; <float*> [#uses=1]
  store float %tmp19, float* %arrayidx21
  %tmp23 = load float addrspace(1)** %v_pnt_org.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx24 = getelementptr inbounds float addrspace(1)* %tmp23, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp25 = load float addrspace(1)* %arrayidx24   ; <float> [#uses=1]
  %arraydecay26 = getelementptr inbounds [3 x float]* %_v_pnt, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx27 = getelementptr inbounds float* %arraydecay26, i32 0 ; <float*> [#uses=1]
  store float %tmp25, float* %arrayidx27
  %tmp28 = load float addrspace(1)** %v_pnt_org.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx29 = getelementptr inbounds float addrspace(1)* %tmp28, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp30 = load float addrspace(1)* %arrayidx29   ; <float> [#uses=1]
  %arraydecay31 = getelementptr inbounds [3 x float]* %_v_pnt, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx32 = getelementptr inbounds float* %arraydecay31, i32 1 ; <float*> [#uses=1]
  store float %tmp30, float* %arrayidx32
  %tmp33 = load float addrspace(1)** %v_pnt_org.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds float addrspace(1)* %tmp33, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp35 = load float addrspace(1)* %arrayidx34   ; <float> [#uses=1]
  %arraydecay36 = getelementptr inbounds [3 x float]* %_v_pnt, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx37 = getelementptr inbounds float* %arraydecay36, i32 2 ; <float*> [#uses=1]
  store float %tmp35, float* %arrayidx37
  store float 5.000000e-001, float* %specular
  %tmp40 = load i32* %m_phongConst.addr           ; <i32> [#uses=1]
  %conv = zext i32 %tmp40 to i64                  ; <i64> [#uses=1]
  store i64 %conv, i64* %phongconst
  %arraydecay42 = getelementptr inbounds [3 x float]* %_ldir, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx43 = getelementptr inbounds float* %arraydecay42, i32 0 ; <float*> [#uses=1]
  %tmp44 = load float* %arrayidx43                ; <float> [#uses=1]
  %vecinit = insertelement <4 x float> undef, float %tmp44, i32 0 ; <<4 x float>> [#uses=1]
  %arraydecay45 = getelementptr inbounds [3 x float]* %_ldir, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx46 = getelementptr inbounds float* %arraydecay45, i32 1 ; <float*> [#uses=1]
  %tmp47 = load float* %arrayidx46                ; <float> [#uses=1]
  %vecinit48 = insertelement <4 x float> %vecinit, float %tmp47, i32 1 ; <<4 x float>> [#uses=1]
  %arraydecay49 = getelementptr inbounds [3 x float]* %_ldir, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx50 = getelementptr inbounds float* %arraydecay49, i32 2 ; <float*> [#uses=1]
  %tmp51 = load float* %arrayidx50                ; <float> [#uses=1]
  %vecinit52 = insertelement <4 x float> %vecinit48, float %tmp51, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit53 = insertelement <4 x float> %vecinit52, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit53, <4 x float>* %.compoundliteral
  %tmp54 = load <4 x float>* %.compoundliteral    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp54, <4 x float>* %_ldir_v4
  %arraydecay57 = getelementptr inbounds [3 x float]* %_v_pnt, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx58 = getelementptr inbounds float* %arraydecay57, i32 0 ; <float*> [#uses=1]
  %tmp59 = load float* %arrayidx58                ; <float> [#uses=1]
  %vecinit60 = insertelement <4 x float> undef, float %tmp59, i32 0 ; <<4 x float>> [#uses=1]
  %arraydecay61 = getelementptr inbounds [3 x float]* %_v_pnt, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx62 = getelementptr inbounds float* %arraydecay61, i32 1 ; <float*> [#uses=1]
  %tmp63 = load float* %arrayidx62                ; <float> [#uses=1]
  %vecinit64 = insertelement <4 x float> %vecinit60, float %tmp63, i32 1 ; <<4 x float>> [#uses=1]
  %arraydecay65 = getelementptr inbounds [3 x float]* %_v_pnt, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx66 = getelementptr inbounds float* %arraydecay65, i32 2 ; <float*> [#uses=1]
  %tmp67 = load float* %arrayidx66                ; <float> [#uses=1]
  %vecinit68 = insertelement <4 x float> %vecinit64, float %tmp67, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit69 = insertelement <4 x float> %vecinit68, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit69, <4 x float>* %.compoundliteral56
  %tmp70 = load <4 x float>* %.compoundliteral56  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp70, <4 x float>* %_v_pnt_v4
  %tmp74 = load float addrspace(1)** %ambient.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx75 = getelementptr inbounds float addrspace(1)* %tmp74, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp76 = load float addrspace(1)* %arrayidx75   ; <float> [#uses=1]
  %vecinit77 = insertelement <4 x float> undef, float %tmp76, i32 0 ; <<4 x float>> [#uses=1]
  %tmp78 = load float addrspace(1)** %ambient.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx79 = getelementptr inbounds float addrspace(1)* %tmp78, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp80 = load float addrspace(1)* %arrayidx79   ; <float> [#uses=1]
  %vecinit81 = insertelement <4 x float> %vecinit77, float %tmp80, i32 1 ; <<4 x float>> [#uses=1]
  %tmp82 = load float addrspace(1)** %ambient.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx83 = getelementptr inbounds float addrspace(1)* %tmp82, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp84 = load float addrspace(1)* %arrayidx83   ; <float> [#uses=1]
  %vecinit85 = insertelement <4 x float> %vecinit81, float %tmp84, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit86 = insertelement <4 x float> %vecinit85, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit86, <4 x float>* %.compoundliteral73
  %tmp87 = load <4 x float>* %.compoundliteral73  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp87, <4 x float>* %ambient_v4
  %tmp90 = load float addrspace(1)** %diffuse.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx91 = getelementptr inbounds float addrspace(1)* %tmp90, i32 0 ; <float addrspace(1)*> [#uses=1]
  %tmp92 = load float addrspace(1)* %arrayidx91   ; <float> [#uses=1]
  %vecinit93 = insertelement <4 x float> undef, float %tmp92, i32 0 ; <<4 x float>> [#uses=1]
  %tmp94 = load float addrspace(1)** %diffuse.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx95 = getelementptr inbounds float addrspace(1)* %tmp94, i32 1 ; <float addrspace(1)*> [#uses=1]
  %tmp96 = load float addrspace(1)* %arrayidx95   ; <float> [#uses=1]
  %vecinit97 = insertelement <4 x float> %vecinit93, float %tmp96, i32 1 ; <<4 x float>> [#uses=1]
  %tmp98 = load float addrspace(1)** %diffuse.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx99 = getelementptr inbounds float addrspace(1)* %tmp98, i32 2 ; <float addrspace(1)*> [#uses=1]
  %tmp100 = load float addrspace(1)* %arrayidx99  ; <float> [#uses=1]
  %vecinit101 = insertelement <4 x float> %vecinit97, float %tmp100, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit102 = insertelement <4 x float> %vecinit101, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit102, <4 x float>* %.compoundliteral89
  %tmp103 = load <4 x float>* %.compoundliteral89 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp103, <4 x float>* %diffuse_v4
  store <4 x float> <float 2.550000e+002, float 2.550000e+002, float 2.550000e+002, float 2.550000e+002>, <4 x float>* %f255_v4
  store i32 0, i32* %offset
  br label %for.cond

for.cond:                                         ; preds = %for.inc242, %entry
  %tmp106 = load i32* %offset                     ; <i32> [#uses=1]
  %tmp107 = load i32* %chuckSize                  ; <i32> [#uses=1]
  %cmp108 = icmp slt i32 %tmp106, %tmp107         ; <i1> [#uses=1]
  br i1 %cmp108, label %for.body, label %for.end245

for.body:                                         ; preds = %for.cond
  %tmp111 = load i32* %chuckSize                  ; <i32> [#uses=1]
  %tmp112 = load i32* %globalIdx                  ; <i32> [#uses=1]
  %mul = mul i32 %tmp111, %tmp112                 ; <i32> [#uses=1]
  %tmp113 = load i32* %offset                     ; <i32> [#uses=1]
  %add = add i32 %mul, %tmp113                    ; <i32> [#uses=1]
  %mul114 = mul i32 %add, 3                       ; <i32> [#uses=1]
  store i32 %mul114, i32* %indexPosition
  %tmp116 = load i32* %indexPosition              ; <i32> [#uses=1]
  %tmp117 = load float addrspace(1)** %norm_org.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx118 = getelementptr inbounds float addrspace(1)* %tmp117, i32 %tmp116 ; <float addrspace(1)*> [#uses=1]
  %tmp119 = load float addrspace(1)* %arrayidx118 ; <float> [#uses=1]
  %vecinit120 = insertelement <4 x float> undef, float %tmp119, i32 0 ; <<4 x float>> [#uses=1]
  %tmp121 = load i32* %indexPosition              ; <i32> [#uses=1]
  %add122 = add i32 %tmp121, 1                    ; <i32> [#uses=1]
  %tmp123 = load float addrspace(1)** %norm_org.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx124 = getelementptr inbounds float addrspace(1)* %tmp123, i32 %add122 ; <float addrspace(1)*> [#uses=1]
  %tmp125 = load float addrspace(1)* %arrayidx124 ; <float> [#uses=1]
  %vecinit126 = insertelement <4 x float> %vecinit120, float %tmp125, i32 1 ; <<4 x float>> [#uses=1]
  %tmp127 = load i32* %indexPosition              ; <i32> [#uses=1]
  %add128 = add i32 %tmp127, 2                    ; <i32> [#uses=1]
  %tmp129 = load float addrspace(1)** %norm_org.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx130 = getelementptr inbounds float addrspace(1)* %tmp129, i32 %add128 ; <float addrspace(1)*> [#uses=1]
  %tmp131 = load float addrspace(1)* %arrayidx130 ; <float> [#uses=1]
  %vecinit132 = insertelement <4 x float> %vecinit126, float %tmp131, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit133 = insertelement <4 x float> %vecinit132, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit133, <4 x float>* %.compoundliteral115
  %tmp134 = load <4 x float>* %.compoundliteral115 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp134, <4 x float>* %_norm_v4
  %tmp140 = load <4 x float>* %_norm_v4           ; <<4 x float>> [#uses=1]
  %tmp141 = load <4 x float>* %_ldir_v4           ; <<4 x float>> [#uses=1]
  %call142 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp140, <4 x float> %tmp141) ; <float> [#uses=1]
  store float %call142, float* %costheta
  %tmp144 = load float* %costheta                 ; <float> [#uses=1]
  %tmp145 = insertelement <4 x float> undef, float %tmp144, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp145, <4 x float> %tmp145, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat, <4 x float>* %costheta_v4
  %tmp147 = load <4 x float>* %costheta_v4        ; <<4 x float>> [#uses=1]
  %mul148 = fmul <4 x float> %tmp147, <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000> ; <<4 x float>> [#uses=1]
  store <4 x float> %mul148, <4 x float>* %dt_v4
  %tmp150 = load <4 x float>* %dt_v4              ; <<4 x float>> [#uses=1]
  %tmp151 = load <4 x float>* %_norm_v4           ; <<4 x float>> [#uses=1]
  %mul152 = fmul <4 x float> %tmp150, %tmp151     ; <<4 x float>> [#uses=1]
  %tmp153 = load <4 x float>* %_ldir_v4           ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %mul152, %tmp153        ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %R_v4
  %tmp154 = load <4 x float>* %costheta_v4        ; <<4 x float>> [#uses=1]
  %call155 = call <4 x float> @_Z4fabsU8__vector4f(<4 x float> %tmp154) ; <<4 x float>> [#uses=1]
  store <4 x float> %call155, <4 x float>* %costheta_v4
  %tmp157 = load <4 x float>* %ambient_v4         ; <<4 x float>> [#uses=1]
  %tmp158 = load <4 x float>* %diffuse_v4         ; <<4 x float>> [#uses=1]
  %tmp159 = load <4 x float>* %costheta_v4        ; <<4 x float>> [#uses=1]
  %mul160 = fmul <4 x float> %tmp158, %tmp159     ; <<4 x float>> [#uses=1]
  %add161 = fadd <4 x float> %tmp157, %mul160     ; <<4 x float>> [#uses=1]
  store <4 x float> %add161, <4 x float>* %intensity_v4
  %tmp162 = load <4 x float>* %R_v4               ; <<4 x float>> [#uses=1]
  %tmp163 = load <4 x float>* %_v_pnt_v4          ; <<4 x float>> [#uses=1]
  %call164 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp162, <4 x float> %tmp163) ; <float> [#uses=1]
  store float %call164, float* %cosalpha
  %tmp165 = load float* %cosalpha                 ; <float> [#uses=1]
  %conv166 = fpext float %tmp165 to double        ; <double> [#uses=1]
  %cmp167 = fcmp ogt double %conv166, 0.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp167, label %if.then, label %if.end197

if.then:                                          ; preds = %for.body
  %tmp169 = load i32* %indexPosition              ; <i32> [#uses=1]
  %cmp170 = icmp eq i32 %tmp169, 3996             ; <i1> [#uses=1]
  br i1 %cmp170, label %if.then172, label %if.end

if.then172:                                       ; preds = %if.then
  br label %if.end

if.end:                                           ; preds = %if.then172, %if.then
  %tmp173 = load float* %specular                 ; <float> [#uses=1]
  store float %tmp173, float* %specularN
  store i32 0, i32* %i
  br label %for.cond174

for.cond174:                                      ; preds = %for.inc, %if.end
  %tmp175 = load i32* %i                          ; <i32> [#uses=1]
  %conv176 = sext i32 %tmp175 to i64              ; <i64> [#uses=1]
  %tmp177 = load i64* %phongconst                 ; <i64> [#uses=1]
  %cmp178 = icmp slt i64 %conv176, %tmp177        ; <i1> [#uses=1]
  br i1 %cmp178, label %for.body180, label %for.end

for.body180:                                      ; preds = %for.cond174
  %tmp181 = load float* %cosalpha                 ; <float> [#uses=1]
  %tmp182 = load float* %specularN                ; <float> [#uses=1]
  %mul183 = fmul float %tmp182, %tmp181           ; <float> [#uses=1]
  store float %mul183, float* %specularN
  br label %for.inc

for.inc:                                          ; preds = %for.body180
  %tmp184 = load i32* %i                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp184, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond174

for.end:                                          ; preds = %for.cond174
  %tmp185 = load i32* %indexPosition              ; <i32> [#uses=1]
  %cmp186 = icmp eq i32 %tmp185, 3996             ; <i1> [#uses=1]
  br i1 %cmp186, label %if.then188, label %if.end189

if.then188:                                       ; preds = %for.end
  br label %if.end189

if.end189:                                        ; preds = %if.then188, %for.end
  %tmp191 = load float* %specularN                ; <float> [#uses=1]
  %tmp192 = insertelement <4 x float> undef, float %tmp191, i32 0 ; <<4 x float>> [#uses=2]
  %splat193 = shufflevector <4 x float> %tmp192, <4 x float> %tmp192, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat193, <4 x float>* %specularN_v4
  %tmp194 = load <4 x float>* %specularN_v4       ; <<4 x float>> [#uses=1]
  %tmp195 = load <4 x float>* %intensity_v4       ; <<4 x float>> [#uses=1]
  %add196 = fadd <4 x float> %tmp195, %tmp194     ; <<4 x float>> [#uses=1]
  store <4 x float> %add196, <4 x float>* %intensity_v4
  br label %if.end197

if.end197:                                        ; preds = %if.end189, %for.body
  %tmp198 = load <4 x float>* %intensity_v4       ; <<4 x float>> [#uses=1]
  %tmp199 = extractelement <4 x float> %tmp198, i32 0 ; <float> [#uses=1]
  %tmp200 = load i32* %indexPosition              ; <i32> [#uses=1]
  %tmp201 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx202 = getelementptr inbounds float addrspace(1)* %tmp201, i32 %tmp200 ; <float addrspace(1)*> [#uses=1]
  store float %tmp199, float addrspace(1)* %arrayidx202
  %tmp203 = load <4 x float>* %intensity_v4       ; <<4 x float>> [#uses=1]
  %tmp204 = extractelement <4 x float> %tmp203, i32 1 ; <float> [#uses=1]
  %tmp205 = load i32* %indexPosition              ; <i32> [#uses=1]
  %add206 = add i32 %tmp205, 1                    ; <i32> [#uses=1]
  %tmp207 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx208 = getelementptr inbounds float addrspace(1)* %tmp207, i32 %add206 ; <float addrspace(1)*> [#uses=1]
  store float %tmp204, float addrspace(1)* %arrayidx208
  %tmp209 = load <4 x float>* %intensity_v4       ; <<4 x float>> [#uses=1]
  %tmp210 = extractelement <4 x float> %tmp209, i32 2 ; <float> [#uses=1]
  %tmp211 = load i32* %indexPosition              ; <i32> [#uses=1]
  %add212 = add i32 %tmp211, 2                    ; <i32> [#uses=1]
  %tmp213 = load float addrspace(1)** %output.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx214 = getelementptr inbounds float addrspace(1)* %tmp213, i32 %add212 ; <float addrspace(1)*> [#uses=1]
  store float %tmp210, float addrspace(1)* %arrayidx214
  %tmp216 = load <4 x float>* %f255_v4            ; <<4 x float>> [#uses=1]
  %tmp217 = load <4 x float>* %intensity_v4       ; <<4 x float>> [#uses=1]
  %tmp218 = load <4 x float>* %f255_v4            ; <<4 x float>> [#uses=1]
  %mul219 = fmul <4 x float> %tmp217, %tmp218     ; <<4 x float>> [#uses=1]
  %add220 = fadd <4 x float> %mul219, <float 5.000000e-001, float 5.000000e-001, float 5.000000e-001, float 5.000000e-001> ; <<4 x float>> [#uses=1]
  %call221 = call <4 x float> @_Z3minU8__vector4fS_(<4 x float> %tmp216, <4 x float> %add220) ; <<4 x float>> [#uses=1]
  store <4 x float> %call221, <4 x float>* %tmp_v4
  %tmp222 = load <4 x float>* %tmp_v4             ; <<4 x float>> [#uses=1]
  %tmp223 = extractelement <4 x float> %tmp222, i32 0 ; <float> [#uses=1]
  %conv224 = fptoui float %tmp223 to i8           ; <i8> [#uses=1]
  %tmp225 = load i32* %indexPosition              ; <i32> [#uses=1]
  %tmp226 = load i8 addrspace(1)** %outRgb.addr   ; <i8 addrspace(1)*> [#uses=1]
  %arrayidx227 = getelementptr inbounds i8 addrspace(1)* %tmp226, i32 %tmp225 ; <i8 addrspace(1)*> [#uses=1]
  store i8 %conv224, i8 addrspace(1)* %arrayidx227
  %tmp228 = load <4 x float>* %tmp_v4             ; <<4 x float>> [#uses=1]
  %tmp229 = extractelement <4 x float> %tmp228, i32 1 ; <float> [#uses=1]
  %conv230 = fptoui float %tmp229 to i8           ; <i8> [#uses=1]
  %tmp231 = load i32* %indexPosition              ; <i32> [#uses=1]
  %add232 = add i32 %tmp231, 1                    ; <i32> [#uses=1]
  %tmp233 = load i8 addrspace(1)** %outRgb.addr   ; <i8 addrspace(1)*> [#uses=1]
  %arrayidx234 = getelementptr inbounds i8 addrspace(1)* %tmp233, i32 %add232 ; <i8 addrspace(1)*> [#uses=1]
  store i8 %conv230, i8 addrspace(1)* %arrayidx234
  %tmp235 = load <4 x float>* %tmp_v4             ; <<4 x float>> [#uses=1]
  %tmp236 = extractelement <4 x float> %tmp235, i32 2 ; <float> [#uses=1]
  %conv237 = fptoui float %tmp236 to i8           ; <i8> [#uses=1]
  %tmp238 = load i32* %indexPosition              ; <i32> [#uses=1]
  %add239 = add i32 %tmp238, 2                    ; <i32> [#uses=1]
  %tmp240 = load i8 addrspace(1)** %outRgb.addr   ; <i8 addrspace(1)*> [#uses=1]
  %arrayidx241 = getelementptr inbounds i8 addrspace(1)* %tmp240, i32 %add239 ; <i8 addrspace(1)*> [#uses=1]
  store i8 %conv237, i8 addrspace(1)* %arrayidx241
  br label %for.inc242

for.inc242:                                       ; preds = %if.end197
  %tmp243 = load i32* %offset                     ; <i32> [#uses=1]
  %inc244 = add nsw i32 %tmp243, 1                ; <i32> [#uses=1]
  store i32 %inc244, i32* %offset
  br label %for.cond

for.end245:                                       ; preds = %for.cond
  call void @barrier(i32 1)
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_local_id(i32)

declare i32 @get_local_size(i32)

declare float @_Z3dotU8__vector4fS_(<4 x float>, <4 x float>)

declare <4 x float> @_Z4fabsU8__vector4f(<4 x float>)

declare <4 x float> @_Z3minU8__vector4fS_(<4 x float>, <4 x float>)

declare void @barrier(i32)
