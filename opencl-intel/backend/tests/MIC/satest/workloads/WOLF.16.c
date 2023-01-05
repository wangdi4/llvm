// Copyright (c) 1997-2004 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
//  pi used by all evaluations (CPP and OCL)
#define BAS_PI 3.141592654f
#define INV_SQRT_2XPI 0.3989422804f
#define OPTIONS_PER_ITEM 128
/*******************************************************************************
 *  Scalar
 ******************************************************************************/
#if 1
#define CND(X, WP, WN)                                                         \
  {                                                                            \
    const float a1 = 0.31938153f, a2 = -0.356563782f, a3 = 1.781477937f,       \
                a4 = -1.821255978f, a5 = 1.330274429f;                         \
    float L = fabs(X);                                                         \
    float K = 1.0f / (1.0f + 0.2316419f * L);                                  \
    float w = INV_SQRT_2XPI * exp(-0.5f * L * L) * K *                         \
              (a1 + K * (a2 + K * (a3 + K * (a4 + a5 * K))));                  \
    if (X < 0.0f) {                                                            \
      WN = 1.0f - w;                                                           \
      WP = w;                                                                  \
    } else {                                                                   \
      WN = w;                                                                  \
      WP = 1.0f - w;                                                           \
    }                                                                          \
  }
#else
#define CND(X, WP, WN)                                                         \
  {                                                                            \
    const float a1 = 0.31938153f, a2 = -0.356563782f, a3 = 1.781477937f,       \
                a4 = -1.821255978f, a5 = 1.330274429f;                         \
    float L = fabs(X);                                                         \
    float K = 1.0f / (1.0f + 0.2316419f * L);                                  \
    float w = INV_SQRT_2XPI * exp(-0.5f * L * L) * K *                         \
              (a1 + K * (a2 + K * (a3 + K * (a4 + a5 * K))));                  \
    unsigned signX = as_uint(X) & 0x80000000;                                  \
    WN = as_float(signX ^ as_uint(w - 0.5f)) + 0.5f;                           \
    WP = as_float(signX ^ as_uint(0.5f - w)) + 0.5f;                           \
  }
#endif
#if 1 // the fastest variant
#define CND4(X, WP, WN)                                                        \
  {                                                                            \
    const float4 a1 = 0.31938153f, a2 = -0.356563782f, a3 = 1.781477937f,      \
                 a4 = -1.821255978f, a5 = 1.330274429f;                        \
    float4 L = fabs(X);                                                        \
    float4 K = 1.0f / (1.0f + 0.2316419f * L);                                 \
    float4 w = INV_SQRT_2XPI * exp(-0.5f * L * L) * K *                        \
               (a1 + K * (a2 + K * (a3 + K * (a4 + a5 * K))));                 \
    WN = select(w, 1.0f - w, X < 0.0f);                                        \
    WP = select(1.0f - w, w, X < 0.0f);                                        \
  }
#define CND8(X, WP, WN)                                                        \
  {                                                                            \
    const float8 a1 = 0.31938153f, a2 = -0.356563782f, a3 = 1.781477937f,      \
                 a4 = -1.821255978f, a5 = 1.330274429f;                        \
    float8 L = fabs(X);                                                        \
    float8 K = 1.0f / (1.0f + 0.2316419f * L);                                 \
    float8 w = INV_SQRT_2XPI * exp(-0.5f * L * L) * K *                        \
               (a1 + K * (a2 + K * (a3 + K * (a4 + a5 * K))));                 \
    WN = select(w, 1.0f - w, X < 0.0f);                                        \
    WP = select(1.0f - w, w, X < 0.0f);                                        \
  }
#else
#define CND4(X, WP, WN)                                                        \
  {                                                                            \
    const float4 a1 = 0.31938153f, a2 = -0.356563782f, a3 = 1.781477937f,      \
                 a4 = -1.821255978f, a5 = 1.330274429f;                        \
    float4 L = fabs(X);                                                        \
    float4 K = 1.0f / (1.0f + 0.2316419f * L);                                 \
    float4 w = INV_SQRT_2XPI * exp(-0.5f * L * L) * K *                        \
               (a1 + K * (a2 + K * (a3 + K * (a4 + a5 * K))));                 \
    uint4 signX = as_uint4(X) & 0x80000000;                                    \
    WN = as_float4(signX ^ as_uint4(w - 0.5f)) + 0.5f;                         \
    WP = as_float4(signX ^ as_uint4(0.5f - w)) + 0.5f;                         \
  }
