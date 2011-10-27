
; ModuleID = 'dx2llvm'


define linkonce [4 x <4 x float>] @dx_soa_load_input_4_float4(<4 x i1>, i8*, <4 x i32>, <4 x i32>) {
load_store_entry:
  %4 = getelementptr i8* %1, i32 0                ; <i8*> [#uses=1]
  %5 = bitcast i8* %4 to [1 x [32 x <4 x float>]*]* ; <[1 x [32 x <4 x float>]*]*> [#uses=4]
  %alloca = alloca <4 x float>, i32 4, align 16   ; <<4 x float>*> [#uses=17]
  %6 = extractelement <4 x i1> %0, i32 0          ; <i1> [#uses=1]
  %7 = icmp eq i1 %6, false                       ; <i1> [#uses=1]
  br i1 %7, label %nexblock, label %instbb

instbb:                                           ; preds = %load_store_entry
  %8 = extractelement <4 x i32> %2, i32 0         ; <i32> [#uses=1]
  %9 = getelementptr [1 x [32 x <4 x float>]*]* %5, i32 0, i32 %8 ; <[32 x <4 x float>]**> [#uses=1]
  %10 = load [32 x <4 x float>]** %9              ; <[32 x <4 x float>]*> [#uses=1]
  %11 = bitcast [32 x <4 x float>]* %10 to float* ; <float*> [#uses=1]
  %12 = extractelement <4 x i32> %3, i32 0        ; <i32> [#uses=1]
  %13 = mul i32 %12, 16                           ; <i32> [#uses=1]
  %14 = getelementptr float* %11, i32 %13         ; <float*> [#uses=4]
  %15 = getelementptr float* %14, i32 0           ; <float*> [#uses=1]
  %16 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar = load float* %15                   ; <float> [#uses=1]
  %17 = load <4 x float>* %16                     ; <<4 x float>> [#uses=1]
  %18 = insertelement <4 x float> %17, float %loadscalar, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %18, <4 x float>* %16
  %19 = getelementptr float* %14, i32 4           ; <float*> [#uses=1]
  %20 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar1 = load float* %19                  ; <float> [#uses=1]
  %21 = load <4 x float>* %20                     ; <<4 x float>> [#uses=1]
  %22 = insertelement <4 x float> %21, float %loadscalar1, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %22, <4 x float>* %20
  %23 = getelementptr float* %14, i32 8           ; <float*> [#uses=1]
  %24 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar2 = load float* %23                  ; <float> [#uses=1]
  %25 = load <4 x float>* %24                     ; <<4 x float>> [#uses=1]
  %26 = insertelement <4 x float> %25, float %loadscalar2, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %26, <4 x float>* %24
  %27 = getelementptr float* %14, i32 12          ; <float*> [#uses=1]
  %28 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar3 = load float* %27                  ; <float> [#uses=1]
  %29 = load <4 x float>* %28                     ; <<4 x float>> [#uses=1]
  %30 = insertelement <4 x float> %29, float %loadscalar3, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %30, <4 x float>* %28
  br label %nexblock

nexblock:                                         ; preds = %instbb, %load_store_entry
  %31 = extractelement <4 x i1> %0, i32 1         ; <i1> [#uses=1]
  %32 = icmp eq i1 %31, false                     ; <i1> [#uses=1]
  br i1 %32, label %nexblock5, label %instbb4

instbb4:                                          ; preds = %nexblock
  %33 = extractelement <4 x i32> %2, i32 1        ; <i32> [#uses=1]
  %34 = getelementptr [1 x [32 x <4 x float>]*]* %5, i32 0, i32 %33 ; <[32 x <4 x float>]**> [#uses=1]
  %35 = load [32 x <4 x float>]** %34             ; <[32 x <4 x float>]*> [#uses=1]
  %36 = bitcast [32 x <4 x float>]* %35 to float* ; <float*> [#uses=1]
  %37 = extractelement <4 x i32> %3, i32 1        ; <i32> [#uses=1]
  %38 = mul i32 %37, 16                           ; <i32> [#uses=1]
  %39 = getelementptr float* %36, i32 %38         ; <float*> [#uses=4]
  %40 = getelementptr float* %39, i32 1           ; <float*> [#uses=1]
  %41 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar6 = load float* %40                  ; <float> [#uses=1]
  %42 = load <4 x float>* %41                     ; <<4 x float>> [#uses=1]
  %43 = insertelement <4 x float> %42, float %loadscalar6, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %43, <4 x float>* %41
  %44 = getelementptr float* %39, i32 5           ; <float*> [#uses=1]
  %45 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar7 = load float* %44                  ; <float> [#uses=1]
  %46 = load <4 x float>* %45                     ; <<4 x float>> [#uses=1]
  %47 = insertelement <4 x float> %46, float %loadscalar7, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %47, <4 x float>* %45
  %48 = getelementptr float* %39, i32 9           ; <float*> [#uses=1]
  %49 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar8 = load float* %48                  ; <float> [#uses=1]
  %50 = load <4 x float>* %49                     ; <<4 x float>> [#uses=1]
  %51 = insertelement <4 x float> %50, float %loadscalar8, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %51, <4 x float>* %49
  %52 = getelementptr float* %39, i32 13          ; <float*> [#uses=1]
  %53 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar9 = load float* %52                  ; <float> [#uses=1]
  %54 = load <4 x float>* %53                     ; <<4 x float>> [#uses=1]
  %55 = insertelement <4 x float> %54, float %loadscalar9, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %55, <4 x float>* %53
  br label %nexblock5

nexblock5:                                        ; preds = %instbb4, %nexblock
  %56 = extractelement <4 x i1> %0, i32 2         ; <i1> [#uses=1]
  %57 = icmp eq i1 %56, false                     ; <i1> [#uses=1]
  br i1 %57, label %nexblock11, label %instbb10

instbb10:                                         ; preds = %nexblock5
  %58 = extractelement <4 x i32> %2, i32 2        ; <i32> [#uses=1]
  %59 = getelementptr [1 x [32 x <4 x float>]*]* %5, i32 0, i32 %58 ; <[32 x <4 x float>]**> [#uses=1]
  %60 = load [32 x <4 x float>]** %59             ; <[32 x <4 x float>]*> [#uses=1]
  %61 = bitcast [32 x <4 x float>]* %60 to float* ; <float*> [#uses=1]
  %62 = extractelement <4 x i32> %3, i32 2        ; <i32> [#uses=1]
  %63 = mul i32 %62, 16                           ; <i32> [#uses=1]
  %64 = getelementptr float* %61, i32 %63         ; <float*> [#uses=4]
  %65 = getelementptr float* %64, i32 2           ; <float*> [#uses=1]
  %66 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar12 = load float* %65                 ; <float> [#uses=1]
  %67 = load <4 x float>* %66                     ; <<4 x float>> [#uses=1]
  %68 = insertelement <4 x float> %67, float %loadscalar12, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %68, <4 x float>* %66
  %69 = getelementptr float* %64, i32 6           ; <float*> [#uses=1]
  %70 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar13 = load float* %69                 ; <float> [#uses=1]
  %71 = load <4 x float>* %70                     ; <<4 x float>> [#uses=1]
  %72 = insertelement <4 x float> %71, float %loadscalar13, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %72, <4 x float>* %70
  %73 = getelementptr float* %64, i32 10          ; <float*> [#uses=1]
  %74 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar14 = load float* %73                 ; <float> [#uses=1]
  %75 = load <4 x float>* %74                     ; <<4 x float>> [#uses=1]
  %76 = insertelement <4 x float> %75, float %loadscalar14, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %76, <4 x float>* %74
  %77 = getelementptr float* %64, i32 14          ; <float*> [#uses=1]
  %78 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar15 = load float* %77                 ; <float> [#uses=1]
  %79 = load <4 x float>* %78                     ; <<4 x float>> [#uses=1]
  %80 = insertelement <4 x float> %79, float %loadscalar15, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %80, <4 x float>* %78
  br label %nexblock11

nexblock11:                                       ; preds = %instbb10, %nexblock5
  %81 = extractelement <4 x i1> %0, i32 3         ; <i1> [#uses=1]
  %82 = icmp eq i1 %81, false                     ; <i1> [#uses=1]
  br i1 %82, label %nexblock17, label %instbb16

instbb16:                                         ; preds = %nexblock11
  %83 = extractelement <4 x i32> %2, i32 3        ; <i32> [#uses=1]
  %84 = getelementptr [1 x [32 x <4 x float>]*]* %5, i32 0, i32 %83 ; <[32 x <4 x float>]**> [#uses=1]
  %85 = load [32 x <4 x float>]** %84             ; <[32 x <4 x float>]*> [#uses=1]
  %86 = bitcast [32 x <4 x float>]* %85 to float* ; <float*> [#uses=1]
  %87 = extractelement <4 x i32> %3, i32 3        ; <i32> [#uses=1]
  %88 = mul i32 %87, 16                           ; <i32> [#uses=1]
  %89 = getelementptr float* %86, i32 %88         ; <float*> [#uses=4]
  %90 = getelementptr float* %89, i32 3           ; <float*> [#uses=1]
  %91 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar18 = load float* %90                 ; <float> [#uses=1]
  %92 = load <4 x float>* %91                     ; <<4 x float>> [#uses=1]
  %93 = insertelement <4 x float> %92, float %loadscalar18, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %93, <4 x float>* %91
  %94 = getelementptr float* %89, i32 7           ; <float*> [#uses=1]
  %95 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar19 = load float* %94                 ; <float> [#uses=1]
  %96 = load <4 x float>* %95                     ; <<4 x float>> [#uses=1]
  %97 = insertelement <4 x float> %96, float %loadscalar19, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %97, <4 x float>* %95
  %98 = getelementptr float* %89, i32 11          ; <float*> [#uses=1]
  %99 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar20 = load float* %98                 ; <float> [#uses=1]
  %100 = load <4 x float>* %99                    ; <<4 x float>> [#uses=1]
  %101 = insertelement <4 x float> %100, float %loadscalar20, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %101, <4 x float>* %99
  %102 = getelementptr float* %89, i32 15         ; <float*> [#uses=1]
  %103 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar21 = load float* %102                ; <float> [#uses=1]
  %104 = load <4 x float>* %103                   ; <<4 x float>> [#uses=1]
  %105 = insertelement <4 x float> %104, float %loadscalar21, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %105, <4 x float>* %103
  br label %nexblock17

nexblock17:                                       ; preds = %instbb16, %nexblock11
  %106 = bitcast <4 x float>* %alloca to [4 x <4 x float>]* ; <[4 x <4 x float>]*> [#uses=1]
  %107 = load [4 x <4 x float>]* %106             ; <[4 x <4 x float>]> [#uses=1]
  ret [4 x <4 x float>] %107
}

define linkonce [4 x <4 x float>] @dx_soa_load_output_4_float4(<4 x i1>, i8*, <4 x i32>, <4 x i32>) {
load_store_entry:
  %4 = getelementptr i8* %1, i32 4                ; <i8*> [#uses=1]
  %5 = bitcast i8* %4 to [1 x [32 x <4 x float>]*]* ; <[1 x [32 x <4 x float>]*]*> [#uses=4]
  %alloca = alloca <4 x float>, i32 4, align 16   ; <<4 x float>*> [#uses=17]
  %6 = extractelement <4 x i1> %0, i32 0          ; <i1> [#uses=1]
  %7 = icmp eq i1 %6, false                       ; <i1> [#uses=1]
  br i1 %7, label %nexblock, label %instbb

instbb:                                           ; preds = %load_store_entry
  %8 = extractelement <4 x i32> %2, i32 0         ; <i32> [#uses=1]
  %9 = getelementptr [1 x [32 x <4 x float>]*]* %5, i32 0, i32 %8 ; <[32 x <4 x float>]**> [#uses=1]
  %10 = load [32 x <4 x float>]** %9              ; <[32 x <4 x float>]*> [#uses=1]
  %11 = bitcast [32 x <4 x float>]* %10 to float* ; <float*> [#uses=1]
  %12 = extractelement <4 x i32> %3, i32 0        ; <i32> [#uses=1]
  %13 = mul i32 %12, 16                           ; <i32> [#uses=1]
  %14 = getelementptr float* %11, i32 %13         ; <float*> [#uses=4]
  %15 = getelementptr float* %14, i32 0           ; <float*> [#uses=1]
  %16 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar = load float* %15                   ; <float> [#uses=1]
  %17 = load <4 x float>* %16                     ; <<4 x float>> [#uses=1]
  %18 = insertelement <4 x float> %17, float %loadscalar, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %18, <4 x float>* %16
  %19 = getelementptr float* %14, i32 4           ; <float*> [#uses=1]
  %20 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar1 = load float* %19                  ; <float> [#uses=1]
  %21 = load <4 x float>* %20                     ; <<4 x float>> [#uses=1]
  %22 = insertelement <4 x float> %21, float %loadscalar1, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %22, <4 x float>* %20
  %23 = getelementptr float* %14, i32 8           ; <float*> [#uses=1]
  %24 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar2 = load float* %23                  ; <float> [#uses=1]
  %25 = load <4 x float>* %24                     ; <<4 x float>> [#uses=1]
  %26 = insertelement <4 x float> %25, float %loadscalar2, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %26, <4 x float>* %24
  %27 = getelementptr float* %14, i32 12          ; <float*> [#uses=1]
  %28 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar3 = load float* %27                  ; <float> [#uses=1]
  %29 = load <4 x float>* %28                     ; <<4 x float>> [#uses=1]
  %30 = insertelement <4 x float> %29, float %loadscalar3, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %30, <4 x float>* %28
  br label %nexblock

nexblock:                                         ; preds = %instbb, %load_store_entry
  %31 = extractelement <4 x i1> %0, i32 1         ; <i1> [#uses=1]
  %32 = icmp eq i1 %31, false                     ; <i1> [#uses=1]
  br i1 %32, label %nexblock5, label %instbb4

instbb4:                                          ; preds = %nexblock
  %33 = extractelement <4 x i32> %2, i32 1        ; <i32> [#uses=1]
  %34 = getelementptr [1 x [32 x <4 x float>]*]* %5, i32 0, i32 %33 ; <[32 x <4 x float>]**> [#uses=1]
  %35 = load [32 x <4 x float>]** %34             ; <[32 x <4 x float>]*> [#uses=1]
  %36 = bitcast [32 x <4 x float>]* %35 to float* ; <float*> [#uses=1]
  %37 = extractelement <4 x i32> %3, i32 1        ; <i32> [#uses=1]
  %38 = mul i32 %37, 16                           ; <i32> [#uses=1]
  %39 = getelementptr float* %36, i32 %38         ; <float*> [#uses=4]
  %40 = getelementptr float* %39, i32 1           ; <float*> [#uses=1]
  %41 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar6 = load float* %40                  ; <float> [#uses=1]
  %42 = load <4 x float>* %41                     ; <<4 x float>> [#uses=1]
  %43 = insertelement <4 x float> %42, float %loadscalar6, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %43, <4 x float>* %41
  %44 = getelementptr float* %39, i32 5           ; <float*> [#uses=1]
  %45 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar7 = load float* %44                  ; <float> [#uses=1]
  %46 = load <4 x float>* %45                     ; <<4 x float>> [#uses=1]
  %47 = insertelement <4 x float> %46, float %loadscalar7, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %47, <4 x float>* %45
  %48 = getelementptr float* %39, i32 9           ; <float*> [#uses=1]
  %49 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar8 = load float* %48                  ; <float> [#uses=1]
  %50 = load <4 x float>* %49                     ; <<4 x float>> [#uses=1]
  %51 = insertelement <4 x float> %50, float %loadscalar8, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %51, <4 x float>* %49
  %52 = getelementptr float* %39, i32 13          ; <float*> [#uses=1]
  %53 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar9 = load float* %52                  ; <float> [#uses=1]
  %54 = load <4 x float>* %53                     ; <<4 x float>> [#uses=1]
  %55 = insertelement <4 x float> %54, float %loadscalar9, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %55, <4 x float>* %53
  br label %nexblock5

nexblock5:                                        ; preds = %instbb4, %nexblock
  %56 = extractelement <4 x i1> %0, i32 2         ; <i1> [#uses=1]
  %57 = icmp eq i1 %56, false                     ; <i1> [#uses=1]
  br i1 %57, label %nexblock11, label %instbb10

instbb10:                                         ; preds = %nexblock5
  %58 = extractelement <4 x i32> %2, i32 2        ; <i32> [#uses=1]
  %59 = getelementptr [1 x [32 x <4 x float>]*]* %5, i32 0, i32 %58 ; <[32 x <4 x float>]**> [#uses=1]
  %60 = load [32 x <4 x float>]** %59             ; <[32 x <4 x float>]*> [#uses=1]
  %61 = bitcast [32 x <4 x float>]* %60 to float* ; <float*> [#uses=1]
  %62 = extractelement <4 x i32> %3, i32 2        ; <i32> [#uses=1]
  %63 = mul i32 %62, 16                           ; <i32> [#uses=1]
  %64 = getelementptr float* %61, i32 %63         ; <float*> [#uses=4]
  %65 = getelementptr float* %64, i32 2           ; <float*> [#uses=1]
  %66 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar12 = load float* %65                 ; <float> [#uses=1]
  %67 = load <4 x float>* %66                     ; <<4 x float>> [#uses=1]
  %68 = insertelement <4 x float> %67, float %loadscalar12, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %68, <4 x float>* %66
  %69 = getelementptr float* %64, i32 6           ; <float*> [#uses=1]
  %70 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar13 = load float* %69                 ; <float> [#uses=1]
  %71 = load <4 x float>* %70                     ; <<4 x float>> [#uses=1]
  %72 = insertelement <4 x float> %71, float %loadscalar13, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %72, <4 x float>* %70
  %73 = getelementptr float* %64, i32 10          ; <float*> [#uses=1]
  %74 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar14 = load float* %73                 ; <float> [#uses=1]
  %75 = load <4 x float>* %74                     ; <<4 x float>> [#uses=1]
  %76 = insertelement <4 x float> %75, float %loadscalar14, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %76, <4 x float>* %74
  %77 = getelementptr float* %64, i32 14          ; <float*> [#uses=1]
  %78 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar15 = load float* %77                 ; <float> [#uses=1]
  %79 = load <4 x float>* %78                     ; <<4 x float>> [#uses=1]
  %80 = insertelement <4 x float> %79, float %loadscalar15, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %80, <4 x float>* %78
  br label %nexblock11

nexblock11:                                       ; preds = %instbb10, %nexblock5
  %81 = extractelement <4 x i1> %0, i32 3         ; <i1> [#uses=1]
  %82 = icmp eq i1 %81, false                     ; <i1> [#uses=1]
  br i1 %82, label %nexblock17, label %instbb16

instbb16:                                         ; preds = %nexblock11
  %83 = extractelement <4 x i32> %2, i32 3        ; <i32> [#uses=1]
  %84 = getelementptr [1 x [32 x <4 x float>]*]* %5, i32 0, i32 %83 ; <[32 x <4 x float>]**> [#uses=1]
  %85 = load [32 x <4 x float>]** %84             ; <[32 x <4 x float>]*> [#uses=1]
  %86 = bitcast [32 x <4 x float>]* %85 to float* ; <float*> [#uses=1]
  %87 = extractelement <4 x i32> %3, i32 3        ; <i32> [#uses=1]
  %88 = mul i32 %87, 16                           ; <i32> [#uses=1]
  %89 = getelementptr float* %86, i32 %88         ; <float*> [#uses=4]
  %90 = getelementptr float* %89, i32 3           ; <float*> [#uses=1]
  %91 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar18 = load float* %90                 ; <float> [#uses=1]
  %92 = load <4 x float>* %91                     ; <<4 x float>> [#uses=1]
  %93 = insertelement <4 x float> %92, float %loadscalar18, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %93, <4 x float>* %91
  %94 = getelementptr float* %89, i32 7           ; <float*> [#uses=1]
  %95 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar19 = load float* %94                 ; <float> [#uses=1]
  %96 = load <4 x float>* %95                     ; <<4 x float>> [#uses=1]
  %97 = insertelement <4 x float> %96, float %loadscalar19, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %97, <4 x float>* %95
  %98 = getelementptr float* %89, i32 11          ; <float*> [#uses=1]
  %99 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar20 = load float* %98                 ; <float> [#uses=1]
  %100 = load <4 x float>* %99                    ; <<4 x float>> [#uses=1]
  %101 = insertelement <4 x float> %100, float %loadscalar20, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %101, <4 x float>* %99
  %102 = getelementptr float* %89, i32 15         ; <float*> [#uses=1]
  %103 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar21 = load float* %102                ; <float> [#uses=1]
  %104 = load <4 x float>* %103                   ; <<4 x float>> [#uses=1]
  %105 = insertelement <4 x float> %104, float %loadscalar21, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %105, <4 x float>* %103
  br label %nexblock17

nexblock17:                                       ; preds = %instbb16, %nexblock11
  %106 = bitcast <4 x float>* %alloca to [4 x <4 x float>]* ; <[4 x <4 x float>]*> [#uses=1]
  %107 = load [4 x <4 x float>]* %106             ; <[4 x <4 x float>]> [#uses=1]
  ret [4 x <4 x float>] %107
}

define linkonce void @dx_soa_store_output_4_float4(<4 x i1>, i8*, <4 x i32>, <4 x i32>, [4 x <4 x float>]) {
load_store_entry:
  %5 = getelementptr i8* %1, i32 4                ; <i8*> [#uses=1]
  %6 = bitcast i8* %5 to [1 x [32 x <4 x float>]*]* ; <[1 x [32 x <4 x float>]*]*> [#uses=4]
  %7 = extractelement <4 x i1> %0, i32 0          ; <i1> [#uses=1]
  %8 = icmp eq i1 %7, false                       ; <i1> [#uses=1]
  br i1 %8, label %nexblock, label %instbb

instbb:                                           ; preds = %load_store_entry
  %9 = extractelement <4 x i32> %2, i32 0         ; <i32> [#uses=1]
  %10 = getelementptr [1 x [32 x <4 x float>]*]* %6, i32 0, i32 %9 ; <[32 x <4 x float>]**> [#uses=1]
  %11 = load [32 x <4 x float>]** %10             ; <[32 x <4 x float>]*> [#uses=1]
  %12 = bitcast [32 x <4 x float>]* %11 to float* ; <float*> [#uses=1]
  %13 = extractelement <4 x i32> %3, i32 0        ; <i32> [#uses=1]
  %14 = mul i32 %13, 16                           ; <i32> [#uses=1]
  %15 = getelementptr float* %12, i32 %14         ; <float*> [#uses=4]
  %16 = getelementptr float* %15, i32 0           ; <float*> [#uses=1]
  %17 = extractvalue [4 x <4 x float>] %4, 0      ; <<4 x float>> [#uses=1]
  %18 = extractelement <4 x float> %17, i32 0     ; <float> [#uses=1]
  store float %18, float* %16
  %19 = getelementptr float* %15, i32 4           ; <float*> [#uses=1]
  %20 = extractvalue [4 x <4 x float>] %4, 1      ; <<4 x float>> [#uses=1]
  %21 = extractelement <4 x float> %20, i32 0     ; <float> [#uses=1]
  store float %21, float* %19
  %22 = getelementptr float* %15, i32 8           ; <float*> [#uses=1]
  %23 = extractvalue [4 x <4 x float>] %4, 2      ; <<4 x float>> [#uses=1]
  %24 = extractelement <4 x float> %23, i32 0     ; <float> [#uses=1]
  store float %24, float* %22
  %25 = getelementptr float* %15, i32 12          ; <float*> [#uses=1]
  %26 = extractvalue [4 x <4 x float>] %4, 3      ; <<4 x float>> [#uses=1]
  %27 = extractelement <4 x float> %26, i32 0     ; <float> [#uses=1]
  store float %27, float* %25
  br label %nexblock

nexblock:                                         ; preds = %instbb, %load_store_entry
  %28 = extractelement <4 x i1> %0, i32 1         ; <i1> [#uses=1]
  %29 = icmp eq i1 %28, false                     ; <i1> [#uses=1]
  br i1 %29, label %nexblock2, label %instbb1

instbb1:                                          ; preds = %nexblock
  %30 = extractelement <4 x i32> %2, i32 1        ; <i32> [#uses=1]
  %31 = getelementptr [1 x [32 x <4 x float>]*]* %6, i32 0, i32 %30 ; <[32 x <4 x float>]**> [#uses=1]
  %32 = load [32 x <4 x float>]** %31             ; <[32 x <4 x float>]*> [#uses=1]
  %33 = bitcast [32 x <4 x float>]* %32 to float* ; <float*> [#uses=1]
  %34 = extractelement <4 x i32> %3, i32 1        ; <i32> [#uses=1]
  %35 = mul i32 %34, 16                           ; <i32> [#uses=1]
  %36 = getelementptr float* %33, i32 %35         ; <float*> [#uses=4]
  %37 = getelementptr float* %36, i32 1           ; <float*> [#uses=1]
  %38 = extractvalue [4 x <4 x float>] %4, 0      ; <<4 x float>> [#uses=1]
  %39 = extractelement <4 x float> %38, i32 1     ; <float> [#uses=1]
  store float %39, float* %37
  %40 = getelementptr float* %36, i32 5           ; <float*> [#uses=1]
  %41 = extractvalue [4 x <4 x float>] %4, 1      ; <<4 x float>> [#uses=1]
  %42 = extractelement <4 x float> %41, i32 1     ; <float> [#uses=1]
  store float %42, float* %40
  %43 = getelementptr float* %36, i32 9           ; <float*> [#uses=1]
  %44 = extractvalue [4 x <4 x float>] %4, 2      ; <<4 x float>> [#uses=1]
  %45 = extractelement <4 x float> %44, i32 1     ; <float> [#uses=1]
  store float %45, float* %43
  %46 = getelementptr float* %36, i32 13          ; <float*> [#uses=1]
  %47 = extractvalue [4 x <4 x float>] %4, 3      ; <<4 x float>> [#uses=1]
  %48 = extractelement <4 x float> %47, i32 1     ; <float> [#uses=1]
  store float %48, float* %46
  br label %nexblock2

nexblock2:                                        ; preds = %instbb1, %nexblock
  %49 = extractelement <4 x i1> %0, i32 2         ; <i1> [#uses=1]
  %50 = icmp eq i1 %49, false                     ; <i1> [#uses=1]
  br i1 %50, label %nexblock4, label %instbb3

instbb3:                                          ; preds = %nexblock2
  %51 = extractelement <4 x i32> %2, i32 2        ; <i32> [#uses=1]
  %52 = getelementptr [1 x [32 x <4 x float>]*]* %6, i32 0, i32 %51 ; <[32 x <4 x float>]**> [#uses=1]
  %53 = load [32 x <4 x float>]** %52             ; <[32 x <4 x float>]*> [#uses=1]
  %54 = bitcast [32 x <4 x float>]* %53 to float* ; <float*> [#uses=1]
  %55 = extractelement <4 x i32> %3, i32 2        ; <i32> [#uses=1]
  %56 = mul i32 %55, 16                           ; <i32> [#uses=1]
  %57 = getelementptr float* %54, i32 %56         ; <float*> [#uses=4]
  %58 = getelementptr float* %57, i32 2           ; <float*> [#uses=1]
  %59 = extractvalue [4 x <4 x float>] %4, 0      ; <<4 x float>> [#uses=1]
  %60 = extractelement <4 x float> %59, i32 2     ; <float> [#uses=1]
  store float %60, float* %58
  %61 = getelementptr float* %57, i32 6           ; <float*> [#uses=1]
  %62 = extractvalue [4 x <4 x float>] %4, 1      ; <<4 x float>> [#uses=1]
  %63 = extractelement <4 x float> %62, i32 2     ; <float> [#uses=1]
  store float %63, float* %61
  %64 = getelementptr float* %57, i32 10          ; <float*> [#uses=1]
  %65 = extractvalue [4 x <4 x float>] %4, 2      ; <<4 x float>> [#uses=1]
  %66 = extractelement <4 x float> %65, i32 2     ; <float> [#uses=1]
  store float %66, float* %64
  %67 = getelementptr float* %57, i32 14          ; <float*> [#uses=1]
  %68 = extractvalue [4 x <4 x float>] %4, 3      ; <<4 x float>> [#uses=1]
  %69 = extractelement <4 x float> %68, i32 2     ; <float> [#uses=1]
  store float %69, float* %67
  br label %nexblock4

nexblock4:                                        ; preds = %instbb3, %nexblock2
  %70 = extractelement <4 x i1> %0, i32 3         ; <i1> [#uses=1]
  %71 = icmp eq i1 %70, false                     ; <i1> [#uses=1]
  br i1 %71, label %nexblock6, label %instbb5

instbb5:                                          ; preds = %nexblock4
  %72 = extractelement <4 x i32> %2, i32 3        ; <i32> [#uses=1]
  %73 = getelementptr [1 x [32 x <4 x float>]*]* %6, i32 0, i32 %72 ; <[32 x <4 x float>]**> [#uses=1]
  %74 = load [32 x <4 x float>]** %73             ; <[32 x <4 x float>]*> [#uses=1]
  %75 = bitcast [32 x <4 x float>]* %74 to float* ; <float*> [#uses=1]
  %76 = extractelement <4 x i32> %3, i32 3        ; <i32> [#uses=1]
  %77 = mul i32 %76, 16                           ; <i32> [#uses=1]
  %78 = getelementptr float* %75, i32 %77         ; <float*> [#uses=4]
  %79 = getelementptr float* %78, i32 3           ; <float*> [#uses=1]
  %80 = extractvalue [4 x <4 x float>] %4, 0      ; <<4 x float>> [#uses=1]
  %81 = extractelement <4 x float> %80, i32 3     ; <float> [#uses=1]
  store float %81, float* %79
  %82 = getelementptr float* %78, i32 7           ; <float*> [#uses=1]
  %83 = extractvalue [4 x <4 x float>] %4, 1      ; <<4 x float>> [#uses=1]
  %84 = extractelement <4 x float> %83, i32 3     ; <float> [#uses=1]
  store float %84, float* %82
  %85 = getelementptr float* %78, i32 11          ; <float*> [#uses=1]
  %86 = extractvalue [4 x <4 x float>] %4, 2      ; <<4 x float>> [#uses=1]
  %87 = extractelement <4 x float> %86, i32 3     ; <float> [#uses=1]
  store float %87, float* %85
  %88 = getelementptr float* %78, i32 15          ; <float*> [#uses=1]
  %89 = extractvalue [4 x <4 x float>] %4, 3      ; <<4 x float>> [#uses=1]
  %90 = extractelement <4 x float> %89, i32 3     ; <float> [#uses=1]
  store float %90, float* %88
  br label %nexblock6

nexblock6:                                        ; preds = %instbb5, %nexblock4
  ret void
}

define linkonce [4 x <4 x float>] @dx_soa_load_constant_4_float4(<4 x i1>, i8*, <4 x i32>, <4 x i32>) {
load_store_entry:
  %4 = getelementptr i8* %1, i32 584              ; <i8*> [#uses=1]
  %5 = bitcast i8* %4 to [128 x <4 x float>*]*    ; <[128 x <4 x float>*]*> [#uses=4]
  %6 = getelementptr i8* %1, i32 1096             ; <i8*> [#uses=1]
  %7 = bitcast i8* %6 to [128 x i32]*             ; <[128 x i32]*> [#uses=4]
  %alloca = alloca <4 x float>, i32 4, align 16   ; <<4 x float>*> [#uses=17]
  %zeroBuffer = alloca float, i32 16              ; <float*> [#uses=36]
  %8 = getelementptr float* %zeroBuffer, i32 0    ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %9 = getelementptr float* %zeroBuffer, i32 1    ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %10 = getelementptr float* %zeroBuffer, i32 2   ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %11 = getelementptr float* %zeroBuffer, i32 3   ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %12 = getelementptr float* %zeroBuffer, i32 4   ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %13 = getelementptr float* %zeroBuffer, i32 5   ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %14 = getelementptr float* %zeroBuffer, i32 6   ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %15 = getelementptr float* %zeroBuffer, i32 7   ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %16 = getelementptr float* %zeroBuffer, i32 8   ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %17 = getelementptr float* %zeroBuffer, i32 9   ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %18 = getelementptr float* %zeroBuffer, i32 10  ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %19 = getelementptr float* %zeroBuffer, i32 11  ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %20 = getelementptr float* %zeroBuffer, i32 12  ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %21 = getelementptr float* %zeroBuffer, i32 13  ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %22 = getelementptr float* %zeroBuffer, i32 14  ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %23 = getelementptr float* %zeroBuffer, i32 15  ; <float*> [#uses=0]
  store float 0.000000e+000, float* %zeroBuffer
  %24 = extractelement <4 x i1> %0, i32 0         ; <i1> [#uses=1]
  %25 = icmp eq i1 %24, false                     ; <i1> [#uses=1]
  br i1 %25, label %nexblock, label %instbb

instbb:                                           ; preds = %load_store_entry
  %26 = extractelement <4 x i32> %2, i32 0        ; <i32> [#uses=1]
  %27 = getelementptr [128 x <4 x float>*]* %5, i32 0, i32 %26 ; <<4 x float>**> [#uses=1]
  %28 = load <4 x float>** %27                    ; <<4 x float>*> [#uses=1]
  %29 = bitcast <4 x float>* %28 to float*        ; <float*> [#uses=1]
  %30 = extractelement <4 x i32> %2, i32 0        ; <i32> [#uses=1]
  %31 = getelementptr [128 x i32]* %7, i32 0, i32 %30 ; <i32*> [#uses=1]
  %32 = load i32* %31                             ; <i32> [#uses=1]
  %33 = extractelement <4 x i32> %3, i32 0        ; <i32> [#uses=2]
  %34 = mul i32 %33, 16                           ; <i32> [#uses=1]
  %cbload = getelementptr float* %29, i32 %34     ; <float*> [#uses=1]
  %35 = icmp ult i32 %33, %32                     ; <i1> [#uses=1]
  %36 = select i1 %35, float* %cbload, float* %zeroBuffer ; <float*> [#uses=4]
  %37 = getelementptr float* %36, i32 0           ; <float*> [#uses=1]
  %38 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar = load float* %37                   ; <float> [#uses=1]
  %39 = load <4 x float>* %38                     ; <<4 x float>> [#uses=1]
  %40 = insertelement <4 x float> %39, float %loadscalar, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %40, <4 x float>* %38
  %41 = getelementptr float* %36, i32 4           ; <float*> [#uses=1]
  %42 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar1 = load float* %41                  ; <float> [#uses=1]
  %43 = load <4 x float>* %42                     ; <<4 x float>> [#uses=1]
  %44 = insertelement <4 x float> %43, float %loadscalar1, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %44, <4 x float>* %42
  %45 = getelementptr float* %36, i32 8           ; <float*> [#uses=1]
  %46 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar2 = load float* %45                  ; <float> [#uses=1]
  %47 = load <4 x float>* %46                     ; <<4 x float>> [#uses=1]
  %48 = insertelement <4 x float> %47, float %loadscalar2, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %48, <4 x float>* %46
  %49 = getelementptr float* %36, i32 12          ; <float*> [#uses=1]
  %50 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar3 = load float* %49                  ; <float> [#uses=1]
  %51 = load <4 x float>* %50                     ; <<4 x float>> [#uses=1]
  %52 = insertelement <4 x float> %51, float %loadscalar3, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %52, <4 x float>* %50
  br label %nexblock

nexblock:                                         ; preds = %instbb, %load_store_entry
  %53 = extractelement <4 x i1> %0, i32 1         ; <i1> [#uses=1]
  %54 = icmp eq i1 %53, false                     ; <i1> [#uses=1]
  br i1 %54, label %nexblock5, label %instbb4

instbb4:                                          ; preds = %nexblock
  %55 = extractelement <4 x i32> %2, i32 1        ; <i32> [#uses=1]
  %56 = getelementptr [128 x <4 x float>*]* %5, i32 0, i32 %55 ; <<4 x float>**> [#uses=1]
  %57 = load <4 x float>** %56                    ; <<4 x float>*> [#uses=1]
  %58 = bitcast <4 x float>* %57 to float*        ; <float*> [#uses=1]
  %59 = extractelement <4 x i32> %2, i32 1        ; <i32> [#uses=1]
  %60 = getelementptr [128 x i32]* %7, i32 0, i32 %59 ; <i32*> [#uses=1]
  %61 = load i32* %60                             ; <i32> [#uses=1]
  %62 = extractelement <4 x i32> %3, i32 1        ; <i32> [#uses=2]
  %63 = mul i32 %62, 16                           ; <i32> [#uses=1]
  %cbload6 = getelementptr float* %58, i32 %63    ; <float*> [#uses=1]
  %64 = icmp ult i32 %62, %61                     ; <i1> [#uses=1]
  %65 = select i1 %64, float* %cbload6, float* %zeroBuffer ; <float*> [#uses=4]
  %66 = getelementptr float* %65, i32 1           ; <float*> [#uses=1]
  %67 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar7 = load float* %66                  ; <float> [#uses=1]
  %68 = load <4 x float>* %67                     ; <<4 x float>> [#uses=1]
  %69 = insertelement <4 x float> %68, float %loadscalar7, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %69, <4 x float>* %67
  %70 = getelementptr float* %65, i32 5           ; <float*> [#uses=1]
  %71 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar8 = load float* %70                  ; <float> [#uses=1]
  %72 = load <4 x float>* %71                     ; <<4 x float>> [#uses=1]
  %73 = insertelement <4 x float> %72, float %loadscalar8, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %73, <4 x float>* %71
  %74 = getelementptr float* %65, i32 9           ; <float*> [#uses=1]
  %75 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar9 = load float* %74                  ; <float> [#uses=1]
  %76 = load <4 x float>* %75                     ; <<4 x float>> [#uses=1]
  %77 = insertelement <4 x float> %76, float %loadscalar9, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %77, <4 x float>* %75
  %78 = getelementptr float* %65, i32 13          ; <float*> [#uses=1]
  %79 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar10 = load float* %78                 ; <float> [#uses=1]
  %80 = load <4 x float>* %79                     ; <<4 x float>> [#uses=1]
  %81 = insertelement <4 x float> %80, float %loadscalar10, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %81, <4 x float>* %79
  br label %nexblock5

nexblock5:                                        ; preds = %instbb4, %nexblock
  %82 = extractelement <4 x i1> %0, i32 2         ; <i1> [#uses=1]
  %83 = icmp eq i1 %82, false                     ; <i1> [#uses=1]
  br i1 %83, label %nexblock12, label %instbb11

instbb11:                                         ; preds = %nexblock5
  %84 = extractelement <4 x i32> %2, i32 2        ; <i32> [#uses=1]
  %85 = getelementptr [128 x <4 x float>*]* %5, i32 0, i32 %84 ; <<4 x float>**> [#uses=1]
  %86 = load <4 x float>** %85                    ; <<4 x float>*> [#uses=1]
  %87 = bitcast <4 x float>* %86 to float*        ; <float*> [#uses=1]
  %88 = extractelement <4 x i32> %2, i32 2        ; <i32> [#uses=1]
  %89 = getelementptr [128 x i32]* %7, i32 0, i32 %88 ; <i32*> [#uses=1]
  %90 = load i32* %89                             ; <i32> [#uses=1]
  %91 = extractelement <4 x i32> %3, i32 2        ; <i32> [#uses=2]
  %92 = mul i32 %91, 16                           ; <i32> [#uses=1]
  %cbload13 = getelementptr float* %87, i32 %92   ; <float*> [#uses=1]
  %93 = icmp ult i32 %91, %90                     ; <i1> [#uses=1]
  %94 = select i1 %93, float* %cbload13, float* %zeroBuffer ; <float*> [#uses=4]
  %95 = getelementptr float* %94, i32 2           ; <float*> [#uses=1]
  %96 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar14 = load float* %95                 ; <float> [#uses=1]
  %97 = load <4 x float>* %96                     ; <<4 x float>> [#uses=1]
  %98 = insertelement <4 x float> %97, float %loadscalar14, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %98, <4 x float>* %96
  %99 = getelementptr float* %94, i32 6           ; <float*> [#uses=1]
  %100 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar15 = load float* %99                 ; <float> [#uses=1]
  %101 = load <4 x float>* %100                   ; <<4 x float>> [#uses=1]
  %102 = insertelement <4 x float> %101, float %loadscalar15, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %102, <4 x float>* %100
  %103 = getelementptr float* %94, i32 10         ; <float*> [#uses=1]
  %104 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar16 = load float* %103                ; <float> [#uses=1]
  %105 = load <4 x float>* %104                   ; <<4 x float>> [#uses=1]
  %106 = insertelement <4 x float> %105, float %loadscalar16, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %106, <4 x float>* %104
  %107 = getelementptr float* %94, i32 14         ; <float*> [#uses=1]
  %108 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar17 = load float* %107                ; <float> [#uses=1]
  %109 = load <4 x float>* %108                   ; <<4 x float>> [#uses=1]
  %110 = insertelement <4 x float> %109, float %loadscalar17, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %110, <4 x float>* %108
  br label %nexblock12

nexblock12:                                       ; preds = %instbb11, %nexblock5
  %111 = extractelement <4 x i1> %0, i32 3        ; <i1> [#uses=1]
  %112 = icmp eq i1 %111, false                   ; <i1> [#uses=1]
  br i1 %112, label %nexblock19, label %instbb18

instbb18:                                         ; preds = %nexblock12
  %113 = extractelement <4 x i32> %2, i32 3       ; <i32> [#uses=1]
  %114 = getelementptr [128 x <4 x float>*]* %5, i32 0, i32 %113 ; <<4 x float>**> [#uses=1]
  %115 = load <4 x float>** %114                  ; <<4 x float>*> [#uses=1]
  %116 = bitcast <4 x float>* %115 to float*      ; <float*> [#uses=1]
  %117 = extractelement <4 x i32> %2, i32 3       ; <i32> [#uses=1]
  %118 = getelementptr [128 x i32]* %7, i32 0, i32 %117 ; <i32*> [#uses=1]
  %119 = load i32* %118                           ; <i32> [#uses=1]
  %120 = extractelement <4 x i32> %3, i32 3       ; <i32> [#uses=2]
  %121 = mul i32 %120, 16                         ; <i32> [#uses=1]
  %cbload20 = getelementptr float* %116, i32 %121 ; <float*> [#uses=1]
  %122 = icmp ult i32 %120, %119                  ; <i1> [#uses=1]
  %123 = select i1 %122, float* %cbload20, float* %zeroBuffer ; <float*> [#uses=4]
  %124 = getelementptr float* %123, i32 3         ; <float*> [#uses=1]
  %125 = getelementptr <4 x float>* %alloca, i32 0 ; <<4 x float>*> [#uses=2]
  %loadscalar21 = load float* %124                ; <float> [#uses=1]
  %126 = load <4 x float>* %125                   ; <<4 x float>> [#uses=1]
  %127 = insertelement <4 x float> %126, float %loadscalar21, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %127, <4 x float>* %125
  %128 = getelementptr float* %123, i32 7         ; <float*> [#uses=1]
  %129 = getelementptr <4 x float>* %alloca, i32 1 ; <<4 x float>*> [#uses=2]
  %loadscalar22 = load float* %128                ; <float> [#uses=1]
  %130 = load <4 x float>* %129                   ; <<4 x float>> [#uses=1]
  %131 = insertelement <4 x float> %130, float %loadscalar22, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %131, <4 x float>* %129
  %132 = getelementptr float* %123, i32 11        ; <float*> [#uses=1]
  %133 = getelementptr <4 x float>* %alloca, i32 2 ; <<4 x float>*> [#uses=2]
  %loadscalar23 = load float* %132                ; <float> [#uses=1]
  %134 = load <4 x float>* %133                   ; <<4 x float>> [#uses=1]
  %135 = insertelement <4 x float> %134, float %loadscalar23, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %135, <4 x float>* %133
  %136 = getelementptr float* %123, i32 15        ; <float*> [#uses=1]
  %137 = getelementptr <4 x float>* %alloca, i32 3 ; <<4 x float>*> [#uses=2]
  %loadscalar24 = load float* %136                ; <float> [#uses=1]
  %138 = load <4 x float>* %137                   ; <<4 x float>> [#uses=1]
  %139 = insertelement <4 x float> %138, float %loadscalar24, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %139, <4 x float>* %137
  br label %nexblock19

nexblock19:                                       ; preds = %instbb18, %nexblock12
  %140 = bitcast <4 x float>* %alloca to [4 x <4 x float>]* ; <[4 x <4 x float>]*> [#uses=1]
  %141 = load [4 x <4 x float>]* %140             ; <[4 x <4 x float>]> [#uses=1]
  ret [4 x <4 x float>] %141
}

