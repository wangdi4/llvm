// vim:ts=2:sw=2:et:

/* Vector pipe built-ins */
code ReadPipe = [{
    __global struct __pipe_t* p = __ocl_rpipe2ptr($Arg0VarName);
    __global struct __pipe_internal_buf* buf = &p->read_buf;

    $ReturnType $ReturnVarName = ($ReturnType)(0);

    // Buffer exists but it doesn't have enough space to read a full
    // vector
    if (buf->size > 0 && get_buffer_capacity(buf) < $VecLength)
      __flush_read_pipe(p);

    // Buffer doesn't exist, try to reserve it
    if (buf->size < 0) {
      // Ensure that the reserved buffer is a multiple of a vector length
      if (!reserve_read_buffer(buf, get_read_capacity(p) & (- $VecLength))) {
        *$Arg1VarName = -1;
        return $ReturnVarName;
      }
    }

    // Handle cases when vector of elements is non-contiguous in memory
    // (e.g. in the end/beginning of circular buffer)
    if ((buf->end + $VecLength) > p->max_packets) {
      int elem_till_end = p->max_packets - buf->end;
      // Copy elements till end of pipe
      __builtin_memcpy((void*)(&$ReturnVarName), get_packet_ptr(p, buf->end),
                       p->packet_size * elem_till_end);
      // Copy elements are left in the vector to the beginning of pipe
      __builtin_memcpy((void*)((char*)(&$ReturnVarName) +  elem_till_end * p->packet_size),
                       get_packet_ptr(p, 0), p->packet_size * ($VecLength - elem_till_end));
    } else {
      $ReturnVarName = vload$Suffix(0, (__global const $ReturnBaseType *)get_packet_ptr(p, buf->end));
    }
    buf->end = advance(p, buf->end, $VecLength);
    buf->size += $VecLength;

    *$Arg1VarName = 0;
    return $ReturnVarName;
  }];

code WritePipe = [{
    __global struct __pipe_t* p = __ocl_wpipe2ptr($Arg0VarName);
    __global struct __pipe_internal_buf* buf = &p->write_buf;

    // Buffer exists but it doesn't have enough space to write a full
    // vector
    if (buf->size > 0 && get_buffer_capacity(buf) < $VecLength)
      __flush_write_pipe(p);

    // Buffer doesn't exist, try to reserve it
    if (buf->size < 0) {
      if (!reserve_write_buffer(buf, get_write_capacity(p)))
        return -1;
    }

    // Handle cases when vector of elements is non-contiguous in memory
    // (e.g. in the end/beginning of circular buffer)
    if ((buf->end + $VecLength) > p->max_packets) {
      int elem_till_end = p->max_packets - buf->end;
      // Copy elements till end of pipe
      __builtin_memcpy(get_packet_ptr(p, buf->end), (void*)(&$Arg1VarName),
                       p->packet_size * elem_till_end);
      // Copy elements are left in the vector to the beginning of pipe
      __builtin_memcpy(get_packet_ptr(p, 0),
                       (void*)((char*)(&$Arg1VarName) + elem_till_end * p->packet_size),
                       p->packet_size * ($VecLength - elem_till_end));
    } else {
      vstore$Suffix($Arg1VarName, 0, (__global $Arg1BaseType *)get_packet_ptr(p, buf->end));
    }

    buf->end = advance(p, buf->end, $VecLength);
    buf->size += $VecLength;

    return 0;
  }];

/* i8, i16, i32, i64 */
OclBuiltinImpl read_pipe_2_fpga_v2i8_impl = OclBuiltinImpl<read_pipe_2_fpga_v2i8, [v2i8], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4i8_impl = OclBuiltinImpl<read_pipe_2_fpga_v4i8, [v4i8], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8i8_impl = OclBuiltinImpl<read_pipe_2_fpga_v8i8, [v8i8], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16i8_impl = OclBuiltinImpl<read_pipe_2_fpga_v16i8, [v16i8], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2i8_impl = OclBuiltinImpl<write_pipe_2_fpga_v2i8, [v2i8], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4i8_impl = OclBuiltinImpl<write_pipe_2_fpga_v4i8, [v4i8], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8i8_impl = OclBuiltinImpl<write_pipe_2_fpga_v8i8, [v8i8], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16i8_impl = OclBuiltinImpl<write_pipe_2_fpga_v16i8, [v16i8], 0, WritePipe>;

