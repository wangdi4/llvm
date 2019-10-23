// genric implementation for transpose function



// ****************************************************************************
//                                 4x4
// ****************************************************************************

OclBuiltinImpl transpose_load_4x4_sse3 = OclBuiltinImpl<__ocl_load_transpose4x4, [v4i8, v4i32, v4f32], 0, [{
    // pLoadAdd[0]  = x0 y0 z0 w0
    // pLoadAdd[1]  = x1 y1 z1 w1
    // pLoadAdd[2]  = x2 y2 z2 w2
    // pLoadAdd[3]  = x3 y3 z3 w3

    __ocl_transpose4x4( $Arg0VarName[0], $Arg0VarName[1], $Arg0VarName[2], $Arg0VarName[3],
                        $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);

    // xOut         = x0 x1 x2 x3
    // yOut         = y0 y1 y2 y3
    // zOut         = z0 z1 z2 z3
    // wOut         = w0 w1 w2 w3
  }]>;
OclBuiltinImpl transpose_store_4x4_sse3 = OclBuiltinImpl<__ocl_transpose_store4x4, [v4i8, v4i32, v4f32], 0, [{
    // xIn          = x0 x1 x2 x3
    // yIn          = y0 y1 y2 y3
    // zIn          = z0 z1 z2 z3
    // wIn          = w0 w1 w2 w3

    __ocl_transpose4x4( $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName,
                        &($Arg0VarName[0]), &($Arg0VarName[1]), &($Arg0VarName[2]), &($Arg0VarName[3]));

    // pStoreAdd[0] = x0 y0 z0 w0
    // pStoreAdd[1] = x1 y1 z1 w1
    // pStoreAdd[2] = x2 y2 z2 w2
    // pStoreAdd[3] = x3 y3 z3 w3
  }]>;
OclBuiltinImpl transpose_gather_4x4_sse3 = OclBuiltinImpl<__ocl_gather_transpose4x4, [v4i8, v4i32, v4f32], 0, [{
    // pLoadAdd0    = x0 y0 z0 w0
    // pLoadAdd1    = x1 y1 z1 w1
    // pLoadAdd2    = x2 y2 z2 w2
    // pLoadAdd3    = x3 y3 z3 w3

    __ocl_transpose4x4( *$Arg0VarName, *$Arg1VarName, *$Arg2VarName, *$Arg3VarName,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);

    // xOut         = x0 x1 x2 x3
    // yOut         = y0 y1 y2 y3
    // zOut         = z0 z1 z2 z3
    // wOut         = w0 w1 w2 w3
  }]>;
OclBuiltinImpl transpose_scatter_4x4_sse3 = OclBuiltinImpl<__ocl_transpose_scatter4x4, [v4i8, v4i32, v4f32], 0, [{
    // xIn          = x0 x1 x2 x3
    // yIn          = y0 y1 y2 y3
    // zIn          = z0 z1 z2 z3
    // wIn          = w0 w1 w2 w3

    __ocl_transpose4x4( $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                        $Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);

    // pStoreAdd0   = x0 y0 z0 w0
    // pStoreAdd1   = x1 y1 z1 w1
    // pStoreAdd2   = x2 y2 z2 w2
    // pStoreAdd3   = x3 y3 z3 w3
  }]>;

OclBuiltinImpl transpose_masked_load_4x4_sse3 = OclBuiltinImpl<__ocl_masked_load_transpose4x4, [v4i8, v4i32], 0, [{
    __private $PtrArg0Type xyzw[4];
    __ocl_masked_load_$Arg0BaseType_4x4($Arg0VarName, xyzw, $Arg5VarName);
    __ocl_load_transpose_$Arg0BaseType_4x4(xyzw, $Arg1VarName, $Arg2VarName, $Arg3VarName,  $Arg4VarName);
  }]>;
OclBuiltinImpl transpose_masked_load_float_4x4_sse3 = OclBuiltinImpl<__ocl_masked_load_transpose4x4, [v4f32], 0, [{
    __private $PtrArg0Type xyzw[4] = {0};
    __ocl_masked_load_float_4x4($Arg0VarName, xyzw, $Arg5VarName);
    __ocl_load_transpose_float_4x4(xyzw, $Arg1VarName, $Arg2VarName, $Arg3VarName,  $Arg4VarName);
  }]>;
