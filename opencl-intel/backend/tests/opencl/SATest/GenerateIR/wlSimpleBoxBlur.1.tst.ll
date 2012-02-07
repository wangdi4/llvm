; ModuleID = 'C:/Users/nrotem/Desktop/LLVM30_migration/src/backend/tests/opencl/SATest/GenerateIR/wlSimpleBoxBlur.1.tst.bin'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%struct._image2d_t = type opaque

define void @wlSimpleBoxBlur_GPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
  %1 = alloca <4 x float> addrspace(1)*, align 4
  %2 = alloca <4 x float> addrspace(1)*, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %dims = alloca i32, align 4
  %globalIdx = alloca i32, align 4
  %globalIdy = alloca i32, align 4
  %localIdx = alloca i32, align 4
  %localIdy = alloca i32, align 4
  %global_szx = alloca i32, align 4
  %global_szy = alloca i32, align 4
  %local_szx = alloca i32, align 4
  %local_szy = alloca i32, align 4
  %groupIdx = alloca i32, align 4
  %groupIdy = alloca i32, align 4
  %denominator = alloca float, align 4
  %colorAccumulator = alloca <4 x float>, align 16
  %sourceIndex = alloca i32, align 4
  %tmpIndex1 = alloca i32, align 4
  %tmpIndex2 = alloca i32, align 4
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %1, align 4
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %2, align 4
  store i32 %width, i32* %3, align 4
  store i32 %height, i32* %4, align 4
  store i32 %buffer_size, i32* %5, align 4
  %6 = call i32 (...)* @get_work_dim()
  store i32 %6, i32* %dims, align 4
  %7 = call i32 @get_global_id(i32 0)
  store i32 %7, i32* %globalIdx, align 4
  %8 = call i32 @get_global_id(i32 1)
  store i32 %8, i32* %globalIdy, align 4
  %9 = call i32 @get_local_id(i32 0)
  store i32 %9, i32* %localIdx, align 4
  %10 = call i32 @get_local_id(i32 1)
  store i32 %10, i32* %localIdy, align 4
  %11 = call i32 @get_global_size(i32 0)
  store i32 %11, i32* %global_szx, align 4
  %12 = call i32 @get_global_size(i32 1)
  store i32 %12, i32* %global_szy, align 4
  %13 = call i32 @get_local_size(i32 0)
  store i32 %13, i32* %local_szx, align 4
  %14 = call i32 @get_local_size(i32 1)
  store i32 %14, i32* %local_szy, align 4
  %15 = call i32 @get_group_id(i32 0)
  store i32 %15, i32* %groupIdx, align 4
  %16 = call i32 @get_group_id(i32 1)
  store i32 %16, i32* %groupIdy, align 4
  store float 9.000000e+00, float* %denominator, align 4
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  %17 = load i32* %globalIdy, align 4
  %18 = load i32* %3, align 4
  %19 = mul i32 %17, %18
  %20 = load i32* %globalIdx, align 4
  %21 = add i32 %19, %20
  store i32 %21, i32* %sourceIndex, align 4
  %22 = load i32* %globalIdy, align 4
  %23 = sub i32 %22, 1
  %24 = load i32* %3, align 4
  %25 = mul i32 %23, %24
  %26 = load i32* %globalIdx, align 4
  %27 = add i32 %25, %26
  store i32 %27, i32* %tmpIndex1, align 4
  %28 = load i32* %globalIdy, align 4
  %29 = add i32 %28, 1
  %30 = load i32* %3, align 4
  %31 = mul i32 %29, %30
  %32 = load i32* %globalIdx, align 4
  %33 = add i32 %31, %32
  store i32 %33, i32* %tmpIndex2, align 4
  %34 = load i32* %sourceIndex, align 4
  %35 = load <4 x float> addrspace(1)** %1, align 4
  %36 = getelementptr inbounds <4 x float> addrspace(1)* %35, i32 %34
  %37 = load <4 x float> addrspace(1)* %36
  store <4 x float> %37, <4 x float>* %colorAccumulator, align 16
  %38 = load i32* %globalIdx, align 4
  %39 = icmp ugt i32 %38, 0
  br i1 %39, label %40, label %48

; <label>:40                                      ; preds = %0
  %41 = load i32* %sourceIndex, align 4
  %42 = sub i32 %41, 1
  %43 = load <4 x float> addrspace(1)** %1, align 4
  %44 = getelementptr inbounds <4 x float> addrspace(1)* %43, i32 %42
  %45 = load <4 x float> addrspace(1)* %44
  %46 = load <4 x float>* %colorAccumulator, align 16
  %47 = fadd <4 x float> %46, %45
  store <4 x float> %47, <4 x float>* %colorAccumulator, align 16
  br label %48

; <label>:48                                      ; preds = %40, %0
  %49 = load i32* %globalIdx, align 4
  %50 = load i32* %global_szx, align 4
  %51 = sub i32 %50, 1
  %52 = icmp ult i32 %49, %51
  br i1 %52, label %53, label %61

; <label>:53                                      ; preds = %48
  %54 = load i32* %sourceIndex, align 4
  %55 = add i32 %54, 1
  %56 = load <4 x float> addrspace(1)** %1, align 4
  %57 = getelementptr inbounds <4 x float> addrspace(1)* %56, i32 %55
  %58 = load <4 x float> addrspace(1)* %57
  %59 = load <4 x float>* %colorAccumulator, align 16
  %60 = fadd <4 x float> %59, %58
  store <4 x float> %60, <4 x float>* %colorAccumulator, align 16
  br label %61

; <label>:61                                      ; preds = %53, %48
  %62 = load i32* %globalIdy, align 4
  %63 = icmp ugt i32 %62, 0
  br i1 %63, label %64, label %95

; <label>:64                                      ; preds = %61
  %65 = load i32* %tmpIndex1, align 4
  %66 = load <4 x float> addrspace(1)** %1, align 4
  %67 = getelementptr inbounds <4 x float> addrspace(1)* %66, i32 %65
  %68 = load <4 x float> addrspace(1)* %67
  %69 = load <4 x float>* %colorAccumulator, align 16
  %70 = fadd <4 x float> %69, %68
  store <4 x float> %70, <4 x float>* %colorAccumulator, align 16
  %71 = load i32* %globalIdx, align 4
  %72 = icmp ugt i32 %71, 0
  br i1 %72, label %73, label %81

; <label>:73                                      ; preds = %64
  %74 = load i32* %tmpIndex1, align 4
  %75 = sub i32 %74, 1
  %76 = load <4 x float> addrspace(1)** %1, align 4
  %77 = getelementptr inbounds <4 x float> addrspace(1)* %76, i32 %75
  %78 = load <4 x float> addrspace(1)* %77
  %79 = load <4 x float>* %colorAccumulator, align 16
  %80 = fadd <4 x float> %79, %78
  store <4 x float> %80, <4 x float>* %colorAccumulator, align 16
  br label %81

; <label>:81                                      ; preds = %73, %64
  %82 = load i32* %globalIdx, align 4
  %83 = load i32* %global_szx, align 4
  %84 = sub i32 %83, 1
  %85 = icmp ult i32 %82, %84
  br i1 %85, label %86, label %94

; <label>:86                                      ; preds = %81
  %87 = load i32* %tmpIndex1, align 4
  %88 = add i32 %87, 1
  %89 = load <4 x float> addrspace(1)** %1, align 4
  %90 = getelementptr inbounds <4 x float> addrspace(1)* %89, i32 %88
  %91 = load <4 x float> addrspace(1)* %90
  %92 = load <4 x float>* %colorAccumulator, align 16
  %93 = fadd <4 x float> %92, %91
  store <4 x float> %93, <4 x float>* %colorAccumulator, align 16
  br label %94

; <label>:94                                      ; preds = %86, %81
  br label %95

; <label>:95                                      ; preds = %94, %61
  %96 = load i32* %globalIdy, align 4
  %97 = load i32* %global_szy, align 4
  %98 = sub i32 %97, 1
  %99 = icmp ult i32 %96, %98
  br i1 %99, label %100, label %131

; <label>:100                                     ; preds = %95
  %101 = load i32* %tmpIndex2, align 4
  %102 = load <4 x float> addrspace(1)** %1, align 4
  %103 = getelementptr inbounds <4 x float> addrspace(1)* %102, i32 %101
  %104 = load <4 x float> addrspace(1)* %103
  %105 = load <4 x float>* %colorAccumulator, align 16
  %106 = fadd <4 x float> %105, %104
  store <4 x float> %106, <4 x float>* %colorAccumulator, align 16
  %107 = load i32* %globalIdx, align 4
  %108 = icmp ugt i32 %107, 0
  br i1 %108, label %109, label %117

; <label>:109                                     ; preds = %100
  %110 = load i32* %tmpIndex2, align 4
  %111 = sub i32 %110, 1
  %112 = load <4 x float> addrspace(1)** %1, align 4
  %113 = getelementptr inbounds <4 x float> addrspace(1)* %112, i32 %111
  %114 = load <4 x float> addrspace(1)* %113
  %115 = load <4 x float>* %colorAccumulator, align 16
  %116 = fadd <4 x float> %115, %114
  store <4 x float> %116, <4 x float>* %colorAccumulator, align 16
  br label %117

; <label>:117                                     ; preds = %109, %100
  %118 = load i32* %globalIdx, align 4
  %119 = load i32* %global_szx, align 4
  %120 = sub i32 %119, 1
  %121 = icmp ult i32 %118, %120
  br i1 %121, label %122, label %130

; <label>:122                                     ; preds = %117
  %123 = load i32* %tmpIndex2, align 4
  %124 = add i32 %123, 1
  %125 = load <4 x float> addrspace(1)** %1, align 4
  %126 = getelementptr inbounds <4 x float> addrspace(1)* %125, i32 %124
  %127 = load <4 x float> addrspace(1)* %126
  %128 = load <4 x float>* %colorAccumulator, align 16
  %129 = fadd <4 x float> %128, %127
  store <4 x float> %129, <4 x float>* %colorAccumulator, align 16
  br label %130

; <label>:130                                     ; preds = %122, %117
  br label %131

; <label>:131                                     ; preds = %130, %95
  %132 = load <4 x float>* %colorAccumulator, align 16
  %133 = load float* %denominator, align 4
  %134 = insertelement <4 x float> undef, float %133, i32 0
  %135 = shufflevector <4 x float> %134, <4 x float> %134, <4 x i32> zeroinitializer
  %136 = fdiv <4 x float> %132, %135
  %137 = load i32* %sourceIndex, align 4
  %138 = load <4 x float> addrspace(1)** %2, align 4
  %139 = getelementptr inbounds <4 x float> addrspace(1)* %138, i32 %137
  store <4 x float> %136, <4 x float> addrspace(1)* %139
  ret void
}

declare i32 @get_work_dim(...)

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)

declare i32 @get_global_size(i32)

declare i32 @get_local_size(i32)

declare i32 @get_group_id(i32)