#endif
//  basScalar - evaluate each option in scalar mode
__kernel void
basScalar(const __global float *restrict S, const __global float *restrict K,
          const __global float *restrict T, __global float *restrict callOutput,
          __global float *restrict putOutput, const float r, const float v,
          const uint optionsPerItem /* total num of options*/
) {
  size_t itemStartingIndex = get_global_id(0);
  size_t i = itemStartingIndex;
  float Si = S[i], Ki = K[i], Ti = T[i];
  float vsqrtT = v * sqrt(Ti);
  float d1 = (log(Si / Ki) + (r + v * v * 0.5f) * Ti) / vsqrtT;
  float d2 = d1 - vsqrtT;
  float cnd_d1p, cnd_d1n, cnd_d2p, cnd_d2n;
  CND(d1, cnd_d1p, cnd_d1n);
  CND(d2, cnd_d2p, cnd_d2n);
  float KexprT = Ki * exp(-r * Ti);
  float call =
      Si * cnd_d1p - KexprT * cnd_d2p; // TODO Try to store in local variable
                                       // first, and copy to dest in the end
  float put = KexprT * cnd_d2n - Si * cnd_d1n;
  callOutput[i] = call;
  putOutput[i] = put;
}
/*******************************************************************************
 *  Vectorized
 ******************************************************************************/
//  basVectorized - evaluate each option in vector mode
__kernel __attribute__((vec_type_hint(float4))) void basVectorized(
    const __global float4 *restrict S, const __global float4 *restrict K,
    const __global float4 *restrict T, __global float4 *restrict callOutput,
    __global float4 *restrict putOutput, const float r, const float v,
    const uint optionsPerItem) {
  size_t itemStartingIndex = optionsPerItem * get_global_id(0);
  size_t totalItemOptions = itemStartingIndex + optionsPerItem;
  for (size_t i = itemStartingIndex; i < totalItemOptions; ++i) {
    float4 Si = S[i], Ki = K[i], Ti = T[i];
    float4 vsqrtT = v * sqrt(Ti);
    float4 d1 = (log(Si / Ki) + (r + v * v * 0.5f) * Ti) / vsqrtT;
    float4 d2 = d1 - vsqrtT;
    float4 cnd_d1p, cnd_d1n, cnd_d2p, cnd_d2n;
    CND4(d1, cnd_d1p, cnd_d1n);
    CND4(d2, cnd_d2p, cnd_d2n);
    float4 KexprT = Ki * exp(-r * Ti);
    float4 call =
        Si * cnd_d1p - KexprT * cnd_d2p; // TODO Try to store in local variable
                                         // first, and copy to dest in the end
    float4 put = KexprT * cnd_d2n - Si * cnd_d1n;
    callOutput[i] = call;
    putOutput[i] = put;
  }
}
__kernel __attribute__((vec_type_hint(float8))) void basVectorized8(
    const __global float8 *restrict S, const __global float8 *restrict K,
    const __global float8 *restrict T, __global float8 *restrict callOutput,
    __global float8 *restrict putOutput, const float r, const float v,
    const uint optionsPerItem) {
  size_t itemStartingIndex = optionsPerItem * get_global_id(0);
  size_t totalItemOptions = itemStartingIndex + optionsPerItem;
  for (size_t i = itemStartingIndex; i < totalItemOptions; ++i) {
    float8 Si = S[i], Ki = K[i], Ti = T[i];
    float8 vsqrtT = v * sqrt(Ti);
    float8 d1 = (log(Si / Ki) + (r + v * v * 0.5f) * Ti) / vsqrtT;
    float8 d2 = d1 - vsqrtT;
    float8 cnd_d1p, cnd_d1n, cnd_d2p, cnd_d2n;
    CND8(d1, cnd_d1p, cnd_d1n);
    CND8(d2, cnd_d2p, cnd_d2n);
    float8 KexprT = Ki * exp(-r * Ti);
    float8 call =
        Si * cnd_d1p - KexprT * cnd_d2p; // TODO Try to store in local variable
                                         // first, and copy to dest in the end
    float8 put = KexprT * cnd_d2n - Si * cnd_d1n;
    callOutput[i] = call;
    putOutput[i] = put;
  }
}
