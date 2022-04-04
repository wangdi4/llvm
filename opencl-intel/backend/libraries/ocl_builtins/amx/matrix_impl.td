def int8_matrix_layout_transform_rowmajor_to_vnni_impl : OclBuiltinImpl<matrix_layout_transform_rowmajor_to_vnni, [v1i8], 0, [{
  int k_full_quads = $Arg2VarName / 4, k_quad, n;
  signed char *in = $Arg0VarName, *out = $Arg1VarName;
  for (k_quad = 0; k_quad < k_full_quads; k_quad++) {
    for (n = 0; n < $Arg3VarName; n += 16) {
      size_t remaining_n = ($Arg3VarName-n) >= 16? 16:($Arg3VarName-n);
      __mmask64 m1 = (1UL<<remaining_n) -1;
      __m512i k0 = _mm512_maskz_loadu_epi8(m1, in+n+$Arg4VarName*0);
      __m512i k1 = _mm512_maskz_loadu_epi8(m1, in+n+$Arg4VarName*1);
      __m512i k2 = _mm512_maskz_loadu_epi8(m1, in+n+$Arg4VarName*2);
      __m512i k3 = _mm512_maskz_loadu_epi8(m1, in+n+$Arg4VarName*3);
      const __m512i selectorb = (__m512i)(__v64qi)
                                {0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0,
                                 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0,
                                 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0,
                                 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64, 0, 64};
      const __m512i offsetsb = (__m512i)(__v64qi)
                               {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7,
                                8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
                                16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 
                                24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31}; 
      const __m512i idxb = _mm512_or_epi32(selectorb, offsetsb);
      __m512i tmp0 = _mm512_permutex2var_epi8(k0, idxb, k1);
      
      __m512i tmp1 = _mm512_permutex2var_epi8(k2, idxb, k3);
      const __m512i selector = (__m512i)(__v32hi){0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32, 0, 32};
      
      const __m512i offsets = (__m512i)(__v32hi){0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7,
                                                 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15};
      const __m512i idx =  _mm512_or_epi32(selector, offsets);
      
      __m512i dst = _mm512_permutex2var_epi16(tmp0, idx, tmp1);
      __mmask64 m2 = remaining_n*4==64?(-1):((1UL<<(remaining_n*4)) -1);
      _mm512_mask_storeu_epi8(out+n*4, m2, dst);
      
    }
    in += 4 * $Arg4VarName;
    out += 4 * $Arg3VarName;
  }
  }]>;

def bf16_matrix_layout_transform_rowmajor_to_vnni_impl : OclBuiltinImpl<matrix_layout_transform_rowmajor_to_vnni, [v1i16], 0, [{
  int k_full_quads = $Arg2VarName / 2, k_quad, n;
  signed char *in = $Arg0VarName, *out = $Arg1VarName;

  for (k_quad = 0; k_quad < k_full_quads; k_quad++) {
    for (n = 0; n < $Arg3VarName; n += 16) {
      size_t remaining_n = ($Arg3VarName-n) >= 16? 16:($Arg3VarName-n);//n might not be divided by 16.
      __mmask32 m1 = (1UL<<remaining_n) -1;
      __m512i k0 = _mm512_maskz_loadu_epi16(m1, in+n*2+$Arg4VarName*0);
      __m512i k1 = _mm512_maskz_loadu_epi16(m1, in+n*2+$Arg4VarName*1);

      const __m512i idxb = (__m512i)(__v32hi)
                               {0, 32, 1, 33, 2, 34, 3, 35, 4, 36, 5, 37, 6, 38, 7, 39,
                                8, 40, 9, 41, 10, 42, 11, 43, 12, 44, 13, 45, 14, 46, 15, 47};

      __m512i dst = _mm512_permutex2var_epi16(k0, idxb, k1);

      __mmask32 m2 = remaining_n*2==32?(-1):((1UL<<(remaining_n*2)) -1);

      _mm512_mask_storeu_epi16(out+n*4, m2, dst);
    }

    in += 2 * $Arg4VarName;
    out += 4 * $Arg3VarName;
  }
  }]>;

