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

     __ocl_transpose4x4($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
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