OclBuiltinImpl transpose_masked_store_4x4_sse3 = OclBuiltinImpl<__ocl_masked_transpose_store4x4, [v4i8, v4i32, v4f32], 0, [{
    __private $PtrArg0Type xyzw[4];
    __ocl_transpose_store_$Arg0BaseType_4x4(xyzw, $Arg1VarName, $Arg2VarName, $Arg3VarName,  $Arg4VarName);
    __ocl_masked_store_$Arg0BaseType_4x4($Arg0VarName, xyzw, $Arg5VarName);
  }]>;
OclBuiltinImpl transpose_masked_gather_4x4_sse3 = OclBuiltinImpl<__ocl_masked_gather_transpose4x4, [v4i8, v4i32, v4f32], 0, [{
    if (all($Arg8VarName)) {
      __ocl_gather_transpose_$Arg0BaseType_4x4(
                        $Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);
      return;
    }

    $PtrArg0Type  dummy = 0;

    $Arg0Type xyzw0 = $Arg8VarName.s0 ? $Arg0VarName : &dummy;
    $Arg0Type xyzw1 = $Arg8VarName.s1 ? $Arg1VarName : &dummy;
    $Arg0Type xyzw2 = $Arg8VarName.s2 ? $Arg2VarName : &dummy;
    $Arg0Type xyzw3 = $Arg8VarName.s3 ? $Arg3VarName : &dummy;

    __ocl_gather_transpose_$Arg0BaseType_4x4(xyzw0, xyzw1, xyzw2, xyzw3, xOut, yOut, zOut, wOut);
  }]>;
OclBuiltinImpl transpose_masked_scatter_4x4_sse3 = OclBuiltinImpl<__ocl_masked_transpose_scatter4x4, [v4i8, v4i32, v4f32], 0, [{
    if (all($Arg8VarName)) {
      __ocl_transpose_scatter_$Arg0BaseType_4x4(
                        $Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);
      return;
    }

    $PtrArg0Type  dummy;


    $Arg0Type xyzw0 = $Arg8VarName.s0 ? $Arg0VarName : &dummy;
    $Arg0Type xyzw1 = $Arg8VarName.s1 ? $Arg1VarName : &dummy;
    $Arg0Type xyzw2 = $Arg8VarName.s2 ? $Arg2VarName : &dummy;
    $Arg0Type xyzw3 = $Arg8VarName.s3 ? $Arg3VarName : &dummy;

    __ocl_transpose_scatter_$Arg0BaseType_4x4(xyzw0, xyzw1, xyzw2, xyzw3, $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);
  }]>;

// ****************************************************************************
//                                 4x8
// ****************************************************************************