define void @wlSimpleBoxBlur_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
  %1 = alloca <4 x float> addrspace(1)*, align 4
  %2 = alloca <4 x float> addrspace(1)*, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %dims = alloca i32, align 4
  %globalIdx = alloca i32, align 4
  %globalIdy = alloca i32, align 4
  %localIdx = alloca i32, align 4
  %localIdy = alloca i32, align 4
  %global_szx = alloca i32, align 4
  %global_szy = alloca i32, align 4
  %local_szx = alloca i32, align 4
  %local_szy = alloca i32, align 4
  %groupIdx = alloca i32, align 4
  %groupIdy = alloca i32, align 4
  %count_x = alloca i32, align 4
  %count_y = alloca i32, align 4
  %index_x = alloca i32, align 4
  %index_y = alloca i32, align 4
  %index_x_orig = alloca i32, align 4
  %denominator = alloca float, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %index_x1 = alloca i32, align 4
  %colorAccumulator = alloca <4 x float>, align 16
  %sourceIndex = alloca i32, align 4
  %tmpIndex1 = alloca i32, align 4
  %tmpIndex2 = alloca i32, align 4
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %1, align 4
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %2, align 4
  store i32 %width, i32* %3, align 4
  store i32 %height, i32* %4, align 4
  store i32 %buffer_size, i32* %5, align 4
  %6 = call i32 (...)* @get_work_dim()
  store i32 %6, i32* %dims, align 4
  %7 = call i32 @get_global_id(i32 0)
  store i32 %7, i32* %globalIdx, align 4
  %8 = call i32 @get_global_id(i32 1)
  store i32 %8, i32* %globalIdy, align 4
  %9 = call i32 @get_local_id(i32 0)
  store i32 %9, i32* %localIdx, align 4
  %10 = call i32 @get_local_id(i32 1)
  store i32 %10, i32* %localIdy, align 4
  %11 = call i32 @get_global_size(i32 0)
  store i32 %11, i32* %global_szx, align 4
  %12 = call i32 @get_global_size(i32 1)
  store i32 %12, i32* %global_szy, align 4
  %13 = call i32 @get_local_size(i32 0)
  store i32 %13, i32* %local_szx, align 4
  %14 = call i32 @get_local_size(i32 1)
  store i32 %14, i32* %local_szy, align 4
  %15 = call i32 @get_group_id(i32 0)
  store i32 %15, i32* %groupIdx, align 4
  %16 = call i32 @get_group_id(i32 1)
  store i32 %16, i32* %groupIdy, align 4
  %17 = load i32* %3, align 4
  %18 = load i32* %global_szx, align 4
  %19 = icmp eq i32 0, %18
  %20 = select i1 %19, i32 1, i32 %18
  %21 = udiv i32 %17, %20
  store i32 %21, i32* %count_x, align 4
  %22 = load i32* %4, align 4
  %23 = load i32* %global_szy, align 4
  %24 = icmp eq i32 0, %23
  %25 = select i1 %24, i32 1, i32 %23
  %26 = udiv i32 %22, %25
  store i32 %26, i32* %count_y, align 4
  %27 = load i32* %3, align 4
  %28 = load i32* %globalIdx, align 4
  %29 = mul i32 %27, %28
  %30 = load i32* %global_szx, align 4
  %31 = icmp eq i32 0, %30
  %32 = select i1 %31, i32 1, i32 %30
  %33 = udiv i32 %29, %32
  store i32 %33, i32* %index_x, align 4
  %34 = load i32* %4, align 4
  %35 = load i32* %globalIdy, align 4
  %36 = mul i32 %34, %35
  %37 = load i32* %global_szy, align 4
  %38 = icmp eq i32 0, %37
  %39 = select i1 %38, i32 1, i32 %37
  %40 = udiv i32 %36, %39
  store i32 %40, i32* %index_y, align 4
  %41 = load i32* %index_x, align 4
  store i32 %41, i32* %index_x_orig, align 4
  store float 9.000000e+00, float* %denominator, align 4
  store i32 0, i32* %i, align 4
  br label %42

; <label>:42                                      ; preds = %182, %0
  %43 = load i32* %i, align 4
  %44 = load i32* %count_y, align 4
  %45 = icmp ult i32 %43, %44
  br i1 %45, label %46, label %187

; <label>:46                                      ; preds = %42
  store i32 0, i32* %j, align 4
  %47 = load i32* %index_x_orig, align 4
  store i32 %47, i32* %index_x1, align 4
  br label %48

; <label>:48                                      ; preds = %176, %46
  %49 = load i32* %j, align 4
  %50 = load i32* %count_x, align 4
  %51 = icmp ult i32 %49, %50
  br i1 %51, label %52, label %181

; <label>:52                                      ; preds = %48
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  %53 = load i32* %index_y, align 4
  %54 = load i32* %3, align 4
  %55 = mul i32 %53, %54
  %56 = load i32* %index_x1, align 4
  %57 = add i32 %55, %56
  store i32 %57, i32* %sourceIndex, align 4
  %58 = load i32* %index_y, align 4
  %59 = sub i32 %58, 1
  %60 = load i32* %3, align 4
  %61 = mul i32 %59, %60
  %62 = load i32* %index_x1, align 4
  %63 = add i32 %61, %62
  store i32 %63, i32* %tmpIndex1, align 4
  %64 = load i32* %index_y, align 4
  %65 = add i32 %64, 1
  %66 = load i32* %3, align 4
  %67 = mul i32 %65, %66
  %68 = load i32* %index_x1, align 4
  %69 = add i32 %67, %68
  store i32 %69, i32* %tmpIndex2, align 4
  %70 = load i32* %sourceIndex, align 4
  %71 = load <4 x float> addrspace(1)** %1, align 4
  %72 = getelementptr inbounds <4 x float> addrspace(1)* %71, i32 %70
  %73 = load <4 x float> addrspace(1)* %72
  store <4 x float> %73, <4 x float>* %colorAccumulator, align 16
  %74 = load i32* %index_x1, align 4
  %75 = icmp ugt i32 %74, 0
  br i1 %75, label %76, label %84

; <label>:76                                      ; preds = %52
  %77 = load i32* %sourceIndex, align 4
  %78 = sub i32 %77, 1
  %79 = load <4 x float> addrspace(1)** %1, align 4
  %80 = getelementptr inbounds <4 x float> addrspace(1)* %79, i32 %78
  %81 = load <4 x float> addrspace(1)* %80
  %82 = load <4 x float>* %colorAccumulator, align 16
  %83 = fadd <4 x float> %82, %81
  store <4 x float> %83, <4 x float>* %colorAccumulator, align 16
  br label %84

; <label>:84                                      ; preds = %76, %52
  %85 = load i32* %index_x1, align 4
  %86 = load i32* %3, align 4
  %87 = sub i32 %86, 1
  %88 = icmp ult i32 %85, %87
  br i1 %88, label %89, label %97

; <label>:89                                      ; preds = %84
  %90 = load i32* %sourceIndex, align 4
  %91 = add i32 %90, 1
  %92 = load <4 x float> addrspace(1)** %1, align 4
  %93 = getelementptr inbounds <4 x float> addrspace(1)* %92, i32 %91
  %94 = load <4 x float> addrspace(1)* %93
  %95 = load <4 x float>* %colorAccumulator, align 16
  %96 = fadd <4 x float> %95, %94
  store <4 x float> %96, <4 x float>* %colorAccumulator, align 16
  br label %97

; <label>:97                                      ; preds = %89, %84
  %98 = load i32* %index_y, align 4
  %99 = icmp ugt i32 %98, 0
  br i1 %99, label %100, label %131

; <label>:100                                     ; preds = %97
  %101 = load i32* %tmpIndex1, align 4
  %102 = load <4 x float> addrspace(1)** %1, align 4
  %103 = getelementptr inbounds <4 x float> addrspace(1)* %102, i32 %101
  %104 = load <4 x float> addrspace(1)* %103
  %105 = load <4 x float>* %colorAccumulator, align 16
  %106 = fadd <4 x float> %105, %104
  store <4 x float> %106, <4 x float>* %colorAccumulator, align 16
  %107 = load i32* %index_x1, align 4
  %108 = icmp ugt i32 %107, 0
  br i1 %108, label %109, label %117

; <label>:109                                     ; preds = %100
  %110 = load i32* %tmpIndex1, align 4
  %111 = sub i32 %110, 1
  %112 = load <4 x float> addrspace(1)** %1, align 4
  %113 = getelementptr inbounds <4 x float> addrspace(1)* %112, i32 %111
  %114 = load <4 x float> addrspace(1)* %113
  %115 = load <4 x float>* %colorAccumulator, align 16
  %116 = fadd <4 x float> %115, %114
  store <4 x float> %116, <4 x float>* %colorAccumulator, align 16
  br label %117

; <label>:117                                     ; preds = %109, %100
  %118 = load i32* %index_x1, align 4
  %119 = load i32* %3, align 4
  %120 = sub i32 %119, 1
  %121 = icmp ult i32 %118, %120
  br i1 %121, label %122, label %130

; <label>:122                                     ; preds = %117
  %123 = load i32* %tmpIndex1, align 4
  %124 = add i32 %123, 1
  %125 = load <4 x float> addrspace(1)** %1, align 4
  %126 = getelementptr inbounds <4 x float> addrspace(1)* %125, i32 %124
  %127 = load <4 x float> addrspace(1)* %126
  %128 = load <4 x float>* %colorAccumulator, align 16
  %129 = fadd <4 x float> %128, %127
  store <4 x float> %129, <4 x float>* %colorAccumulator, align 16
  br label %130

; <label>:130                                     ; preds = %122, %117
  br label %131

; <label>:131                                     ; preds = %130, %97
  %132 = load i32* %index_y, align 4
  %133 = load i32* %4, align 4
  %134 = sub i32 %133, 1
  %135 = icmp ult i32 %132, %134
  br i1 %135, label %136, label %167

; <label>:136                                     ; preds = %131
  %137 = load i32* %tmpIndex2, align 4
  %138 = load <4 x float> addrspace(1)** %1, align 4
  %139 = getelementptr inbounds <4 x float> addrspace(1)* %138, i32 %137
  %140 = load <4 x float> addrspace(1)* %139
  %141 = load <4 x float>* %colorAccumulator, align 16
  %142 = fadd <4 x float> %141, %140
  store <4 x float> %142, <4 x float>* %colorAccumulator, align 16
  %143 = load i32* %index_x1, align 4
  %144 = icmp ugt i32 %143, 0
  br i1 %144, label %145, label %153

; <label>:145                                     ; preds = %136
  %146 = load i32* %tmpIndex2, align 4
  %147 = sub i32 %146, 1
  %148 = load <4 x float> addrspace(1)** %1, align 4
  %149 = getelementptr inbounds <4 x float> addrspace(1)* %148, i32 %147
  %150 = load <4 x float> addrspace(1)* %149
  %151 = load <4 x float>* %colorAccumulator, align 16
  %152 = fadd <4 x float> %151, %150
  store <4 x float> %152, <4 x float>* %colorAccumulator, align 16
  br label %153

; <label>:153                                     ; preds = %145, %136
  %154 = load i32* %index_x1, align 4
  %155 = load i32* %3, align 4
  %156 = sub i32 %155, 1
  %157 = icmp ult i32 %154, %156
  br i1 %157, label %158, label %166

; <label>:158                                     ; preds = %153
  %159 = load i32* %tmpIndex2, align 4
  %160 = add i32 %159, 1
  %161 = load <4 x float> addrspace(1)** %1, align 4
  %162 = getelementptr inbounds <4 x float> addrspace(1)* %161, i32 %160
  %163 = load <4 x float> addrspace(1)* %162
  %164 = load <4 x float>* %colorAccumulator, align 16
  %165 = fadd <4 x float> %164, %163
  store <4 x float> %165, <4 x float>* %colorAccumulator, align 16
  br label %166

; <label>:166                                     ; preds = %158, %153
  br label %167

; <label>:167                                     ; preds = %166, %131
  %168 = load <4 x float>* %colorAccumulator, align 16
  %169 = load float* %denominator, align 4
  %170 = insertelement <4 x float> undef, float %169, i32 0
  %171 = shufflevector <4 x float> %170, <4 x float> %170, <4 x i32> zeroinitializer
  %172 = fdiv <4 x float> %168, %171
  %173 = load i32* %sourceIndex, align 4
  %174 = load <4 x float> addrspace(1)** %2, align 4
  %175 = getelementptr inbounds <4 x float> addrspace(1)* %174, i32 %173
  store <4 x float> %172, <4 x float> addrspace(1)* %175
  br label %176

; <label>:176                                     ; preds = %167
  %177 = load i32* %j, align 4
  %178 = add i32 %177, 1
  store i32 %178, i32* %j, align 4
  %179 = load i32* %index_x1, align 4
  %180 = add i32 %179, 1
  store i32 %180, i32* %index_x1, align 4
  br label %48

; <label>:181                                     ; preds = %48
  br label %182

; <label>:182                                     ; preds = %181
  %183 = load i32* %i, align 4
  %184 = add i32 %183, 1
  store i32 %184, i32* %i, align 4
  %185 = load i32* %index_y, align 4
  %186 = add i32 %185, 1
  store i32 %186, i32* %index_y, align 4
  br label %42

; <label>:187                                     ; preds = %42
  ret void
}

define <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %outCrd) nounwind {
  %1 = alloca %struct._image2d_t*, align 4
  %2 = alloca <2 x i32>, align 8
  %samplerNearest = alloca i32, align 4
  %outputColor = alloca <4 x float>, align 16
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %1, align 4
  store <2 x i32> %outCrd, <2 x i32>* %2, align 8
  store i32 1, i32* %samplerNearest, align 4
  %3 = load %struct._image2d_t** %1, align 4
  %4 = load <2 x i32>* %2, align 8
  %5 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %3, i32 1, <2 x i32> %4)
  store <4 x float> %5, <4 x float>* %outputColor, align 16
  %6 = load <4 x float>* %outputColor, align 16
  ret <4 x float> %6
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t*, i32, <2 x i32>)

