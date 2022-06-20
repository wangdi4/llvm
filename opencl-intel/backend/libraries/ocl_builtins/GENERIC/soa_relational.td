//
//Scalar relational
//
defvar v1_relational = [{
    $ReturnType ret = $RemovePrefixFunc($Arg0VarName, $Arg1VarName);
    return ret * ($ReturnType)-1;
  }];

defvar v_relational_d = [{
    $ReturnType ret = downConvert($RemovePrefixFunc($Arg0VarName, $Arg1VarName));
    return ret & ($ReturnType)1;
  }];

defvar v_relational_f = [{
    $ReturnType ret = $RemovePrefixFunc($Arg0VarName, $Arg1VarName);
    return ret & ($ReturnType)1;
  }];

defvar downCvt4 = [{
    return convert_int4($Arg0VarName);
  }];

defvar downCvt8 = [{
    return convert_int8($Arg0VarName);
  }];

defvar downCvt16 = [{
    return convert_int16($Arg0VarName);
  }];
//
//downConvert utilty
//
OclBuiltinImpl downConvertImpl4 = OclBuiltinImpl<downConvert, [v4i64], 0, downCvt4>;
OclBuiltinImpl downConvertImpl8 = OclBuiltinImpl<downConvert, [v8i64], 0, downCvt8>;
OclBuiltinImpl downConvertImpl16= OclBuiltinImpl<downConvert, [v16i64], 0, downCvt16>;
