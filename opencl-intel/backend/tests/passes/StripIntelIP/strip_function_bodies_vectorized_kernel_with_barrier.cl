;
RUN : % oclopt - strip - intel - ip - verify - S % s | FileCheck % s

    ;
kernel wrapper must be present
    declare !kernel_wrapper !1 !vectorized_kernel !2 !scalarized_kernel !4 void
    @kernel_separated_args();
CHECK : declare {
  { .* }
}
@kernel_separated_args

    ;
kernel function is expected to be emptied define void
    @kernel(i32 addrspace(1) * % global) !scalarized_kernel
    !0 !vectorized_kernel !2 {
      entry : % x = load i32,
      i32 * % global,
      align 8 % y = add i32 % x,
      1 ret void
    };
CHECK : define {
  { .* }
}
@kernel;
CHECK - NEXT : exit:;
CHECK - NEXT : ret void

    ;
vectorized function wrapper
    declare !kernel wrapper !3 !scalarized_kernel !0 !vectorized_kernel !4 void
    @__Vectorized_.kernel_separated_args();
CHECK : declare {
  { .* }
}
@__Vectorized_.kernel_separated_args

    ;
vectorized function body is expected to be emptied define void
    @__Vectorized_.kernel(<4 x i32> addrspace(1) * % global) !scalarized_kernel
    !0 !vectorized_kernel !4 {
      entry : % x = load<4 x i32>,
      <4 x i32> addrspace(1) * % global,
      align 16 % y = add<4 x i32>,
      <4 x i32><i32 1, i32 1, i32 1, i32 1>
          ret void
    };
CHECK : define {
  { .* }
}
@__Vectorized_.kernel;
CHECK - NEXT : exit:;
CHECK - NEXT : ret void

               !sycl.kernels = !{
  !0
}

!0 = !{ void() * @kernel_separated_args }
!1 = !{ void() * @kernel }
!2 = !{ void() * @__Vectorized_.kernel_separated_args }
!3 = !{ void() * @__Vectorized_.kernel }
!4 = !{ null }