define void @wlSimpleBoxBlur_image2d(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, i32 %rowCountPerGlobalID) nounwind {
  %1 = alloca %struct._image2d_t*, align 4
  %2 = alloca <4 x float> addrspace(1)*, align 4
  %3 = alloca i32, align 4
  %global_id = alloca i32, align 4
  %row = alloca i32, align 4
  %imgSize = alloca <2 x i32>, align 8
  %lastRow = alloca i32, align 4
  %index = alloca i32, align 4
  %denominator = alloca float, align 4
  %colorAccumulator = alloca <4 x float>, align 16
  %curCrd = alloca <2 x i32>, align 8
  %curLeftCrd = alloca <2 x i32>, align 8
  %curRightCrd = alloca <2 x i32>, align 8
  %upCrd = alloca <2 x i32>, align 8
  %upLeftCrd = alloca <2 x i32>, align 8
  %upRightCrd = alloca <2 x i32>, align 8
  %lowCrd = alloca <2 x i32>, align 8
  %lowLeftCrd = alloca <2 x i32>, align 8
  %lowRightCrd = alloca <2 x i32>, align 8
  %col = alloca i32, align 4
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %1, align 4
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %2, align 4
  store i32 %rowCountPerGlobalID, i32* %3, align 4
  %4 = call i32 @get_global_id(i32 0)
  store i32 %4, i32* %global_id, align 4
  %5 = load i32* %3, align 4
  %6 = load i32* %global_id, align 4
  %7 = mul i32 %5, %6
  store i32 %7, i32* %row, align 4
  %8 = load %struct._image2d_t** %1, align 4
  %9 = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %8)
  store <2 x i32> %9, <2 x i32>* %imgSize, align 8
  %10 = load i32* %row, align 4
  %11 = load i32* %3, align 4
  %12 = add nsw i32 %10, %11
  %13 = load <2 x i32>* %imgSize
  %14 = extractelement <2 x i32> %13, i32 1
  %15 = call i32 @_Z3minii(i32 %12, i32 %14)
  store i32 %15, i32* %lastRow, align 4
  %16 = load i32* %row, align 4
  %17 = load <2 x i32>* %imgSize
  %18 = extractelement <2 x i32> %17, i32 0
  %19 = mul nsw i32 %16, %18
  store i32 %19, i32* %index, align 4
  store float 9.000000e+00, float* %denominator, align 4
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  br label %20

; <label>:20                                      ; preds = %153, %0
  %21 = load i32* %row, align 4
  %22 = load i32* %lastRow, align 4
  %23 = icmp slt i32 %21, %22
  br i1 %23, label %24, label %156

; <label>:24                                      ; preds = %20
  %25 = load i32* %row, align 4
  %26 = load <2 x i32>* %curCrd
  %27 = insertelement <2 x i32> %26, i32 %25, i32 1
  store <2 x i32> %27, <2 x i32>* %curCrd
  %28 = load i32* %row, align 4
  %29 = load <2 x i32>* %curLeftCrd
  %30 = insertelement <2 x i32> %29, i32 %28, i32 1
  store <2 x i32> %30, <2 x i32>* %curLeftCrd
  %31 = load i32* %row, align 4
  %32 = load <2 x i32>* %curRightCrd
  %33 = insertelement <2 x i32> %32, i32 %31, i32 1
  store <2 x i32> %33, <2 x i32>* %curRightCrd
  %34 = load i32* %row, align 4
  %35 = sub nsw i32 %34, 1
  %36 = load <2 x i32>* %upCrd
  %37 = insertelement <2 x i32> %36, i32 %35, i32 1
  store <2 x i32> %37, <2 x i32>* %upCrd
  %38 = load i32* %row, align 4
  %39 = sub nsw i32 %38, 1
  %40 = load <2 x i32>* %upLeftCrd
  %41 = insertelement <2 x i32> %40, i32 %39, i32 1
  store <2 x i32> %41, <2 x i32>* %upLeftCrd
  %42 = load i32* %row, align 4
  %43 = sub nsw i32 %42, 1
  %44 = load <2 x i32>* %upRightCrd
  %45 = insertelement <2 x i32> %44, i32 %43, i32 1
  store <2 x i32> %45, <2 x i32>* %upRightCrd
  %46 = load i32* %row, align 4
  %47 = add nsw i32 %46, 1
  %48 = load <2 x i32>* %lowCrd
  %49 = insertelement <2 x i32> %48, i32 %47, i32 1
  store <2 x i32> %49, <2 x i32>* %lowCrd
  %50 = load i32* %row, align 4
  %51 = add nsw i32 %50, 1
  %52 = load <2 x i32>* %lowLeftCrd
  %53 = insertelement <2 x i32> %52, i32 %51, i32 1
  store <2 x i32> %53, <2 x i32>* %lowLeftCrd
  %54 = load i32* %row, align 4
  %55 = add nsw i32 %54, 1
  %56 = load <2 x i32>* %lowRightCrd
  %57 = insertelement <2 x i32> %56, i32 %55, i32 1
  store <2 x i32> %57, <2 x i32>* %lowRightCrd
  store i32 0, i32* %col, align 4
  br label %58

; <label>:58                                      ; preds = %149, %24
  %59 = load i32* %col, align 4
  %60 = load <2 x i32>* %imgSize
  %61 = extractelement <2 x i32> %60, i32 0
  %62 = icmp slt i32 %59, %61
  br i1 %62, label %63, label %152

; <label>:63                                      ; preds = %58
  %64 = load i32* %col, align 4
  %65 = load <2 x i32>* %curCrd
  %66 = insertelement <2 x i32> %65, i32 %64, i32 0
  store <2 x i32> %66, <2 x i32>* %curCrd
  %67 = load i32* %col, align 4
  %68 = sub nsw i32 %67, 1
  %69 = load <2 x i32>* %curLeftCrd
  %70 = insertelement <2 x i32> %69, i32 %68, i32 0
  store <2 x i32> %70, <2 x i32>* %curLeftCrd
  %71 = load i32* %col, align 4
  %72 = add nsw i32 %71, 1
  %73 = load <2 x i32>* %curRightCrd
  %74 = insertelement <2 x i32> %73, i32 %72, i32 0
  store <2 x i32> %74, <2 x i32>* %curRightCrd
  %75 = load i32* %col, align 4
  %76 = load <2 x i32>* %upCrd
  %77 = insertelement <2 x i32> %76, i32 %75, i32 0
  store <2 x i32> %77, <2 x i32>* %upCrd
  %78 = load i32* %col, align 4
  %79 = sub nsw i32 %78, 1
  %80 = load <2 x i32>* %upLeftCrd
  %81 = insertelement <2 x i32> %80, i32 %79, i32 0
  store <2 x i32> %81, <2 x i32>* %upLeftCrd
  %82 = load i32* %col, align 4
  %83 = add nsw i32 %82, 1
  %84 = load <2 x i32>* %upRightCrd
  %85 = insertelement <2 x i32> %84, i32 %83, i32 0
  store <2 x i32> %85, <2 x i32>* %upRightCrd
  %86 = load i32* %col, align 4
  %87 = load <2 x i32>* %lowCrd
  %88 = insertelement <2 x i32> %87, i32 %86, i32 0
  store <2 x i32> %88, <2 x i32>* %lowCrd
  %89 = load i32* %col, align 4
  %90 = sub nsw i32 %89, 1
  %91 = load <2 x i32>* %lowLeftCrd
  %92 = insertelement <2 x i32> %91, i32 %90, i32 0
  store <2 x i32> %92, <2 x i32>* %lowLeftCrd
  %93 = load i32* %col, align 4
  %94 = add nsw i32 %93, 1
  %95 = load <2 x i32>* %lowRightCrd
  %96 = insertelement <2 x i32> %95, i32 %94, i32 0
  store <2 x i32> %96, <2 x i32>* %lowRightCrd
  %97 = load %struct._image2d_t** %1, align 4
  %98 = load <2 x i32>* %curCrd, align 8
  %99 = call <4 x float> @evaluatePixel(%struct._image2d_t* %97, <2 x i32> %98)
  store <4 x float> %99, <4 x float>* %colorAccumulator, align 16
  %100 = load %struct._image2d_t** %1, align 4
  %101 = load <2 x i32>* %curLeftCrd, align 8
  %102 = call <4 x float> @evaluatePixel(%struct._image2d_t* %100, <2 x i32> %101)
  %103 = load <4 x float>* %colorAccumulator, align 16
  %104 = fadd <4 x float> %103, %102
  store <4 x float> %104, <4 x float>* %colorAccumulator, align 16
  %105 = load %struct._image2d_t** %1, align 4
  %106 = load <2 x i32>* %curRightCrd, align 8
  %107 = call <4 x float> @evaluatePixel(%struct._image2d_t* %105, <2 x i32> %106)
  %108 = load <4 x float>* %colorAccumulator, align 16
  %109 = fadd <4 x float> %108, %107
  store <4 x float> %109, <4 x float>* %colorAccumulator, align 16
  %110 = load %struct._image2d_t** %1, align 4
  %111 = load <2 x i32>* %upCrd, align 8
  %112 = call <4 x float> @evaluatePixel(%struct._image2d_t* %110, <2 x i32> %111)
  %113 = load <4 x float>* %colorAccumulator, align 16
  %114 = fadd <4 x float> %113, %112
  store <4 x float> %114, <4 x float>* %colorAccumulator, align 16
  %115 = load %struct._image2d_t** %1, align 4
  %116 = load <2 x i32>* %upLeftCrd, align 8
  %117 = call <4 x float> @evaluatePixel(%struct._image2d_t* %115, <2 x i32> %116)
  %118 = load <4 x float>* %colorAccumulator, align 16
  %119 = fadd <4 x float> %118, %117
  store <4 x float> %119, <4 x float>* %colorAccumulator, align 16
  %120 = load %struct._image2d_t** %1, align 4
  %121 = load <2 x i32>* %upRightCrd, align 8
  %122 = call <4 x float> @evaluatePixel(%struct._image2d_t* %120, <2 x i32> %121)
  %123 = load <4 x float>* %colorAccumulator, align 16
  %124 = fadd <4 x float> %123, %122
  store <4 x float> %124, <4 x float>* %colorAccumulator, align 16
  %125 = load %struct._image2d_t** %1, align 4
  %126 = load <2 x i32>* %lowCrd, align 8
  %127 = call <4 x float> @evaluatePixel(%struct._image2d_t* %125, <2 x i32> %126)
  %128 = load <4 x float>* %colorAccumulator, align 16
  %129 = fadd <4 x float> %128, %127
  store <4 x float> %129, <4 x float>* %colorAccumulator, align 16
  %130 = load %struct._image2d_t** %1, align 4
  %131 = load <2 x i32>* %lowLeftCrd, align 8
  %132 = call <4 x float> @evaluatePixel(%struct._image2d_t* %130, <2 x i32> %131)
  %133 = load <4 x float>* %colorAccumulator, align 16
  %134 = fadd <4 x float> %133, %132
  store <4 x float> %134, <4 x float>* %colorAccumulator, align 16
  %135 = load %struct._image2d_t** %1, align 4
  %136 = load <2 x i32>* %lowRightCrd, align 8
  %137 = call <4 x float> @evaluatePixel(%struct._image2d_t* %135, <2 x i32> %136)
  %138 = load <4 x float>* %colorAccumulator, align 16
  %139 = fadd <4 x float> %138, %137
  store <4 x float> %139, <4 x float>* %colorAccumulator, align 16
  %140 = load <4 x float>* %colorAccumulator, align 16
  %141 = load float* %denominator, align 4
  %142 = insertelement <4 x float> undef, float %141, i32 0
  %143 = shufflevector <4 x float> %142, <4 x float> %142, <4 x i32> zeroinitializer
  %144 = fdiv <4 x float> %140, %143
  %145 = load i32* %index, align 4
  %146 = add nsw i32 %145, 1
  store i32 %146, i32* %index, align 4
  %147 = load <4 x float> addrspace(1)** %2, align 4
  %148 = getelementptr inbounds <4 x float> addrspace(1)* %147, i32 %145
  store <4 x float> %144, <4 x float> addrspace(1)* %148
  br label %149

; <label>:149                                     ; preds = %63
  %150 = load i32* %col, align 4
  %151 = add nsw i32 %150, 1
  store i32 %151, i32* %col, align 4
  br label %58

