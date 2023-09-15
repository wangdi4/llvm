// Copyright (c) 2006 Intel Corporation
// Geometric Built-In Functions
// Generic

/*****************************************************************\
 * length:                                                       *
 *   float length (floatn p)                                     *
 *   double length (doublen p)                                   *
 *   Return the length of vector p.                              *
\*****************************************************************/

OclBuiltinImpl length_scalar = OclBuiltinImpl<length, [v1f32, v1f64], 0, [{
    return fabs($Arg0VarName);
  }]>;

OclBuiltinImpl length_v2v3v4f32 = OclBuiltinImpl<length, [v2f32, v3f32, v4f32], 0, [{
    double$VecLength double$Arg0VarName = convert_double$VecLength($Arg0VarName);
    double temp = dot(double$Arg0VarName, double$Arg0VarName);
    temp = sqrt(temp);
    float res = convert_float(temp);
    return res;
  }]>;

OclBuiltinImpl length_v2v3v4f64 = OclBuiltinImpl<length, [v2f64, v3f64, v4f64], 0, [{
    double res = dot($Arg0VarName, $Arg0VarName);

    if( res == INFINITY )
    {
      $Arg0VarName = $Arg0VarName * expMinus600;
      res = dot($Arg0VarName, $Arg0VarName);
      res = sqrt(res);
      res = res * exp600;
    }
    else if( res < $VecLength * DBL_MIN / DBL_EPSILON )
    {
      $Arg0VarName = $Arg0VarName * exp700;
      res = dot($Arg0VarName, $Arg0VarName);
      res = sqrt(res);
      res = res * expMinus700;
    }
    else
    {
      res = sqrt(res);
    }

    return res;
  }]>;


/*****************************************************************\
 * normalize:                                                    *
 *   floatn normalize (floatn p)                                 *
 *   doublen normalize (doublen p)                               *
 *   Returns a vector in the same direction as p but with a      *
 *   length of 1.                                                *
\*****************************************************************/

OclBuiltinImpl normalize_f32f64_scalar = OclBuiltinImpl<normalize, [v1f32, v1f64], 0, [{
    if( 0 == $Arg0VarName)
    {
      // Special edge case: copy vector over without change
      return $Arg0VarName;
    }
    if( $Arg0VarName != $Arg0VarName)
    {
      //$Arg0VarName is Nan: return $Arg0VarName
      return $Arg0VarName;
    }
    return ($Arg0VarName > 0.f) ? 1.f : -1.f;
  }]>;

  OclBuiltinImpl normalize_v2v3v4f32 = OclBuiltinImpl<normalize, [v2f32, v3f32, v4f32], 0, [{
    double$VecLength double$Arg0VarName = convert_double$VecLength($Arg0VarName);
    double len = length(double$Arg0VarName);
    double$Arg0VarName = double$Arg0VarName / len;
    return convert_float$VecLength(double$Arg0VarName);
  }]>;

OclBuiltinImpl normalize_v2v3v4f64 = OclBuiltinImpl<normalize, [v2f64, v3f64, v4f64], 0, [{
    double res = dot($Arg0VarName, $Arg0VarName);
    if( res == INFINITY )
    {
      $Arg0VarName = $Arg0VarName * expMinus512_2;
      res = dot($Arg0VarName, $Arg0VarName);
    }
    else if( res < $VecLength * DBL_MIN / DBL_EPSILON )
    {
      $Arg0VarName = $Arg0VarName * exp700;
      res = dot($Arg0VarName, $Arg0VarName);
      if( res == 0 )
      {
        return $Arg0VarName;
      }
    }
    res = sqrt(res);
    return $Arg0VarName / res;
  }]>;


/*****************************************************************\
 * distance:                                                     *
 *   float distance (floatn p0, floatn p1)                       *
 *   double distance (doublen p0, doublen p1)                    *
 *   Returns the distance between p0 and p1. This is             *
 *   calculated as length(p0 – p1).                              *
\*****************************************************************/

OclBuiltinImpl distance_vector = OclBuiltinImpl<distance, distance.Types, 0, [{
    return length($Arg0VarName - $Arg1VarName);
  }]>;