OclBuiltinImpl read_pipe_2_fpga_v2i16_impl = OclBuiltinImpl<read_pipe_2_fpga_v2i16, [v2i16], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4i16_impl = OclBuiltinImpl<read_pipe_2_fpga_v4i16, [v4i16], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8i16_impl = OclBuiltinImpl<read_pipe_2_fpga_v8i16, [v8i16], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16i16_impl = OclBuiltinImpl<read_pipe_2_fpga_v16i16, [v16i16], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2i16_impl = OclBuiltinImpl<write_pipe_2_fpga_v2i16, [v2i16], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4i16_impl = OclBuiltinImpl<write_pipe_2_fpga_v4i16, [v4i16], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8i16_impl = OclBuiltinImpl<write_pipe_2_fpga_v8i16, [v8i16], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16i16_impl = OclBuiltinImpl<write_pipe_2_fpga_v16i16, [v16i16], 0, WritePipe>;

OclBuiltinImpl read_pipe_2_fpga_v2i32_impl = OclBuiltinImpl<read_pipe_2_fpga_v2i32, [v2i32], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4i32_impl = OclBuiltinImpl<read_pipe_2_fpga_v4i32, [v4i32], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8i32_impl = OclBuiltinImpl<read_pipe_2_fpga_v8i32, [v8i32], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16i32_impl = OclBuiltinImpl<read_pipe_2_fpga_v16i32, [v16i32], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2i32_impl = OclBuiltinImpl<write_pipe_2_fpga_v2i32, [v2i32], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4i32_impl = OclBuiltinImpl<write_pipe_2_fpga_v4i32, [v4i32], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8i32_impl = OclBuiltinImpl<write_pipe_2_fpga_v8i32, [v8i32], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16i32_impl = OclBuiltinImpl<write_pipe_2_fpga_v16i32, [v16i32], 0, WritePipe>;

OclBuiltinImpl read_pipe_2_fpga_v2i64_impl = OclBuiltinImpl<read_pipe_2_fpga_v2i64, [v2i64], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4i64_impl = OclBuiltinImpl<read_pipe_2_fpga_v4i64, [v4i64], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8i64_impl = OclBuiltinImpl<read_pipe_2_fpga_v8i64, [v8i64], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16i64_impl = OclBuiltinImpl<read_pipe_2_fpga_v16i64, [v16i64], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2i64_impl = OclBuiltinImpl<write_pipe_2_fpga_v2i64, [v2i64], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4i64_impl = OclBuiltinImpl<write_pipe_2_fpga_v4i64, [v4i64], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8i64_impl = OclBuiltinImpl<write_pipe_2_fpga_v8i64, [v8i64], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16i64_impl = OclBuiltinImpl<write_pipe_2_fpga_v16i64, [v16i64], 0, WritePipe>;

/* u8, u16, u32, u64 */
OclBuiltinImpl read_pipe_2_fpga_v2u8_impl = OclBuiltinImpl<read_pipe_2_fpga_v2u8, [v2u8], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4u8_impl = OclBuiltinImpl<read_pipe_2_fpga_v4u8, [v4u8], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8u8_impl = OclBuiltinImpl<read_pipe_2_fpga_v8u8, [v8u8], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16u8_impl = OclBuiltinImpl<read_pipe_2_fpga_v16u8, [v16u8], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2u8_impl = OclBuiltinImpl<write_pipe_2_fpga_v2u8, [v2u8], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4u8_impl = OclBuiltinImpl<write_pipe_2_fpga_v4u8, [v4u8], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8u8_impl = OclBuiltinImpl<write_pipe_2_fpga_v8u8, [v8u8], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16u8_impl = OclBuiltinImpl<write_pipe_2_fpga_v16u8, [v16u8], 0, WritePipe>;

OclBuiltinImpl read_pipe_2_fpga_v2u16_impl = OclBuiltinImpl<read_pipe_2_fpga_v2u16, [v2u16], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4u16_impl = OclBuiltinImpl<read_pipe_2_fpga_v4u16, [v4u16], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8u16_impl = OclBuiltinImpl<read_pipe_2_fpga_v8u16, [v8u16], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16u16_impl = OclBuiltinImpl<read_pipe_2_fpga_v16u16, [v16u16], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2u16_impl = OclBuiltinImpl<write_pipe_2_fpga_v2u16, [v2u16], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4u16_impl = OclBuiltinImpl<write_pipe_2_fpga_v4u16, [v4u16], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8u16_impl = OclBuiltinImpl<write_pipe_2_fpga_v8u16, [v8u16], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16u16_impl = OclBuiltinImpl<write_pipe_2_fpga_v16u16, [v16u16], 0, WritePipe>;