; <label>:152                                     ; preds = %58
  br label %153

; <label>:153                                     ; preds = %152
  %154 = load i32* %row, align 4
  %155 = add nsw i32 %154, 1
  store i32 %155, i32* %row, align 4
  br label %20

; <label>:156                                     ; preds = %20
  ret void
}

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t*)

declare i32 @_Z3minii(i32, i32)

define void @wlSimpleBoxBlur_Optimized_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
  %1 = alloca <4 x float> addrspace(1)*, align 4
  %2 = alloca <4 x float> addrspace(1)*, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %topEdge = alloca i8, align 1
  %bottomEdge = alloca i8, align 1
  %leftEdge = alloca i8, align 1
  %rightEdge = alloca i8, align 1
  %dims = alloca i32, align 4
  %globalIdx = alloca i32, align 4
  %globalIdy = alloca i32, align 4
  %global_szx = alloca i32, align 4
  %global_szy = alloca i32, align 4
  %count_y = alloca i32, align 4
  %count_x = alloca i32, align 4
  %index_x = alloca i32, align 4
  %index_y = alloca i32, align 4
  %denominator = alloca float, align 4
  %colorAccumulator = alloca <4 x float>, align 16
  %firstBlockAccumulator = alloca <4 x float>, align 16
  %leftIndex = alloca i32, align 4
  %y = alloca i32, align 4
  %sourceIndex = alloca i32, align 4
  %leftColumnIndex = alloca i32, align 4
  %rightColumnIndex = alloca i32, align 4
  %tmpIndex1 = alloca i32, align 4
  %tmpIndex2 = alloca i32, align 4
  %topRowIndex = alloca i32, align 4
  %bottomRowIndex = alloca i32, align 4
  %row = alloca i32, align 4
  %column = alloca i32, align 4
  %column1 = alloca i32, align 4
  %column2 = alloca i32, align 4
  %row3 = alloca i32, align 4
  %column4 = alloca i32, align 4
  %row5 = alloca i32, align 4
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %1, align 4
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %2, align 4
  store i32 %width, i32* %3, align 4
  store i32 %height, i32* %4, align 4
  store i32 %buffer_size, i32* %5, align 4
  store i8 0, i8* %topEdge, align 1
  store i8 0, i8* %bottomEdge, align 1
  store i8 0, i8* %leftEdge, align 1
  store i8 0, i8* %rightEdge, align 1
  %6 = call i32 (...)* @get_work_dim()
  store i32 %6, i32* %dims, align 4
  %7 = call i32 @get_global_id(i32 0)
  store i32 %7, i32* %globalIdx, align 4
  %8 = call i32 @get_global_id(i32 1)
  store i32 %8, i32* %globalIdy, align 4
  %9 = call i32 @get_global_size(i32 0)
  store i32 %9, i32* %global_szx, align 4
  %10 = call i32 @get_global_size(i32 1)
  store i32 %10, i32* %global_szy, align 4
  %11 = load i32* %4, align 4
  %12 = load i32* %global_szy, align 4
  %13 = icmp eq i32 0, %12
  %14 = select i1 %13, i32 1, i32 %12
  %15 = udiv i32 %11, %14
  store i32 %15, i32* %count_y, align 4
  %16 = load i32* %3, align 4
  %17 = load i32* %global_szx, align 4
  %18 = icmp eq i32 0, %17
  %19 = select i1 %18, i32 1, i32 %17
  %20 = udiv i32 %16, %19
  store i32 %20, i32* %count_x, align 4
  %21 = load i32* %3, align 4
  %22 = load i32* %globalIdx, align 4
  %23 = mul i32 %21, %22
  %24 = load i32* %global_szx, align 4
  %25 = icmp eq i32 0, %24
  %26 = select i1 %25, i32 1, i32 %24
  %27 = udiv i32 %23, %26
  store i32 %27, i32* %index_x, align 4
  %28 = load i32* %4, align 4
  %29 = load i32* %globalIdy, align 4
  %30 = mul i32 %28, %29
  %31 = load i32* %global_szy, align 4
  %32 = icmp eq i32 0, %31
  %33 = select i1 %32, i32 1, i32 %31
  %34 = udiv i32 %30, %33
  store i32 %34, i32* %index_y, align 4
  %35 = load i32* %index_y, align 4
  %36 = load i32* %count_y, align 4
  %37 = add i32 %35, %36
  %38 = add i32 %37, 1
  %39 = load i32* %4, align 4
  %40 = icmp uge i32 %38, %39
  br i1 %40, label %41, label %46

; <label>:41                                      ; preds = %0
  store i8 1, i8* %bottomEdge, align 1
  %42 = load i32* %4, align 4
  %43 = load i32* %index_y, align 4
  %44 = sub i32 %42, %43
  %45 = sub i32 %44, 1
  store i32 %45, i32* %count_y, align 4
  br label %46

; <label>:46                                      ; preds = %41, %0
  %47 = load i32* %index_x, align 4
  %48 = load i32* %count_x, align 4
  %49 = add i32 %47, %48
  %50 = add i32 %49, 1
  %51 = load i32* %3, align 4
  %52 = icmp uge i32 %50, %51
  br i1 %52, label %53, label %58

; <label>:53                                      ; preds = %46
  store i8 1, i8* %rightEdge, align 1
  %54 = load i32* %3, align 4
  %55 = load i32* %index_x, align 4
  %56 = sub i32 %54, %55
  %57 = sub i32 %56, 1
  store i32 %57, i32* %count_x, align 4
  br label %58

; <label>:58                                      ; preds = %53, %46
  %59 = load i32* %index_y, align 4
  %60 = icmp ult i32 %59, 1
  br i1 %60, label %61, label %64

; <label>:61                                      ; preds = %58
  store i8 1, i8* %topEdge, align 1
  store i32 1, i32* %index_y, align 4
  %62 = load i32* %count_y, align 4
  %63 = sub i32 %62, 1
  store i32 %63, i32* %count_y, align 4
  br label %64

; <label>:64                                      ; preds = %61, %58
  %65 = load i32* %index_x, align 4
  %66 = icmp ult i32 %65, 1
  br i1 %66, label %67, label %70

; <label>:67                                      ; preds = %64
  store i8 1, i8* %leftEdge, align 1
  store i32 1, i32* %index_x, align 4
  %68 = load i32* %count_x, align 4
  %69 = sub i32 %68, 1
  store i32 %69, i32* %count_x, align 4
  br label %70

; <label>:70                                      ; preds = %67, %64
  store float 9.000000e+00, float* %denominator, align 4
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  store <4 x float> zeroinitializer, <4 x float>* %firstBlockAccumulator, align 16
  %71 = load i32* %index_y, align 4
  %72 = sub i32 %71, 1
  %73 = load i32* %3, align 4
  %74 = mul i32 %72, %73
  %75 = load i32* %index_x, align 4
  %76 = add i32 %74, %75
  %77 = sub i32 %76, 1
  store i32 %77, i32* %leftIndex, align 4
  store i32 0, i32* %y, align 4
  br label %78

; <label>:78                                      ; preds = %105, %70
  %79 = load i32* %y, align 4
  %80 = icmp ult i32 %79, 3
  br i1 %80, label %81, label %108

; <label>:81                                      ; preds = %78
  %82 = load i32* %leftIndex, align 4
  %83 = load <4 x float> addrspace(1)** %1, align 4
  %84 = getelementptr inbounds <4 x float> addrspace(1)* %83, i32 %82
  %85 = load <4 x float> addrspace(1)* %84
  %86 = load <4 x float>* %firstBlockAccumulator, align 16
  %87 = fadd <4 x float> %86, %85
  store <4 x float> %87, <4 x float>* %firstBlockAccumulator, align 16
  %88 = load i32* %leftIndex, align 4
  %89 = add i32 %88, 1
  %90 = load <4 x float> addrspace(1)** %1, align 4
  %91 = getelementptr inbounds <4 x float> addrspace(1)* %90, i32 %89
  %92 = load <4 x float> addrspace(1)* %91
  %93 = load <4 x float>* %firstBlockAccumulator, align 16
  %94 = fadd <4 x float> %93, %92
  store <4 x float> %94, <4 x float>* %firstBlockAccumulator, align 16
  %95 = load i32* %leftIndex, align 4
  %96 = add i32 %95, 2
  %97 = load <4 x float> addrspace(1)** %1, align 4
  %98 = getelementptr inbounds <4 x float> addrspace(1)* %97, i32 %96
  %99 = load <4 x float> addrspace(1)* %98
  %100 = load <4 x float>* %firstBlockAccumulator, align 16
  %101 = fadd <4 x float> %100, %99
  store <4 x float> %101, <4 x float>* %firstBlockAccumulator, align 16
  %102 = load i32* %3, align 4
  %103 = load i32* %leftIndex, align 4
  %104 = add i32 %103, %102
  store i32 %104, i32* %leftIndex, align 4
  br label %105

; <label>:105                                     ; preds = %81
  %106 = load i32* %y, align 4
  %107 = add i32 %106, 1
  store i32 %107, i32* %y, align 4
  br label %78

; <label>:108                                     ; preds = %78
  %109 = load i32* %index_y, align 4
  %110 = load i32* %3, align 4
  %111 = mul i32 %109, %110
  %112 = load i32* %index_x, align 4
  %113 = add i32 %111, %112
  store i32 %113, i32* %sourceIndex, align 4
  %114 = load <4 x float>* %firstBlockAccumulator, align 16
  %115 = load float* %denominator, align 4
  %116 = insertelement <4 x float> undef, float %115, i32 0
  %117 = shufflevector <4 x float> %116, <4 x float> %116, <4 x i32> zeroinitializer
  %118 = fdiv <4 x float> %114, %117
  %119 = load i32* %sourceIndex, align 4
  %120 = load <4 x float> addrspace(1)** %2, align 4
  %121 = getelementptr inbounds <4 x float> addrspace(1)* %120, i32 %119
  store <4 x float> %118, <4 x float> addrspace(1)* %121
  %122 = load <4 x float>* %firstBlockAccumulator, align 16
  store <4 x float> %122, <4 x float>* %colorAccumulator, align 16
  %123 = load i32* %sourceIndex, align 4
  %124 = add i32 %123, 1
  store i32 %124, i32* %sourceIndex, align 4
  %125 = load i32* %index_y, align 4
  %126 = sub i32 %125, 1
  %127 = load i32* %3, align 4
  %128 = mul i32 %126, %127
  %129 = load i32* %index_x, align 4
  %130 = add i32 %128, %129
  %131 = sub i32 %130, 1
  store i32 %131, i32* %topRowIndex, align 4
  %132 = load i32* %index_y, align 4
  %133 = add i32 %132, 1
  %134 = load i32* %3, align 4
  %135 = mul i32 %133, %134
  %136 = load i32* %index_x, align 4
  %137 = add i32 %135, %136
  %138 = sub i32 %137, 1
  store i32 %138, i32* %bottomRowIndex, align 4
  store i32 0, i32* %row, align 4
  br label %139

; <label>:139                                     ; preds = %220, %108
  %140 = load i32* %row, align 4
  %141 = load i32* %count_y, align 4
  %142 = sub i32 %141, 1
  %143 = icmp ult i32 %140, %142
  br i1 %143, label %144, label %293

; <label>:144                                     ; preds = %139
  %145 = load i32* %topRowIndex, align 4
  store i32 %145, i32* %leftColumnIndex, align 4
  %146 = load i32* %topRowIndex, align 4
  %147 = add i32 %146, 2
  store i32 %147, i32* %rightColumnIndex, align 4
  store i32 1, i32* %column, align 4
  br label %148

; <label>:148                                     ; preds = %217, %144
  %149 = load i32* %column, align 4
  %150 = load i32* %count_x, align 4
  %151 = icmp ult i32 %149, %150
  br i1 %151, label %152, label %220