OclBuiltinImpl transpose_load_4x8_sse3 = OclBuiltinImpl<__ocl_load_transpose4x8, [v4i8, v4i16, v4i32, v4f32], 0, [{
    __ocl_transpose4x8( $Arg0VarName[0], $Arg0VarName[1], $Arg0VarName[2], $Arg0VarName[3],
                        $Arg0VarName[4], $Arg0VarName[5], $Arg0VarName[6], $Arg0VarName[7],
                        $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;
OclBuiltinImpl transpose_store_4x8_sse3 = OclBuiltinImpl<__ocl_transpose_store4x8, [v4i8, v4i16, v4i32, v4f32], 0, [{
    __ocl_transpose8x4( $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName,
                        &($Arg0VarName[0]), &($Arg0VarName[1]), &($Arg0VarName[2]), &($Arg0VarName[3]),
                        &($Arg0VarName[4]), &($Arg0VarName[5]), &($Arg0VarName[6]), &($Arg0VarName[7]));
  }]>;
OclBuiltinImpl transpose_gather_4x8_sse3 = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4i8, v4i16, v4i32, v4f32], 0, [{
    $PtrArg0Type xyzw0 = *$Arg0VarName;
    $PtrArg0Type xyzw1 = *$Arg1VarName;
    $PtrArg0Type xyzw2 = *$Arg2VarName;
    $PtrArg0Type xyzw3 = *$Arg3VarName;
    $PtrArg0Type xyzw4 = *$Arg4VarName;
    $PtrArg0Type xyzw5 = *$Arg5VarName;
    $PtrArg0Type xyzw6 = *$Arg6VarName;
    $PtrArg0Type xyzw7 = *$Arg7VarName;

    __ocl_transpose4x8( xyzw0, xyzw1, xyzw2, xyzw3,
                        xyzw4, xyzw5, xyzw6, xyzw7,
                        $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
  }]>;
OclBuiltinImpl transpose_scatter_4x8_sse3 = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4i8, v4i16, v4i32, v4f32], 0, [{
    __ocl_transpose8x4( $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName,
                        $Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);
  }]>;

OclBuiltinImpl transpose_masked_load_4x8_sse3 = OclBuiltinImpl<__ocl_masked_load_transpose4x8, [v4i8, v4i16, v4i32], 0, [{
    __private $PtrArg0Type xyzw[8];
    __ocl_masked_load_$Arg0BaseType_4x8($Arg0VarName, xyzw, $Arg5VarName);
    __ocl_load_transpose_$Arg0BaseType_4x8(xyzw, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;
OclBuiltinImpl transpose_masked_load_float_4x8_sse3 = OclBuiltinImpl<__ocl_masked_load_transpose4x8, [v4f32], 0, [{
    __private $PtrArg0Type xyzw[8] = {0};
    __ocl_masked_load_float_4x8($Arg0VarName, xyzw, $Arg5VarName);
    __ocl_load_transpose_float_4x8(xyzw, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;
OclBuiltinImpl transpose_masked_store_4x8_sse3 = OclBuiltinImpl<__ocl_masked_transpose_store4x8, [v4i8, v4i16, v4i32, v4f32], 0, [{
    __private $PtrArg0Type xyzw[8];
    __ocl_transpose_store_$Arg0BaseType_4x8(xyzw, $Arg1VarName, $Arg2VarName, $Arg3VarName,  $Arg4VarName);
    __ocl_masked_store_$Arg0BaseType_4x8($Arg0VarName, xyzw, $Arg5VarName);

  }]>;
OclBuiltinImpl transpose_masked_gather_4x8_sse3 = OclBuiltinImpl<__ocl_masked_gather_transpose4x8, [v4i8, v4i16, v4i32, v4f32], 0, [{
    if (all($Arg12VarName)) {
      __ocl_gather_transpose_$Arg0BaseType_4x8(
                        $Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                        $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
      return;
    }

    $PtrArg0Type  dummy = 0;

    $Arg0Type xyzw0 = $Arg12VarName.s0 ? $Arg0VarName : &dummy;
    $Arg0Type xyzw1 = $Arg12VarName.s1 ? $Arg1VarName : &dummy;
    $Arg0Type xyzw2 = $Arg12VarName.s2 ? $Arg2VarName : &dummy;
    $Arg0Type xyzw3 = $Arg12VarName.s3 ? $Arg3VarName : &dummy;
    $Arg0Type xyzw4 = $Arg12VarName.s4 ? $Arg4VarName : &dummy;
    $Arg0Type xyzw5 = $Arg12VarName.s5 ? $Arg5VarName : &dummy;
    $Arg0Type xyzw6 = $Arg12VarName.s6 ? $Arg6VarName : &dummy;
    $Arg0Type xyzw7 = $Arg12VarName.s7 ? $Arg7VarName : &dummy;

    __ocl_gather_transpose_$Arg0BaseType_4x8(
                      xyzw0, xyzw1, xyzw2, xyzw3,
                      xyzw4, xyzw5, xyzw6, xyzw7,
                      $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
  }]>;
OclBuiltinImpl transpose_masked_scatter_4x8_sse3 = OclBuiltinImpl<__ocl_masked_transpose_scatter4x8, [v4i8, v4i16, v4i32, v4f32], 0, [{
    if (all($Arg12VarName)) {
      __ocl_transpose_scatter_$Arg0BaseType_4x8(
                        $Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                        $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
      return;
    }

    $PtrArg0Type  dummy;

    $Arg0Type xyzw0 = $Arg12VarName.s0 ? $Arg0VarName : &dummy;
    $Arg0Type xyzw1 = $Arg12VarName.s1 ? $Arg1VarName : &dummy;
    $Arg0Type xyzw2 = $Arg12VarName.s2 ? $Arg2VarName : &dummy;
    $Arg0Type xyzw3 = $Arg12VarName.s3 ? $Arg3VarName : &dummy;
    $Arg0Type xyzw4 = $Arg12VarName.s4 ? $Arg4VarName : &dummy;
    $Arg0Type xyzw5 = $Arg12VarName.s5 ? $Arg5VarName : &dummy;
    $Arg0Type xyzw6 = $Arg12VarName.s6 ? $Arg6VarName : &dummy;
    $Arg0Type xyzw7 = $Arg12VarName.s7 ? $Arg7VarName : &dummy;

    __ocl_transpose_scatter_$Arg0BaseType_4x8(
                      xyzw0, xyzw1, xyzw2, xyzw3,
                      xyzw4, xyzw5, xyzw6, xyzw7,
                      $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
  }]>;

// ****************************************************************************
//                                 4x16
// ****************************************************************************

OclBuiltinImpl transpose_load_4x16_sse3 = OclBuiltinImpl<__ocl_load_transpose4x16, [v4i8], 0, [{
    __ocl_transpose4x16( $Arg0VarName[0], $Arg0VarName[1], $Arg0VarName[2], $Arg0VarName[3],
                        $Arg0VarName[4], $Arg0VarName[5], $Arg0VarName[6], $Arg0VarName[7],
                        $Arg0VarName[8], $Arg0VarName[9], $Arg0VarName[10], $Arg0VarName[11],
                        $Arg0VarName[12], $Arg0VarName[13], $Arg0VarName[14], $Arg0VarName[15],
                        $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;
OclBuiltinImpl transpose_gather_4x16_sse3 = OclBuiltinImpl<__ocl_gather_transpose4x16, [v4i8, v4i16], 0, [{
    $PtrArg0Type xyzw0 = *$Arg0VarName;
    $PtrArg0Type xyzw1 = *$Arg1VarName;
    $PtrArg0Type xyzw2 = *$Arg2VarName;
    $PtrArg0Type xyzw3 = *$Arg3VarName;
    $PtrArg0Type xyzw4 = *$Arg4VarName;
    $PtrArg0Type xyzw5 = *$Arg5VarName;
    $PtrArg0Type xyzw6 = *$Arg6VarName;
    $PtrArg0Type xyzw7 = *$Arg7VarName;
    $PtrArg0Type xyzw8 = *$Arg8VarName;
    $PtrArg0Type xyzw9 = *$Arg9VarName;
    $PtrArg0Type xyzwA = *$Arg10VarName;
    $PtrArg0Type xyzwB = *$Arg11VarName;
    $PtrArg0Type xyzwC = *$Arg12VarName;
    $PtrArg0Type xyzwD = *$Arg13VarName;
    $PtrArg0Type xyzwE = *$Arg14VarName;
    $PtrArg0Type xyzwF = *$Arg15VarName;

    __ocl_transpose4x16( xyzw0, xyzw1, xyzw2, xyzw3,
                         xyzw4, xyzw5, xyzw6, xyzw7,
                         xyzw8, xyzw9, xyzwA, xyzwB,
                         xyzwC, xyzwD, xyzwE, xyzwF,
                         $Arg16VarName, $Arg17VarName, $Arg18VarName, $Arg19VarName);
  }]>;
OclBuiltinImpl transpose_scatter_4x16_sse3 = OclBuiltinImpl<__ocl_transpose_scatter4x16, [v4i8], 0, [{
    __ocl_transpose16x4( $Arg16VarName, $Arg17VarName, $Arg18VarName, $Arg19VarName,
                         $Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                         $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                         $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName,
                         $Arg12VarName, $Arg13VarName, $Arg14VarName, $Arg15VarName);
  }]>;
OclBuiltinImpl transpose_masked_gather_4x16_sse3 = OclBuiltinImpl<__ocl_masked_gather_transpose4x16, [v4i8, v4i16], 0, [{
    if (all($Arg20VarName)) {
      __ocl_gather_transpose_$Arg0BaseType_4x16(
                        $Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                        $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName,
                        $Arg12VarName, $Arg13VarName, $Arg14VarName, $Arg15VarName,
                        $Arg16VarName, $Arg17VarName, $Arg18VarName, $Arg19VarName);
      return;
    }

    $PtrArg0Type  dummy = 0;

    $Arg0Type xyzw0 = $Arg20VarName.s0 ? $Arg0VarName : &dummy;
    $Arg0Type xyzw1 = $Arg20VarName.s1 ? $Arg1VarName : &dummy;
    $Arg0Type xyzw2 = $Arg20VarName.s2 ? $Arg2VarName : &dummy;
    $Arg0Type xyzw3 = $Arg20VarName.s3 ? $Arg3VarName : &dummy;
    $Arg0Type xyzw4 = $Arg20VarName.s4 ? $Arg4VarName : &dummy;
    $Arg0Type xyzw5 = $Arg20VarName.s5 ? $Arg5VarName : &dummy;
    $Arg0Type xyzw6 = $Arg20VarName.s6 ? $Arg6VarName : &dummy;
    $Arg0Type xyzw7 = $Arg20VarName.s7 ? $Arg7VarName : &dummy;
    $Arg0Type xyzw8 = $Arg20VarName.s8 ? $Arg8VarName : &dummy;
    $Arg0Type xyzw9 = $Arg20VarName.s9 ? $Arg9VarName : &dummy;
    $Arg0Type xyzwA = $Arg20VarName.sA ? $Arg10VarName : &dummy;
    $Arg0Type xyzwB = $Arg20VarName.sB ? $Arg11VarName : &dummy;
    $Arg0Type xyzwC = $Arg20VarName.sC ? $Arg12VarName : &dummy;
    $Arg0Type xyzwD = $Arg20VarName.sD ? $Arg13VarName : &dummy;
    $Arg0Type xyzwE = $Arg20VarName.sE ? $Arg14VarName : &dummy;
    $Arg0Type xyzwF = $Arg20VarName.sF ? $Arg15VarName : &dummy;

    __ocl_gather_transpose_$Arg0BaseType_4x16(
                      xyzw0, xyzw1, xyzw2, xyzw3,
                      xyzw4, xyzw5, xyzw6, xyzw7,
                      xyzw8, xyzw9, xyzwA, xyzwB,
                      xyzwC, xyzwD, xyzwE, xyzwF,
                      $Arg16VarName, $Arg17VarName, $Arg18VarName, $Arg19VarName);
  }]>;
OclBuiltinImpl transpose_masked_scatter_4x16_sse3 = OclBuiltinImpl<__ocl_masked_transpose_scatter4x16, [v4i8], 0, [{
   if (all($Arg20VarName)) {
      __ocl_transpose_scatter_$Arg0BaseType_4x16(
                        $Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                        $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName,
                        $Arg12VarName, $Arg13VarName, $Arg14VarName, $Arg15VarName,
                        $Arg16VarName, $Arg17VarName, $Arg18VarName, $Arg19VarName);
      return;
    }

    $PtrArg0Type dummy;

    $Arg0Type xyzw0 = $Arg20VarName.s0 ? $Arg0VarName : &dummy;
    $Arg0Type xyzw1 = $Arg20VarName.s1 ? $Arg1VarName : &dummy;
    $Arg0Type xyzw2 = $Arg20VarName.s2 ? $Arg2VarName : &dummy;
    $Arg0Type xyzw3 = $Arg20VarName.s3 ? $Arg3VarName : &dummy;
    $Arg0Type xyzw4 = $Arg20VarName.s4 ? $Arg4VarName : &dummy;
    $Arg0Type xyzw5 = $Arg20VarName.s5 ? $Arg5VarName : &dummy;
    $Arg0Type xyzw6 = $Arg20VarName.s6 ? $Arg6VarName : &dummy;
    $Arg0Type xyzw7 = $Arg20VarName.s7 ? $Arg7VarName : &dummy;
    $Arg0Type xyzw8 = $Arg20VarName.s8 ? $Arg8VarName : &dummy;
    $Arg0Type xyzw9 = $Arg20VarName.s9 ? $Arg9VarName : &dummy;
    $Arg0Type xyzwA = $Arg20VarName.sA ? $Arg10VarName : &dummy;
    $Arg0Type xyzwB = $Arg20VarName.sB ? $Arg11VarName : &dummy;
    $Arg0Type xyzwC = $Arg20VarName.sC ? $Arg12VarName : &dummy;
    $Arg0Type xyzwD = $Arg20VarName.sD ? $Arg13VarName : &dummy;
    $Arg0Type xyzwE = $Arg20VarName.sE ? $Arg14VarName : &dummy;
    $Arg0Type xyzwF = $Arg20VarName.sF ? $Arg15VarName : &dummy;

    __ocl_transpose_scatter_$Arg0BaseType_4x16(
                      xyzw0, xyzw1, xyzw2, xyzw3,
                      xyzw4, xyzw5, xyzw6, xyzw7,
                      xyzw8, xyzw9, xyzwA, xyzwB,
                      xyzwC, xyzwD, xyzwE, xyzwF,
                      $Arg16VarName, $Arg17VarName, $Arg18VarName, $Arg19VarName);
  }]>;

// ****************************************************************************
//                                 helper function
// ****************************************************************************
OclBuiltinImpl __ocl_transpose4x4_sse3 = OclBuiltinImpl<__ocl_transpose4x4, [v4i8, v4i16, v4i32, v4f32], 0, [{
    (*$Arg4VarName).s0 = $Arg0VarName.s0;
    (*$Arg4VarName).s1 = $Arg1VarName.s0;
    (*$Arg4VarName).s2 = $Arg2VarName.s0;
    (*$Arg4VarName).s3 = $Arg3VarName.s0;

    (*$Arg5VarName).s0 = $Arg0VarName.s1;
    (*$Arg5VarName).s1 = $Arg1VarName.s1;
    (*$Arg5VarName).s2 = $Arg2VarName.s1;
    (*$Arg5VarName).s3 = $Arg3VarName.s1;

    (*$Arg6VarName).s0 = $Arg0VarName.s2;
    (*$Arg6VarName).s1 = $Arg1VarName.s2;
    (*$Arg6VarName).s2 = $Arg2VarName.s2;
    (*$Arg6VarName).s3 = $Arg3VarName.s2;

    (*$Arg7VarName).s0 = $Arg0VarName.s3;
    (*$Arg7VarName).s1 = $Arg1VarName.s3;
    (*$Arg7VarName).s2 = $Arg2VarName.s3;
    (*$Arg7VarName).s3 = $Arg3VarName.s3;
  }]>;

OclBuiltinImpl __ocl_transpose4x8_sse3 = OclBuiltinImpl<__ocl_transpose4x8, [v4i8, v4i16, v4i32, v4f32], 0, [{
    $Arg0Type xLow;
    $Arg0Type yLow;
    $Arg0Type zLow;
    $Arg0Type wLow;

    __ocl_transpose4x4($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                       &xLow, &yLow, &zLow, &wLow);

    $Arg0Type xHigh;
    $Arg0Type yHigh;
    $Arg0Type zHigh;
    $Arg0Type wHigh;

     __ocl_transpose4x4($Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                        &xHigh, &yHigh, &zHigh, &wHigh);

    (*$Arg8VarName).lo = xLow;
    (*$Arg8VarName).hi = xHigh;
    (*$Arg9VarName).lo = yLow;
    (*$Arg9VarName).hi = yHigh;
    (*$Arg10VarName).lo = zLow;
    (*$Arg10VarName).hi = zHigh;
    (*$Arg11VarName).lo = wLow;
    (*$Arg11VarName).hi = wHigh;
  }]>;

OclBuiltinImpl __ocl_transpose8x4_sse3 = OclBuiltinImpl<__ocl_transpose8x4, [v4i8, v4i16, v4i32, v4f32], 0, [{
    $PtrArg4Type xLow = $Arg0VarName.lo;
    $PtrArg4Type yLow = $Arg1VarName.lo;
    $PtrArg4Type zLow = $Arg2VarName.lo;
    $PtrArg4Type wLow = $Arg3VarName.lo;

    __ocl_transpose4x4( xLow, yLow, zLow, wLow,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);

    $PtrArg4Type xHigh = $Arg0VarName.hi;
    $PtrArg4Type yHigh = $Arg1VarName.hi;
    $PtrArg4Type zHigh = $Arg2VarName.hi;
    $PtrArg4Type wHigh = $Arg3VarName.hi;

    __ocl_transpose4x4( xHigh, yHigh, zHigh, wHigh,
                        $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
  }]>;

OclBuiltinImpl __ocl_transpose4x16_sse3 = OclBuiltinImpl<__ocl_transpose4x16, [v4i8, v4i16], 0, [{
    $Arg0BaseType#8 xLow;
    $Arg0BaseType#8 yLow;
    $Arg0BaseType#8 zLow;
    $Arg0BaseType#8 wLow;

    __ocl_transpose4x8($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                       $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                       &xLow, &yLow, &zLow, &wLow);

    $Arg0BaseType#8 xHigh;
    $Arg0BaseType#8 yHigh;
    $Arg0BaseType#8 zHigh;
    $Arg0BaseType#8 wHigh;

     __ocl_transpose4x8($Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName,
                        $Arg12VarName, $Arg13VarName, $Arg14VarName, $Arg15VarName,
                        &xHigh, &yHigh, &zHigh, &wHigh);

    (*$Arg16VarName).lo = xLow;
    (*$Arg16VarName).hi = xHigh;
    (*$Arg17VarName).lo = yLow;
    (*$Arg17VarName).hi = yHigh;
    (*$Arg18VarName).lo = zLow;
    (*$Arg18VarName).hi = zHigh;
    (*$Arg19VarName).lo = wLow;
    (*$Arg19VarName).hi = wHigh;
  }]>;

OclBuiltinImpl __ocl_transpose16x4_sse3 = OclBuiltinImpl<__ocl_transpose16x4, [v4i8, v4i16], 0, [{
    $Arg0BaseType#8 xLow = $Arg0VarName.lo;
    $Arg0BaseType#8 yLow = $Arg1VarName.lo;
    $Arg0BaseType#8 zLow = $Arg2VarName.lo;
    $Arg0BaseType#8 wLow = $Arg3VarName.lo;

    __ocl_transpose8x4( xLow, yLow, zLow, wLow,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                        $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);

    $Arg0BaseType#8 xHigh = $Arg0VarName.hi;
    $Arg0BaseType#8 yHigh = $Arg1VarName.hi;
    $Arg0BaseType#8 zHigh = $Arg2VarName.hi;
    $Arg0BaseType#8 wHigh = $Arg3VarName.hi;

    __ocl_transpose8x4( xHigh, yHigh, zHigh, wHigh,
                        $Arg12VarName, $Arg13VarName, $Arg14VarName, $Arg15VarName,
                        $Arg16VarName, $Arg17VarName, $Arg18VarName, $Arg19VarName);
  }]>;


list<OclType> __ocl_shuffle_transpose_v32_types = [ v32i64, v32u64,
                                                    v32i32, v32u32, v32f32,
                                                    v32i16, v32u16,
                                                    v32i8,  v32u8 ];

OclBuiltinImpl __ocl_shuffle_transpose4x8_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_4x8, __ocl_shuffle_transpose_v32_types, 0, [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
            0, 4, 8, 12, 16, 20, 24, 28,
            1, 5, 9, 13, 17, 21, 25, 29,
            2, 6, 10, 14, 18, 22, 26, 30,
            3, 7, 11, 15, 19, 23, 27, 31);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x4_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_8x4, __ocl_shuffle_transpose_v32_types, 0, [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
            0, 8, 16, 24, 1, 9, 17, 25,
            2, 10, 18, 26, 3, 11, 19, 27,
            4, 12, 20, 28, 5, 13, 21, 29,
            6, 14, 22, 30, 7, 15, 23, 31);
}]>;

list<OclType> __ocl_shuffle_transpose_v64_types = [ v64i64, v64u64,
                                                    v64i32, v64u32, v64f32,
                                                    v64i16, v64u16,
                                                    v64i8,  v64u8 ];

OclBuiltinImpl __ocl_shuffle_transpose4x16_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_4x16, __ocl_shuffle_transpose_v64_types, 0, [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
            0, 4,  8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
            1, 5,  9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61,
            2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62,
            3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose16x4_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_16x4, __ocl_shuffle_transpose_v64_types, 0, [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
            0, 16, 32, 48,   1, 17, 33, 49,   2, 18, 34, 50,   3, 19, 35, 51,
            4, 20, 36, 52,   5, 21, 37, 53,   6, 22, 38, 54,   7, 23, 39, 55,
            8, 24, 40, 56,   9, 25, 41, 57,  10, 26, 42, 58,  11, 27, 43, 59,
            12, 28, 44, 60,  13, 29, 45, 61, 14, 30, 46, 62,  15, 31, 47, 63);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x8_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_8x8, __ocl_shuffle_transpose_v64_types, 0, [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
            0, 8, 16, 24, 32, 40, 48, 56,
            1, 9, 17, 25, 33, 41, 49, 57,
            2, 10, 18, 26, 34, 42, 50, 58,
            3, 11, 19, 27, 35, 43, 51, 59,
            4, 12, 20, 28, 36, 44, 52, 60,
            5, 13, 21, 29, 37, 45, 53, 61,
            6, 14, 22, 30, 38, 46, 54, 62,
            7, 15, 23, 31, 39, 47, 55, 63);
}]>;

list<OclType> __ocl_shuffle_transpose_v128_types = [ v128i64, v128u64,
                                                     v128i32, v128u32, v128f32,
                                                     v128i16, v128u16,
                                                     v128i8,  v128u8 ];

OclBuiltinImpl __ocl_shuffle_transpose8x16_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_8x16, __ocl_shuffle_transpose_v128_types, 0, [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
            0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120,
            1, 9, 17, 25, 33, 41, 49, 57, 65, 73, 81, 89, 97, 105, 113, 121,
            2, 10, 18, 26, 34, 42, 50, 58, 66, 74, 82, 90, 98, 106, 114, 122,
            3, 11, 19, 27, 35, 43, 51, 59, 67, 75, 83, 91, 99, 107, 115, 123,
            4, 12, 20, 28, 36, 44, 52, 60, 68, 76, 84, 92, 100, 108, 116, 124,
            5, 13, 21, 29, 37, 45, 53, 61, 69, 77, 85, 93, 101, 109, 117, 125,
            6, 14, 22, 30, 38, 46, 54, 62, 70, 78, 86, 94, 102, 110, 118, 126,
            7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119, 127);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose16x8_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_16x8, __ocl_shuffle_transpose_v128_types, 0, [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
            0, 16, 32, 48, 64, 80,  96, 112,  1, 17, 33, 49, 65, 81, 97, 113,
            2, 18, 34, 50, 66, 82,  98, 114,  3, 19, 35, 51, 67, 83, 99, 115,
            4, 20, 36, 52, 68, 84, 100, 116,  5, 21, 37, 53, 69, 85, 101, 117,
            6, 22, 38, 54, 70, 86, 102, 118,  7, 23, 39, 55, 71, 87, 103, 119,
            8, 24, 40, 56, 72, 88, 104, 120,  9, 25, 41, 57, 73, 89, 105, 121,
            10, 26, 42, 58, 74, 90, 106, 122, 11, 27, 43, 59, 75, 91, 107, 123,
            12, 28, 44, 60, 76, 92, 108, 124, 13, 29, 45, 61, 77, 93, 109, 125,
            14, 30, 46, 62, 78, 94, 110, 126, 15, 31, 47, 63, 79, 95, 111, 127);
}]>;

list<OclType> __ocl_shuffle_transpose_v256_types = [ v256i64, v256u64,
                                                     v256i32, v256u32, v256f32,
                                                     v256i16, v256u16,
                                                     v256i8,  v256u8 ];

OclBuiltinImpl __ocl_shuffle_transpose16x16_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_16x16, __ocl_shuffle_transpose_v256_types, 0, [{
    return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
            0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240,
            1, 17, 33, 49, 65, 81, 97, 113, 129, 145, 161, 177, 193, 209, 225, 241,
            2, 18, 34, 50, 66, 82, 98, 114, 130, 146, 162, 178, 194, 210, 226, 242,
            3, 19, 35, 51, 67, 83, 99, 115, 131, 147, 163, 179, 195, 211, 227, 243,
            4, 20, 36, 52, 68, 84, 100, 116, 132, 148, 164, 180, 196, 212, 228, 244,
            5, 21, 37, 53, 69, 85, 101, 117, 133, 149, 165, 181, 197, 213, 229, 245,
            6, 22, 38, 54, 70, 86, 102, 118, 134, 150, 166, 182, 198, 214, 230, 246,
            7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215, 231, 247,
            8, 24, 40, 56, 72, 88, 104, 120, 136, 152, 168, 184, 200, 216, 232, 248,
            9, 25, 41, 57, 73, 89, 105, 121, 137, 153, 169, 185, 201, 217, 233, 249,
            10, 26, 42, 58, 74, 90, 106, 122, 138, 154, 170, 186, 202, 218, 234, 250,
            11, 27, 43, 59, 75, 91, 107, 123, 139, 155, 171, 187, 203, 219, 235, 251,
            12, 28, 44, 60, 76, 92, 108, 124, 140, 156, 172, 188, 204, 220, 236, 252,
            13, 29, 45, 61, 77, 93, 109, 125, 141, 157, 173, 189, 205, 221, 237, 253,
            14, 30, 46, 62, 78, 94, 110, 126, 142, 158, 174, 190, 206, 222, 238, 254,
            15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255);
}]>;