OclBuiltinImpl read_pipe_2_fpga_v2u32_impl = OclBuiltinImpl<read_pipe_2_fpga_v2u32, [v2u32], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4u32_impl = OclBuiltinImpl<read_pipe_2_fpga_v4u32, [v4u32], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8u32_impl = OclBuiltinImpl<read_pipe_2_fpga_v8u32, [v8u32], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16u32_impl = OclBuiltinImpl<read_pipe_2_fpga_v16u32, [v16u32], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2u32_impl = OclBuiltinImpl<write_pipe_2_fpga_v2u32, [v2u32], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4u32_impl = OclBuiltinImpl<write_pipe_2_fpga_v4u32, [v4u32], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8u32_impl = OclBuiltinImpl<write_pipe_2_fpga_v8u32, [v8u32], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16u32_impl = OclBuiltinImpl<write_pipe_2_fpga_v16u32, [v16u32], 0, WritePipe>;

OclBuiltinImpl read_pipe_2_fpga_v2u64_impl = OclBuiltinImpl<read_pipe_2_fpga_v2u64, [v2u64], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4u64_impl = OclBuiltinImpl<read_pipe_2_fpga_v4u64, [v4u64], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8u64_impl = OclBuiltinImpl<read_pipe_2_fpga_v8u64, [v8u64], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16u64_impl = OclBuiltinImpl<read_pipe_2_fpga_v16u64, [v16u64], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2u64_impl = OclBuiltinImpl<write_pipe_2_fpga_v2u64, [v2u64], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4u64_impl = OclBuiltinImpl<write_pipe_2_fpga_v4u64, [v4u64], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8u64_impl = OclBuiltinImpl<write_pipe_2_fpga_v8u64, [v8u64], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16u64_impl = OclBuiltinImpl<write_pipe_2_fpga_v16u64, [v16u64], 0, WritePipe>;


/* half, float, double */
OclBuiltinImpl read_pipe_2_fpga_v2f16_impl = OclBuiltinImpl<read_pipe_2_fpga_v2f16, [v2f16], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4f16_impl = OclBuiltinImpl<read_pipe_2_fpga_v4f16, [v4f16], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8f16_impl = OclBuiltinImpl<read_pipe_2_fpga_v8f16, [v8f16], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16f16_impl = OclBuiltinImpl<read_pipe_2_fpga_v16f16, [v16f16], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2f16_impl = OclBuiltinImpl<write_pipe_2_fpga_v2f16, [v2f16], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4f16_impl = OclBuiltinImpl<write_pipe_2_fpga_v4f16, [v4f16], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8f16_impl = OclBuiltinImpl<write_pipe_2_fpga_v8f16, [v8f16], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16f16_impl = OclBuiltinImpl<write_pipe_2_fpga_v16f16, [v16f16], 0, WritePipe>;

OclBuiltinImpl read_pipe_2_fpga_v2f32_impl = OclBuiltinImpl<read_pipe_2_fpga_v2f32, [v2f32], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4f32_impl = OclBuiltinImpl<read_pipe_2_fpga_v4f32, [v4f32], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8f32_impl = OclBuiltinImpl<read_pipe_2_fpga_v8f32, [v8f32], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16f32_impl = OclBuiltinImpl<read_pipe_2_fpga_v16f32, [v16f32], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2f32_impl = OclBuiltinImpl<write_pipe_2_fpga_v2f32, [v2f32], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4f32_impl = OclBuiltinImpl<write_pipe_2_fpga_v4f32, [v4f32], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8f32_impl = OclBuiltinImpl<write_pipe_2_fpga_v8f32, [v8f32], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16f32_impl = OclBuiltinImpl<write_pipe_2_fpga_v16f32, [v16f32], 0, WritePipe>;

OclBuiltinImpl read_pipe_2_fpga_v2f64_impl = OclBuiltinImpl<read_pipe_2_fpga_v2f64, [v2f64], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v4f64_impl = OclBuiltinImpl<read_pipe_2_fpga_v4f64, [v4f64], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v8f64_impl = OclBuiltinImpl<read_pipe_2_fpga_v8f64, [v8f64], 0, ReadPipe>;
OclBuiltinImpl read_pipe_2_fpga_v16f64_impl = OclBuiltinImpl<read_pipe_2_fpga_v16f64, [v16f64], 0, ReadPipe>;

OclBuiltinImpl write_pipe_2_fpga_v2f64_impl = OclBuiltinImpl<write_pipe_2_fpga_v2f64, [v2f64], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v4f64_impl = OclBuiltinImpl<write_pipe_2_fpga_v4f64, [v4f64], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v8f64_impl = OclBuiltinImpl<write_pipe_2_fpga_v8f64, [v8f64], 0, WritePipe>;
OclBuiltinImpl write_pipe_2_fpga_v16f64_impl = OclBuiltinImpl<write_pipe_2_fpga_v16f64, [v16f64], 0, WritePipe>;