; <label>:152                                     ; preds = %148
  %153 = load i32* %rightColumnIndex, align 4
  %154 = add i32 %153, 1
  store i32 %154, i32* %rightColumnIndex, align 4
  %155 = load i32* %leftColumnIndex, align 4
  store i32 %155, i32* %tmpIndex1, align 4
  %156 = load i32* %rightColumnIndex, align 4
  store i32 %156, i32* %tmpIndex2, align 4
  %157 = load i32* %tmpIndex1, align 4
  %158 = load <4 x float> addrspace(1)** %1, align 4
  %159 = getelementptr inbounds <4 x float> addrspace(1)* %158, i32 %157
  %160 = load <4 x float> addrspace(1)* %159
  %161 = load <4 x float>* %colorAccumulator, align 16
  %162 = fsub <4 x float> %161, %160
  store <4 x float> %162, <4 x float>* %colorAccumulator, align 16
  %163 = load i32* %3, align 4
  %164 = load i32* %tmpIndex1, align 4
  %165 = add i32 %164, %163
  store i32 %165, i32* %tmpIndex1, align 4
  %166 = load i32* %tmpIndex2, align 4
  %167 = load <4 x float> addrspace(1)** %1, align 4
  %168 = getelementptr inbounds <4 x float> addrspace(1)* %167, i32 %166
  %169 = load <4 x float> addrspace(1)* %168
  %170 = load <4 x float>* %colorAccumulator, align 16
  %171 = fadd <4 x float> %170, %169
  store <4 x float> %171, <4 x float>* %colorAccumulator, align 16
  %172 = load i32* %3, align 4
  %173 = load i32* %tmpIndex2, align 4
  %174 = add i32 %173, %172
  store i32 %174, i32* %tmpIndex2, align 4
  %175 = load i32* %tmpIndex1, align 4
  %176 = load <4 x float> addrspace(1)** %1, align 4
  %177 = getelementptr inbounds <4 x float> addrspace(1)* %176, i32 %175
  %178 = load <4 x float> addrspace(1)* %177
  %179 = load <4 x float>* %colorAccumulator, align 16
  %180 = fsub <4 x float> %179, %178
  store <4 x float> %180, <4 x float>* %colorAccumulator, align 16
  %181 = load i32* %3, align 4
  %182 = load i32* %tmpIndex1, align 4
  %183 = add i32 %182, %181
  store i32 %183, i32* %tmpIndex1, align 4
  %184 = load i32* %tmpIndex2, align 4
  %185 = load <4 x float> addrspace(1)** %1, align 4
  %186 = getelementptr inbounds <4 x float> addrspace(1)* %185, i32 %184
  %187 = load <4 x float> addrspace(1)* %186
  %188 = load <4 x float>* %colorAccumulator, align 16
  %189 = fadd <4 x float> %188, %187
  store <4 x float> %189, <4 x float>* %colorAccumulator, align 16
  %190 = load i32* %3, align 4
  %191 = load i32* %tmpIndex2, align 4
  %192 = add i32 %191, %190
  store i32 %192, i32* %tmpIndex2, align 4
  %193 = load i32* %tmpIndex1, align 4
  %194 = load <4 x float> addrspace(1)** %1, align 4
  %195 = getelementptr inbounds <4 x float> addrspace(1)* %194, i32 %193
  %196 = load <4 x float> addrspace(1)* %195
  %197 = load <4 x float>* %colorAccumulator, align 16
  %198 = fsub <4 x float> %197, %196
  store <4 x float> %198, <4 x float>* %colorAccumulator, align 16
  %199 = load i32* %tmpIndex2, align 4
  %200 = load <4 x float> addrspace(1)** %1, align 4
  %201 = getelementptr inbounds <4 x float> addrspace(1)* %200, i32 %199
  %202 = load <4 x float> addrspace(1)* %201
  %203 = load <4 x float>* %colorAccumulator, align 16
  %204 = fadd <4 x float> %203, %202
  store <4 x float> %204, <4 x float>* %colorAccumulator, align 16
  %205 = load <4 x float>* %colorAccumulator, align 16
  %206 = load float* %denominator, align 4
  %207 = insertelement <4 x float> undef, float %206, i32 0
  %208 = shufflevector <4 x float> %207, <4 x float> %207, <4 x i32> zeroinitializer
  %209 = fdiv <4 x float> %205, %208
  %210 = load i32* %sourceIndex, align 4
  %211 = load <4 x float> addrspace(1)** %2, align 4
  %212 = getelementptr inbounds <4 x float> addrspace(1)* %211, i32 %210
  store <4 x float> %209, <4 x float> addrspace(1)* %212
  %213 = load i32* %sourceIndex, align 4
  %214 = add i32 %213, 1
  store i32 %214, i32* %sourceIndex, align 4
  %215 = load i32* %leftColumnIndex, align 4
  %216 = add i32 %215, 1
  store i32 %216, i32* %leftColumnIndex, align 4
  br label %217

; <label>:217                                     ; preds = %152
  %218 = load i32* %column, align 4
  %219 = add i32 %218, 1
  store i32 %219, i32* %column, align 4
  br label %148

; <label>:220                                     ; preds = %148
  %221 = load i32* %3, align 4
  %222 = load i32* %bottomRowIndex, align 4
  %223 = add i32 %222, %221
  store i32 %223, i32* %bottomRowIndex, align 4
  %224 = load i32* %row, align 4
  %225 = add i32 %224, 1
  store i32 %225, i32* %row, align 4
  %226 = load i32* %topRowIndex, align 4
  store i32 %226, i32* %tmpIndex1, align 4
  %227 = load i32* %bottomRowIndex, align 4
  store i32 %227, i32* %tmpIndex2, align 4
  %228 = load i32* %tmpIndex1, align 4
  %229 = load <4 x float> addrspace(1)** %1, align 4
  %230 = getelementptr inbounds <4 x float> addrspace(1)* %229, i32 %228
  %231 = load <4 x float> addrspace(1)* %230
  %232 = load <4 x float>* %firstBlockAccumulator, align 16
  %233 = fsub <4 x float> %232, %231
  store <4 x float> %233, <4 x float>* %firstBlockAccumulator, align 16
  %234 = load i32* %tmpIndex1, align 4
  %235 = add i32 %234, 1
  store i32 %235, i32* %tmpIndex1, align 4
  %236 = load i32* %tmpIndex2, align 4
  %237 = load <4 x float> addrspace(1)** %1, align 4
  %238 = getelementptr inbounds <4 x float> addrspace(1)* %237, i32 %236
  %239 = load <4 x float> addrspace(1)* %238
  %240 = load <4 x float>* %firstBlockAccumulator, align 16
  %241 = fadd <4 x float> %240, %239
  store <4 x float> %241, <4 x float>* %firstBlockAccumulator, align 16
  %242 = load i32* %tmpIndex2, align 4
  %243 = add i32 %242, 1
  store i32 %243, i32* %tmpIndex2, align 4
  %244 = load i32* %tmpIndex1, align 4
  %245 = load <4 x float> addrspace(1)** %1, align 4
  %246 = getelementptr inbounds <4 x float> addrspace(1)* %245, i32 %244
  %247 = load <4 x float> addrspace(1)* %246
  %248 = load <4 x float>* %firstBlockAccumulator, align 16
  %249 = fsub <4 x float> %248, %247
  store <4 x float> %249, <4 x float>* %firstBlockAccumulator, align 16
  %250 = load i32* %tmpIndex1, align 4
  %251 = add i32 %250, 1
  store i32 %251, i32* %tmpIndex1, align 4
  %252 = load i32* %tmpIndex2, align 4
  %253 = load <4 x float> addrspace(1)** %1, align 4
  %254 = getelementptr inbounds <4 x float> addrspace(1)* %253, i32 %252
  %255 = load <4 x float> addrspace(1)* %254
  %256 = load <4 x float>* %firstBlockAccumulator, align 16
  %257 = fadd <4 x float> %256, %255
  store <4 x float> %257, <4 x float>* %firstBlockAccumulator, align 16
  %258 = load i32* %tmpIndex2, align 4
  %259 = add i32 %258, 1
  store i32 %259, i32* %tmpIndex2, align 4
  %260 = load i32* %tmpIndex1, align 4
  %261 = load <4 x float> addrspace(1)** %1, align 4
  %262 = getelementptr inbounds <4 x float> addrspace(1)* %261, i32 %260
  %263 = load <4 x float> addrspace(1)* %262
  %264 = load <4 x float>* %firstBlockAccumulator, align 16
  %265 = fsub <4 x float> %264, %263
  store <4 x float> %265, <4 x float>* %firstBlockAccumulator, align 16
  %266 = load i32* %tmpIndex2, align 4
  %267 = load <4 x float> addrspace(1)** %1, align 4
  %268 = getelementptr inbounds <4 x float> addrspace(1)* %267, i32 %266
  %269 = load <4 x float> addrspace(1)* %268
  %270 = load <4 x float>* %firstBlockAccumulator, align 16
  %271 = fadd <4 x float> %270, %269
  store <4 x float> %271, <4 x float>* %firstBlockAccumulator, align 16
  %272 = load i32* %index_y, align 4
  %273 = load i32* %row, align 4
  %274 = add i32 %272, %273
  %275 = load i32* %3, align 4
  %276 = mul i32 %274, %275
  %277 = load i32* %index_x, align 4
  %278 = add i32 %276, %277
  store i32 %278, i32* %sourceIndex, align 4
  %279 = load <4 x float>* %firstBlockAccumulator, align 16
  %280 = load float* %denominator, align 4
  %281 = insertelement <4 x float> undef, float %280, i32 0
  %282 = shufflevector <4 x float> %281, <4 x float> %281, <4 x i32> zeroinitializer
  %283 = fdiv <4 x float> %279, %282
  %284 = load i32* %sourceIndex, align 4
  %285 = load <4 x float> addrspace(1)** %2, align 4
  %286 = getelementptr inbounds <4 x float> addrspace(1)* %285, i32 %284
  store <4 x float> %283, <4 x float> addrspace(1)* %286
  %287 = load <4 x float>* %firstBlockAccumulator, align 16
  store <4 x float> %287, <4 x float>* %colorAccumulator, align 16
  %288 = load i32* %sourceIndex, align 4
  %289 = add i32 %288, 1
  store i32 %289, i32* %sourceIndex, align 4
  %290 = load i32* %3, align 4
  %291 = load i32* %topRowIndex, align 4
  %292 = add i32 %291, %290
  store i32 %292, i32* %topRowIndex, align 4
  br label %139

; <label>:293                                     ; preds = %139
  %294 = load i32* %topRowIndex, align 4
  store i32 %294, i32* %leftColumnIndex, align 4
  %295 = load i32* %topRowIndex, align 4
  %296 = add i32 %295, 2
  store i32 %296, i32* %rightColumnIndex, align 4
  store i32 1, i32* %column1, align 4
  br label %297

; <label>:297                                     ; preds = %366, %293
  %298 = load i32* %column1, align 4
  %299 = load i32* %count_x, align 4
  %300 = icmp ult i32 %298, %299
  br i1 %300, label %301, label %369