/*****************************************************************\
 * fast_length:                                                  *
 *   float fast_length (floatn p)                                *
 *   Returns the length of vector p computed as:                 *
 *   half_sqrt(p.x2 + p.y2 + ….)                                 *
\*****************************************************************/

OclBuiltinImpl fast_length_scalar = OclBuiltinImpl<fast_length, [v1f32], 0, [{
    return fabs($Arg0VarName);
  }]>;

OclBuiltinImpl fast_length_vector = OclBuiltinImpl<fast_length, [v2f32, v3f32, v4f32], 0, [{
    return half_sqrt(dot($Arg0VarName,$Arg0VarName));
  }]>;


/*****************************************************************\
 * fast_distance:                                                *
 *   float fast_distance (floatn p0, floatn p1)                  *
 *   Returns fast_length(p0 – p1).                               *
\*****************************************************************/

OclBuiltinImpl fast_distance_all = OclBuiltinImpl<fast_distance, fast_distance.Types, 0, [{
    return fast_length($Arg0VarName - $Arg1VarName);
  }]>;


/*****************************************************************\
 * Extend SOA Geometry built-in functions                        *
\*****************************************************************/
list<OclType> fp_soa_types    = [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64];
list<OclType> sp_fp_soa_types = [v4f32, v8f32, v16f32];
list<OclType> dp_fp_soa_types = [v4f64, v8f64, v16f64];

/*****************************************************************\
 * soa_distance:                                                 *
\*****************************************************************/
// soa_distance1
OclBuiltinImpl soa_distance1_all = OclBuiltinImpl<soa_distance1 , fp_soa_types, 0, [{
    return soa_length1($Arg0VarName - $Arg1VarName);
  }]>;
// soa_distance2
OclBuiltinImpl soa_distance2_all = OclBuiltinImpl<soa_distance2 , fp_soa_types, 0, [{
    return soa_length2(($Arg0VarName - $Arg2VarName), ($Arg1VarName - $Arg3VarName));
  }]>;
// soa_distance3
OclBuiltinImpl soa_distance3_all = OclBuiltinImpl<soa_distance3 , fp_soa_types, 0, [{
    return soa_length3(($Arg0VarName - $Arg3VarName) ,($Arg1VarName - $Arg4VarName), ($Arg2VarName - $Arg5VarName));
  }]>;
// soa_distance4
OclBuiltinImpl soa_distance4_all = OclBuiltinImpl<soa_distance4 , fp_soa_types, 0, [{
    return soa_length4(($Arg0VarName - $Arg4VarName) ,($Arg1VarName - $Arg5VarName), ($Arg2VarName - $Arg6VarName), ($Arg3VarName - $Arg7VarName));
  }]>;


/*****************************************************************\
 * soa_fast_distance:                                            *
\*****************************************************************/
// soa_fast_distance1
OclBuiltinImpl soa_fast_distance1_all = OclBuiltinImpl<soa_fast_distance1 , sp_fp_soa_types, 0, [{
    return soa_fast_length1($Arg0VarName - $Arg1VarName);
  }]>;
// soa_fast_distance2
OclBuiltinImpl soa_fast_distance2_all = OclBuiltinImpl<soa_fast_distance2 , sp_fp_soa_types, 0, [{
    return soa_fast_length2(($Arg0VarName - $Arg2VarName), ($Arg1VarName - $Arg3VarName));
  }]>;
// soa_fast_distance3
OclBuiltinImpl soa_fast_distance3_all = OclBuiltinImpl<soa_fast_distance3 , sp_fp_soa_types, 0, [{
    return soa_fast_length3(($Arg0VarName - $Arg3VarName) ,($Arg1VarName - $Arg4VarName), ($Arg2VarName - $Arg5VarName));
  }]>;
// soa_fast_distance4
OclBuiltinImpl soa_fast_distance4_all = OclBuiltinImpl<soa_fast_distance4 , sp_fp_soa_types, 0, [{
    return soa_fast_length4(($Arg0VarName - $Arg4VarName) ,($Arg1VarName - $Arg5VarName), ($Arg2VarName - $Arg6VarName), ($Arg3VarName - $Arg7VarName));
  }]>;
