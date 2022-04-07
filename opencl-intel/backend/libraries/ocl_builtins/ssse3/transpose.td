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



OclBuiltinImpl __ocl_shuffle_transpose4x1_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_4x1, __ocl_shuffle_transpose_4x1.Types, 0, [{
    return $Arg0VarName;
}]>;

OclBuiltinImpl __ocl_shuffle_transpose1x4_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_1x4, __ocl_shuffle_transpose_1x4.Types, 0, [{
    return $Arg0VarName;
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x1_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_8x1, __ocl_shuffle_transpose_8x1.Types, 0, [{
    return $Arg0VarName;
}]>;

OclBuiltinImpl __ocl_shuffle_transpose1x8_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_1x8, __ocl_shuffle_transpose_1x8.Types, 0, [{
    return $Arg0VarName;
}]>;

OclBuiltinImpl __ocl_shuffle_transpose16x1_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_16x1, __ocl_shuffle_transpose_16x1.Types, 0, [{
    return $Arg0VarName;
}]>;

OclBuiltinImpl __ocl_shuffle_transpose1x16_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_1x16, __ocl_shuffle_transpose_1x16.Types, 0, [{
    return $Arg0VarName;
}]>;

OclBuiltinImpl __ocl_shuffle_transpose32x1_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_32x1, __ocl_shuffle_transpose_32x1.Types, 0, [{
    return $Arg0VarName;
}]>;

OclBuiltinImpl __ocl_shuffle_transpose1x32_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_1x32, __ocl_shuffle_transpose_1x32.Types, 0, [{
    return $Arg0VarName;
}]>;

OclBuiltinImpl __ocl_shuffle_transpose64x1_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_64x1, __ocl_shuffle_transpose_64x1.Types, 0, [{
    return $Arg0VarName;
}]>;

OclBuiltinImpl __ocl_shuffle_transpose1x64_sse3 = OclBuiltinImpl<__ocl_shuffle_transpose_1x64, __ocl_shuffle_transpose_1x64.Types, 0, [{
    return $Arg0VarName;
}]>;



// Generated by:
// vfs = [4, 8, 16, 32, 64]
// widths = [2, 3, 4, 8, 16]
// for vf in vfs:
//   for width in widths:
//     for i in range(2):
//       print("\nOclBuiltinImpl __ocl_shuffle_transpose{0}x{1}_sse3 =\
// OclBuiltinImpl<__ocl_shuffle_transpose_{0}x{1}, __ocl_shuffle_transpose_{0}x{1}.Types, 0, \
// [{2}\nreturn __builtin_shufflevector($Arg0VarName, $Arg0VarName,".format(vf, width, '{'))
//       l0 = []
//       for i in range(vf):
//         l1 = []
//         for j in range(width):
//           l1.append(str(i+ j * vf))
//         l0.append(', '.join(l1))
//       print(',\n'.join(l0) + ');\n{}]>;'.format('}'))
//       if vf == width:
//         break;
//       vf, width = width, vf

OclBuiltinImpl __ocl_shuffle_transpose4x2_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_4x2, __ocl_shuffle_transpose_4x2.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 4,
1, 5,
2, 6,
3, 7);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose2x4_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_2x4, __ocl_shuffle_transpose_2x4.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 2, 4, 6,
1, 3, 5, 7);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose4x3_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_4x3, __ocl_shuffle_transpose_4x3.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 4, 8,
1, 5, 9,
2, 6, 10,
3, 7, 11);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose3x4_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_3x4, __ocl_shuffle_transpose_3x4.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 3, 6, 9,
1, 4, 7, 10,
2, 5, 8, 11);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose4x4_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_4x4, __ocl_shuffle_transpose_4x4.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 4, 8, 12,
1, 5, 9, 13,
2, 6, 10, 14,
3, 7, 11, 15);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose4x8_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_4x8, __ocl_shuffle_transpose_4x8.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 4, 8, 12, 16, 20, 24, 28,
1, 5, 9, 13, 17, 21, 25, 29,
2, 6, 10, 14, 18, 22, 26, 30,
3, 7, 11, 15, 19, 23, 27, 31);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x4_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_8x4, __ocl_shuffle_transpose_8x4.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 8, 16, 24,
1, 9, 17, 25,
2, 10, 18, 26,
3, 11, 19, 27,
4, 12, 20, 28,
5, 13, 21, 29,
6, 14, 22, 30,
7, 15, 23, 31);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose4x16_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_4x16, __ocl_shuffle_transpose_4x16.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61,
2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62,
3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose16x4_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_16x4, __ocl_shuffle_transpose_16x4.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 16, 32, 48,
1, 17, 33, 49,
2, 18, 34, 50,
3, 19, 35, 51,
4, 20, 36, 52,
5, 21, 37, 53,
6, 22, 38, 54,
7, 23, 39, 55,
8, 24, 40, 56,
9, 25, 41, 57,
10, 26, 42, 58,
11, 27, 43, 59,
12, 28, 44, 60,
13, 29, 45, 61,
14, 30, 46, 62,
15, 31, 47, 63);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x2_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_8x2, __ocl_shuffle_transpose_8x2.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 8,
1, 9,
2, 10,
3, 11,
4, 12,
5, 13,
6, 14,
7, 15);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose2x8_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_2x8, __ocl_shuffle_transpose_2x8.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 2, 4, 6, 8, 10, 12, 14,
1, 3, 5, 7, 9, 11, 13, 15);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x3_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_8x3, __ocl_shuffle_transpose_8x3.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 8, 16,
1, 9, 17,
2, 10, 18,
3, 11, 19,
4, 12, 20,
5, 13, 21,
6, 14, 22,
7, 15, 23);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose3x8_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_3x8, __ocl_shuffle_transpose_3x8.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 3, 6, 9, 12, 15, 18, 21,
1, 4, 7, 10, 13, 16, 19, 22,
2, 5, 8, 11, 14, 17, 20, 23);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x4_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_8x4, __ocl_shuffle_transpose_8x4.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 8, 16, 24,
1, 9, 17, 25,
2, 10, 18, 26,
3, 11, 19, 27,
4, 12, 20, 28,
5, 13, 21, 29,
6, 14, 22, 30,
7, 15, 23, 31);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose4x8_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_4x8, __ocl_shuffle_transpose_4x8.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 4, 8, 12, 16, 20, 24, 28,
1, 5, 9, 13, 17, 21, 25, 29,
2, 6, 10, 14, 18, 22, 26, 30,
3, 7, 11, 15, 19, 23, 27, 31);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x8_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_8x8, __ocl_shuffle_transpose_8x8.Types, 0, [{
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

OclBuiltinImpl __ocl_shuffle_transpose8x16_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_8x16, __ocl_shuffle_transpose_8x16.Types, 0, [{
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

OclBuiltinImpl __ocl_shuffle_transpose16x8_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_16x8, __ocl_shuffle_transpose_16x8.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 16, 32, 48, 64, 80, 96, 112,
1, 17, 33, 49, 65, 81, 97, 113,
2, 18, 34, 50, 66, 82, 98, 114,
3, 19, 35, 51, 67, 83, 99, 115,
4, 20, 36, 52, 68, 84, 100, 116,
5, 21, 37, 53, 69, 85, 101, 117,
6, 22, 38, 54, 70, 86, 102, 118,
7, 23, 39, 55, 71, 87, 103, 119,
8, 24, 40, 56, 72, 88, 104, 120,
9, 25, 41, 57, 73, 89, 105, 121,
10, 26, 42, 58, 74, 90, 106, 122,
11, 27, 43, 59, 75, 91, 107, 123,
12, 28, 44, 60, 76, 92, 108, 124,
13, 29, 45, 61, 77, 93, 109, 125,
14, 30, 46, 62, 78, 94, 110, 126,
15, 31, 47, 63, 79, 95, 111, 127);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose16x2_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_16x2, __ocl_shuffle_transpose_16x2.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 16,
1, 17,
2, 18,
3, 19,
4, 20,
5, 21,
6, 22,
7, 23,
8, 24,
9, 25,
10, 26,
11, 27,
12, 28,
13, 29,
14, 30,
15, 31);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose2x16_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_2x16, __ocl_shuffle_transpose_2x16.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose16x3_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_16x3, __ocl_shuffle_transpose_16x3.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 16, 32,
1, 17, 33,
2, 18, 34,
3, 19, 35,
4, 20, 36,
5, 21, 37,
6, 22, 38,
7, 23, 39,
8, 24, 40,
9, 25, 41,
10, 26, 42,
11, 27, 43,
12, 28, 44,
13, 29, 45,
14, 30, 46,
15, 31, 47);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose3x16_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_3x16, __ocl_shuffle_transpose_3x16.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45,
1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46,
2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose16x4_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_16x4, __ocl_shuffle_transpose_16x4.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 16, 32, 48,
1, 17, 33, 49,
2, 18, 34, 50,
3, 19, 35, 51,
4, 20, 36, 52,
5, 21, 37, 53,
6, 22, 38, 54,
7, 23, 39, 55,
8, 24, 40, 56,
9, 25, 41, 57,
10, 26, 42, 58,
11, 27, 43, 59,
12, 28, 44, 60,
13, 29, 45, 61,
14, 30, 46, 62,
15, 31, 47, 63);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose4x16_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_4x16, __ocl_shuffle_transpose_4x16.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61,
2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62,
3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose16x8_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_16x8, __ocl_shuffle_transpose_16x8.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 16, 32, 48, 64, 80, 96, 112,
1, 17, 33, 49, 65, 81, 97, 113,
2, 18, 34, 50, 66, 82, 98, 114,
3, 19, 35, 51, 67, 83, 99, 115,
4, 20, 36, 52, 68, 84, 100, 116,
5, 21, 37, 53, 69, 85, 101, 117,
6, 22, 38, 54, 70, 86, 102, 118,
7, 23, 39, 55, 71, 87, 103, 119,
8, 24, 40, 56, 72, 88, 104, 120,
9, 25, 41, 57, 73, 89, 105, 121,
10, 26, 42, 58, 74, 90, 106, 122,
11, 27, 43, 59, 75, 91, 107, 123,
12, 28, 44, 60, 76, 92, 108, 124,
13, 29, 45, 61, 77, 93, 109, 125,
14, 30, 46, 62, 78, 94, 110, 126,
15, 31, 47, 63, 79, 95, 111, 127);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x16_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_8x16, __ocl_shuffle_transpose_8x16.Types, 0, [{
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

OclBuiltinImpl __ocl_shuffle_transpose16x16_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_16x16, __ocl_shuffle_transpose_16x16.Types, 0, [{
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

OclBuiltinImpl __ocl_shuffle_transpose32x2_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_32x2, __ocl_shuffle_transpose_32x2.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 32,
1, 33,
2, 34,
3, 35,
4, 36,
5, 37,
6, 38,
7, 39,
8, 40,
9, 41,
10, 42,
11, 43,
12, 44,
13, 45,
14, 46,
15, 47,
16, 48,
17, 49,
18, 50,
19, 51,
20, 52,
21, 53,
22, 54,
23, 55,
24, 56,
25, 57,
26, 58,
27, 59,
28, 60,
29, 61,
30, 62,
31, 63);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose2x32_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_2x32, __ocl_shuffle_transpose_2x32.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose32x3_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_32x3, __ocl_shuffle_transpose_32x3.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 32, 64,
1, 33, 65,
2, 34, 66,
3, 35, 67,
4, 36, 68,
5, 37, 69,
6, 38, 70,
7, 39, 71,
8, 40, 72,
9, 41, 73,
10, 42, 74,
11, 43, 75,
12, 44, 76,
13, 45, 77,
14, 46, 78,
15, 47, 79,
16, 48, 80,
17, 49, 81,
18, 50, 82,
19, 51, 83,
20, 52, 84,
21, 53, 85,
22, 54, 86,
23, 55, 87,
24, 56, 88,
25, 57, 89,
26, 58, 90,
27, 59, 91,
28, 60, 92,
29, 61, 93,
30, 62, 94,
31, 63, 95);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose3x32_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_3x32, __ocl_shuffle_transpose_3x32.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 72, 75, 78, 81, 84, 87, 90, 93,
1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 52, 55, 58, 61, 64, 67, 70, 73, 76, 79, 82, 85, 88, 91, 94,
2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47, 50, 53, 56, 59, 62, 65, 68, 71, 74, 77, 80, 83, 86, 89, 92, 95);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose32x4_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_32x4, __ocl_shuffle_transpose_32x4.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 32, 64, 96,
1, 33, 65, 97,
2, 34, 66, 98,
3, 35, 67, 99,
4, 36, 68, 100,
5, 37, 69, 101,
6, 38, 70, 102,
7, 39, 71, 103,
8, 40, 72, 104,
9, 41, 73, 105,
10, 42, 74, 106,
11, 43, 75, 107,
12, 44, 76, 108,
13, 45, 77, 109,
14, 46, 78, 110,
15, 47, 79, 111,
16, 48, 80, 112,
17, 49, 81, 113,
18, 50, 82, 114,
19, 51, 83, 115,
20, 52, 84, 116,
21, 53, 85, 117,
22, 54, 86, 118,
23, 55, 87, 119,
24, 56, 88, 120,
25, 57, 89, 121,
26, 58, 90, 122,
27, 59, 91, 123,
28, 60, 92, 124,
29, 61, 93, 125,
30, 62, 94, 126,
31, 63, 95, 127);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose4x32_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_4x32, __ocl_shuffle_transpose_4x32.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124,
1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125,
2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62, 66, 70, 74, 78, 82, 86, 90, 94, 98, 102, 106, 110, 114, 118, 122, 126,
3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63, 67, 71, 75, 79, 83, 87, 91, 95, 99, 103, 107, 111, 115, 119, 123, 127);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose32x8_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_32x8, __ocl_shuffle_transpose_32x8.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 32, 64, 96, 128, 160, 192, 224,
1, 33, 65, 97, 129, 161, 193, 225,
2, 34, 66, 98, 130, 162, 194, 226,
3, 35, 67, 99, 131, 163, 195, 227,
4, 36, 68, 100, 132, 164, 196, 228,
5, 37, 69, 101, 133, 165, 197, 229,
6, 38, 70, 102, 134, 166, 198, 230,
7, 39, 71, 103, 135, 167, 199, 231,
8, 40, 72, 104, 136, 168, 200, 232,
9, 41, 73, 105, 137, 169, 201, 233,
10, 42, 74, 106, 138, 170, 202, 234,
11, 43, 75, 107, 139, 171, 203, 235,
12, 44, 76, 108, 140, 172, 204, 236,
13, 45, 77, 109, 141, 173, 205, 237,
14, 46, 78, 110, 142, 174, 206, 238,
15, 47, 79, 111, 143, 175, 207, 239,
16, 48, 80, 112, 144, 176, 208, 240,
17, 49, 81, 113, 145, 177, 209, 241,
18, 50, 82, 114, 146, 178, 210, 242,
19, 51, 83, 115, 147, 179, 211, 243,
20, 52, 84, 116, 148, 180, 212, 244,
21, 53, 85, 117, 149, 181, 213, 245,
22, 54, 86, 118, 150, 182, 214, 246,
23, 55, 87, 119, 151, 183, 215, 247,
24, 56, 88, 120, 152, 184, 216, 248,
25, 57, 89, 121, 153, 185, 217, 249,
26, 58, 90, 122, 154, 186, 218, 250,
27, 59, 91, 123, 155, 187, 219, 251,
28, 60, 92, 124, 156, 188, 220, 252,
29, 61, 93, 125, 157, 189, 221, 253,
30, 62, 94, 126, 158, 190, 222, 254,
31, 63, 95, 127, 159, 191, 223, 255);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x32_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_8x32, __ocl_shuffle_transpose_8x32.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248,
1, 9, 17, 25, 33, 41, 49, 57, 65, 73, 81, 89, 97, 105, 113, 121, 129, 137, 145, 153, 161, 169, 177, 185, 193, 201, 209, 217, 225, 233, 241, 249,
2, 10, 18, 26, 34, 42, 50, 58, 66, 74, 82, 90, 98, 106, 114, 122, 130, 138, 146, 154, 162, 170, 178, 186, 194, 202, 210, 218, 226, 234, 242, 250,
3, 11, 19, 27, 35, 43, 51, 59, 67, 75, 83, 91, 99, 107, 115, 123, 131, 139, 147, 155, 163, 171, 179, 187, 195, 203, 211, 219, 227, 235, 243, 251,
4, 12, 20, 28, 36, 44, 52, 60, 68, 76, 84, 92, 100, 108, 116, 124, 132, 140, 148, 156, 164, 172, 180, 188, 196, 204, 212, 220, 228, 236, 244, 252,
5, 13, 21, 29, 37, 45, 53, 61, 69, 77, 85, 93, 101, 109, 117, 125, 133, 141, 149, 157, 165, 173, 181, 189, 197, 205, 213, 221, 229, 237, 245, 253,
6, 14, 22, 30, 38, 46, 54, 62, 70, 78, 86, 94, 102, 110, 118, 126, 134, 142, 150, 158, 166, 174, 182, 190, 198, 206, 214, 222, 230, 238, 246, 254,
7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119, 127, 135, 143, 151, 159, 167, 175, 183, 191, 199, 207, 215, 223, 231, 239, 247, 255);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose32x16_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_32x16, __ocl_shuffle_transpose_32x16.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480,
1, 33, 65, 97, 129, 161, 193, 225, 257, 289, 321, 353, 385, 417, 449, 481,
2, 34, 66, 98, 130, 162, 194, 226, 258, 290, 322, 354, 386, 418, 450, 482,
3, 35, 67, 99, 131, 163, 195, 227, 259, 291, 323, 355, 387, 419, 451, 483,
4, 36, 68, 100, 132, 164, 196, 228, 260, 292, 324, 356, 388, 420, 452, 484,
5, 37, 69, 101, 133, 165, 197, 229, 261, 293, 325, 357, 389, 421, 453, 485,
6, 38, 70, 102, 134, 166, 198, 230, 262, 294, 326, 358, 390, 422, 454, 486,
7, 39, 71, 103, 135, 167, 199, 231, 263, 295, 327, 359, 391, 423, 455, 487,
8, 40, 72, 104, 136, 168, 200, 232, 264, 296, 328, 360, 392, 424, 456, 488,
9, 41, 73, 105, 137, 169, 201, 233, 265, 297, 329, 361, 393, 425, 457, 489,
10, 42, 74, 106, 138, 170, 202, 234, 266, 298, 330, 362, 394, 426, 458, 490,
11, 43, 75, 107, 139, 171, 203, 235, 267, 299, 331, 363, 395, 427, 459, 491,
12, 44, 76, 108, 140, 172, 204, 236, 268, 300, 332, 364, 396, 428, 460, 492,
13, 45, 77, 109, 141, 173, 205, 237, 269, 301, 333, 365, 397, 429, 461, 493,
14, 46, 78, 110, 142, 174, 206, 238, 270, 302, 334, 366, 398, 430, 462, 494,
15, 47, 79, 111, 143, 175, 207, 239, 271, 303, 335, 367, 399, 431, 463, 495,
16, 48, 80, 112, 144, 176, 208, 240, 272, 304, 336, 368, 400, 432, 464, 496,
17, 49, 81, 113, 145, 177, 209, 241, 273, 305, 337, 369, 401, 433, 465, 497,
18, 50, 82, 114, 146, 178, 210, 242, 274, 306, 338, 370, 402, 434, 466, 498,
19, 51, 83, 115, 147, 179, 211, 243, 275, 307, 339, 371, 403, 435, 467, 499,
20, 52, 84, 116, 148, 180, 212, 244, 276, 308, 340, 372, 404, 436, 468, 500,
21, 53, 85, 117, 149, 181, 213, 245, 277, 309, 341, 373, 405, 437, 469, 501,
22, 54, 86, 118, 150, 182, 214, 246, 278, 310, 342, 374, 406, 438, 470, 502,
23, 55, 87, 119, 151, 183, 215, 247, 279, 311, 343, 375, 407, 439, 471, 503,
24, 56, 88, 120, 152, 184, 216, 248, 280, 312, 344, 376, 408, 440, 472, 504,
25, 57, 89, 121, 153, 185, 217, 249, 281, 313, 345, 377, 409, 441, 473, 505,
26, 58, 90, 122, 154, 186, 218, 250, 282, 314, 346, 378, 410, 442, 474, 506,
27, 59, 91, 123, 155, 187, 219, 251, 283, 315, 347, 379, 411, 443, 475, 507,
28, 60, 92, 124, 156, 188, 220, 252, 284, 316, 348, 380, 412, 444, 476, 508,
29, 61, 93, 125, 157, 189, 221, 253, 285, 317, 349, 381, 413, 445, 477, 509,
30, 62, 94, 126, 158, 190, 222, 254, 286, 318, 350, 382, 414, 446, 478, 510,
31, 63, 95, 127, 159, 191, 223, 255, 287, 319, 351, 383, 415, 447, 479, 511);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose16x32_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_16x32, __ocl_shuffle_transpose_16x32.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304, 320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480, 496,
1, 17, 33, 49, 65, 81, 97, 113, 129, 145, 161, 177, 193, 209, 225, 241, 257, 273, 289, 305, 321, 337, 353, 369, 385, 401, 417, 433, 449, 465, 481, 497,
2, 18, 34, 50, 66, 82, 98, 114, 130, 146, 162, 178, 194, 210, 226, 242, 258, 274, 290, 306, 322, 338, 354, 370, 386, 402, 418, 434, 450, 466, 482, 498,
3, 19, 35, 51, 67, 83, 99, 115, 131, 147, 163, 179, 195, 211, 227, 243, 259, 275, 291, 307, 323, 339, 355, 371, 387, 403, 419, 435, 451, 467, 483, 499,
4, 20, 36, 52, 68, 84, 100, 116, 132, 148, 164, 180, 196, 212, 228, 244, 260, 276, 292, 308, 324, 340, 356, 372, 388, 404, 420, 436, 452, 468, 484, 500,
5, 21, 37, 53, 69, 85, 101, 117, 133, 149, 165, 181, 197, 213, 229, 245, 261, 277, 293, 309, 325, 341, 357, 373, 389, 405, 421, 437, 453, 469, 485, 501,
6, 22, 38, 54, 70, 86, 102, 118, 134, 150, 166, 182, 198, 214, 230, 246, 262, 278, 294, 310, 326, 342, 358, 374, 390, 406, 422, 438, 454, 470, 486, 502,
7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215, 231, 247, 263, 279, 295, 311, 327, 343, 359, 375, 391, 407, 423, 439, 455, 471, 487, 503,
8, 24, 40, 56, 72, 88, 104, 120, 136, 152, 168, 184, 200, 216, 232, 248, 264, 280, 296, 312, 328, 344, 360, 376, 392, 408, 424, 440, 456, 472, 488, 504,
9, 25, 41, 57, 73, 89, 105, 121, 137, 153, 169, 185, 201, 217, 233, 249, 265, 281, 297, 313, 329, 345, 361, 377, 393, 409, 425, 441, 457, 473, 489, 505,
10, 26, 42, 58, 74, 90, 106, 122, 138, 154, 170, 186, 202, 218, 234, 250, 266, 282, 298, 314, 330, 346, 362, 378, 394, 410, 426, 442, 458, 474, 490, 506,
11, 27, 43, 59, 75, 91, 107, 123, 139, 155, 171, 187, 203, 219, 235, 251, 267, 283, 299, 315, 331, 347, 363, 379, 395, 411, 427, 443, 459, 475, 491, 507,
12, 28, 44, 60, 76, 92, 108, 124, 140, 156, 172, 188, 204, 220, 236, 252, 268, 284, 300, 316, 332, 348, 364, 380, 396, 412, 428, 444, 460, 476, 492, 508,
13, 29, 45, 61, 77, 93, 109, 125, 141, 157, 173, 189, 205, 221, 237, 253, 269, 285, 301, 317, 333, 349, 365, 381, 397, 413, 429, 445, 461, 477, 493, 509,
14, 30, 46, 62, 78, 94, 110, 126, 142, 158, 174, 190, 206, 222, 238, 254, 270, 286, 302, 318, 334, 350, 366, 382, 398, 414, 430, 446, 462, 478, 494, 510,
15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255, 271, 287, 303, 319, 335, 351, 367, 383, 399, 415, 431, 447, 463, 479, 495, 511);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose64x2_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_64x2, __ocl_shuffle_transpose_64x2.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 64,
1, 65,
2, 66,
3, 67,
4, 68,
5, 69,
6, 70,
7, 71,
8, 72,
9, 73,
10, 74,
11, 75,
12, 76,
13, 77,
14, 78,
15, 79,
16, 80,
17, 81,
18, 82,
19, 83,
20, 84,
21, 85,
22, 86,
23, 87,
24, 88,
25, 89,
26, 90,
27, 91,
28, 92,
29, 93,
30, 94,
31, 95,
32, 96,
33, 97,
34, 98,
35, 99,
36, 100,
37, 101,
38, 102,
39, 103,
40, 104,
41, 105,
42, 106,
43, 107,
44, 108,
45, 109,
46, 110,
47, 111,
48, 112,
49, 113,
50, 114,
51, 115,
52, 116,
53, 117,
54, 118,
55, 119,
56, 120,
57, 121,
58, 122,
59, 123,
60, 124,
61, 125,
62, 126,
63, 127);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose2x64_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_2x64, __ocl_shuffle_transpose_2x64.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 108, 110, 112, 114, 116, 118, 120, 122, 124, 126,
1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105, 107, 109, 111, 113, 115, 117, 119, 121, 123, 125, 127);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose64x3_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_64x3, __ocl_shuffle_transpose_64x3.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 64, 128,
1, 65, 129,
2, 66, 130,
3, 67, 131,
4, 68, 132,
5, 69, 133,
6, 70, 134,
7, 71, 135,
8, 72, 136,
9, 73, 137,
10, 74, 138,
11, 75, 139,
12, 76, 140,
13, 77, 141,
14, 78, 142,
15, 79, 143,
16, 80, 144,
17, 81, 145,
18, 82, 146,
19, 83, 147,
20, 84, 148,
21, 85, 149,
22, 86, 150,
23, 87, 151,
24, 88, 152,
25, 89, 153,
26, 90, 154,
27, 91, 155,
28, 92, 156,
29, 93, 157,
30, 94, 158,
31, 95, 159,
32, 96, 160,
33, 97, 161,
34, 98, 162,
35, 99, 163,
36, 100, 164,
37, 101, 165,
38, 102, 166,
39, 103, 167,
40, 104, 168,
41, 105, 169,
42, 106, 170,
43, 107, 171,
44, 108, 172,
45, 109, 173,
46, 110, 174,
47, 111, 175,
48, 112, 176,
49, 113, 177,
50, 114, 178,
51, 115, 179,
52, 116, 180,
53, 117, 181,
54, 118, 182,
55, 119, 183,
56, 120, 184,
57, 121, 185,
58, 122, 186,
59, 123, 187,
60, 124, 188,
61, 125, 189,
62, 126, 190,
63, 127, 191);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose3x64_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_3x64, __ocl_shuffle_transpose_3x64.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 72, 75, 78, 81, 84, 87, 90, 93, 96, 99, 102, 105, 108, 111, 114, 117, 120, 123, 126, 129, 132, 135, 138, 141, 144, 147, 150, 153, 156, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 189,
1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 52, 55, 58, 61, 64, 67, 70, 73, 76, 79, 82, 85, 88, 91, 94, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 127, 130, 133, 136, 139, 142, 145, 148, 151, 154, 157, 160, 163, 166, 169, 172, 175, 178, 181, 184, 187, 190,
2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47, 50, 53, 56, 59, 62, 65, 68, 71, 74, 77, 80, 83, 86, 89, 92, 95, 98, 101, 104, 107, 110, 113, 116, 119, 122, 125, 128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173, 176, 179, 182, 185, 188, 191);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose64x4_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_64x4, __ocl_shuffle_transpose_64x4.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 64, 128, 192,
1, 65, 129, 193,
2, 66, 130, 194,
3, 67, 131, 195,
4, 68, 132, 196,
5, 69, 133, 197,
6, 70, 134, 198,
7, 71, 135, 199,
8, 72, 136, 200,
9, 73, 137, 201,
10, 74, 138, 202,
11, 75, 139, 203,
12, 76, 140, 204,
13, 77, 141, 205,
14, 78, 142, 206,
15, 79, 143, 207,
16, 80, 144, 208,
17, 81, 145, 209,
18, 82, 146, 210,
19, 83, 147, 211,
20, 84, 148, 212,
21, 85, 149, 213,
22, 86, 150, 214,
23, 87, 151, 215,
24, 88, 152, 216,
25, 89, 153, 217,
26, 90, 154, 218,
27, 91, 155, 219,
28, 92, 156, 220,
29, 93, 157, 221,
30, 94, 158, 222,
31, 95, 159, 223,
32, 96, 160, 224,
33, 97, 161, 225,
34, 98, 162, 226,
35, 99, 163, 227,
36, 100, 164, 228,
37, 101, 165, 229,
38, 102, 166, 230,
39, 103, 167, 231,
40, 104, 168, 232,
41, 105, 169, 233,
42, 106, 170, 234,
43, 107, 171, 235,
44, 108, 172, 236,
45, 109, 173, 237,
46, 110, 174, 238,
47, 111, 175, 239,
48, 112, 176, 240,
49, 113, 177, 241,
50, 114, 178, 242,
51, 115, 179, 243,
52, 116, 180, 244,
53, 117, 181, 245,
54, 118, 182, 246,
55, 119, 183, 247,
56, 120, 184, 248,
57, 121, 185, 249,
58, 122, 186, 250,
59, 123, 187, 251,
60, 124, 188, 252,
61, 125, 189, 253,
62, 126, 190, 254,
63, 127, 191, 255);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose4x64_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_4x64, __ocl_shuffle_transpose_4x64.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252,
1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 129, 133, 137, 141, 145, 149, 153, 157, 161, 165, 169, 173, 177, 181, 185, 189, 193, 197, 201, 205, 209, 213, 217, 221, 225, 229, 233, 237, 241, 245, 249, 253,
2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62, 66, 70, 74, 78, 82, 86, 90, 94, 98, 102, 106, 110, 114, 118, 122, 126, 130, 134, 138, 142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 214, 218, 222, 226, 230, 234, 238, 242, 246, 250, 254,
3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63, 67, 71, 75, 79, 83, 87, 91, 95, 99, 103, 107, 111, 115, 119, 123, 127, 131, 135, 139, 143, 147, 151, 155, 159, 163, 167, 171, 175, 179, 183, 187, 191, 195, 199, 203, 207, 211, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose64x8_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_64x8, __ocl_shuffle_transpose_64x8.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 64, 128, 192, 256, 320, 384, 448,
1, 65, 129, 193, 257, 321, 385, 449,
2, 66, 130, 194, 258, 322, 386, 450,
3, 67, 131, 195, 259, 323, 387, 451,
4, 68, 132, 196, 260, 324, 388, 452,
5, 69, 133, 197, 261, 325, 389, 453,
6, 70, 134, 198, 262, 326, 390, 454,
7, 71, 135, 199, 263, 327, 391, 455,
8, 72, 136, 200, 264, 328, 392, 456,
9, 73, 137, 201, 265, 329, 393, 457,
10, 74, 138, 202, 266, 330, 394, 458,
11, 75, 139, 203, 267, 331, 395, 459,
12, 76, 140, 204, 268, 332, 396, 460,
13, 77, 141, 205, 269, 333, 397, 461,
14, 78, 142, 206, 270, 334, 398, 462,
15, 79, 143, 207, 271, 335, 399, 463,
16, 80, 144, 208, 272, 336, 400, 464,
17, 81, 145, 209, 273, 337, 401, 465,
18, 82, 146, 210, 274, 338, 402, 466,
19, 83, 147, 211, 275, 339, 403, 467,
20, 84, 148, 212, 276, 340, 404, 468,
21, 85, 149, 213, 277, 341, 405, 469,
22, 86, 150, 214, 278, 342, 406, 470,
23, 87, 151, 215, 279, 343, 407, 471,
24, 88, 152, 216, 280, 344, 408, 472,
25, 89, 153, 217, 281, 345, 409, 473,
26, 90, 154, 218, 282, 346, 410, 474,
27, 91, 155, 219, 283, 347, 411, 475,
28, 92, 156, 220, 284, 348, 412, 476,
29, 93, 157, 221, 285, 349, 413, 477,
30, 94, 158, 222, 286, 350, 414, 478,
31, 95, 159, 223, 287, 351, 415, 479,
32, 96, 160, 224, 288, 352, 416, 480,
33, 97, 161, 225, 289, 353, 417, 481,
34, 98, 162, 226, 290, 354, 418, 482,
35, 99, 163, 227, 291, 355, 419, 483,
36, 100, 164, 228, 292, 356, 420, 484,
37, 101, 165, 229, 293, 357, 421, 485,
38, 102, 166, 230, 294, 358, 422, 486,
39, 103, 167, 231, 295, 359, 423, 487,
40, 104, 168, 232, 296, 360, 424, 488,
41, 105, 169, 233, 297, 361, 425, 489,
42, 106, 170, 234, 298, 362, 426, 490,
43, 107, 171, 235, 299, 363, 427, 491,
44, 108, 172, 236, 300, 364, 428, 492,
45, 109, 173, 237, 301, 365, 429, 493,
46, 110, 174, 238, 302, 366, 430, 494,
47, 111, 175, 239, 303, 367, 431, 495,
48, 112, 176, 240, 304, 368, 432, 496,
49, 113, 177, 241, 305, 369, 433, 497,
50, 114, 178, 242, 306, 370, 434, 498,
51, 115, 179, 243, 307, 371, 435, 499,
52, 116, 180, 244, 308, 372, 436, 500,
53, 117, 181, 245, 309, 373, 437, 501,
54, 118, 182, 246, 310, 374, 438, 502,
55, 119, 183, 247, 311, 375, 439, 503,
56, 120, 184, 248, 312, 376, 440, 504,
57, 121, 185, 249, 313, 377, 441, 505,
58, 122, 186, 250, 314, 378, 442, 506,
59, 123, 187, 251, 315, 379, 443, 507,
60, 124, 188, 252, 316, 380, 444, 508,
61, 125, 189, 253, 317, 381, 445, 509,
62, 126, 190, 254, 318, 382, 446, 510,
63, 127, 191, 255, 319, 383, 447, 511);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose8x64_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_8x64, __ocl_shuffle_transpose_8x64.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 256, 264, 272, 280, 288, 296, 304, 312, 320, 328, 336, 344, 352, 360, 368, 376, 384, 392, 400, 408, 416, 424, 432, 440, 448, 456, 464, 472, 480, 488, 496, 504,
1, 9, 17, 25, 33, 41, 49, 57, 65, 73, 81, 89, 97, 105, 113, 121, 129, 137, 145, 153, 161, 169, 177, 185, 193, 201, 209, 217, 225, 233, 241, 249, 257, 265, 273, 281, 289, 297, 305, 313, 321, 329, 337, 345, 353, 361, 369, 377, 385, 393, 401, 409, 417, 425, 433, 441, 449, 457, 465, 473, 481, 489, 497, 505,
2, 10, 18, 26, 34, 42, 50, 58, 66, 74, 82, 90, 98, 106, 114, 122, 130, 138, 146, 154, 162, 170, 178, 186, 194, 202, 210, 218, 226, 234, 242, 250, 258, 266, 274, 282, 290, 298, 306, 314, 322, 330, 338, 346, 354, 362, 370, 378, 386, 394, 402, 410, 418, 426, 434, 442, 450, 458, 466, 474, 482, 490, 498, 506,
3, 11, 19, 27, 35, 43, 51, 59, 67, 75, 83, 91, 99, 107, 115, 123, 131, 139, 147, 155, 163, 171, 179, 187, 195, 203, 211, 219, 227, 235, 243, 251, 259, 267, 275, 283, 291, 299, 307, 315, 323, 331, 339, 347, 355, 363, 371, 379, 387, 395, 403, 411, 419, 427, 435, 443, 451, 459, 467, 475, 483, 491, 499, 507,
4, 12, 20, 28, 36, 44, 52, 60, 68, 76, 84, 92, 100, 108, 116, 124, 132, 140, 148, 156, 164, 172, 180, 188, 196, 204, 212, 220, 228, 236, 244, 252, 260, 268, 276, 284, 292, 300, 308, 316, 324, 332, 340, 348, 356, 364, 372, 380, 388, 396, 404, 412, 420, 428, 436, 444, 452, 460, 468, 476, 484, 492, 500, 508,
5, 13, 21, 29, 37, 45, 53, 61, 69, 77, 85, 93, 101, 109, 117, 125, 133, 141, 149, 157, 165, 173, 181, 189, 197, 205, 213, 221, 229, 237, 245, 253, 261, 269, 277, 285, 293, 301, 309, 317, 325, 333, 341, 349, 357, 365, 373, 381, 389, 397, 405, 413, 421, 429, 437, 445, 453, 461, 469, 477, 485, 493, 501, 509,
6, 14, 22, 30, 38, 46, 54, 62, 70, 78, 86, 94, 102, 110, 118, 126, 134, 142, 150, 158, 166, 174, 182, 190, 198, 206, 214, 222, 230, 238, 246, 254, 262, 270, 278, 286, 294, 302, 310, 318, 326, 334, 342, 350, 358, 366, 374, 382, 390, 398, 406, 414, 422, 430, 438, 446, 454, 462, 470, 478, 486, 494, 502, 510,
7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119, 127, 135, 143, 151, 159, 167, 175, 183, 191, 199, 207, 215, 223, 231, 239, 247, 255, 263, 271, 279, 287, 295, 303, 311, 319, 327, 335, 343, 351, 359, 367, 375, 383, 391, 399, 407, 415, 423, 431, 439, 447, 455, 463, 471, 479, 487, 495, 503, 511);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose64x16_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_64x16, __ocl_shuffle_transpose_64x16.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960,
1, 65, 129, 193, 257, 321, 385, 449, 513, 577, 641, 705, 769, 833, 897, 961,
2, 66, 130, 194, 258, 322, 386, 450, 514, 578, 642, 706, 770, 834, 898, 962,
3, 67, 131, 195, 259, 323, 387, 451, 515, 579, 643, 707, 771, 835, 899, 963,
4, 68, 132, 196, 260, 324, 388, 452, 516, 580, 644, 708, 772, 836, 900, 964,
5, 69, 133, 197, 261, 325, 389, 453, 517, 581, 645, 709, 773, 837, 901, 965,
6, 70, 134, 198, 262, 326, 390, 454, 518, 582, 646, 710, 774, 838, 902, 966,
7, 71, 135, 199, 263, 327, 391, 455, 519, 583, 647, 711, 775, 839, 903, 967,
8, 72, 136, 200, 264, 328, 392, 456, 520, 584, 648, 712, 776, 840, 904, 968,
9, 73, 137, 201, 265, 329, 393, 457, 521, 585, 649, 713, 777, 841, 905, 969,
10, 74, 138, 202, 266, 330, 394, 458, 522, 586, 650, 714, 778, 842, 906, 970,
11, 75, 139, 203, 267, 331, 395, 459, 523, 587, 651, 715, 779, 843, 907, 971,
12, 76, 140, 204, 268, 332, 396, 460, 524, 588, 652, 716, 780, 844, 908, 972,
13, 77, 141, 205, 269, 333, 397, 461, 525, 589, 653, 717, 781, 845, 909, 973,
14, 78, 142, 206, 270, 334, 398, 462, 526, 590, 654, 718, 782, 846, 910, 974,
15, 79, 143, 207, 271, 335, 399, 463, 527, 591, 655, 719, 783, 847, 911, 975,
16, 80, 144, 208, 272, 336, 400, 464, 528, 592, 656, 720, 784, 848, 912, 976,
17, 81, 145, 209, 273, 337, 401, 465, 529, 593, 657, 721, 785, 849, 913, 977,
18, 82, 146, 210, 274, 338, 402, 466, 530, 594, 658, 722, 786, 850, 914, 978,
19, 83, 147, 211, 275, 339, 403, 467, 531, 595, 659, 723, 787, 851, 915, 979,
20, 84, 148, 212, 276, 340, 404, 468, 532, 596, 660, 724, 788, 852, 916, 980,
21, 85, 149, 213, 277, 341, 405, 469, 533, 597, 661, 725, 789, 853, 917, 981,
22, 86, 150, 214, 278, 342, 406, 470, 534, 598, 662, 726, 790, 854, 918, 982,
23, 87, 151, 215, 279, 343, 407, 471, 535, 599, 663, 727, 791, 855, 919, 983,
24, 88, 152, 216, 280, 344, 408, 472, 536, 600, 664, 728, 792, 856, 920, 984,
25, 89, 153, 217, 281, 345, 409, 473, 537, 601, 665, 729, 793, 857, 921, 985,
26, 90, 154, 218, 282, 346, 410, 474, 538, 602, 666, 730, 794, 858, 922, 986,
27, 91, 155, 219, 283, 347, 411, 475, 539, 603, 667, 731, 795, 859, 923, 987,
28, 92, 156, 220, 284, 348, 412, 476, 540, 604, 668, 732, 796, 860, 924, 988,
29, 93, 157, 221, 285, 349, 413, 477, 541, 605, 669, 733, 797, 861, 925, 989,
30, 94, 158, 222, 286, 350, 414, 478, 542, 606, 670, 734, 798, 862, 926, 990,
31, 95, 159, 223, 287, 351, 415, 479, 543, 607, 671, 735, 799, 863, 927, 991,
32, 96, 160, 224, 288, 352, 416, 480, 544, 608, 672, 736, 800, 864, 928, 992,
33, 97, 161, 225, 289, 353, 417, 481, 545, 609, 673, 737, 801, 865, 929, 993,
34, 98, 162, 226, 290, 354, 418, 482, 546, 610, 674, 738, 802, 866, 930, 994,
35, 99, 163, 227, 291, 355, 419, 483, 547, 611, 675, 739, 803, 867, 931, 995,
36, 100, 164, 228, 292, 356, 420, 484, 548, 612, 676, 740, 804, 868, 932, 996,
37, 101, 165, 229, 293, 357, 421, 485, 549, 613, 677, 741, 805, 869, 933, 997,
38, 102, 166, 230, 294, 358, 422, 486, 550, 614, 678, 742, 806, 870, 934, 998,
39, 103, 167, 231, 295, 359, 423, 487, 551, 615, 679, 743, 807, 871, 935, 999,
40, 104, 168, 232, 296, 360, 424, 488, 552, 616, 680, 744, 808, 872, 936, 1000,
41, 105, 169, 233, 297, 361, 425, 489, 553, 617, 681, 745, 809, 873, 937, 1001,
42, 106, 170, 234, 298, 362, 426, 490, 554, 618, 682, 746, 810, 874, 938, 1002,
43, 107, 171, 235, 299, 363, 427, 491, 555, 619, 683, 747, 811, 875, 939, 1003,
44, 108, 172, 236, 300, 364, 428, 492, 556, 620, 684, 748, 812, 876, 940, 1004,
45, 109, 173, 237, 301, 365, 429, 493, 557, 621, 685, 749, 813, 877, 941, 1005,
46, 110, 174, 238, 302, 366, 430, 494, 558, 622, 686, 750, 814, 878, 942, 1006,
47, 111, 175, 239, 303, 367, 431, 495, 559, 623, 687, 751, 815, 879, 943, 1007,
48, 112, 176, 240, 304, 368, 432, 496, 560, 624, 688, 752, 816, 880, 944, 1008,
49, 113, 177, 241, 305, 369, 433, 497, 561, 625, 689, 753, 817, 881, 945, 1009,
50, 114, 178, 242, 306, 370, 434, 498, 562, 626, 690, 754, 818, 882, 946, 1010,
51, 115, 179, 243, 307, 371, 435, 499, 563, 627, 691, 755, 819, 883, 947, 1011,
52, 116, 180, 244, 308, 372, 436, 500, 564, 628, 692, 756, 820, 884, 948, 1012,
53, 117, 181, 245, 309, 373, 437, 501, 565, 629, 693, 757, 821, 885, 949, 1013,
54, 118, 182, 246, 310, 374, 438, 502, 566, 630, 694, 758, 822, 886, 950, 1014,
55, 119, 183, 247, 311, 375, 439, 503, 567, 631, 695, 759, 823, 887, 951, 1015,
56, 120, 184, 248, 312, 376, 440, 504, 568, 632, 696, 760, 824, 888, 952, 1016,
57, 121, 185, 249, 313, 377, 441, 505, 569, 633, 697, 761, 825, 889, 953, 1017,
58, 122, 186, 250, 314, 378, 442, 506, 570, 634, 698, 762, 826, 890, 954, 1018,
59, 123, 187, 251, 315, 379, 443, 507, 571, 635, 699, 763, 827, 891, 955, 1019,
60, 124, 188, 252, 316, 380, 444, 508, 572, 636, 700, 764, 828, 892, 956, 1020,
61, 125, 189, 253, 317, 381, 445, 509, 573, 637, 701, 765, 829, 893, 957, 1021,
62, 126, 190, 254, 318, 382, 446, 510, 574, 638, 702, 766, 830, 894, 958, 1022,
63, 127, 191, 255, 319, 383, 447, 511, 575, 639, 703, 767, 831, 895, 959, 1023);
}]>;