; <label>:301                                     ; preds = %297
  %302 = load i32* %rightColumnIndex, align 4
  %303 = add i32 %302, 1
  store i32 %303, i32* %rightColumnIndex, align 4
  %304 = load i32* %leftColumnIndex, align 4
  store i32 %304, i32* %tmpIndex1, align 4
  %305 = load i32* %rightColumnIndex, align 4
  store i32 %305, i32* %tmpIndex2, align 4
  %306 = load i32* %tmpIndex1, align 4
  %307 = load <4 x float> addrspace(1)** %1, align 4
  %308 = getelementptr inbounds <4 x float> addrspace(1)* %307, i32 %306
  %309 = load <4 x float> addrspace(1)* %308
  %310 = load <4 x float>* %colorAccumulator, align 16
  %311 = fsub <4 x float> %310, %309
  store <4 x float> %311, <4 x float>* %colorAccumulator, align 16
  %312 = load i32* %3, align 4
  %313 = load i32* %tmpIndex1, align 4
  %314 = add i32 %313, %312
  store i32 %314, i32* %tmpIndex1, align 4
  %315 = load i32* %tmpIndex2, align 4
  %316 = load <4 x float> addrspace(1)** %1, align 4
  %317 = getelementptr inbounds <4 x float> addrspace(1)* %316, i32 %315
  %318 = load <4 x float> addrspace(1)* %317
  %319 = load <4 x float>* %colorAccumulator, align 16
  %320 = fadd <4 x float> %319, %318
  store <4 x float> %320, <4 x float>* %colorAccumulator, align 16
  %321 = load i32* %3, align 4
  %322 = load i32* %tmpIndex2, align 4
  %323 = add i32 %322, %321
  store i32 %323, i32* %tmpIndex2, align 4
  %324 = load i32* %tmpIndex1, align 4
  %325 = load <4 x float> addrspace(1)** %1, align 4
  %326 = getelementptr inbounds <4 x float> addrspace(1)* %325, i32 %324
  %327 = load <4 x float> addrspace(1)* %326
  %328 = load <4 x float>* %colorAccumulator, align 16
  %329 = fsub <4 x float> %328, %327
  store <4 x float> %329, <4 x float>* %colorAccumulator, align 16
  %330 = load i32* %3, align 4
  %331 = load i32* %tmpIndex1, align 4
  %332 = add i32 %331, %330
  store i32 %332, i32* %tmpIndex1, align 4
  %333 = load i32* %tmpIndex2, align 4
  %334 = load <4 x float> addrspace(1)** %1, align 4
  %335 = getelementptr inbounds <4 x float> addrspace(1)* %334, i32 %333
  %336 = load <4 x float> addrspace(1)* %335
  %337 = load <4 x float>* %colorAccumulator, align 16
  %338 = fadd <4 x float> %337, %336
  store <4 x float> %338, <4 x float>* %colorAccumulator, align 16
  %339 = load i32* %3, align 4
  %340 = load i32* %tmpIndex2, align 4
  %341 = add i32 %340, %339
  store i32 %341, i32* %tmpIndex2, align 4
  %342 = load i32* %tmpIndex1, align 4
  %343 = load <4 x float> addrspace(1)** %1, align 4
  %344 = getelementptr inbounds <4 x float> addrspace(1)* %343, i32 %342
  %345 = load <4 x float> addrspace(1)* %344
  %346 = load <4 x float>* %colorAccumulator, align 16
  %347 = fsub <4 x float> %346, %345
  store <4 x float> %347, <4 x float>* %colorAccumulator, align 16
  %348 = load i32* %tmpIndex2, align 4
  %349 = load <4 x float> addrspace(1)** %1, align 4
  %350 = getelementptr inbounds <4 x float> addrspace(1)* %349, i32 %348
  %351 = load <4 x float> addrspace(1)* %350
  %352 = load <4 x float>* %colorAccumulator, align 16
  %353 = fadd <4 x float> %352, %351
  store <4 x float> %353, <4 x float>* %colorAccumulator, align 16
  %354 = load <4 x float>* %colorAccumulator, align 16
  %355 = load float* %denominator, align 4
  %356 = insertelement <4 x float> undef, float %355, i32 0
  %357 = shufflevector <4 x float> %356, <4 x float> %356, <4 x i32> zeroinitializer
  %358 = fdiv <4 x float> %354, %357
  %359 = load i32* %sourceIndex, align 4
  %360 = load <4 x float> addrspace(1)** %2, align 4
  %361 = getelementptr inbounds <4 x float> addrspace(1)* %360, i32 %359
  store <4 x float> %358, <4 x float> addrspace(1)* %361
  %362 = load i32* %sourceIndex, align 4
  %363 = add i32 %362, 1
  store i32 %363, i32* %sourceIndex, align 4
  %364 = load i32* %leftColumnIndex, align 4
  %365 = add i32 %364, 1
  store i32 %365, i32* %leftColumnIndex, align 4
  br label %366

; <label>:366                                     ; preds = %301
  %367 = load i32* %column1, align 4
  %368 = add i32 %367, 1
  store i32 %368, i32* %column1, align 4
  br label %297

; <label>:369                                     ; preds = %297
  %370 = load i8* %topEdge, align 1
  %371 = trunc i8 %370 to i1
  br i1 %371, label %372, label %406

; <label>:372                                     ; preds = %369
  %373 = load i8* %leftEdge, align 1
  %374 = trunc i8 %373 to i1
  br i1 %374, label %375, label %406

; <label>:375                                     ; preds = %372
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  %376 = load <4 x float> addrspace(1)** %1, align 4
  %377 = getelementptr inbounds <4 x float> addrspace(1)* %376, i32 0
  %378 = load <4 x float> addrspace(1)* %377
  %379 = load <4 x float>* %colorAccumulator, align 16
  %380 = fadd <4 x float> %379, %378
  store <4 x float> %380, <4 x float>* %colorAccumulator, align 16
  %381 = load <4 x float> addrspace(1)** %1, align 4
  %382 = getelementptr inbounds <4 x float> addrspace(1)* %381, i32 1
  %383 = load <4 x float> addrspace(1)* %382
  %384 = load <4 x float>* %colorAccumulator, align 16
  %385 = fadd <4 x float> %384, %383
  store <4 x float> %385, <4 x float>* %colorAccumulator, align 16
  %386 = load i32* %3, align 4
  %387 = load <4 x float> addrspace(1)** %1, align 4
  %388 = getelementptr inbounds <4 x float> addrspace(1)* %387, i32 %386
  %389 = load <4 x float> addrspace(1)* %388
  %390 = load <4 x float>* %colorAccumulator, align 16
  %391 = fadd <4 x float> %390, %389
  store <4 x float> %391, <4 x float>* %colorAccumulator, align 16
  %392 = load i32* %3, align 4
  %393 = add i32 %392, 1
  %394 = load <4 x float> addrspace(1)** %1, align 4
  %395 = getelementptr inbounds <4 x float> addrspace(1)* %394, i32 %393
  %396 = load <4 x float> addrspace(1)* %395
  %397 = load <4 x float>* %colorAccumulator, align 16
  %398 = fadd <4 x float> %397, %396
  store <4 x float> %398, <4 x float>* %colorAccumulator, align 16
  %399 = load <4 x float>* %colorAccumulator, align 16
  %400 = load float* %denominator, align 4
  %401 = insertelement <4 x float> undef, float %400, i32 0
  %402 = shufflevector <4 x float> %401, <4 x float> %401, <4 x i32> zeroinitializer
  %403 = fdiv <4 x float> %399, %402
  %404 = load <4 x float> addrspace(1)** %2, align 4
  %405 = getelementptr inbounds <4 x float> addrspace(1)* %404, i32 0
  store <4 x float> %403, <4 x float> addrspace(1)* %405
  br label %406

; <label>:406                                     ; preds = %375, %372, %369
  %407 = load i8* %topEdge, align 1
  %408 = trunc i8 %407 to i1
  br i1 %408, label %409, label %476

; <label>:409                                     ; preds = %406
  %410 = load i32* %index_x, align 4
  store i32 %410, i32* %column2, align 4
  br label %411

; <label>:411                                     ; preds = %472, %409
  %412 = load i32* %column2, align 4
  %413 = load i32* %index_x, align 4
  %414 = load i32* %count_x, align 4
  %415 = add i32 %413, %414
  %416 = icmp ult i32 %412, %415
  br i1 %416, label %417, label %475

; <label>:417                                     ; preds = %411
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  %418 = load i32* %column2, align 4
  %419 = sub nsw i32 %418, 1
  %420 = load <4 x float> addrspace(1)** %1, align 4
  %421 = getelementptr inbounds <4 x float> addrspace(1)* %420, i32 %419
  %422 = load <4 x float> addrspace(1)* %421
  %423 = load <4 x float>* %colorAccumulator, align 16
  %424 = fadd <4 x float> %423, %422
  store <4 x float> %424, <4 x float>* %colorAccumulator, align 16
  %425 = load i32* %column2, align 4
  %426 = load <4 x float> addrspace(1)** %1, align 4
  %427 = getelementptr inbounds <4 x float> addrspace(1)* %426, i32 %425
  %428 = load <4 x float> addrspace(1)* %427
  %429 = load <4 x float>* %colorAccumulator, align 16
  %430 = fadd <4 x float> %429, %428
  store <4 x float> %430, <4 x float>* %colorAccumulator, align 16
  %431 = load i32* %column2, align 4
  %432 = add nsw i32 %431, 1
  %433 = load <4 x float> addrspace(1)** %1, align 4
  %434 = getelementptr inbounds <4 x float> addrspace(1)* %433, i32 %432
  %435 = load <4 x float> addrspace(1)* %434
  %436 = load <4 x float>* %colorAccumulator, align 16
  %437 = fadd <4 x float> %436, %435
  store <4 x float> %437, <4 x float>* %colorAccumulator, align 16
  %438 = load i32* %3, align 4
  %439 = load i32* %column2, align 4
  %440 = add i32 %438, %439
  %441 = sub i32 %440, 1
  %442 = load <4 x float> addrspace(1)** %1, align 4
  %443 = getelementptr inbounds <4 x float> addrspace(1)* %442, i32 %441
  %444 = load <4 x float> addrspace(1)* %443
  %445 = load <4 x float>* %colorAccumulator, align 16
  %446 = fadd <4 x float> %445, %444
  store <4 x float> %446, <4 x float>* %colorAccumulator, align 16
  %447 = load i32* %3, align 4
  %448 = load i32* %column2, align 4
  %449 = add i32 %447, %448
  %450 = load <4 x float> addrspace(1)** %1, align 4
  %451 = getelementptr inbounds <4 x float> addrspace(1)* %450, i32 %449
  %452 = load <4 x float> addrspace(1)* %451
  %453 = load <4 x float>* %colorAccumulator, align 16
  %454 = fadd <4 x float> %453, %452
  store <4 x float> %454, <4 x float>* %colorAccumulator, align 16
  %455 = load i32* %3, align 4
  %456 = load i32* %column2, align 4
  %457 = add i32 %455, %456
  %458 = add i32 %457, 1
  %459 = load <4 x float> addrspace(1)** %1, align 4
  %460 = getelementptr inbounds <4 x float> addrspace(1)* %459, i32 %458
  %461 = load <4 x float> addrspace(1)* %460
  %462 = load <4 x float>* %colorAccumulator, align 16
  %463 = fadd <4 x float> %462, %461
  store <4 x float> %463, <4 x float>* %colorAccumulator, align 16
  %464 = load <4 x float>* %colorAccumulator, align 16
  %465 = load float* %denominator, align 4
  %466 = insertelement <4 x float> undef, float %465, i32 0
  %467 = shufflevector <4 x float> %466, <4 x float> %466, <4 x i32> zeroinitializer
  %468 = fdiv <4 x float> %464, %467
  %469 = load i32* %column2, align 4
  %470 = load <4 x float> addrspace(1)** %2, align 4
  %471 = getelementptr inbounds <4 x float> addrspace(1)* %470, i32 %469
  store <4 x float> %468, <4 x float> addrspace(1)* %471
  br label %472

; <label>:472                                     ; preds = %417
  %473 = load i32* %column2, align 4
  %474 = add nsw i32 %473, 1
  store i32 %474, i32* %column2, align 4
  br label %411

; <label>:475                                     ; preds = %411
  br label %476

; <label>:476                                     ; preds = %475, %406
  %477 = load i8* %topEdge, align 1
  %478 = trunc i8 %477 to i1
  br i1 %478, label %479, label %524

; <label>:479                                     ; preds = %476
  %480 = load i8* %rightEdge, align 1
  %481 = trunc i8 %480 to i1
  br i1 %481, label %482, label %524

; <label>:482                                     ; preds = %479
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  %483 = load i32* %3, align 4
  %484 = sub i32 %483, 2
  %485 = load <4 x float> addrspace(1)** %1, align 4
  %486 = getelementptr inbounds <4 x float> addrspace(1)* %485, i32 %484
  %487 = load <4 x float> addrspace(1)* %486
  %488 = load <4 x float>* %colorAccumulator, align 16
  %489 = fadd <4 x float> %488, %487
  store <4 x float> %489, <4 x float>* %colorAccumulator, align 16
  %490 = load i32* %3, align 4
  %491 = sub i32 %490, 1
  %492 = load <4 x float> addrspace(1)** %1, align 4
  %493 = getelementptr inbounds <4 x float> addrspace(1)* %492, i32 %491
  %494 = load <4 x float> addrspace(1)* %493
  %495 = load <4 x float>* %colorAccumulator, align 16
  %496 = fadd <4 x float> %495, %494
  store <4 x float> %496, <4 x float>* %colorAccumulator, align 16
  %497 = load i32* %3, align 4
  %498 = load i32* %3, align 4
  %499 = add i32 %497, %498
  %500 = sub i32 %499, 2
  %501 = load <4 x float> addrspace(1)** %1, align 4
  %502 = getelementptr inbounds <4 x float> addrspace(1)* %501, i32 %500
  %503 = load <4 x float> addrspace(1)* %502
  %504 = load <4 x float>* %colorAccumulator, align 16
  %505 = fadd <4 x float> %504, %503
  store <4 x float> %505, <4 x float>* %colorAccumulator, align 16
  %506 = load i32* %3, align 4
  %507 = load i32* %3, align 4
  %508 = add i32 %506, %507
  %509 = sub i32 %508, 1
  %510 = load <4 x float> addrspace(1)** %1, align 4
  %511 = getelementptr inbounds <4 x float> addrspace(1)* %510, i32 %509
  %512 = load <4 x float> addrspace(1)* %511
  %513 = load <4 x float>* %colorAccumulator, align 16
  %514 = fadd <4 x float> %513, %512
  store <4 x float> %514, <4 x float>* %colorAccumulator, align 16
  %515 = load <4 x float>* %colorAccumulator, align 16
  %516 = load float* %denominator, align 4
  %517 = insertelement <4 x float> undef, float %516, i32 0
  %518 = shufflevector <4 x float> %517, <4 x float> %517, <4 x i32> zeroinitializer
  %519 = fdiv <4 x float> %515, %518
  %520 = load i32* %3, align 4
  %521 = sub i32 %520, 1
  %522 = load <4 x float> addrspace(1)** %2, align 4
  %523 = getelementptr inbounds <4 x float> addrspace(1)* %522, i32 %521
  store <4 x float> %519, <4 x float> addrspace(1)* %523
  br label %524

; <label>:524                                     ; preds = %482, %479, %476
  %525 = load i8* %leftEdge, align 1
  %526 = trunc i8 %525 to i1
  br i1 %526, label %527, label %605

; <label>:527                                     ; preds = %524
  %528 = load i32* %index_y, align 4
  store i32 %528, i32* %row3, align 4
  br label %529

; <label>:529                                     ; preds = %601, %527
  %530 = load i32* %row3, align 4
  %531 = load i32* %index_y, align 4
  %532 = load i32* %count_y, align 4
  %533 = add i32 %531, %532
  %534 = icmp ult i32 %530, %533
  br i1 %534, label %535, label %604

; <label>:535                                     ; preds = %529
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  %536 = load i32* %row3, align 4
  %537 = sub nsw i32 %536, 1
  %538 = load i32* %3, align 4
  %539 = mul i32 %537, %538
  %540 = load <4 x float> addrspace(1)** %1, align 4
  %541 = getelementptr inbounds <4 x float> addrspace(1)* %540, i32 %539
  %542 = load <4 x float> addrspace(1)* %541
  %543 = load <4 x float>* %colorAccumulator, align 16
  %544 = fadd <4 x float> %543, %542
  store <4 x float> %544, <4 x float>* %colorAccumulator, align 16
  %545 = load i32* %row3, align 4
  %546 = load i32* %3, align 4
  %547 = mul i32 %545, %546
  %548 = load <4 x float> addrspace(1)** %1, align 4
  %549 = getelementptr inbounds <4 x float> addrspace(1)* %548, i32 %547
  %550 = load <4 x float> addrspace(1)* %549
  %551 = load <4 x float>* %colorAccumulator, align 16
  %552 = fadd <4 x float> %551, %550
  store <4 x float> %552, <4 x float>* %colorAccumulator, align 16
  %553 = load i32* %row3, align 4
  %554 = add nsw i32 %553, 1
  %555 = load i32* %3, align 4
  %556 = mul i32 %554, %555
  %557 = load <4 x float> addrspace(1)** %1, align 4
  %558 = getelementptr inbounds <4 x float> addrspace(1)* %557, i32 %556
  %559 = load <4 x float> addrspace(1)* %558
  %560 = load <4 x float>* %colorAccumulator, align 16
  %561 = fadd <4 x float> %560, %559
  store <4 x float> %561, <4 x float>* %colorAccumulator, align 16
  %562 = load i32* %row3, align 4
  %563 = sub nsw i32 %562, 1
  %564 = load i32* %3, align 4
  %565 = mul i32 %563, %564
  %566 = add i32 %565, 1
  %567 = load <4 x float> addrspace(1)** %1, align 4
  %568 = getelementptr inbounds <4 x float> addrspace(1)* %567, i32 %566
  %569 = load <4 x float> addrspace(1)* %568
  %570 = load <4 x float>* %colorAccumulator, align 16
  %571 = fadd <4 x float> %570, %569
  store <4 x float> %571, <4 x float>* %colorAccumulator, align 16
  %572 = load i32* %row3, align 4
  %573 = load i32* %3, align 4
  %574 = mul i32 %572, %573
  %575 = add i32 %574, 1
  %576 = load <4 x float> addrspace(1)** %1, align 4
  %577 = getelementptr inbounds <4 x float> addrspace(1)* %576, i32 %575
  %578 = load <4 x float> addrspace(1)* %577
  %579 = load <4 x float>* %colorAccumulator, align 16
  %580 = fadd <4 x float> %579, %578
  store <4 x float> %580, <4 x float>* %colorAccumulator, align 16
  %581 = load i32* %row3, align 4
  %582 = add nsw i32 %581, 1
  %583 = load i32* %3, align 4
  %584 = mul i32 %582, %583
  %585 = add i32 %584, 1
  %586 = load <4 x float> addrspace(1)** %1, align 4
  %587 = getelementptr inbounds <4 x float> addrspace(1)* %586, i32 %585
  %588 = load <4 x float> addrspace(1)* %587
  %589 = load <4 x float>* %colorAccumulator, align 16
  %590 = fadd <4 x float> %589, %588
  store <4 x float> %590, <4 x float>* %colorAccumulator, align 16
  %591 = load <4 x float>* %colorAccumulator, align 16
  %592 = load float* %denominator, align 4
  %593 = insertelement <4 x float> undef, float %592, i32 0
  %594 = shufflevector <4 x float> %593, <4 x float> %593, <4 x i32> zeroinitializer
  %595 = fdiv <4 x float> %591, %594
  %596 = load i32* %row3, align 4
  %597 = load i32* %3, align 4
  %598 = mul i32 %596, %597
  %599 = load <4 x float> addrspace(1)** %2, align 4
  %600 = getelementptr inbounds <4 x float> addrspace(1)* %599, i32 %598
  store <4 x float> %595, <4 x float> addrspace(1)* %600
  br label %601

; <label>:601                                     ; preds = %535
  %602 = load i32* %row3, align 4
  %603 = add nsw i32 %602, 1
  store i32 %603, i32* %row3, align 4
  br label %529

; <label>:604                                     ; preds = %529
  br label %605

; <label>:605                                     ; preds = %604, %524
  %606 = load i8* %bottomEdge, align 1
  %607 = trunc i8 %606 to i1
  br i1 %607, label %608, label %661

; <label>:608                                     ; preds = %605
  %609 = load i8* %leftEdge, align 1
  %610 = trunc i8 %609 to i1
  br i1 %610, label %611, label %661

; <label>:611                                     ; preds = %608
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  %612 = load i32* %4, align 4
  %613 = sub i32 %612, 2
  %614 = load i32* %3, align 4
  %615 = mul i32 %613, %614
  %616 = load <4 x float> addrspace(1)** %1, align 4
  %617 = getelementptr inbounds <4 x float> addrspace(1)* %616, i32 %615
  %618 = load <4 x float> addrspace(1)* %617
  %619 = load <4 x float>* %colorAccumulator, align 16
  %620 = fadd <4 x float> %619, %618
  store <4 x float> %620, <4 x float>* %colorAccumulator, align 16
  %621 = load i32* %4, align 4
  %622 = sub i32 %621, 2
  %623 = load i32* %3, align 4
  %624 = mul i32 %622, %623
  %625 = add i32 %624, 1
  %626 = load <4 x float> addrspace(1)** %1, align 4
  %627 = getelementptr inbounds <4 x float> addrspace(1)* %626, i32 %625
  %628 = load <4 x float> addrspace(1)* %627
  %629 = load <4 x float>* %colorAccumulator, align 16
  %630 = fadd <4 x float> %629, %628
  store <4 x float> %630, <4 x float>* %colorAccumulator, align 16
  %631 = load i32* %4, align 4
  %632 = sub i32 %631, 1
  %633 = load i32* %3, align 4
  %634 = mul i32 %632, %633
  %635 = load <4 x float> addrspace(1)** %1, align 4
  %636 = getelementptr inbounds <4 x float> addrspace(1)* %635, i32 %634
  %637 = load <4 x float> addrspace(1)* %636
  %638 = load <4 x float>* %colorAccumulator, align 16
  %639 = fadd <4 x float> %638, %637
  store <4 x float> %639, <4 x float>* %colorAccumulator, align 16
  %640 = load i32* %4, align 4
  %641 = sub i32 %640, 1
  %642 = load i32* %3, align 4
  %643 = mul i32 %641, %642
  %644 = add i32 %643, 1
  %645 = load <4 x float> addrspace(1)** %1, align 4
  %646 = getelementptr inbounds <4 x float> addrspace(1)* %645, i32 %644
  %647 = load <4 x float> addrspace(1)* %646
  %648 = load <4 x float>* %colorAccumulator, align 16
  %649 = fadd <4 x float> %648, %647
  store <4 x float> %649, <4 x float>* %colorAccumulator, align 16
  %650 = load <4 x float>* %colorAccumulator, align 16
  %651 = load float* %denominator, align 4
  %652 = insertelement <4 x float> undef, float %651, i32 0
  %653 = shufflevector <4 x float> %652, <4 x float> %652, <4 x i32> zeroinitializer
  %654 = fdiv <4 x float> %650, %653
  %655 = load i32* %4, align 4
  %656 = sub i32 %655, 1
  %657 = load i32* %3, align 4
  %658 = mul i32 %656, %657
  %659 = load <4 x float> addrspace(1)** %2, align 4
  %660 = getelementptr inbounds <4 x float> addrspace(1)* %659, i32 %658
  store <4 x float> %654, <4 x float> addrspace(1)* %660
  br label %661

; <label>:661                                     ; preds = %611, %608, %605
  %662 = load i8* %bottomEdge, align 1
  %663 = trunc i8 %662 to i1
  br i1 %663, label %664, label %760

; <label>:664                                     ; preds = %661
  %665 = load i32* %index_x, align 4
  store i32 %665, i32* %column4, align 4
  br label %666

; <label>:666                                     ; preds = %756, %664
  %667 = load i32* %column4, align 4
  %668 = load i32* %index_x, align 4
  %669 = load i32* %count_x, align 4
  %670 = add i32 %668, %669
  %671 = icmp ult i32 %667, %670
  br i1 %671, label %672, label %759