OclBuiltinImpl __ocl_shuffle_transpose16x64_sse3 =OclBuiltinImpl<__ocl_shuffle_transpose_16x64, __ocl_shuffle_transpose_16x64.Types, 0, [{
return __builtin_shufflevector($Arg0VarName, $Arg0VarName,
0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304, 320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480, 496, 512, 528, 544, 560, 576, 592, 608, 624, 640, 656, 672, 688, 704, 720, 736, 752, 768, 784, 800, 816, 832, 848, 864, 880, 896, 912, 928, 944, 960, 976, 992, 1008,
1, 17, 33, 49, 65, 81, 97, 113, 129, 145, 161, 177, 193, 209, 225, 241, 257, 273, 289, 305, 321, 337, 353, 369, 385, 401, 417, 433, 449, 465, 481, 497, 513, 529, 545, 561, 577, 593, 609, 625, 641, 657, 673, 689, 705, 721, 737, 753, 769, 785, 801, 817, 833, 849, 865, 881, 897, 913, 929, 945, 961, 977, 993, 1009,
2, 18, 34, 50, 66, 82, 98, 114, 130, 146, 162, 178, 194, 210, 226, 242, 258, 274, 290, 306, 322, 338, 354, 370, 386, 402, 418, 434, 450, 466, 482, 498, 514, 530, 546, 562, 578, 594, 610, 626, 642, 658, 674, 690, 706, 722, 738, 754, 770, 786, 802, 818, 834, 850, 866, 882, 898, 914, 930, 946, 962, 978, 994, 1010,
3, 19, 35, 51, 67, 83, 99, 115, 131, 147, 163, 179, 195, 211, 227, 243, 259, 275, 291, 307, 323, 339, 355, 371, 387, 403, 419, 435, 451, 467, 483, 499, 515, 531, 547, 563, 579, 595, 611, 627, 643, 659, 675, 691, 707, 723, 739, 755, 771, 787, 803, 819, 835, 851, 867, 883, 899, 915, 931, 947, 963, 979, 995, 1011,
4, 20, 36, 52, 68, 84, 100, 116, 132, 148, 164, 180, 196, 212, 228, 244, 260, 276, 292, 308, 324, 340, 356, 372, 388, 404, 420, 436, 452, 468, 484, 500, 516, 532, 548, 564, 580, 596, 612, 628, 644, 660, 676, 692, 708, 724, 740, 756, 772, 788, 804, 820, 836, 852, 868, 884, 900, 916, 932, 948, 964, 980, 996, 1012,
5, 21, 37, 53, 69, 85, 101, 117, 133, 149, 165, 181, 197, 213, 229, 245, 261, 277, 293, 309, 325, 341, 357, 373, 389, 405, 421, 437, 453, 469, 485, 501, 517, 533, 549, 565, 581, 597, 613, 629, 645, 661, 677, 693, 709, 725, 741, 757, 773, 789, 805, 821, 837, 853, 869, 885, 901, 917, 933, 949, 965, 981, 997, 1013,
6, 22, 38, 54, 70, 86, 102, 118, 134, 150, 166, 182, 198, 214, 230, 246, 262, 278, 294, 310, 326, 342, 358, 374, 390, 406, 422, 438, 454, 470, 486, 502, 518, 534, 550, 566, 582, 598, 614, 630, 646, 662, 678, 694, 710, 726, 742, 758, 774, 790, 806, 822, 838, 854, 870, 886, 902, 918, 934, 950, 966, 982, 998, 1014,
7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215, 231, 247, 263, 279, 295, 311, 327, 343, 359, 375, 391, 407, 423, 439, 455, 471, 487, 503, 519, 535, 551, 567, 583, 599, 615, 631, 647, 663, 679, 695, 711, 727, 743, 759, 775, 791, 807, 823, 839, 855, 871, 887, 903, 919, 935, 951, 967, 983, 999, 1015,
8, 24, 40, 56, 72, 88, 104, 120, 136, 152, 168, 184, 200, 216, 232, 248, 264, 280, 296, 312, 328, 344, 360, 376, 392, 408, 424, 440, 456, 472, 488, 504, 520, 536, 552, 568, 584, 600, 616, 632, 648, 664, 680, 696, 712, 728, 744, 760, 776, 792, 808, 824, 840, 856, 872, 888, 904, 920, 936, 952, 968, 984, 1000, 1016,
9, 25, 41, 57, 73, 89, 105, 121, 137, 153, 169, 185, 201, 217, 233, 249, 265, 281, 297, 313, 329, 345, 361, 377, 393, 409, 425, 441, 457, 473, 489, 505, 521, 537, 553, 569, 585, 601, 617, 633, 649, 665, 681, 697, 713, 729, 745, 761, 777, 793, 809, 825, 841, 857, 873, 889, 905, 921, 937, 953, 969, 985, 1001, 1017,
10, 26, 42, 58, 74, 90, 106, 122, 138, 154, 170, 186, 202, 218, 234, 250, 266, 282, 298, 314, 330, 346, 362, 378, 394, 410, 426, 442, 458, 474, 490, 506, 522, 538, 554, 570, 586, 602, 618, 634, 650, 666, 682, 698, 714, 730, 746, 762, 778, 794, 810, 826, 842, 858, 874, 890, 906, 922, 938, 954, 970, 986, 1002, 1018,
11, 27, 43, 59, 75, 91, 107, 123, 139, 155, 171, 187, 203, 219, 235, 251, 267, 283, 299, 315, 331, 347, 363, 379, 395, 411, 427, 443, 459, 475, 491, 507, 523, 539, 555, 571, 587, 603, 619, 635, 651, 667, 683, 699, 715, 731, 747, 763, 779, 795, 811, 827, 843, 859, 875, 891, 907, 923, 939, 955, 971, 987, 1003, 1019,
12, 28, 44, 60, 76, 92, 108, 124, 140, 156, 172, 188, 204, 220, 236, 252, 268, 284, 300, 316, 332, 348, 364, 380, 396, 412, 428, 444, 460, 476, 492, 508, 524, 540, 556, 572, 588, 604, 620, 636, 652, 668, 684, 700, 716, 732, 748, 764, 780, 796, 812, 828, 844, 860, 876, 892, 908, 924, 940, 956, 972, 988, 1004, 1020,
13, 29, 45, 61, 77, 93, 109, 125, 141, 157, 173, 189, 205, 221, 237, 253, 269, 285, 301, 317, 333, 349, 365, 381, 397, 413, 429, 445, 461, 477, 493, 509, 525, 541, 557, 573, 589, 605, 621, 637, 653, 669, 685, 701, 717, 733, 749, 765, 781, 797, 813, 829, 845, 861, 877, 893, 909, 925, 941, 957, 973, 989, 1005, 1021,
14, 30, 46, 62, 78, 94, 110, 126, 142, 158, 174, 190, 206, 222, 238, 254, 270, 286, 302, 318, 334, 350, 366, 382, 398, 414, 430, 446, 462, 478, 494, 510, 526, 542, 558, 574, 590, 606, 622, 638, 654, 670, 686, 702, 718, 734, 750, 766, 782, 798, 814, 830, 846, 862, 878, 894, 910, 926, 942, 958, 974, 990, 1006, 1022,
15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255, 271, 287, 303, 319, 335, 351, 367, 383, 399, 415, 431, 447, 463, 479, 495, 511, 527, 543, 559, 575, 591, 607, 623, 639, 655, 671, 687, 703, 719, 735, 751, 767, 783, 799, 815, 831, 847, 863, 879, 895, 911, 927, 943, 959, 975, 991, 1007, 1023);
}]>;