; <label>:672                                     ; preds = %666
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  %673 = load i32* %4, align 4
  %674 = sub i32 %673, 2
  %675 = load i32* %3, align 4
  %676 = mul i32 %674, %675
  %677 = load i32* %column4, align 4
  %678 = add i32 %676, %677
  %679 = sub i32 %678, 1
  %680 = load <4 x float> addrspace(1)** %1, align 4
  %681 = getelementptr inbounds <4 x float> addrspace(1)* %680, i32 %679
  %682 = load <4 x float> addrspace(1)* %681
  %683 = load <4 x float>* %colorAccumulator, align 16
  %684 = fadd <4 x float> %683, %682
  store <4 x float> %684, <4 x float>* %colorAccumulator, align 16
  %685 = load i32* %4, align 4
  %686 = sub i32 %685, 2
  %687 = load i32* %3, align 4
  %688 = mul i32 %686, %687
  %689 = load i32* %column4, align 4
  %690 = add i32 %688, %689
  %691 = load <4 x float> addrspace(1)** %1, align 4
  %692 = getelementptr inbounds <4 x float> addrspace(1)* %691, i32 %690
  %693 = load <4 x float> addrspace(1)* %692
  %694 = load <4 x float>* %colorAccumulator, align 16
  %695 = fadd <4 x float> %694, %693
  store <4 x float> %695, <4 x float>* %colorAccumulator, align 16
  %696 = load i32* %4, align 4
  %697 = sub i32 %696, 2
  %698 = load i32* %3, align 4
  %699 = mul i32 %697, %698
  %700 = load i32* %column4, align 4
  %701 = add i32 %699, %700
  %702 = add i32 %701, 1
  %703 = load <4 x float> addrspace(1)** %1, align 4
  %704 = getelementptr inbounds <4 x float> addrspace(1)* %703, i32 %702
  %705 = load <4 x float> addrspace(1)* %704
  %706 = load <4 x float>* %colorAccumulator, align 16
  %707 = fadd <4 x float> %706, %705
  store <4 x float> %707, <4 x float>* %colorAccumulator, align 16
  %708 = load i32* %4, align 4
  %709 = sub i32 %708, 1
  %710 = load i32* %3, align 4
  %711 = mul i32 %709, %710
  %712 = load i32* %column4, align 4
  %713 = add i32 %711, %712
  %714 = sub i32 %713, 1
  %715 = load <4 x float> addrspace(1)** %1, align 4
  %716 = getelementptr inbounds <4 x float> addrspace(1)* %715, i32 %714
  %717 = load <4 x float> addrspace(1)* %716
  %718 = load <4 x float>* %colorAccumulator, align 16
  %719 = fadd <4 x float> %718, %717
  store <4 x float> %719, <4 x float>* %colorAccumulator, align 16
  %720 = load i32* %4, align 4
  %721 = sub i32 %720, 1
  %722 = load i32* %3, align 4
  %723 = mul i32 %721, %722
  %724 = load i32* %column4, align 4
  %725 = add i32 %723, %724
  %726 = load <4 x float> addrspace(1)** %1, align 4
  %727 = getelementptr inbounds <4 x float> addrspace(1)* %726, i32 %725
  %728 = load <4 x float> addrspace(1)* %727
  %729 = load <4 x float>* %colorAccumulator, align 16
  %730 = fadd <4 x float> %729, %728
  store <4 x float> %730, <4 x float>* %colorAccumulator, align 16
  %731 = load i32* %4, align 4
  %732 = sub i32 %731, 1
  %733 = load i32* %3, align 4
  %734 = mul i32 %732, %733
  %735 = load i32* %column4, align 4
  %736 = add i32 %734, %735
  %737 = add i32 %736, 1
  %738 = load <4 x float> addrspace(1)** %1, align 4
  %739 = getelementptr inbounds <4 x float> addrspace(1)* %738, i32 %737
  %740 = load <4 x float> addrspace(1)* %739
  %741 = load <4 x float>* %colorAccumulator, align 16
  %742 = fadd <4 x float> %741, %740
  store <4 x float> %742, <4 x float>* %colorAccumulator, align 16
  %743 = load <4 x float>* %colorAccumulator, align 16
  %744 = load float* %denominator, align 4
  %745 = insertelement <4 x float> undef, float %744, i32 0
  %746 = shufflevector <4 x float> %745, <4 x float> %745, <4 x i32> zeroinitializer
  %747 = fdiv <4 x float> %743, %746
  %748 = load i32* %4, align 4
  %749 = sub i32 %748, 1
  %750 = load i32* %3, align 4
  %751 = mul i32 %749, %750
  %752 = load i32* %column4, align 4
  %753 = add i32 %751, %752
  %754 = load <4 x float> addrspace(1)** %2, align 4
  %755 = getelementptr inbounds <4 x float> addrspace(1)* %754, i32 %753
  store <4 x float> %747, <4 x float> addrspace(1)* %755
  br label %756

; <label>:756                                     ; preds = %672
  %757 = load i32* %column4, align 4
  %758 = add nsw i32 %757, 1
  store i32 %758, i32* %column4, align 4
  br label %666

; <label>:759                                     ; preds = %666
  br label %760

; <label>:760                                     ; preds = %759, %661
  %761 = load i8* %rightEdge, align 1
  %762 = trunc i8 %761 to i1
  br i1 %762, label %763, label %846

; <label>:763                                     ; preds = %760
  %764 = load i32* %index_y, align 4
  store i32 %764, i32* %row5, align 4
  br label %765

; <label>:765                                     ; preds = %842, %763
  %766 = load i32* %row5, align 4
  %767 = load i32* %index_y, align 4
  %768 = load i32* %count_y, align 4
  %769 = add i32 %767, %768
  %770 = icmp ult i32 %766, %769
  br i1 %770, label %771, label %845

; <label>:771                                     ; preds = %765
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  %772 = load i32* %row5, align 4
  %773 = load i32* %3, align 4
  %774 = mul i32 %772, %773
  %775 = sub i32 %774, 1
  %776 = load <4 x float> addrspace(1)** %1, align 4
  %777 = getelementptr inbounds <4 x float> addrspace(1)* %776, i32 %775
  %778 = load <4 x float> addrspace(1)* %777
  %779 = load <4 x float>* %colorAccumulator, align 16
  %780 = fadd <4 x float> %779, %778
  store <4 x float> %780, <4 x float>* %colorAccumulator, align 16
  %781 = load i32* %row5, align 4
  %782 = add nsw i32 %781, 1
  %783 = load i32* %3, align 4
  %784 = mul i32 %782, %783
  %785 = sub i32 %784, 1
  %786 = load <4 x float> addrspace(1)** %1, align 4
  %787 = getelementptr inbounds <4 x float> addrspace(1)* %786, i32 %785
  %788 = load <4 x float> addrspace(1)* %787
  %789 = load <4 x float>* %colorAccumulator, align 16
  %790 = fadd <4 x float> %789, %788
  store <4 x float> %790, <4 x float>* %colorAccumulator, align 16
  %791 = load i32* %row5, align 4
  %792 = add nsw i32 %791, 2
  %793 = load i32* %3, align 4
  %794 = mul i32 %792, %793
  %795 = sub i32 %794, 1
  %796 = load <4 x float> addrspace(1)** %1, align 4
  %797 = getelementptr inbounds <4 x float> addrspace(1)* %796, i32 %795
  %798 = load <4 x float> addrspace(1)* %797
  %799 = load <4 x float>* %colorAccumulator, align 16
  %800 = fadd <4 x float> %799, %798
  store <4 x float> %800, <4 x float>* %colorAccumulator, align 16
  %801 = load i32* %row5, align 4
  %802 = load i32* %3, align 4
  %803 = mul i32 %801, %802
  %804 = sub i32 %803, 2
  %805 = load <4 x float> addrspace(1)** %1, align 4
  %806 = getelementptr inbounds <4 x float> addrspace(1)* %805, i32 %804
  %807 = load <4 x float> addrspace(1)* %806
  %808 = load <4 x float>* %colorAccumulator, align 16
  %809 = fadd <4 x float> %808, %807
  store <4 x float> %809, <4 x float>* %colorAccumulator, align 16
  %810 = load i32* %row5, align 4
  %811 = add nsw i32 %810, 1
  %812 = load i32* %3, align 4
  %813 = mul i32 %811, %812
  %814 = sub i32 %813, 2
  %815 = load <4 x float> addrspace(1)** %1, align 4
  %816 = getelementptr inbounds <4 x float> addrspace(1)* %815, i32 %814
  %817 = load <4 x float> addrspace(1)* %816
  %818 = load <4 x float>* %colorAccumulator, align 16
  %819 = fadd <4 x float> %818, %817
  store <4 x float> %819, <4 x float>* %colorAccumulator, align 16
  %820 = load i32* %row5, align 4
  %821 = add nsw i32 %820, 2
  %822 = load i32* %3, align 4
  %823 = mul i32 %821, %822
  %824 = sub i32 %823, 2
  %825 = load <4 x float> addrspace(1)** %1, align 4
  %826 = getelementptr inbounds <4 x float> addrspace(1)* %825, i32 %824
  %827 = load <4 x float> addrspace(1)* %826
  %828 = load <4 x float>* %colorAccumulator, align 16
  %829 = fadd <4 x float> %828, %827
  store <4 x float> %829, <4 x float>* %colorAccumulator, align 16
  %830 = load <4 x float>* %colorAccumulator, align 16
  %831 = load float* %denominator, align 4
  %832 = insertelement <4 x float> undef, float %831, i32 0
  %833 = shufflevector <4 x float> %832, <4 x float> %832, <4 x i32> zeroinitializer
  %834 = fdiv <4 x float> %830, %833
  %835 = load i32* %row5, align 4
  %836 = add nsw i32 %835, 1
  %837 = load i32* %3, align 4
  %838 = mul i32 %836, %837
  %839 = sub i32 %838, 1
  %840 = load <4 x float> addrspace(1)** %2, align 4
  %841 = getelementptr inbounds <4 x float> addrspace(1)* %840, i32 %839
  store <4 x float> %834, <4 x float> addrspace(1)* %841
  br label %842

; <label>:842                                     ; preds = %771
  %843 = load i32* %row5, align 4
  %844 = add nsw i32 %843, 1
  store i32 %844, i32* %row5, align 4
  br label %765

; <label>:845                                     ; preds = %765
  br label %846

; <label>:846                                     ; preds = %845, %760
  %847 = load i8* %bottomEdge, align 1
  %848 = trunc i8 %847 to i1
  br i1 %848, label %849, label %902

; <label>:849                                     ; preds = %846
  %850 = load i8* %rightEdge, align 1
  %851 = trunc i8 %850 to i1
  br i1 %851, label %852, label %902

; <label>:852                                     ; preds = %849
  store <4 x float> zeroinitializer, <4 x float>* %colorAccumulator, align 16
  %853 = load i32* %4, align 4
  %854 = sub i32 %853, 1
  %855 = load i32* %3, align 4
  %856 = mul i32 %854, %855
  %857 = sub i32 %856, 2
  %858 = load <4 x float> addrspace(1)** %1, align 4
  %859 = getelementptr inbounds <4 x float> addrspace(1)* %858, i32 %857
  %860 = load <4 x float> addrspace(1)* %859
  %861 = load <4 x float>* %colorAccumulator, align 16
  %862 = fadd <4 x float> %861, %860
  store <4 x float> %862, <4 x float>* %colorAccumulator, align 16
  %863 = load i32* %4, align 4
  %864 = sub i32 %863, 1
  %865 = load i32* %3, align 4
  %866 = mul i32 %864, %865
  %867 = sub i32 %866, 1
  %868 = load <4 x float> addrspace(1)** %1, align 4
  %869 = getelementptr inbounds <4 x float> addrspace(1)* %868, i32 %867
  %870 = load <4 x float> addrspace(1)* %869
  %871 = load <4 x float>* %colorAccumulator, align 16
  %872 = fadd <4 x float> %871, %870
  store <4 x float> %872, <4 x float>* %colorAccumulator, align 16
  %873 = load i32* %4, align 4
  %874 = load i32* %3, align 4
  %875 = mul i32 %873, %874
  %876 = sub i32 %875, 2
  %877 = load <4 x float> addrspace(1)** %1, align 4
  %878 = getelementptr inbounds <4 x float> addrspace(1)* %877, i32 %876
  %879 = load <4 x float> addrspace(1)* %878
  %880 = load <4 x float>* %colorAccumulator, align 16
  %881 = fadd <4 x float> %880, %879
  store <4 x float> %881, <4 x float>* %colorAccumulator, align 16
  %882 = load i32* %4, align 4
  %883 = load i32* %3, align 4
  %884 = mul i32 %882, %883
  %885 = sub i32 %884, 1
  %886 = load <4 x float> addrspace(1)** %1, align 4
  %887 = getelementptr inbounds <4 x float> addrspace(1)* %886, i32 %885
  %888 = load <4 x float> addrspace(1)* %887
  %889 = load <4 x float>* %colorAccumulator, align 16
  %890 = fadd <4 x float> %889, %888
  store <4 x float> %890, <4 x float>* %colorAccumulator, align 16
  %891 = load <4 x float>* %colorAccumulator, align 16
  %892 = load float* %denominator, align 4
  %893 = insertelement <4 x float> undef, float %892, i32 0
  %894 = shufflevector <4 x float> %893, <4 x float> %893, <4 x i32> zeroinitializer
  %895 = fdiv <4 x float> %891, %894
  %896 = load i32* %4, align 4
  %897 = load i32* %3, align 4
  %898 = mul i32 %896, %897
  %899 = sub i32 %898, 1
  %900 = load <4 x float> addrspace(1)** %2, align 4
  %901 = getelementptr inbounds <4 x float> addrspace(1)* %900, i32 %899
  store <4 x float> %895, <4 x float> addrspace(1)* %901
  br label %902

; <label>:902                                     ; preds = %852, %849, %846
  ret void
}

!opencl.kernels = !{!0, !2, !3, !4}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_GPU, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, uint const, uint const", metadata !"opencl_wlSimpleBoxBlur_GPU_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_CPU, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, uint const, uint const", metadata !"opencl_wlSimpleBoxBlur_CPU_locals_anchor"}
!3 = metadata !{void (%struct._image2d_t*, <4 x float> addrspace(1)*, i32)* @wlSimpleBoxBlur_image2d, metadata !1, metadata !1, metadata !"", metadata !"__rd image2d_t, float4 __attribute__((address_space(1))) *, uint const", metadata !"opencl_wlSimpleBoxBlur_image2d_locals_anchor"}
!4 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_Optimized_CPU, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, uint const, uint const", metadata !"opencl_wlSimpleBoxBlur_Optimized_CPU_locals_anchor"}
