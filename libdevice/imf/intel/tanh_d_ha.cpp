/*******************************************************************************
* INTEL CONFIDENTIAL
* Copyright 1996-2022 Intel Corporation.
*
* This software and the related documents are Intel copyrighted  materials,  and
* your use of  them is  governed by the  express license  under which  they were
* provided to you (License).  Unless the License provides otherwise, you may not
* use, modify, copy, publish, distribute,  disclose or transmit this software or
* the related documents without Intel's prior written permission.
*
* This software and the related documents  are provided as  is,  with no express
* or implied  warranties,  other  than those  that are  expressly stated  in the
* License.
*******************************************************************************/
/*
// ALGORITHM DESCRIPTION:
//
//   NOTE: Since the hyperbolic tangent function is odd
//         (tanh(x) = -tanh(-x)), below algorithm deals with the absolute
//         value of the argument |x|: tanh(x) = sign(x) * tanh(|x|)
//
//   We use a table lookup method to compute tanh(|x|).
//   The basic idea is to split the input range into a number of subintervals
//   and to approximate tanh(.) with a polynomial on each of them.
//
//   IEEE SPECIAL CONDITIONS:
//   x = [+,-]0, r = [+,-]0
//   x = +Inf,   r = +1
//   x = -Inf,   r = -1
//   x = QNaN,   r = QNaN
//   x = SNaN,   r = QNaN
//
//
//   ALGORITHM DETAILS
//   We handle special values in a callout function, aside from main path
//   computations. "Special" for this algorithm are:
//   INF, NAN, |x| > HUGE_THRESHOLD
//
//
//   Main path computations are organized as follows:
//   Actually we split the interval [0, SATURATION_THRESHOLD)
//   into a number of subintervals.  On each subinterval we approximate tanh(.)
//   with a minimax polynomial of pre-defined degree. Polynomial coefficients
//   are computed beforehand and stored in table. We also use
//
//       y := |x| + B,
//
//   here B depends on subinterval and is used to make argument
//   closer to zero.
//   We also add large fake interval [SATURATION_THRESHOLD, HUGE_THRESHOLD],
//   where 1.0 + 0.0*y + 0.0*y^2 ... coefficients are stored - just to
//   preserve main path computation logic but return 1.0 for all arguments.
//
//   Hence reconstruction looks as follows:
//   we extract proper polynomial and range reduction coefficients
//        (Pj and B), corresponding to subinterval, to which |x| belongs,
//        and return
//
//       r := sign(x) * (P0 + P1 * y + ... + Pn * y^n)
//
//   NOTE: we use multiprecision technique to multiply and sum the first
//         K terms of the polynomial. So Pj, j = 0..K are stored in
//         table each as a pair of target precision numbers (Pj and PLj) to
//         achieve wider than target precision.
//
// --
//
*/
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_tanh_d_ha {
namespace {
typedef struct {
  // Use small table
  VUINT64 _dC[16];
  VUINT64 _dP0[16];
  VUINT64 _dP1[16];
  VUINT64 _dP2[16];
  VUINT64 _dP3[16];
  VUINT64 _dP4[16];
  VUINT64 _dP5[16];
  VUINT64 _dP6[16];
  VUINT64 _dP7[16];
  VUINT64 _dP8[16];
  VUINT64 _dP9[16];
  VUINT64 _dP10[16];
  VUINT64 _dP11[16];
  VUINT64 _dP12[16];
  VUINT64 _dP13[16];
  VUINT64 _dP14[16];
  VUINT64 _dP15[16];
  VUINT64 _dP16[16];
  VUINT64 _dP17[16];
  VUINT32 _iExpMantMask_UISA;
  VUINT32 _iMinIdxOfsMask_UISA;
  VUINT32 _iMaxIdxMask_UISA;
  // Use large table
  VUINT64 _dbP[60 * 16];
  VUINT64 _dbHighMask;
  VUINT64 _dbSignMask;
  VUINT64 _dbAbsMask;
  VUINT32 _iExpMantMask;
  VUINT32 _iExpMask;
  VUINT32 _iMinIdxOfsMask;
  VUINT32 _iMaxIdxMask;
  VUINT32 _iMaxTinyArg;
} __devicelib_imf_internal_dtanh_data_t;
static const __devicelib_imf_internal_dtanh_data_t
    __devicelib_imf_internal_dtanh_data = {
        {
            /*== _dC ==*/
            0x0000000000000000uL,
            0x3fcc000000000000uL,
            0x3fd4000000000000uL,
            0x3fdc000000000000uL,
            0x3fe4000000000000uL,
            0x3fec000000000000uL,
            0x3ff4000000000000uL,
            0x3ffc000000000000uL,
            0x4004000000000000uL,
            0x400c000000000000uL,
            0x4014000000000000uL,
            0x401c000000000000uL,
            0x4024000000000000uL,
            0x402c000000000000uL,
            0x4034000000000000uL,
            0x0000000000000000uL,
        }, /* c */
        {
            /*== p0 ==*/
            0x0000000000000000uL,
            0x3fcb8fd0416a7c92uL,
            0x3fd35f98a0ea650euL,
            0x3fda5729ee488037uL,
            0x3fe1bf47eabb8f95uL,
            0x3fe686650b8c2015uL,
            0x3feb2523bb6b2deeuL,
            0x3fee1fbf97e33527uL,
            0x3fef9258260a71c2uL,
            0x3feff112c63a9077uL,
            0x3fefff419668df11uL,
            0x3feffffc832750f2uL,
            0x3feffffffdc96f35uL,
            0x3fefffffffffcf58uL,
            0x3ff0000000000000uL,
            0x3ff0000000000000uL,
        }, /* p0 */
        {
            /*== p1 ==*/
            0x0000000000000000uL,
            0x3c65e23ebcd3bcbeuL,
            0xbc4c600bac3adf00uL,
            0x3c6c44091785d040uL,
            0x3c8221d7a6e3674buL,
            0x3c69f89d2cf6b85cuL,
            0x3c73b3e9ec0b8f1cuL,
            0xbc7f8d4b0428aadauL,
            0xbc7c52d880cf43c0uL,
            0x3c7dd36e37096480uL,
            0x3c7b4f6380c442cauL,
            0xbc729755de470096uL,
            0x3c84cf852845efbduL,
            0x3c6fc4fb440a5378uL,
            0xbc63981083b55870uL,
            0x0000000000000000uL,
        },
        /* p1 */
        {
            /*== p2 ==*/
            0x3ff0000000000000uL,
            0x3fee842ca3f08532uL,
            0x3fed11574af58f1buL,
            0x3fea945b9c24e4f9uL,
            0x3fe6284c3374f815uL,
            0x3fe02500a09f8d6euL,
            0x3fd1f25131e3a8c0uL,
            0x3fbd22ca1c24a139uL,
            0x3f9b3afe1fba5c76uL,
            0x3f6dd37d19b22b21uL,
            0x3f27ccec13a9ef96uL,
            0x3ecbe6c3f33250aeuL,
            0x3e41b4865394f75fuL,
            0x3d8853f01bda5f28uL,
            0x3c73953c0197ef58uL,
            0x0000000000000000uL,
        }, /* p2 */
        {
            /*== p3 ==*/
            0xbbf0b3ea3fdfaa19uL,
            0xbfca48aaeb53bc21uL,
            0xbfd19921f4329916uL,
            0xbfd5e0f09bef8011uL,
            0xbfd893b59c35c882uL,
            0xbfd6ba7cb7576538uL,
            0xbfce7291743d7555uL,
            0xbfbb6d85a01efb80uL,
            0xbf9addae58c7141auL,
            0xbf6dc59376c7aa19uL,
            0xbf27cc5e74677410uL,
            0xbecbe6c0e8b4cc87uL,
            0xbe41b486526b0565uL,
            0xbd8853f01bef63a4uL,
            0xbc73955be519be31uL,
            0x0000000000000000uL,
        }, /* p3 */
        {
            /*== p4 ==*/
            0xbfd5555555555555uL,
            0xbfd183afc292ba11uL,
            0xbfcc1a4b039c9bfauL,
            0xbfc16e1e6d8d0be6uL,
            0xbf92426c751e48a2uL,
            0x3fb4f152b2bad124uL,
            0x3fbbba40cbef72beuL,
            0x3fb01ba038be6a3duL,
            0x3f916df44871efc8uL,
            0x3f63c6869dfc8870uL,
            0x3f1fb9aef915d828uL,
            0x3ec299d1e27c6e11uL,
            0x3e379b5ddcca334cuL,
            0x3d8037f57bc62c9auL,
            0x3c6a2d4b50a2cff7uL,
            0x0000000000000000uL,
        }, /* p4 */
        {
            /*== p5 ==*/
            0xbce6863ee44ed636uL,
            0x3fc04dcd0476c75euL,
            0x3fc43d3449a80f08uL,
            0x3fc5c26f3699b7e7uL,
            0x3fc1a686f6ab2533uL,
            0x3faf203c316ce730uL,
            0xbf89c7a02788557cuL,
            0xbf98157e26e0d541uL,
            0xbf807b55c1c7d278uL,
            0xbf53a18d5843190fuL,
            0xbf0fb6bbc89b1a5buL,
            0xbeb299c9c684a963uL,
            0xbe279b5dd4fb3d01uL,
            0xbd7037f57ae72aa6uL,
            0xbc5a2ca2bba78e86uL,
            0x0000000000000000uL,
        }, /* p5 */
        {
            /*== p6 ==*/
            0x3fc1111111112ab5uL,
            0x3fb5c19efdfc08aduL,
            0x3fa74c98dc34fbacuL,
            0xbf790d6a8eff0a77uL,
            0xbfac3c021789a786uL,
            0xbfae2196b7326859uL,
            0xbf93a7a011ff8c2auL,
            0x3f6e4709c7e8430euL,
            0x3f67682afa611151uL,
            0x3f3ef2ee77717cbfuL,
            0x3ef95a4482f180b7uL,
            0x3e9dc2c27da3b603uL,
            0x3e12e2afd9f7433euL,
            0x3d59f320348679bauL,
            0x3c44b61d9bbcc940uL,
            0x0000000000000000uL,
        }, /* p6 */
        {
            /*== p7 ==*/
            0xbda1ea19ddddb3b4uL,
            0xbfb0b8df995ce4dfuL,
            0xbfb2955cf41e8164uL,
            0xbfaf9d05c309f7c6uL,
            0xbf987d27ccff4291uL,
            0x3f8b2ca62572b098uL,
            0x3f8f1cf6c7f5b00auL,
            0x3f60379811e43dd5uL,
            0xbf4793826f78537euL,
            0xbf2405695e36240fuL,
            0xbee0e08de39ce756uL,
            0xbe83d709ba5f714euL,
            0xbdf92e3fc5ee63e0uL,
            0xbd414cc030f2110euL,
            0xbc2ba022e8d82a87uL,
            0x0000000000000000uL,
        }, /* p7 */
        {
            /*== p8 ==*/
            0xbfaba1ba1990520buL,
            0xbf96e37bba52f6fcuL,
            0x3ecff7df18455399uL,
            0x3f97362834d33a4euL,
            0x3f9e7f8380184b45uL,
            0x3f869543e7c420d4uL,
            0xbf7326bd4914222auL,
            0xbf5fc15b0a9d98fauL,
            0x3f14cffcfa69fbb6uL,
            0x3f057e48e5b79d10uL,
            0x3ec33b66d7d77264uL,
            0x3e66ac4e578b9b10uL,
            0x3ddcc74b8d3d5c42uL,
            0x3d23c589137f92b4uL,
            0x3c107f8e2c8707a1uL,
            0x0000000000000000uL,
        }, /* p8 */
        {
            /*== p9 ==*/
            0xbe351ca7f096011fuL,
            0x3f9eaaf3320c3851uL,
            0x3f9cf823fe761fc1uL,
            0x3f9022271754ff1fuL,
            0xbf731fe77c9c60afuL,
            0xbf84a6046865ec7duL,
            0xbf4ca3f1f2b9192buL,
            0x3f4c77dee0afd227uL,
            0x3f04055bce68597auL,
            0xbee2bf0cb4a71647uL,
            0xbea31eaafe73efd5uL,
            0xbe46abb02c4368eduL,
            0xbdbcc749ca8079dduL,
            0xbd03c5883836b9d2uL,
            0xbbf07a5416264aecuL,
            0x0000000000000000uL,
        }, /* p9 */
        {
            /*== p10 ==*/
            0x3f9664f94e6ac14euL,
            0xbf94d3343bae39dduL,
            0xbf7bc748e60df843uL,
            0xbf8c89372b43ba85uL,
            0xbf8129a092de747auL,
            0x3f60c85b4d538746uL,
            0x3f5be9392199ec18uL,
            0xbf2a0c68a4489f10uL,
            0xbf00462601dc2faauL,
            0x3eb7b6a219dea9f4uL,
            0x3e80cbcc8d4c5c8auL,
            0x3e2425bb231a5e29uL,
            0x3d9992a4beac8662uL,
            0x3ce191ba5ed3fb67uL,
            0x3bc892450bad44c4uL,
            0x0000000000000000uL,
        }, /* p10 */
        {
            /*== p11 ==*/
            0xbea8c4c1fd7852feuL,
            0xbfccce16b1046f13uL,
            0xbf81a16f224bb7b6uL,
            0xbf62cbf00406bc09uL,
            0x3f75b29bb02cf69buL,
            0x3f607df0f9f90c17uL,
            0xbf4b852a6e0758d5uL,
            0xbf0078c63d1b8445uL,
            0x3eec12eadd55be7auL,
            0xbe6fa600f593181buL,
            0xbe5a3c935dce3f7duL,
            0xbe001c6d95e3ae96uL,
            0xbd74755a00ea1fd3uL,
            0xbcbc1c6c063bb7acuL,
            0xbba3be9a4460fe00uL,
            0x0000000000000000uL,
        }, /* p11 */
        {
            /*== p12 ==*/
            0xbf822404577aa9dduL,
            0x403d8b07f7a82aa3uL,
            0xbf9f44ab92fbab0auL,
            0x3fb2eac604473d6auL,
            0x3f45f87d903aaac8uL,
            0xbf5e104671036300uL,
            0x3f19bc98ddf0f340uL,
            0x3f0d4304bc9246e8uL,
            0xbed13c415f7b9d41uL,
            0xbe722b8d9720cdb0uL,
            0x3e322666d739bec0uL,
            0x3dd76a553d7e7918uL,
            0x3d4de0fa59416a39uL,
            0x3c948716cf3681b4uL,
            0x3b873f9f2d2fda99uL,
            0x0000000000000000uL,
        }, /* p12 */
        {
            /*== p13 ==*/
            0xbefdd99a221ed573uL,
            0x4070593a3735bab4uL,
            0xbfccab654e44835euL,
            0x3fd13ed80037dbacuL,
            0xbf6045b9076cc487uL,
            0x3f2085ee7e8ac170uL,
            0x3f23524622610430uL,
            0xbeff12a6626911b4uL,
            0x3eab9008bca408afuL,
            0x3e634df71865f620uL,
            0xbe05bb1bcf83ca73uL,
            0xbdaf2ac143fb6762uL,
            0xbd23eae52a3dbf57uL,
            0xbc6b5e3e9ca0955euL,
            0xbb5eca68e2c1ba2euL,
            0x0000000000000000uL,
        }, /* p13 */
        {
            /*== p14 ==*/
            0x3f6e3be689423841uL,
            0xc0d263511f5baac1uL,
            0x40169f73b15ebe5cuL,
            0xc025c1dd41cd6cb5uL,
            0xbf58fd89fe05e0d1uL,
            0x3f73f7af01d5af7auL,
            0xbf1e40bdead17e6buL,
            0x3ee224cd6c4513e5uL,
            0xbe24b645e68eeaa3uL,
            0xbe4abfebfb72bc83uL,
            0x3dd51c38f8695ed3uL,
            0x3d8313ac38c6832buL,
            0x3cf7787935626685uL,
            0x3c401ffc49c6bc29uL,
            0xbabf0b21acfa52abuL,
            0x0000000000000000uL,
        }, /* p14 */
        {
            /*== p15 ==*/
            0xbf2a1306713a4f3auL,
            0xc1045e509116b066uL,
            0x4041fab9250984ceuL,
            0xc0458d090ec3de95uL,
            0xbf74949d60113d63uL,
            0x3f7c9fd6200d0adeuL,
            0x3f02cd40e0ad0a9fuL,
            0xbe858ab8e019f311uL,
            0xbe792fa6323b7cf8uL,
            0x3e2df04d67876402uL,
            0xbd95c72be95e4d2cuL,
            0xbd55a89c30203106uL,
            0xbccad6b3bb9eff65uL,
            0xbc12705ccd3dd884uL,
            0xba8e0a4c47ae75f5uL,
            0x0000000000000000uL,
        }, /* p15 */
        {
            /*== p16 ==*/
            0xbf55d7e76dc56871uL,
            0x41528c38809c90c7uL,
            0xc076d57fb5190b02uL,
            0x4085f09f888f8adauL,
            0x3fa246332a2fcba5uL,
            0xbfb29d851a896fcduL,
            0x3ed9065ae369b212uL,
            0xbeb8e1ba4c98a030uL,
            0x3e6ffd0766ad4016uL,
            0xbe0c63c29f505f5buL,
            0xbd7fab216b9e0e49uL,
            0x3d2826b62056aa27uL,
            0x3ca313e31762f523uL,
            0x3bea37aa21895319uL,
            0x3ae5c7f1fd871496uL,
            0x0000000000000000uL,
        }, /* p16 */
        {
            /*== p17 ==*/
            0x3f35e67ab76a26e7uL,
            0x41848ee0627d8206uL,
            0xc0a216d618b489ecuL,
            0x40a5b89107c8af4fuL,
            0x3fb69d8374520edauL,
            0xbfbded519f981716uL,
            0xbef02d288b5b3371uL,
            0x3eb290981209c1a6uL,
            0xbe567e924bf5ff6euL,
            0x3de3f7f7de6b0eb6uL,
            0x3d69ed18bae3ebbcuL,
            0xbcf7534c4f3dfa71uL,
            0xbc730b73f1eaff20uL,
            0xbbba2cff8135d462uL,
            0xbab5a71b5f7d9035uL,
            0x0000000000000000uL,
        },           /* p17 */
        0x7ff80000u, /* _iExpMantMask_UISA     */
        0x3fc00000u, /* _iMinIdxOfsMask_UISA   */
        0x00780000u, /* _iMaxIdxMask_UISA      */
        {
            /* index = 0 , 0.00000 <= |a[i]| < 0.12500 */
            /* Polynomial coefficients */
            0x0000000000000000uL, /* PL0 = +0.000000000000000000000e-01 */
            0x0000000000000000uL, /* PH0 = +0.000000000000000000000e-01 */
            0x3FF0000000000000uL, /* P1  = +1.000000000000000014103e+00 */
            0xBD197DEAD79668D3uL, /* P2  = -2.264132406596103056796e-14 */
            0xBFD555555553AF3CuL, /* P3  = -3.333333333273349741024e-01 */
            0xBE052F7CCA134846uL, /* P4  = -6.165791385711493738399e-10 */
            0x3FC11111563849D6uL, /* P5  = +1.333333655353061107201e-01 */
            0xBEB038623673FFB2uL, /* P6  = -9.668021563879858950855e-07 */
            0xBFAB9F685E64022EuL, /* P7  = -5.395055916051593179252e-02 */
            0xBF2A54E2B28F2207uL, /* P8  = -2.008940439550829012647e-04 */
            0x3F97CFB9328A230EuL, /* P9  = +2.325333949059698582189e-02 */
            0xBF75CA6D61723E02uL, /* P10 = -5.320002811586290441790e-03 */
            /* Range reduction coefficients */
            0x0000000000000000uL, /* B = +0        */
            0x3FF0000000000000uL, /* A = +1.0      */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 1 , 0.12500]| < 0.14063 */
            /* Polynomial coefficients */
            0x3C3708A564FAD29AuL, /* PL0 = +1.248663375337163807466e-18 */
            0x3FC0E6973998DA48uL, /* PH0 = +1.320370703922029154143e-01 */
            0x3FEF712EB25C0888uL, /* P1  = +9.825662120422444519229e-01 */
            0xBFC09B296F7C1EA9uL, /* P2  = -1.297351641044220078331e-01 */
            0xBFD3DD77541EDDA7uL, /* P3  = -3.103922196855485849143e-01 */
            0x3FB58FFCF4309615uL, /* P4  = +8.422833406128689275566e-02 */
            0x3FBD3ABE845DCF49uL, /* P5  = +1.141776154670967208833e-01 */
            0xBFA791DF538C37FAuL, /* P6  = -4.603479285115947936529e-02 */
            0xBFA4F872F69CD6E8uL, /* P7  = -4.095801601799370195284e-02 */
            0x3F9772E49EF6412BuL, /* P8  = +2.289921970583567527179e-02 */
            0x3F8CBC0807393909uL, /* P9  = +1.403051635784581776625e-02 */
            0xBF85F06A30F93319uL, /* P10 = -1.071246110873285040939e-02 */
            /* Range reduction coefficients */
            0xBFC1000000000000uL, /* B = -.132813 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 2 , 0.14063]| < 0.15625 */
            /* Polynomial coefficients */
            0x3C6004EE5739DEACuL, /* PL0 = +6.947247374112211856530e-18 */
            0x3FC2DC968E6E0D62uL, /* PH0 = +1.473568149050193398786e-01 */
            0x3FEF4E1E606D96DFuL, /* P1  = +9.782859691010478680677e-01 */
            0xBFC273BD70994AB9uL, /* P2  = -1.441571044730005866646e-01 */
            0xBFD382B548270D2CuL, /* P3  = -3.048527912726111386771e-01 */
            0x3FB7CD2D582A6B29uL, /* P4  = +9.297450449450351894400e-02 */
            0x3FBC1278CCCBF0DBuL, /* P5  = +1.096568584434324642303e-01 */
            0xBFA9C7F5115B86A1uL, /* P6  = -5.035367810138536095866e-02 */
            0xBFA371C21BAF618EuL, /* P7  = -3.797728145554222910481e-02 */
            0x3F9958943F68417EuL, /* P8  = +2.475196492201935923783e-02 */
            0x3F8930D5CFFD4152uL, /* P9  = +1.230017701132682667572e-02 */
            0xBF875CF7ADD31B76uL, /* P10 = -1.140779017658897660092e-02 */
            /* Range reduction coefficients */
            0xBFC3000000000000uL, /* B = -.148438 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 3 , 0.15625]| < 0.17188 */
            /* Polynomial coefficients */
            0x3C7EABE24E052A1FuL, /* PL0 = +2.660321779421749543501e-17 */
            0x3FC4D04783618C71uL, /* PH0 = +1.626061812886266111366e-01 */
            0x3FEF2765AF97A4B3uL, /* P1  = +9.735592298067302883212e-01 */
            0xBFC443654205FEA5uL, /* P2  = -1.583067486171689074207e-01 */
            0xBFD31F2E208A5B97uL, /* P3  = -2.987780874040536844467e-01 */
            0x3FB9F235BD339878uL, /* P4  = +1.013520800512156573576e-01 */
            0x3FBAD0B0DFCCA141uL, /* P5  = +1.047468706498238100104e-01 */
            0xBFABD1B9600E608EuL, /* P6  = -5.433444306908184548967e-02 */
            0xBFA1CEBEAF07DB58uL, /* P7  = -3.478046309094534453598e-02 */
            0x3F9AFC9FB1D8EFD2uL, /* P8  = +2.635430834764902126383e-02 */
            0x3F8573444F1AB502uL, /* P9  = +1.047376028449287564018e-02 */
            0xBF8874FBC8F24406uL, /* P10 = -1.194187838544459322219e-02 */
            /* Range reduction coefficients */
            0xBFC5000000000000uL, /* B = -.164063 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 4 , 0.17188]| < 0.18750 */
            /* Polynomial coefficients */
            0x3C7FB199D361A790uL, /* PL0 = +2.748994907060158996213e-17 */
            0x3FC6C170259E21F7uL, /* PH0 = +1.777782615356639783766e-01 */
            0x3FEEFD17479F7C65uL, /* P1  = +9.683948897253570478266e-01 */
            0xBFC609530FE4DF8DuL, /* P2  = -1.721595599753950294577e-01 */
            0xBFD2B3465D71B4DEuL, /* P3  = -2.921920692959484052676e-01 */
            0x3FBBFD2D34AC509BuL, /* P4  = +1.093319181057403192166e-01 */
            0x3FB9778C3C16A0FEuL, /* P5  = +9.948040453912551395183e-02 */
            0xBFADAC4D9E63C665uL, /* P6  = -5.795519407719210697372e-02 */
            0xBFA0139CCAD02D60uL, /* P7  = -3.139963126894929339124e-02 */
            0x3F9C5BF43BA6F19DuL, /* P8  = +2.769452680671379432854e-02 */
            0x3F8190B703350341uL, /* P9  = +8.576803002712575184772e-03 */
            0xBF8936606782858AuL, /* P10 = -1.231074634444230850234e-02 */
            /* Range reduction coefficients */
            0xBFC7000000000000uL, /* B = -.179688 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 5 , 0.18750]| < 0.20313 */
            /* Polynomial coefficients */
            0x3C6A917CA3624D50uL, /* PL0 = +1.152216693509785660691e-17 */
            0x3FC8AFD7B974FABBuL, /* PH0 = +1.928662925292508878439e-01 */
            0x3FEECF47624A5D03uL, /* P1  = +9.628025932060214187231e-01 */
            0xBFC7C4C2CB4FDE4DuL, /* P2  = -1.856921665891938814679e-01 */
            0xBFD23F69CB2C1F9DuL, /* P3  = -2.851204380135586155453e-01 */
            0x3FBDEC5703A03814uL, /* P4  = +1.168875106670557712458e-01 */
            0x3FB8095003D0CF15uL, /* P5  = +9.389209836154706616487e-02 */
            0xBFAF554B47B10CBBuL, /* P6  = -6.119761705533607365968e-02 */
            0xBF9C89743FE7BC1BuL, /* P7  = -2.786809577986213853937e-02 */
            0x3F9D74725B746E7CuL, /* P8  = +2.876452143855921824991e-02 */
            0x3F7B2D8AFB70B88CuL, /* P9  = +6.635229968237631511880e-03 */
            0xBF89A0A2883EF6CBuL, /* P10 = -1.251341799058582545252e-02 */
            /* Range reduction coefficients */
            0xBFC9000000000000uL, /* B = -.195313 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 6 , 0.20313]| < 0.21875 */
            /* Polynomial coefficients */
            0x3C7608279E8609CBuL, /* PL0 = +1.910958764623660748269e-17 */
            0x3FCA9B46D2DDC5E3uL, /* PH0 = +2.078636674519166172015e-01 */
            0x3FEE9E0BB72A01A1uL, /* P1  = +9.567926957534390123919e-01 */
            0xBFC974FAD10C5330uL, /* P2  = -1.988824387305156976885e-01 */
            0xBFD1C40ACCBA4044uL, /* P3  = -2.775904654781735703430e-01 */
            0x3FBFBE24E2987853uL, /* P4  = +1.239951184474830487522e-01 */
            0x3FB6885B4345E47FuL, /* P5  = +8.801813499839460539687e-02 */
            0xBFB06563D5670584uL, /* P6  = -6.404708824176991770896e-02 */
            0xBF98CD1D620DF6E2uL, /* P7  = -2.421995078065365147772e-02 */
            0x3F9E44EF3E844D21uL, /* P8  = +2.955983943054463683119e-02 */
            0x3F7325FA0148CAAEuL, /* P9  = +4.674889165971292322643e-03 */
            0xBF89B4C8556C2D92uL, /* P10 = -1.255184660614964011319e-02 */
            /* Range reduction coefficients */
            0xBFCB000000000000uL, /* B = -.210938 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 7 , 0.21875]| < 0.23438 */
            /* Polynomial coefficients */
            0x3C6F19DAA20F51D5uL, /* PL0 = +1.348790537832000351176e-17 */
            0x3FCC83876CA98E15uL, /* PH0 = +2.227639465883021474557e-01 */
            0x3FEE697B662D07CDuL, /* P1  = +9.503762241004040620296e-01 */
            0xBFCB194C7ED76ACFuL, /* P2  = -2.117095584242946953999e-01 */
            0xBFD141A19E419762uL, /* P3  = -2.696308179350720680191e-01 */
            0x3FC0B89C64BC7B98uL, /* P4  = +1.306338779331468503007e-01 */
            0x3FB4F721150BBFC5uL, /* P5  = +8.189589275184434216748e-02 */
            0xBFB105AAFAB87898uL, /* P6  = -6.649273511036069461061e-02 */
            0xBF94FB3B31248C01uL, /* P7  = -2.048962104266749732921e-02 */
            0x3F9ECD31E588709CuL, /* P8  = +3.007963145692880855964e-02 */
            0x3F664A91A335C105uL, /* P9  = +2.721104095762541127495e-03 */
            0xBF89754E32E1E26EuL, /* P10 = -1.243077366619723806134e-02 */
            /* Range reduction coefficients */
            0xBFCD000000000000uL, /* B = -.226563 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 8 , 0.23438]| < 0.25000 */
            /* Polynomial coefficients */
            0x3C6AC6C889D8111DuL, /* PL0 = +1.161245469312620769170e-17 */
            0x3FCE6864FE55A3D0uL, /* PH0 = +2.375608674877001114112e-01 */
            0x3FEE31AEE116B82BuL, /* P1  = +9.435648342384913826391e-01 */
            0xBFCCB114B69E808BuL, /* P2  = -2.241540805525839833707e-01 */
            0xBFD0B8AB913BA99DuL, /* P3  = -2.612713735858507980441e-01 */
            0x3FC1823322BED48AuL, /* P4  = +1.367858810096190233514e-01 */
            0x3FB35822B7929893uL, /* P5  = +7.556359273675842651653e-02 */
            0xBFB18B03CC78D2DAuL, /* P6  = -6.852744810096158580830e-02 */
            0xBF911CCC3C8D5E5DuL, /* P7  = -1.671141738492420009734e-02 */
            0x3F9F0DEC2D99B12FuL, /* P8  = +3.032654789278515819797e-02 */
            0x3F4A28398B4EBD98uL, /* P9  = +7.982521989244205404918e-04 */
            0xBF88E60CB2FAB9A4uL, /* P10 = -1.215753480150000985458e-02 */
            /* Range reduction coefficients */
            0xBFCF000000000000uL, /* B = -.242188 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 9 , 0.25000]| < 0.28125 */
            /* Polynomial coefficients */
            0x3C89D2B6774FB61DuL, /* PL0 = +4.479593208720169247958e-17 */
            0x3FD09C744F539BE4uL, /* PH0 = +2.595492148088267558848e-01 */
            0x3FEDD823B0400D42uL, /* P1  = +9.326342050921214825882e-01 */
            0xBFCEFBF7FF305FCCuL, /* P2  = -2.420644756355144687086e-01 */
            0xBFCFC01DC4F24A41uL, /* P3  = -2.480504237797323303990e-01 */
            0x3FC291A2C26D5548uL, /* P4  = +1.450694512701977626753e-01 */
            0x3FB0D562E672D188uL, /* P5  = +6.575601698097532991976e-02 */
            0xBFB2201ECC119E06uL, /* P6  = -7.080261690281738261872e-02 */
            0xBF8695D50F778D31uL, /* P7  = -1.102796987010509974642e-02 */
            0x3F9EEC8CFBC031A0uL, /* P8  = +3.019924437107734972427e-02 */
            0xBF6030F0A4D3660AuL, /* P9  = -1.976461417694923328722e-03 */
            0xBF87845288A4AEF5uL, /* P10 = -1.148285369398347838494e-02 */
            /* Range reduction coefficients */
            0xBFD1000000000000uL, /* B = -.265625 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 10, 0.28125]| < 0.31250 */
            /* Polynomial coefficients */
            0x3C8B6AAB614D1C8DuL, /* PL0 = +4.756035418366735312727e-17 */
            0x3FD275F7E1CF7F63uL, /* PH0 = +2.884502129727392616410e-01 */
            0x3FED56658F74C9CCuL, /* P1  = +9.167964746359813351341e-01 */
            0xBFD0ECC045EBD596uL, /* P2  = -2.644501383614054083635e-01 */
            0xBFCD5A4BDE179180uL, /* P3  = -2.293181261476426808811e-01 */
            0x3FC3C00047D34767uL, /* P4  = +1.542969084462655120552e-01 */
            0x3FAAC7CE84FD609FuL, /* P5  = +5.230565427217581251974e-02 */
            0xBFB288948D2E8B43uL, /* P6  = -7.239654967137902384931e-02 */
            0xBF6D6605AAD5A1C0uL, /* P7  = -3.588687008847041164896e-03 */
            0x3F9DDB0790848E97uL, /* P8  = +2.915584392134337382866e-02 */
            0xBF75FDE291BAD5B4uL, /* P9  = -5.369076763306269573660e-03 */
            0xBF84CEA5C52E0A78uL, /* P10 = -1.015977390284671071888e-02 */
            /* Range reduction coefficients */
            0xBFD3000000000000uL, /* B = -.296875 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 11, 0.31250]| < 0.34375 */
            /* Polynomial coefficients */
            0x3C7139A81C8A6ECFuL, /* PL0 = +1.494049799478574591322e-17 */
            0x3FD4470650036407uL, /* PH0 = +3.168350011233659890841e-01 */
            0x3FECC9A69DFDDD48uL, /* P1  = +8.996155820631566629678e-01 */
            0xBFD23DED3A37A09FuL, /* P2  = -2.850297039535778028925e-01 */
            0xBFCAD302395D51C1uL, /* P3  = -2.095644741153943890185e-01 */
            0x3FC4A8FE3F309C22uL, /* P4  = +1.614072617096278705115e-01 */
            0x3FA3D161188AA436uL, /* P5  = +3.870681213931741151586e-02 */
            0xBFB288CFE5494E98uL, /* P6  = -7.240008685885823969403e-02 */
            0x3F6C7903EED8D334uL, /* P7  = +3.475673371918475361081e-03 */
            0x3F9BE023CDFB02F6uL, /* P8  = +2.722221321778569498033e-02 */
            0xBF80F8296F2C3A95uL, /* P9  = -8.285831170295390358336e-03 */
            0xBF8152DF4790049BuL, /* P10 = -8.458847400108650973189e-03 */
            /* Range reduction coefficients */
            0xBFD5000000000000uL, /* B = -.328125 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 12, 0.34375]| < 0.37500 */
            /* Polynomial coefficients */
            0x3C7751FE0FEE8335uL, /* PL0 = +2.022712113430213599928e-17 */
            0x3FD60EF7120502A9uL, /* PH0 = +3.446633983585721261456e-01 */
            0x3FEC32D951E56E6FuL, /* P1  = +8.812071418319202070776e-01 */
            0xBFD370255FC004F8uL, /* P2  = -3.037198481616338996824e-01 */
            0xBFC832F0EBC6BB41uL, /* P3  = -1.890545989276351359107e-01 */
            0x3FC54C99A0FF432FuL, /* P4  = +1.664001499289269127540e-01 */
            0x3F99DAC0CC283C18uL, /* P5  = +2.524853941036661688369e-02 */
            0xBFB227B3896A026DuL, /* P6  = -7.091829399906553280461e-02 */
            0x3F84663364E1FB19uL, /* P7  = +9.960557476231411602383e-03 */
            0x3F9922D70DE07C57uL, /* P8  = +2.454696676442965935283e-02 */
            0xBF85C4A4EB6F86BCuL, /* P9  = -1.062897532932837635222e-02 */
            0xBF7AAB61214FFE17uL, /* P10 = -6.511096396024671890972e-03 */
            /* Range reduction coefficients */
            0xBFD7000000000000uL, /* B = -.359375 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 13, 0.37500]| < 0.40625 */
            /* Polynomial coefficients */
            0x3BFE67F266843B2CuL, /* PL0 = +1.030196791298162288777e-19 */
            0x3FD7CD3115FC0F16uL, /* PH0 = +3.718989100163850869407e-01 */
            0x3FEB92F96CCC2C5BuL, /* P1  = +8.616912007286247079761e-01 */
            0xBFD4827320135092uL, /* P2  = -3.204620183216856200247e-01 */
            0xBFC582B15550168AuL, /* P3  = -1.680509249273891977521e-01 */
            0x3FC5AC3B9A2E4C31uL, /* P4  = +1.693186285816366254244e-01 */
            0x3F88FA599FCADAFBuL, /* P5  = +1.219625491044728129762e-02 */
            0xBFB16EC8F5CA169EuL, /* P6  = -6.809669495313605642174e-02 */
            0x3F90140EFC748BBEuL, /* P7  = +1.570151725639922719844e-02 */
            0x3F95CFC49C1A28DCuL, /* P8  = +2.130038454792147768770e-02 */
            0xBF8946ED8B1BF454uL, /* P9  = -1.234231549050882816697e-02 */
            0xBF7239E55C1DD50FuL, /* P10 = -4.449745117985472755606e-03 */
            /* Range reduction coefficients */
            0xBFD9000000000000uL, /* B = -.390625 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 14, 0.40625]| < 0.43750 */
            /* Polynomial coefficients */
            0x3C6412330191189CuL, /* PL0 = +8.704448096175471149661e-18 */
            0x3FD9812B3B03F0A5uL, /* PH0 = +3.985088421175169703936e-01 */
            0x3FEAEB08C3C0E84DuL, /* P1  = +8.411907027541559254748e-01 */
            0xBFD57446B1BC46CFuL, /* P2  = -3.352219329545790787820e-01 */
            0xBFC2CA9ABC0444ADuL, /* P3  = -1.468079965639267634401e-01 */
            0x3FC5CA95F9460D18uL, /* P4  = +1.702449290424759093710e-01 */
            0xBF2C2DAA35DD05C3uL, /* P5  = -2.149839664813813012186e-04 */
            0xBFB069A516EEB75DuL, /* P6  = -6.411201295733578195472e-02 */
            0x3F9512716416FDC7uL, /* P7  = +2.057816670798986720058e-02 */
            0x3F921630CB1319A3uL, /* P8  = +1.766277541607908852593e-02 */
            0xBF8B76DA2EC99526uL, /* P9  = -1.341028647693549562145e-02 */
            0xBF63A97474A161E4uL, /* P10 = -2.400138332671485493040e-03 */
            /* Range reduction coefficients */
            0xBFDB000000000000uL, /* B = -.421875 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 15, 0.43750]| < 0.46875 */
            /* Polynomial coefficients */
            0x3C89B79F5783381CuL, /* PL0 = +4.461236087774530799537e-17 */
            0x3FDB2A6C993B829DuL, /* PH0 = +4.244643684778937609003e-01 */
            0x3FEA3C0C1FBA328CuL, /* P1  = +8.198299998926627915155e-01 */
            0xBFD6457212F78DE0uL, /* P2  = -3.479886231636708581604e-01 */
            0xBFC0129BDA380A66uL, /* P3  = -1.255678954622282824818e-01 */
            0x3FC5AB77F388FBDEuL, /* P4  = +1.692953051696965507089e-01 */
            0xBF8822F3A6CADB7CuL, /* P5  = -1.178541519889874597783e-02 */
            0xBFAE4A876370A4BDuL, /* P6  = -5.916236008517603590739e-02 */
            0x3F991A89BC3B7710uL, /* P7  = +2.451529704455085335710e-02 */
            0x3F8C4A4328204D4BuL, /* P8  = +1.381351915555364098800e-02 */
            0xBF8C5F921D01EC0BuL, /* P9  = -1.385416174911393178490e-02 */
            0xBF3EE844C5B79FB8uL, /* P10 = -4.716079617694784908234e-04 */
            /* Range reduction coefficients */
            0xBFDD000000000000uL, /* B = -.453125 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 16, 0.46875]| < 0.50000 */
            /* Polynomial coefficients */
            0x3C73FA437AD7AD87uL, /* PL0 = +1.732779905745858845932e-17 */
            0x3FDCC88C9902CF45uL, /* PH0 = +4.497405523536495697279e-01 */
            0x3FE9870845162D1DuL, /* P1  = +7.977334355686341748810e-01 */
            0xBFD6F62358F73DA8uL, /* P2  = -3.587730759436120677668e-01 */
            0xBFBAC4345D675FE1uL, /* P3  = -1.045563438450467661101e-01 */
            0x3FC5539DA8287019uL, /* P4  = +1.666142531474868131862e-01 */
            0xBF96E3E0DC04A09FuL, /* P5  = -2.235366194614185212822e-02 */
            0xBFAB5EC7147C207DuL, /* P6  = -5.345747113284546871398e-02 */
            0x3F9C24166FFA7A58uL, /* P7  = +2.748141344511120915667e-02 */
            0x3F8451B907819844uL, /* P8  = +9.921498815128277696693e-03 */
            0xBF8C1C6D19191FCBuL, /* P9  = -1.372609360545586670239e-02 */
            0x3F547372DF72E35AuL, /* P10 = +1.248228245272117756098e-03 */
            /* Range reduction coefficients */
            0xBFDF000000000000uL, /* B = -.484375 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 17, 0.50000]| < 0.56250 */
            /* Polynomial coefficients */
            0x3C848FE06EE49950uL, /* PL0 = +3.566941590788961528958e-17 */
            0x3FDF20211A36475DuL, /* PH0 = +4.863360172249622803697e-01 */
            0x3FE86E67E6B80AC2uL, /* P1  = +7.634772783497611574659e-01 */
            0xBFD7C37C55474D9BuL, /* P2  = -3.713064987943767913461e-01 */
            0xBFB2EBF15F3CB036uL, /* P3  = -7.391270232318521952684e-02 */
            0x3FC4718C8EF6E3AAuL, /* P4  = +1.597152422016539530950e-01 */
            0xBFA277F8394E9B07uL, /* P5  = -3.607154559658991932071e-02 */
            0xBFA680312AB207E3uL, /* P6  = -4.394677778419955009224e-02 */
            0x3F9EDC9A8B57E286uL, /* P7  = +3.013841128810892143223e-02 */
            0x3F71B8C5E648EAF6uL, /* P8  = +4.326603932492947851719e-03 */
            0xBF89DB218356730CuL, /* P9  = -1.262499029217558458029e-02 */
            0x3F6B05728E6EBC8EuL, /* P10 = +3.298496001171330815865e-03 */
            /* Range reduction coefficients */
            0xBFE1000000000000uL, /* B = -.53125  */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 18, 0.56250]| < 0.62500 */
            /* Polynomial coefficients */
            0x3C8429831EDD94DEuL, /* PL0 = +3.497576705878673192147e-17 */
            0x3FE10AF47E0BF610uL, /* PH0 = +5.325872861719194162333e-01 */
            0x3FE6EC5879F87EEEuL, /* P1  = +7.163507826080299761242e-01 */
            0xBFD86AD001BFE200uL, /* P2  = -3.815193192563413204129e-01 */
            0xBFA239045B661385uL, /* P3  = -3.559125533778398983564e-02 */
            0x3FC2B4572D9CC147uL, /* P4  = +1.461285565105845078038e-01 */
            0xBFA99F4F01740705uL, /* P5  = -5.004355328311586406115e-02 */
            0xBF9F449C484F4879uL, /* P6  = -3.053516570418721511214e-02 */
            0x3F9F5F42169D7DDEuL, /* P7  = +3.063681853325116830798e-02 */
            0xBF6111B1BA632A97uL, /* P8  = -2.083632588527460989469e-03 */
            0xBF84725FBE5B6E61uL, /* P9  = -9.983776089419639342530e-03 */
            0x3F7438A2986CFA9CuL, /* P10 = +4.936823976832951342488e-03 */
            /* Range reduction coefficients */
            0xBFE3000000000000uL, /* B = -.59375  */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 19, 0.62500]| < 0.68750 */
            /* Polynomial coefficients */
            0x3C6BE9160BFB3505uL, /* PL0 = +1.210424670976053242391e-17 */
            0x3FE26D76F73233C7uL, /* PH0 = +5.758623912857893101247e-01 */
            0x3FE56363B5B93937uL, /* P1  = +6.683825063026124740752e-01 */
            0xBFD8A2244B27297EuL, /* P2  = -3.848963483730115724200e-01 */
            0xBF52CA2F101EEF63uL, /* P3  = -1.146837196286797844817e-03 */
            0x3FC081BC342243ADuL, /* P4  = +1.289592032012739958675e-01 */
            0xBFAE38DB4A932344uL, /* P5  = -5.902753148399722719732e-02 */
            0xBF91F814D4AE90C6uL, /* P6  = -1.754791782481459457885e-02 */
            0x3F9D056AE193C4F3uL, /* P7  = +2.834097863973723355792e-02 */
            0xBF7BD0B502D8F3A0uL, /* P8  = -6.790835451792626336974e-03 */
            0xBF7B763F7BB8AE2FuL, /* P9  = -6.704566938008179114124e-03 */
            0x3F76036F42D9AB69uL, /* P10 = +5.374369252971835729099e-03 */
            /* Range reduction coefficients */
            0xBFE5000000000000uL, /* B = -.65625  */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 20, 0.68750]| < 0.75000 */
            /* Polynomial coefficients */
            0x3C8B64AF0450486EuL, /* PL0 = +4.751979286662385162741e-17 */
            0x3FE3B75F8BCB742DuL, /* PH0 = +6.161344271055263499548e-01 */
            0x3FE3DA23BC12369FuL, /* P1  = +6.203783677353447780947e-01 */
            0xBFD8768FF4B46416uL, /* P2  = -3.822364701932782367281e-01 */
            0x3F9D67CB8AD9CB1AuL, /* P3  = +2.871625933625941117406e-02 */
            0x3FBC168CB7827DF4uL, /* P4  = +1.097190807363331305006e-01 */
            0xBFB03A2B83C9272EuL, /* P5  = -6.338760344911228324430e-02 */
            0xBF789FEB595297DCuL, /* P6  = -6.011885959344067548074e-03 */
            0x3F98BD01B4C335E7uL, /* P7  = +2.415850320612902513532e-02 */
            0xBF83BADC303D6535uL, /* P8  = -9.633751127398152979976e-03 */
            0xBF6C54E7A1C1E3F3uL, /* P9  = -3.458454519258407989501e-03 */
            0x3F7408394B7EF3E7uL, /* P10 = +4.890655334688332484537e-03 */
            /* Range reduction coefficients */
            0xBFE7000000000000uL, /* B = -.71875  */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 21, 0.75000]| < 0.81250 */
            /* Polynomial coefficients */
            0x3C6A48557F6E0D3EuL, /* PL0 = +1.139824111505584215867e-17 */
            0x3FE4E8D895B010DCuL, /* PH0 = +6.534235881413468227663e-01 */
            0x3FE25652FAAF8A73uL, /* P1  = +5.730376144604875448991e-01 */
            0xBFD7F6C3A57C444BuL, /* P2  = -3.744362941807295084434e-01 */
            0x3FAB7866E3F99EBEuL, /* P3  = +5.365296872042567001598e-02 */
            0x3FB6FA1DF47CCD40uL, /* P4  = +8.975398272450707099784e-02 */
            0xBFB05508D3741B8EuL, /* P5  = -6.379752314033580026840e-02 */
            0x3F6C3EFDF7BB279CuL, /* P6  = +3.448005705512137236209e-03 */
            0x3F9372BADD6D3E27uL, /* P7  = +1.899234749299530050806e-02 */
            0xBF860FD5AE65F3DAuL, /* P8  = -1.077238977881649471165e-02 */
            0xBF47266FFB07E628uL, /* P9  = -7.064863949032872448118e-04 */
            0x3F6F9763992C2A05uL, /* P10 = +3.856367614735181120799e-03 */
            /* Range reduction coefficients */
            0xBFE9000000000000uL, /* B = -.78125  */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 22, 0.81250]| < 0.87500 */
            /* Polynomial coefficients */
            0x3C6BB6A2B194E3ABuL, /* PL0 = +1.201878007209462528697e-17 */
            0x3FE602609AAE7C22uL, /* PH0 = +6.877902051090851731630e-01 */
            0x3FE0DCBAFE191C7FuL, /* P1  = +5.269446337560025312137e-01 */
            0xBFD732028428A9FBuL, /* P2  = -3.624273577321727538225e-01 */
            0x3FB2D92389BE065BuL, /* P3  = +7.362577545975439796588e-02 */
            0x3FB1F6A9C8C49993uL, /* P4  = +7.017003203927733370937e-02 */
            0xBFAF47C0B50B56EEuL, /* P5  = -6.109430513394707378526e-02 */
            0x3F85A8EDD1356223uL, /* P6  = +1.057611269668352068104e-02 */
            0x3F8BE05C5CD1B4FAuL, /* P7  = +1.361152799855823798207e-02 */
            0xBF85A0EFE4552F76uL, /* P8  = -1.056086936537046752272e-02 */
            0x3F559F2A6A356194uL, /* P9  = +1.319686337259627831943e-03 */
            0x3F6576F5E989208DuL, /* P10 = +2.620201394425042596201e-03 */
            /* Range reduction coefficients */
            0xBFEB000000000000uL, /* B = -.84375  */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 23, 0.87500]| < 0.93750 */
            /* Polynomial coefficients */
            0x3C80328BD86C8B74uL, /* PL0 = +2.809809047161267929701e-17 */
            0x3FE704BB1B7FCB81uL, /* PH0 = +7.193275010198335595035e-01 */
            0x3FDEE264AAD6C40CuL, /* P1  = +4.825679462765613089739e-01 */
            0xBFD637493CE659F1uL, /* P2  = -3.471243948673921548357e-01 */
            0x3FB6BE3A3DEE6F4AuL, /* P3  = +8.884014141079635303208e-02 */
            0x3FAA85EB6470AC0FuL, /* P4  = +5.180297471118688523488e-02 */
            0xBFACC0146EA4858DuL, /* P5  = -5.615295267694895314457e-02 */
            0x3F8F8FB683CDDAC5uL, /* P6  = +1.541082944616557159055e-02 */
            0x3F819515DEE2CB91uL, /* P7  = +8.585139145315585602547e-03 */
            0xBF834E45E6AF9EA1uL, /* P8  = -9.426637747267209169415e-03 */
            0x3F65250F197CA56DuL, /* P9  = +2.581147662472352252568e-03 */
            0x3F57A766026D036CuL, /* P10 = +1.443719500187702367690e-03 */
            /* Range reduction coefficients */
            0xBFED000000000000uL, /* B = -.90625  */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 24, 0.93750]| < 1.00000 */
            /* Polynomial coefficients */
            0x3C716F7EEF7B61ADuL, /* PL0 = +1.512291215142578135651e-17 */
            0x3FE7F0E1A4CD846EuL, /* PH0 = +7.481544703297353660076e-01 */
            0x3FDC2D4CC872DC09uL, /* P1  = +4.402648885256331012598e-01 */
            0xBFD514A99F92ED53uL, /* P2  = -3.293861444796750250530e-01 */
            0x3FB9846A6CF2F337uL, /* P3  = +9.967675361526749494844e-02 */
            0x3FA20896939AB161uL, /* P4  = +3.522177268800664413493e-02 */
            0xBFA97E801F31EE0DuL, /* P5  = -4.979324703978358553405e-02 */
            0x3F92A11F47B82085uL, /* P6  = +1.819275737037219740638e-02 */
            0x3F717D70FE289C34uL, /* P7  = +4.270020845559097605514e-03 */
            0xBF7FDCF1D3F6CE2DuL, /* P8  = -7.779068604054678540132e-03 */
            0x3F69F607E81AF6B6uL, /* P9  = +3.169074480722534625181e-03 */
            0x3F3F925C80D0F889uL, /* P10 = +4.817462766516585511824e-04 */
            /* Range reduction coefficients */
            0xBFEF000000000000uL, /* B = -.96875  */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 25, 1.00000]| < 1.12500 */
            /* Polynomial coefficients */
            0x3C931A11D7E8606EuL, /* PL0 = +6.627280241435322692188e-17 */
            0x3FE92BFB370D9B71uL, /* PH0 = +7.866188121086975515439e-01 */
            0x3FD866160E454111uL, /* P1  = +3.812308444367014680480e-01 */
            0xBFD33149F3801DBAuL, /* P2  = -2.998833539899937679796e-01 */
            0x3FBBDB6D4C949899uL, /* P3  = +1.088169395412442909023e-01 */
            0x3F8D6AB2A74B9343uL, /* P4  = +1.436366627735597372494e-02 */
            0xBFA404D1047C5D72uL, /* P5  = -3.909924678571997970917e-02 */
            0x3F93C47D9ACCD919uL, /* P6  = +1.930423981976856424661e-02 */
            0xBF41B755642CFF1BuL, /* P7  = -5.406538915408738478158e-04 */
            0xBF74B5301AA1E788uL, /* P8  = -5.055606752756853900641e-03 */
            0x3F69A84C5B2A3E68uL, /* P9  = +3.132008679422249529120e-03 */
            0xBF3CF47830328C11uL, /* P10 = -4.418176105877589308931e-04 */
            /* Range reduction coefficients */
            0xBFF1000000000000uL, /* B = -1.0625   */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 26, 1.12500]| < 1.25000 */
            /* Polynomial coefficients */
            0x3C884D471B8FD396uL, /* PL0 = +4.215701792312937090514e-17 */
            0x3FEA8DBCBC31897AuL, /* PH0 = +8.298019099859594849278e-01 */
            0x3FD3EE730537C8EAuL, /* P1  = +3.114287901836535219818e-01 */
            0xBFD08A05AD27CE32uL, /* P2  = -2.584242049190123217982e-01 */
            0x3FBC5255406F84B6uL, /* P3  = +1.106313021005175045399e-01 */
            0xBF772FA2F633AA5EuL, /* P4  = -5.660664147607434209241e-03 */
            0xBF99DD8E4C473FC4uL, /* P5  = -2.525923100057504533247e-02 */
            0x3F9183C935B6495DuL, /* P6  = +1.710428610165003372069e-02 */
            0xBF70471A3A591480uL, /* P7  = -3.974058583087303228038e-03 */
            0xBF603DDD4DEBB9A4uL, /* P8  = -1.982624278176818987264e-03 */
            0x3F62591E44D3C17FuL, /* P9  = +2.239760512218135956425e-03 */
            0xBF4C195D3A9B1AB4uL, /* P10 = -8.575158328419569430544e-04 */
            /* Range reduction coefficients */
            0xBFF3000000000000uL, /* B = -1.1875   */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 27, 1.25000]| < 1.37500 */
            /* Polynomial coefficients */
            0x3C90DD1C9BFF7F64uL, /* PL0 = +5.850777430004479798187e-17 */
            0x3FEBAD50A4A68BC1uL, /* PH0 = +8.649066177207417327466e-01 */
            0x3FD01FBA72CEE1A5uL, /* P1  = +2.519365426228666233893e-01 */
            0xBFCBE432F647C4D6uL, /* P2  = -2.179015829602010702633e-01 */
            0x3FBABF92B6E5AC73uL, /* P3  = +1.044856735731387955105e-01 */
            0xBF922983AA24E217uL, /* P4  = -1.773648954369563555378e-02 */
            0xBF8C72214C14E23AuL, /* P5  = -1.388956082756564056328e-02 */
            0x3F8ACB4D1F388E8BuL, /* P6  = +1.308307887581540972153e-02 */
            0xBF740EF8B4A2EE3BuL, /* P7  = -4.897090441029978580995e-03 */
            0xBF0EA9F30C8DC900uL, /* P8  = -5.848668076326342477133e-05 */
            0x3F53CC40D18713AEuL, /* P9  = +1.208365725788622757410e-03 */
            0xBF4848B86029CBA1uL, /* P10 = -7.410908004444779592485e-04 */
            /* Range reduction coefficients */
            0xBFF5000000000000uL, /* B = -1.3125   */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 28, 1.37500]| < 1.50000 */
            /* Polynomial coefficients */
            0x3C8FB61781D22681uL, /* PL0 = +5.501032995458057064843e-17 */
            0x3FEC950A3340C8BFuL, /* PH0 = +8.931933404003514764824e-01 */
            0x3FC9E1DFFD385423uL, /* P1  = +2.022056566644617586005e-01 */
            0xBFC71E2FF88EBA23uL, /* P2  = -1.806087459239772032583e-01 */
            0x3FB80AEBD07AB5BAuL, /* P3  = +9.391664352252506838449e-02 */
            0xBF98404E27EAE6EDuL, /* P4  = -2.368280523908243895884e-02 */
            0xBF772DA520B5006EuL, /* P5  = -5.658764868087568802107e-03 */
            0x3F824C9268AF9423uL, /* P6  = +8.935111827620250551925e-03 */
            0xBF722AE76D206AE3uL, /* P7  = -4.435447701349490160113e-03 */
            0x3F4B807F56298D5EuL, /* P8  = +8.392926941493230644497e-04 */
            0x3F3D71027DF95D2AuL, /* P9  = +4.492407879061627603159e-04 */
            0xBF3EBD17676755FBuL, /* P10 = -4.690343988874298905483e-04 */
            /* Range reduction coefficients */
            0xBFF7000000000000uL, /* B = -1.4375   */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 29, 1.50000]| < 1.62500 */
            /* Polynomial coefficients */
            0x3C95393C63CE8224uL, /* PL0 = +7.363407705201031038415e-17 */
            0x3FED4E6F464286B0uL, /* PH0 = +9.158245441687622445670e-01 */
            0x3FC4A45842B7DE1EuL, /* P1  = +1.612654042980787191461e-01 */
            0xBFC2E7885AFDD3D0uL, /* P2  = -1.476908153814791087327e-01 */
            0x3FB4DD6DD51D3FEBuL, /* P3  = +8.150373890862254580204e-02 */
            0xBF9A05D3ADAB489CuL, /* P4  = -2.541285274021075503042e-02 */
            0xBF3459B643B4995CuL, /* P5  = -3.105230313899165257622e-04 */
            0x3F766B30745F2E3AuL, /* P6  = +5.473317409222350365811e-03 */
            0xBF6C2C891E555BDFuL, /* P7  = -3.439204988051155730940e-03 */
            0x3F5194F30D6C576DuL, /* P8  = +1.073109966176012791522e-03 */
            0x3EF4DBB43C3132A2uL, /* P9  = +1.989194766975849961365e-05 */
            0xBF2E45EBAB3C15A0uL, /* P10 = -2.309656316514087783666e-04 */
            /* Range reduction coefficients */
            0xBFF9000000000000uL, /* B = -1.5625   */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 30, 1.62500]| < 1.75000 */
            /* Polynomial coefficients */
            0x3C75111669651DAAuL, /* PL0 = +1.827249135453834384396e-17 */
            0x3FEDE1EB5937518FuL, /* PH0 = +9.338280432225917193634e-01 */
            0x3FC06129C7C8EBB1uL, /* P1  = +1.279651856910653382507e-01 */
            0xBFBE9763041064E1uL, /* P2  = -1.194974789545031421774e-01 */
            0x3FB1A5B9F9113928uL, /* P3  = +6.893503504509068635308e-02 */
            0xBF992145039F9AFEuL, /* P4  = -2.454097590080105816526e-02 */
            0x3F66CB116EA49C89uL, /* P5  = +2.782377288116648315142e-03 */
            0x3F67F972FDF30001uL, /* P6  = +2.926563829163342740100e-03 */
            0xBF63A7B5975F02F3uL, /* P7  = -2.399305983061922438601e-03 */
            0x3F4FDE7B8777F4C8uL, /* P8  = +9.725669069095216373599e-04 */
            0xBF25918876626BA4uL, /* P9  = -1.645545082212515656240e-04 */
            0xBF1495123C991F00uL, /* P10 = -7.851527984669912693674e-05 */
            /* Range reduction coefficients */
            0xBFFB000000000000uL, /* B = -1.6875   */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 31, 1.75000]| < 1.87500 */
            /* Polynomial coefficients */
            0x3C9F29A5B7426D27uL, /* PL0 = +1.081172820484012446345e-16 */
            0x3FEE56B6F3EFABFCuL, /* PH0 = +9.480852856044061915952e-01 */
            0x3FB9E3EFD94BB9FCuL, /* P1  = +1.011342912204113371518e-01 */
            0xBFB88BD9760FECA7uL, /* P2  = -9.588393337610288420285e-02 */
            0x3FAD48A0350B3ACFuL, /* P3  = +5.719471595295077387313e-02 */
            0xBF96CC6A5110F129uL, /* P4  = -2.226415748394675367257e-02 */
            0x3F71934687170384uL, /* P5  = +4.290843485649345772606e-03 */
            0x3F5407BAF73B3DF9uL, /* P6  = +1.222546180475235334287e-03 */
            0xBF591B626C0646DDuL, /* P7  = -1.532407870488964407324e-03 */
            0x3F48B0E1DD283558uL, /* P8  = +7.535078860329375669277e-04 */
            0xBF2B322292840D2BuL, /* P9  = -2.074877932117605962646e-04 */
            0xBE99E4061120C741uL, /* P10 = -3.858017559892704559672e-07 */
            /* Range reduction coefficients */
            0xBFFD000000000000uL, /* B = -1.8125   */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 32, 1.87500]| < 2.00000 */
            /* Polynomial coefficients */
            0x3C6AF8C2041C67CDuL, /* PL0 = +1.169711482626385762338e-17 */
            0x3FEEB2DFEDD5EC93uL, /* PH0 = +9.593352933146824801369e-01 */
            0x3FB465A205CFB638uL, /* P1  = +7.967579500083210999681e-02 */
            0xBFB3914BF68D39FFuL, /* P2  = -7.643580216720378576778e-02 */
            0x3FA7F21A08C5C734uL, /* P3  = +4.676896435820623621673e-02 */
            0xBF93DA9560EA9960uL, /* P4  = -1.938851741820124550772e-02 */
            0x3F73953FEC62820EuL, /* P5  = +4.781007481284861359820e-03 */
            0x3F2749D5E1273E3CuL, /* P6  = +1.776765426044646108071e-04 */
            0xBF4D46B0B498CE5AuL, /* P7  = -8.934367007839658352859e-04 */
            0x3F4153D680E1F4C4uL, /* P8  = +5.287930851093571206574e-04 */
            0xBF28477014ECA6A2uL, /* P9  = -1.852344816708944640949e-04 */
            0x3EFFAC54E07CEB4BuL, /* P10 = +3.020588886147182143902e-05 */
            /* Range reduction coefficients */
            0xBFFF000000000000uL, /* B = -1.9375   */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 33, 2.00000]| < 2.25000 */
            /* Polynomial coefficients */
            0x3C7A8AF2BB2231F2uL, /* PL0 = +2.302217989249372577466e-17 */
            0x3FEF1994DF724FC8uL, /* PH0 = +9.718727459135090285258e-01 */
            0x3FAC65B1BC0C9D58uL, /* P1  = +5.546336575053583942603e-02 */
            0xBFAB9937BDA747C8uL, /* P2  = -5.390333356957871365599e-02 */
            0x3FA15B42D9EF931CuL, /* P3  = +3.389939222669210777241e-02 */
            0xBF8EACD8E8507A3CuL, /* P4  = -1.497811755149058215502e-02 */
            0x3F7263A15721C682uL, /* P5  = +4.489546046998806349050e-03 */
            0xBF42A032ACDC3B32uL, /* P6  = -5.684134900735048121829e-04 */
            0xBF3431E79B5AD185uL, /* P7  = -3.081503340170088810438e-04 */
            0x3F31B51667C7DF5EuL, /* P8  = +2.701930714290502424828e-04 */
            0xBF1F8709579250ADuL, /* P9  = -1.202678157759563704341e-04 */
            0x3F01ED8ED1BF9595uL, /* P10 = +3.419487094883790833778e-05 */
            /* Range reduction coefficients */
            0xC001000000000000uL, /* B = -2.125    */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 34, 2.25000]| < 2.50000 */
            /* Polynomial coefficients */
            0x3C86F3F7C3DAFC55uL, /* PL0 = +3.981710680748877459333e-17 */
            0x3FEF73776B2AA2DBuL, /* PH0 = +9.828450291725759901951e-01 */
            0x3FA16A7FC4D7B900uL, /* P1  = +3.401564863075812007064e-02 */
            0xBFA11E03803AD621uL, /* P2  = -3.343211117082156940532e-02 */
            0x3F9609591597297FuL, /* P3  = +2.152003473546803654658e-02 */
            0xBF847E74ED9BBB0CuL, /* P4  = -1.000682211039596246436e-02 */
            0x3F6BFF771725CD65uL, /* P5  = +3.417713736035987187864e-03 */
            0xBF491D1FF73C18FAuL, /* P6  = -7.664114077392807421000e-04 */
            0x3EF53EE467B51DC5uL, /* P7  = +2.026145237479599375099e-05 */
            0x3F160135BE0D94A0uL, /* P8  = +8.394136922403255700685e-05 */
            0xBF0B32CB1D276A40uL, /* P9  = -5.187685350778849443841e-05 */
            0x3EF4DAF70C12D555uL, /* P10 = +1.988919462255396826584e-05 */
            /* Range reduction coefficients */
            0xC003000000000000uL, /* B = -2.375    */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 35, 2.50000]| < 2.75000 */
            /* Polynomial coefficients */
            0x3C19DBF4E2E5B7DCuL, /* PL0 = +3.504575836708380670219e-19 */
            0x3FEFAA7934B75EBDuL, /* PH0 = +9.895597486128832054320e-01 */
            0x3F9545200830A42CuL, /* P1  = +2.077150392520736492125e-02 */
            0xBF950C46D285F6BCuL, /* P2  = -2.055464420253970271376e-02 */
            0x3F8B79F5BFC6513FuL, /* P3  = +1.341621390819425058164e-02 */
            0xBF7A50ADAD777898uL, /* P4  = -6.424597194806612772505e-03 */
            0x3F633A19BE8255E3uL, /* P5  = +2.347040444940816227383e-03 */
            0xBF44E609BC2557B7uL, /* P6  = -6.377742322836087134324e-04 */
            0x3F1AFCBAD60EAACDuL, /* P7  = +1.029480968230231421206e-04 */
            0x3EE80476AC34A8EFuL, /* P8  = +1.145240583485084317660e-05 */
            0xBEF278E23DE463E9uL, /* P9  = -1.761646478213091821804e-05 */
            0x3EE209FAF377264DuL, /* P10 = +8.601658563106529694651e-06 */
            /* Range reduction coefficients */
            0xC005000000000000uL, /* B = -2.625    */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 36, 2.75000]| < 3.00000 */
            /* Polynomial coefficients */
            0x3C979D62702C631CuL, /* PL0 = +8.193023793215066385979e-17 */
            0x3FEFCC04CDBCDC4BuL, /* PH0 = +9.936546343150295390600e-01 */
            0x3F89E87D088D269AuL, /* P1  = +1.265046770426474576547e-02 */
            0xBF89BE6721012B80uL, /* P2  = -1.257019586059526836624e-02 */
            0x3F80F1C13E8D39D3uL, /* P3  = +8.273610803056031004326e-03 */
            0xBF7082DBC9602757uL, /* P4  = -4.031046430108839563004e-03 */
            0x3F590BE9BD4E0A11uL, /* P5  = +1.528719197467002507978e-03 */
            0xBF3DCC2BEF6D0283uL, /* P6  = -4.546744598208711809986e-04 */
            0x3F1A08065C4A8E85uL, /* P7  = +9.930170842636406837764e-05 */
            0xBEE528117D0410F3uL, /* P8  = -1.008821337267942266431e-05 */
            0xBED0BE73A44FF565uL, /* P9  = -3.992069257383521775961e-06 */
            0x3EC9B0C11E342E38uL, /* P10 = +3.062539904901699218737e-06 */
            /* Range reduction coefficients */
            0xC007000000000000uL, /* B = -2.875    */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 37, 3.00000]| < 3.25000 */
            /* Polynomial coefficients */
            0x3C804B931AD7A3CCuL, /* PL0 = +2.826768921701616830245e-17 */
            0x3FEFE06EB0688212uL, /* PH0 = +9.961465306733450209009e-01 */
            0x3F7F81BD8876224DuL, /* P1  = +7.692089427458426472642e-03 */
            0xBF7F62A8C699A963uL, /* P2  = -7.662448196791823756776e-03 */
            0x3F74C31E2B2A6A28uL, /* P3  = +5.068891378551522166321e-03 */
            0xBF6470D537F16227uL, /* P4  = -2.495209162173734080001e-03 */
            0x3F4FAEEF61C89673uL, /* P5  = +9.668988091717359455754e-04 */
            0xBF33C5E80B349783uL, /* P6  = -3.017131341088651514023e-04 */
            0x3F138F3D31037A6BuL, /* P7  = +7.461367590931028650557e-05 */
            0xBEEB3C780996FFE3uL, /* P8  = -1.298723536791163711556e-05 */
            0x3E9D0C75BC8BFEFCuL, /* P9  = +4.328589367358221917138e-07 */
            0x3EAC3865227764D4uL, /* P10 = +8.410302755848104487452e-07 */
            /* Range reduction coefficients */
            0xC009000000000000uL, /* B = -3.125    */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 38, 3.25000]| < 3.50000 */
            /* Polynomial coefficients */
            0x3C5B978B202749F9uL, /* PL0 = +5.983054034451594408315e-18 */
            0x3FEFECD6B7EA3128uL, /* PH0 = +9.976609794698889643882e-01 */
            0x3F73238B786137FEuL, /* P1  = +4.672570043181776968058e-03 */
            0xBF731815ACEA072EuL, /* P2  = -4.661640805922390930706e-03 */
            0x3F6956F0816D5AEEuL, /* P3  = +3.093213784647877798933e-03 */
            0xBF591A16286C4885uL, /* P4  = -1.532098425461232453877e-03 */
            0x3F43B3E3A00C6096uL, /* P5  = +6.012784434430592468442e-04 */
            0xBF29441B2A56DEC7uL, /* P6  = -1.927645836710038499293e-04 */
            0x3F0A99C3A2E857B6uL, /* P7  = +5.073669705184196724674e-05 */
            0xBEE61CB034DDC151uL, /* P8  = -1.054385361573597042258e-05 */
            0x3EB792BBC76D6107uL, /* P9  = +1.405070887824641788698e-06 */
            0x3E761472362A16F0uL, /* P10 = +8.225391704739515383837e-08 */
            /* Range reduction coefficients */
            0xC00B000000000000uL, /* B = -3.375    */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 39, 3.50000]| < 3.75000 */
            /* Polynomial coefficients */
            0x3C9C290AFCBDE00DuL, /* PL0 = +9.770074992945060684926e-17 */
            0x3FEFF45F6D36133AuL, /* PH0 = +9.985806592017987259879e-01 */
            0x3F673CEC093032DEuL, /* P1  = +2.836667068100913999228e-03 */
            0xBF67347A7CD844D5uL, /* P2  = -2.832640870800243808078e-03 */
            0x3F5EDA25530355DBuL, /* P3  = +1.883064698679040793627e-03 */
            0xBF4EAD3BBABC1BA9uL, /* P4  = -9.361783645268534848806e-04 */
            0x3F3842E61CD35432uL, /* P5  = +3.701984213198588740338e-04 */
            0xBF1F9AB7FD1A3DDDuL, /* P6  = -1.205611036090218544867e-04 */
            0x3F0136C154EA3DEDuL, /* P7  = +3.283288480304320224929e-05 */
            0xBEDF12807F721E66uL, /* P8  = -7.408207230892235753013e-06 */
            0x3EB5B53687AD5112uL, /* P9  = +1.293889481520047941659e-06 */
            0xBE801E90FBFED147uL, /* P10 = -1.200988872775447204019e-07 */
            /* Range reduction coefficients */
            0xC00D000000000000uL, /* B = -3.625    */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 40, 3.75000]| < 4.00000 */
            /* Polynomial coefficients */
            0x3C9E323294294877uL, /* PL0 = +1.047637125334028950603e-16 */
            0x3FEFF8F21CDAAA62uL, /* PH0 = +9.991388858373506653976e-01 */
            0x3F5C3470628813F2uL, /* P1  = +1.721486807697344658108e-03 */
            0xBF5C2E38AC6FF8D2uL, /* P2  = -1.720004411026422324849e-03 */
            0x3F52C13234626F43uL, /* P3  = +1.144694354969070234454e-03 */
            0xBF42B0A47DF47BB4uL, /* P4  = -5.703738387728891173354e-04 */
            0x3F2DB2889E32FBFDuL, /* P5  = +2.265731592156760387344e-04 */
            0xBF1385FBD54C5A55uL, /* P6  = -7.447576110695385196414e-05 */
            0x3EF5AFA812C6984EuL, /* P7  = +2.068153223579892541184e-05 */
            0xBED47097C188A03CuL, /* P8  = -4.873231795467276043290e-06 */
            0x3EAFF2B982F7EE8CuL, /* P9  = +9.521288628073486288914e-07 */
            0xBE828EC5B57D424DuL, /* P10 = -1.382656715739529384702e-07 */
            /* Range reduction coefficients */
            0xC00F000000000000uL, /* B = -3.875    */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 41, 4.00000]| < 4.50000 */
            /* Polynomial coefficients */
            0x3C9BA40DA6983BECuL, /* PL0 = +9.589840482158163453169e-17 */
            0x3FEFFCAAC3F20E65uL, /* PH0 = +9.995931460438894911036e-01 */
            0x3F4AA87CF664754CuL, /* P1  = +8.135423820793490331956e-04 */
            0xBF4AA5B62919E224uL, /* P2  = -8.132113891426467676310e-04 */
            0x3F41C01B53B0B312uL, /* P3  = +5.416997368051531710388e-04 */
            0xBF31B8B54D091751uL, /* P4  = -2.704088811110632606347e-04 */
            0x3F1C431305954ECCuL, /* P5  = +1.078110084525254933728e-04 */
            0xBF02B7DEAD0D44E6uL, /* P6  = -3.570221236393906131126e-05 */
            0x3EE51C6EFF109EA9uL, /* P7  = +1.006654199116272154479e-05 */
            0xBEC48CFB08072D17uL, /* P8  = -2.449834994621594976610e-06 */
            0x3EA1585EC59CAE34uL, /* P9  = +5.169271261920604503617e-07 */
            0xBE78832BAF950BA9uL, /* P10 = -9.131575131209528255629e-08 */
            /* Range reduction coefficients */
            0xC011000000000000uL, /* B = -4.25     */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 42, 4.50000]| < 5.00000 */
            /* Polynomial coefficients */
            0x3C8FBF237F4AFE10uL, /* PL0 = +5.507163370275307643966e-17 */
            0x3FEFFEC61279A3A4uL, /* PH0 = +9.998503075449787225182e-01 */
            0x3F339E78281A00EAuL, /* P1  = +2.993625022114214863645e-04 */
            0xBF339DB7B072AD62uL, /* P2  = -2.993176899035080028902e-04 */
            0x3F2A259E658EF4E4uL, /* P3  = +1.994853835451177669594e-04 */
            0xBF1A219C312B10BAuL, /* P4  = -9.968295880030927192162e-05 */
            0x3F04E146B4F5F4B7uL, /* P5  = +3.982541113154699160876e-05 */
            0xBEEBC5F137088210uL, /* P6  = -1.324329943580649487333e-05 */
            0x3ECF96736E300B00uL, /* P7  = +3.765547135882256916132e-06 */
            0xBEAF4874840B91EBuL, /* P8  = -9.323068824421825762292e-07 */
            0x3E8B6AB2B5C8FD3FuL, /* P9  = +2.042709991312793245971e-07 */
            0xBE650BCCE62FD2B7uL, /* P10 = -3.920140725219944650830e-08 */
            /* Range reduction coefficients */
            0xC013000000000000uL, /* B = -4.75     */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 43, 5.00000]| < 5.50000 */
            /* Polynomial coefficients */
            0x3C9C869C85471703uL, /* PL0 = +9.896883942603146946483e-17 */
            0x3FEFFF8C81C6DC33uL, /* PH0 = +9.999449286177707341139e-01 */
            0x3F1CDF5A2E4D7C69uL, /* P1  = +1.101397316012206760643e-04 */
            0xBF1CDEF1F9BE63BEuL, /* P2  = -1.101336660539594564027e-04 */
            0x3F133EC10C83AAA0uL, /* P3  = +7.341435696487731017506e-05 */
            0xBF033DAB325FAACBuL, /* P4  = -3.669909192168459445238e-05 */
            0x3EEEC598FA98BAD8uL, /* P5  = +1.467316890843338172161e-05 */
            0xBED47F1A15BA368EuL, /* P6  = -4.886744445221253126882e-06 */
            0x3EB761FBE7D201C1uL, /* P7  = +1.393720509029845064726e-06 */
            0xBE974CD75A43BF6BuL, /* P8  = -3.471994551992448536007e-07 */
            0x3E74B02965BBF8DCuL, /* P9  = +7.706929621914905669946e-08 */
            0xBE504EF4E3892A66uL, /* P10 = -1.518840362012570189110e-08 */
            /* Range reduction coefficients */
            0xC015000000000000uL, /* B = -5.25     */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 44, 5.50000]| < 6.00000 */
            /* Polynomial coefficients */
            0x3C643810400471B0uL, /* PL0 = +8.768592603904887599187e-18 */
            0x3FEFFFD583014825uL, /* PH0 = +9.999797400180382433987e-01 */
            0x3F053E71416C43CAuL, /* P1  = +4.051955345663706869871e-05 */
            0xBF053E550C7C8CC9uL, /* P2  = -4.051873253121394012080e-05 */
            0x3EFC52D0D90D4843uL, /* P3  = +2.701139380018752534477e-05 */
            0xBEEC523A6ADBE142uL, /* P4  = -1.350460237457883558350e-05 */
            0x3ED6A73E22D844B3uL, /* P5  = +5.400965660055565196396e-06 */
            0xBEBE31D10F23ACD0uL, /* P6  = -1.799738182979224868919e-06 */
            0x3EA13E14264DEAB2uL, /* P7  = +5.138663935333241981438e-07 */
            0xBE81385ABB98EDCCuL, /* P8  = -1.282999997786486835638e-07 */
            0x3E5EB9164593E0B6uL, /* P9  = +2.861301981891537161158e-08 */
            0xBE387218CFE7772EuL, /* P10 = -5.691705994073124478195e-09 */
            /* Range reduction coefficients */
            0xC017000000000000uL, /* B = -5.75     */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 45, 6.00000]| < 6.50000 */
            /* Polynomial coefficients */
            0x3C92530433F4C703uL, /* PL0 = +6.357512739163799046861e-17 */
            0x3FEFFFF05E8D3191uL, /* PH0 = +9.999925467214315633058e-01 */
            0x3EEF42DDFA52B575uL, /* P1  = +1.490650158538873335176e-05 */
            0xBEEF42CEB54212AAuL, /* P2  = -1.490639048307961378200e-05 */
            0x3EE4D7201CBCB853uL, /* P3  = +9.937445518550804010127e-06 */
            0xBED4D6F764B66C37uL, /* P4  = -4.968574624976280456686e-06 */
            0x3EC0ABB806EBDE71uL, /* P5  = +1.987311456171617620608e-06 */
            0xBEA6399CF854F876uL, /* P6  = -6.623581475862682369330e-07 */
            0x3E8964B91728D7C9uL, /* P7  = +1.891959403186505598965e-07 */
            0xBE6961A0528444D6uL, /* P8  = -4.727645325404986954168e-08 */
            0x3E46AE3B0814EE00uL, /* P9  = +1.056147192151514779549e-08 */
            0xBE221B8194DACD16uL, /* P10 = -2.107984154277957626641e-09 */
            /* Range reduction coefficients */
            0xC019000000000000uL, /* B = -6.25     */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 46, 6.50000]| < 7.00000 */
            /* Polynomial coefficients */
            0x3C7BB5622CE1A79EuL, /* PL0 = +2.403331811901679167526e-17 */
            0x3FEFFFFA3FF22708uL, /* PH0 = +9.999972580855862602789e-01 */
            0x3ED7003552D53503uL, /* P1  = +5.483821309338170039906e-06 */
            0xBED7003130C1AB92uL, /* P2  = -5.483806273169366545037e-06 */
            0x3ECEAAE13B699C45uL, /* P3  = +3.655850800133043324271e-06 */
            0xBEBEAACB305F3D07uL, /* P4  = -1.827905351959291114416e-06 */
            0x3EA8887F5F9C87EFuL, /* P5  = +7.311461438267648556646e-07 */
            0xBE905AD08DF8454FuL, /* P6  = -2.437046884027860662692e-07 */
            0x3E72B068300B703FuL, /* P7  = +6.962228483613086736676e-08 */
            0xBE52AF921A71C058uL, /* P8  = -1.740252888706390465423e-08 */
            0x3E30B53EAA35300DuL, /* P9  = +3.890131469838137725119e-09 */
            0xBE0AB60CDAD7E22EuL, /* P10 = -7.773963050435300060566e-10 */
            /* Range reduction coefficients */
            0xC01B000000000000uL, /* B = -6.75     */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 47, 7.00000]| < 7.50000 */
            /* Polynomial coefficients */
            0x3C8BD1ACF80D7256uL, /* PL0 = +4.825835138930451121169e-17 */
            0x3FEFFFFDE2760A41uL, /* PH0 = +9.999989913051835488389e-01 */
            0x3EC0EC4F1EC27E55uL, /* P1  = +2.017388615341105998718e-06 */
            0xBEC0EC4E005E6EACuL, /* P2  = -2.017386580411626200507e-06 */
            0x3EB6906504BC4610uL, /* P3  = +1.344921673533307001969e-06 */
            0xBEA6905F0D52C8B5uL, /* P4  = -6.724581235377781360384e-07 */
            0x3E920D0F5CCE152BuL, /* P5  = +2.689810941136721216499e-07 */
            0xBE7811505B10E753uL, /* P6  = -8.965891741619763761543e-08 */
            0x3E5B811EE4F9B8EEuL, /* P7  = +2.561544781706659619288e-08 */
            0xBE3B80ABC067E840uL, /* P8  = -6.403452884688571158579e-09 */
            0x3E1898E394E09335uL, /* P9  = +1.431746793613569087489e-09 */
            0xBDF3ABB5BA711DB7uL, /* P10 = -2.862469657501951918569e-10 */
            /* Range reduction coefficients */
            0xC01D000000000000uL, /* B = -7.25     */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 48, 7.50000]| < 8.00000 */
            /* Polynomial coefficients */
            0x3C8AE01DB39A3791uL, /* PL0 = +4.662147961093911873193e-17 */
            0x3FEFFFFF38C76668uL, /* PH0 = +9.999996289217962797125e-01 */
            0x3EA8E712E56E1188uL, /* P1  = +7.421562696484951529573e-07 */
            0xBEA8E7124A650791uL, /* P2  = -7.421559942504648535596e-07 */
            0x3EA09A0B62D8EF94uL, /* P3  = +4.947702955735978541097e-07 */
            0xBE909A09C56C2107uL, /* P4  = -2.473847805916120382218e-07 */
            0x3E7A900A90A54A6EuL, /* P5  = +9.895362410487317236618e-08 */
            0xBE61B5557BB449B6uL, /* P6  = -3.298434544432568302770e-08 */
            0x3E443CC74732CDCAuL, /* P7  = +9.423781066565733462466e-09 */
            0xBE243CA8AA8D6E54uL, /* P8  = -2.355890888986360997159e-09 */
            0x3E0219C341E0D1B4uL, /* P9  = +5.267978308406275552691e-10 */
            0xBDDCF49A10950F13uL, /* P10 = -1.053394074620716018815e-10 */
            /* Range reduction coefficients */
            0xC01F000000000000uL, /* B = -7.75     */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 49, 8.00000]| < 9.00000 */
            /* Polynomial coefficients */
            0x3C75CB18F3775414uL, /* PL0 = +1.890271747518592444083e-17 */
            0x3FEFFFFFD38C39F0uL, /* PH0 = +9.999999172012490333827e-01 */
            0x3E8639E2F89493BBuL, /* P1  = +1.655974950855472979393e-07 */
            0xBE8639E2D9B29562uL, /* P2  = -1.655974813708346974914e-07 */
            0x3E7DA2836A1F706EuL, /* P3  = +1.103982989742589616541e-07 */
            0xBE6DA282C6733DAEuL, /* P4  = -5.519913131581509871840e-08 */
            0x3E57B53A278851FDuL, /* P5  = +2.207971980430773309147e-08 */
            0xBE3F9C4A72536E22uL, /* P6  = -7.359895614149337484810e-09 */
            0x3E220E81FBE19CDDuL, /* P7  = +2.102073153607135257714e-09 */
            0xBE020E8875ADA8D8uL, /* P8  = -5.255211642212584097407e-10 */
            0x3DE07634328384FCuL, /* P9  = +1.197748786062966341989e-10 */
            0xBDBA54078E3C351FuL, /* P10 = -2.394539505021488953905e-11 */
            /* Range reduction coefficients */
            0xC021000000000000uL, /* B = -8.5      */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index = <= |a[i 50, 9.00000]| < 10.00000 */
            /* Polynomial coefficients */
            0x3C98B78738B0EDEFuL, /* PL0 = +8.575399788039081964921e-17 */
            0x3FEFFFFFF9FBEA40uL, /* PH0 = +9.999999887944071019774e-01 */
            0x3E581056FAC28C46uL, /* P1  = +2.241118550516412682327e-08 */
            0xBE581056F63A4351uL, /* P2  = -2.241118525356742542550e-08 */
            0x3E500AE49533790AuL, /* P3  = +1.494078933911655875521e-08 */
            0xBE400AE489ACBA90uL, /* P4  = -7.470394349637968945652e-09 */
            0x3E29AB0D59A1967BuL, /* P5  = +2.988168557255271725494e-09 */
            0xBE111CB32D6EEF2BuL, /* P6  = -9.960558400070350772418e-10 */
            0x3DF38CBADF396908uL, /* P7  = +2.844859618921805216353e-10 */
            0xBDD38CC7B92CECD3uL, /* P8  = -7.112220386749926320915e-11 */
            0x3DB1D2BBE2705032uL, /* P9  = +1.621008722427575444686e-11 */
            0xBD8C8199294E6380uL, /* P10 = -3.240784656869469020111e-12 */
            /* Range reduction coefficients */
            0xC023000000000000uL, /* B = -9.5      */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index =0 <= |a[ 51, 10.0000i]| < 11.00000 */
            /* Polynomial coefficients */
            0x3C8EEEC16618B984uL, /* PL0 = +5.365957423487855307906e-17 */
            0x3FEFFFFFFF2F9279uL, /* PH0 = +9.999999984834878619111e-01 */
            0x3E2A0DB0D052B148uL, /* P1  = +3.033024167396880687734e-09 */
            0xBE2A0DB0CFA6AB71uL, /* P2  = -3.033024162734192808028e-09 */
            0x3E215E75D53A3105uL, /* P3  = +2.022016035353114070618e-09 */
            0xBE115E75D40AA47FuL, /* P4  = -1.011008013562702155050e-09 */
            0x3DFBCA5CDC12ED1CuL, /* P5  = +4.044047007631481841556e-10 */
            0xBDE286E85704FC22uL, /* P6  = -1.348015410318274576187e-10 */
            0x3DC52A8925354517uL, /* P7  = +3.850101197145027796396e-11 */
            0xBDA52A97EA3F5F4AuL, /* P8  = -9.625355478142550638468e-12 */
            0x3D834C011A2AC0F7uL, /* P9  = +2.193802608697321032841e-12 */
            0xBD5EDD05BDCB3A62uL, /* P10 = -4.385948508419928563300e-13 */
            /* Range reduction coefficients */
            0xC025000000000000uL, /* B = -10.5      */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index =0 <= |a[ 52, 11.0000i]| < 12.00000 */
            /* Polynomial coefficients */
            0x3C6BD8B474BBF792uL, /* PL0 = +1.207649585364892639612e-17 */
            0x3FEFFFFFFFE3CAD8uL, /* PH0 = +9.999999997947623953110e-01 */
            0x3DFC3527E43C565FuL, /* P1  = +4.104751852963940338559e-10 */
            0xBDFC3527E420F415uL, /* P2  = -4.104751852036136216697e-10 */
            0x3DF2CE1A8D806DADuL, /* P3  = +2.736501142887952919489e-10 */
            0xBDE2CE1A8DDF690AuL, /* P4  = -1.368250573053032426141e-10 */
            0x3DCE169832D8BD68uL, /* P5  = +5.473022586854025789680e-11 */
            0xBDB40F0FE853DA5BuL, /* P6  = -1.824340550195944358477e-11 */
            0x3D96EA8D930D31A1uL, /* P7  = +5.210545794901128943676e-12 */
            0xBD76EA9DB0D09839uL, /* P8  = -1.302650427355019556441e-12 */
            0x3D54E474FD4303A1uL, /* P9  = +2.968990047962355000258e-13 */
            0xBD30B526CA2B228AuL, /* P10 = -5.935740124899435401321e-14 */
            /* Range reduction coefficients */
            0xC027000000000000uL, /* B = -11.5      */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index =0 <= |a[ 53, 12.0000i]| < 13.00000 */
            /* Polynomial coefficients */
            0x3C56E8953D525FD5uL, /* PL0 = +4.967494994909661698725e-18 */
            0x3FEFFFFFFFFC2EB9uL, /* PH0 = +9.999999999722241073030e-01 */
            0x3DCE8A37A48016C2uL, /* P1  = +5.555177547354687971427e-11 */
            0xBDCE8A37A479B7D4uL, /* P2  = -5.555177547084873157964e-11 */
            0x3DC45C250CFA9C16uL, /* P3  = +3.703451575129414499553e-11 */
            0xBDB45C250D9F8467uL, /* P4  = -1.851725791056759260154e-11 */
            0x3DA049BB33CBD4E9uL, /* P5  = +7.406930640558963265190e-12 */
            0xBD85B7A407C422C1uL, /* P6  = -2.468976464832073512208e-12 */
            0x3D68CF9CED2B3FD5uL, /* P7  = +7.051706989348171774536e-13 */
            0xBD48CFAE64C352B3uL, /* P8  = -1.762945685274427023683e-13 */
            0x3D269EAE08690D52uL, /* P9  = +4.018091287355461204663e-14 */
            0xBD0216CBEAFFF5AAuL, /* P10 = -8.033151495672990022322e-15 */
            /* Range reduction coefficients */
            0xC029000000000000uL, /* B = -12.5      */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index =0 <= |a[ 54, 13.0000i]| < 14.00000 */
            /* Polynomial coefficients */
            0x3C8ACF1392B106D3uL, /* PL0 = +4.650601502940921454330e-17 */
            0x3FEFFFFFFFFF7BBDuL, /* PH0 = +9.999999999962408958609e-01 */
            0x3DA088529889B316uL, /* P1  = +7.518115268189742464885e-12 */
            0xBDA088529887F4C4uL, /* P2  = -7.518115268005149164680e-12 */
            0x3D960B18BF1DF711uL, /* P3  = +5.012076679213679703380e-12 */
            0xBD860B18BFD99A48uL, /* P4  = -2.506038344573564868987e-12 */
            0x3D71A27E7CA64143uL, /* P5  = +1.002419056539285288454e-12 */
            0xBD5783530EA76D91uL, /* P6  = -3.341396294294381580191e-13 */
            0x3D3ADCC75CBD2A03uL, /* P7  = +9.543447641637910477850e-14 */
            0xBD1ADCDA46BE5F17uL, /* P8  = -2.385887543769010971872e-14 */
            0x3CF87D77650BE5B8uL, /* P9  = +5.437895260471143131391e-15 */
            0xBCD395AE6E74C6D2uL, /* P10 = -1.087168847335561258239e-15 */
            /* Range reduction coefficients */
            0xC02B000000000000uL, /* B = -13.5      */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index =0 <= |a[ 55, 14.0000i]| < 15.00000 */
            /* Polynomial coefficients */
            0x3C97A8A295292858uL, /* PL0 = +8.208271151146829171896e-17 */
            0x3FEFFFFFFFFFEE19uL, /* PH0 = +9.999999999994911847878e-01 */
            0x3D71E642BB008F95uL, /* P1  = +1.017466259229268282255e-12 */
            0xBD71E642BAFEEC54uL, /* P2  = -1.017466259207593392022e-12 */
            0x3D67DDAE41647741uL, /* P3  = +6.783108169938233581038e-13 */
            0xBD57DDAE4230F34BuL, /* P4  = -3.391554091734942426856e-13 */
            0x3D4317C33FAE2536uL, /* P5  = +1.356626669455791324801e-13 */
            0xBD2975040D3E26B9uL, /* P6  = -4.522088139411435138867e-14 */
            0x3D0D155DCD0F0AFBuL, /* P7  = +1.291565189902030307333e-14 */
            0xBCED157247832B20uL, /* P8  = -3.228947666403019234175e-15 */
            0x3CCA83D70F607C28uL, /* P9  = +7.359390959466796619024e-16 */
            0xBCA5343952C1E19EuL, /* P10 = -1.471323041436694087188e-16 */
            /* Range reduction coefficients */
            0xC02D000000000000uL, /* B = -14.5      */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index =0 <= |a[ 56, 15.0000i]| < 16.00000 */
            /* Polynomial coefficients */
            0x3C9B7876CBC5306EuL, /* PL0 = +9.530765996816607711732e-17 */
            0x3FEFFFFFFFFFFD93uL, /* PH0 = +9.999999999999310551502e-01 */
            0x3D436121E2640D76uL, /* P1  = +1.376990843765503869546e-13 */
            0xBD436121E26250EAuL, /* P2  = -1.376990843736775811281e-13 */
            0x3D39D6D7CA259186uL, /* P3  = +9.179938654047876451320e-14 */
            0xBD29D6D7CB0327CEuL, /* P4  = -4.589969336188563660531e-14 */
            0x3D14ABE4DC31244AuL, /* P5  = +1.835994545584345768382e-14 */
            0xBCFB8FDB82AB6BB7uL, /* P6  = -6.119980791767901275443e-15 */
            0x3CDF7CF757491B60uL, /* P7  = +1.747943407988343076526e-15 */
            0xBCBF7D0D833640FBuL, /* P8  = -4.369905470133249448357e-16 */
            0x3C9CB512F6BDC754uL, /* P9  = +9.959852600692493655511e-17 */
            0xBC76F50AB1B0E9BAuL, /* P10 = -1.991219205936492089091e-17 */
            /* Range reduction coefficients */
            0xC02F000000000000uL, /* B = -15.5      */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index =0 <= |a[ 57, 16.0000i]| < 18.00000 */
            /* Polynomial coefficients */
            0x3C6FFE15D5F78543uL, /* PL0 = +1.387454417328248962819e-17 */
            0x3FEFFFFFFFFFFFE1uL, /* PH0 = +9.999999999999965583086e-01 */
            0x3CFEE00288B99C26uL, /* P1  = +6.855635762864742358597e-15 */
            0xBCFEE0027D060EE2uL, /* P2  = -6.855635607998342735403e-15 */
            0x3CF4954AA23148A2uL, /* P3  = +4.570381865813341696777e-15 */
            0xBCE4954B5DAD3010uL, /* P4  = -2.285192173571711474199e-15 */
            0x3CD07883DD8793BDuL, /* P5  = +9.143109661358222028007e-16 */
            0xBCB5F5F4BB87ADCFuL, /* P6  = -3.047668447080103869032e-16 */
            0x3C98F1A905097685uL, /* P7  = +8.654183371862458774513e-17 */
            0xBC78F2D585007222uL, /* P8  = -2.163943551222030413627e-17 */
            0x3C58A37CC5082B5FuL, /* P9  = +5.342649626494471588064e-18 */
            0xBC33AE7917F94D17uL, /* P10 = -1.066938163384541013918e-18 */
            /* Range reduction coefficients */
            0xC031000000000000uL, /* B = -17        */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index =0 <= |a[ 58, 18.0000i]| < 19.06155 */
            /* Polynomial coefficients */
            0x3C91BF1D80474F0FuL, /* PL0 = +6.157069264461989135096e-17 */
            0x3FEFFFFFFFFFFFFEuL, /* PH0 = +9.999999999999997779554e-01 */
            0x3CB72071400E6275uL, /* P1  = +3.209478247225075961360e-16 */
            0xBCB72071400A9F37uL, /* P2  = -3.209478247103497434502e-16 */
            0x3CAED5EC39A77629uL, /* P3  = +2.139652050028423711308e-16 */
            0xBC9ED5EC3B530600uL, /* P4  = -1.069826028468029104719e-16 */
            0x3C88AB2BFED159DEuL, /* P5  = +4.279326904335078988705e-17 */
            0xBC70721D1220B3FCuL, /* P6  = -1.426441958074916244382e-17 */
            0x3C52C96049721FB8uL, /* P7  = +4.073700029965821523731e-18 */
            0xBC32C971215735DCuL, /* P8  = -1.018438939975201710113e-18 */
            0x3C112EF658AB41A9uL, /* P9  = +2.328791246104218830028e-19 */
            0xBBEB7B598C6AD3DEuL, /* P10 = -4.655603964908654142787e-20 */
            /* Range reduction coefficients */
            0xC03287E0C98F84E5uL, /* B = -18.530774 */
            0x3FF0000000000000uL, /* A = +1        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            /* index =5 <= |a[ 59, 19.0615i]| < +Inf */
            /* Polynomial coefficients */
            0x0000000000000000uL, /* PL0 = +0.000000000000000000000e-01 */
            0x3FF0000000000000uL, /* PH0 = +1.000000000000000000000e+00 */
            0x0000000000000000uL, /* P1  = +0.000000000000000000000e-01 */
            0x0000000000000000uL, /* P2  = +0.000000000000000000000e-01 */
            0x0000000000000000uL, /* P3  = +0.000000000000000000000e-01 */
            0x0000000000000000uL, /* P4  = +0.000000000000000000000e-01 */
            0x0000000000000000uL, /* P5  = +0.000000000000000000000e-01 */
            0x0000000000000000uL, /* P6  = +0.000000000000000000000e-01 */
            0x0000000000000000uL, /* P7  = +0.000000000000000000000e-01 */
            0x0000000000000000uL, /* P8  = +0.000000000000000000000e-01 */
            0x0000000000000000uL, /* P9  = +0.000000000000000000000e-01 */
            0x0000000000000000uL, /* P10 = +0.000000000000000000000e-01 */
            /* Range reduction coefficients */
            0x0000000000000000uL, /* B = +0        */
            0x0000000000000000uL, /* A = +0        */
            0x0000000000000000uL, /* Align value = +0        */
            0x0000000000000000uL, /* Align value = +0        */
        },
        0xFFFFFFFF00000000uL, /* _dbHighMask       */
        0x8000000000000000uL, /* _dbSignMask       */
        0x7fffffffffffffffuL, /* _dbAbsMask        */
        0x7ffe0000u,          /* _iExpMantMask     */
        0x7fe00000u,          /* _iExpMask         */
        0x3fbe0000u,          /* _iMinIdxOfsMask   */
        0x00760000u,          /* _iMaxIdxMask      */
        // 2^(-512) x^2 shall not underflow
        0x1FF00000u, /* _iMaxTinyArg      */
        // VHEX_BROADCAST( D, 40330FC1931F09CB ), /* SATURATION_THRESHOLD */
}; /*dErf_Table*/
static const _iml_dp_union_t __dtanh_ha__imldTanhTab[2] = {
    0x00000000, 0x3FF00000, /* ONES(0)       =  1.0    */
    0x00000000, 0xBFF00000, /* ONES(1)       = -1.0    */
};
inline int __devicelib_imf_internal_dtanh(const double *a, double *r) {
  int nRet = 0;
  double dSign;
  _iml_uint32_t expnt;
  /* Obtain sign of a[0] in a double precision constant [+,-]1 */
  dSign =
      ((const double *)
           __dtanh_ha__imldTanhTab)[((((_iml_dp_union_t *)&a[0])->bits.sign))];
  if (((((_iml_dp_union_t *)&a[0])->bits.exponent) != 0x7FF)) {
    /* Here if a[0] is finite */
    /* Get exponent of a[0] */
    expnt = (((_iml_dp_union_t *)&a[0])->bits.exponent);
    if (expnt == 0x7fe) {
      /* Huge argument doesn't fit main path */
      r[0] = dSign;
    } else {
      /* Tiny argument, raise inexact, return a[0] */
      r[0] = a[0] * (a[0] + (((const double *)__dtanh_ha__imldTanhTab)[(0)]));
    }
  } else if ((((((_iml_dp_union_t *)&a[0])->bits.hi_significand) == 0) &&
              ((((_iml_dp_union_t *)&a[0])->bits.lo_significand) == 0))) {
    /* Here if a[0] is Inf */
    r[0] = dSign;
  } else {
    /* Path 1) Here if a[0] = [S,Q]NaN */
    r[0] = a[0] + a[0];
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_tanh_d_ha */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_tanh(double x) {
  using namespace __imf_impl_tanh_d_ha;
  double r;
  VUINT32 vm;
  double va1;
  double vr1;
  va1 = x;
  {
    double dSignX;
    double dAbsX;
    double p[18];
    double poly;
    double dScaledX;
    double dC; /* p[12] */
    VUINT32 iMaskedIn;
    VUINT32 iSpecIndex;
    VUINT32 iSpecIndexTiny;
    VUINT32 iMaxTinyArg;
    VUINT64 lX;
    VUINT32 iX;
    VUINT32 iZero;
    VUINT32 iMask1;
    VUINT32 iMask2;
    VUINT32 iIndex;
    VUINT64 lIndex;
    double dbSignMask;
    double dbAbsMask;
    VUINT32 iExpMantMask;
    VUINT32 iExpMask;
    VUINT32 iMinIdxOfsMask;
    VUINT32 iMaxIdxMask;
    dbSignMask = as_double(__devicelib_imf_internal_dtanh_data._dbSignMask);
    dbAbsMask = as_double(__devicelib_imf_internal_dtanh_data._dbAbsMask);
    iExpMask = (__devicelib_imf_internal_dtanh_data._iExpMask);
    iExpMantMask = (__devicelib_imf_internal_dtanh_data._iExpMantMask);
    iMinIdxOfsMask = (__devicelib_imf_internal_dtanh_data._iMinIdxOfsMask);
    iMaxIdxMask = (__devicelib_imf_internal_dtanh_data._iMaxIdxMask);
    iZero = 0;
    // Absolute argument
    dAbsX = as_double((as_ulong(va1) & as_ulong(dbAbsMask)));
    // Sign of argument
    dSignX = as_double((as_ulong(va1) & as_ulong(dbSignMask)));
    // Represent argument as 64-bit integer
    lX = as_ulong(va1);
    // Get high 32-bit parts only
    iX = ((VUINT32)((VUINT64)lX >> 32));
    iMaskedIn = (iX & iExpMantMask);
    iSpecIndex =
        ((VUINT32)(-(VSINT32)((VSINT32)iMaskedIn > (VSINT32)iExpMask)));
    vm = 0;
    vm = iSpecIndex;
    iIndex = (iMaskedIn - iMinIdxOfsMask);
    /* Put index between zero and _iMaxIdxMask */
    iMask1 = ((VUINT32)(-(VSINT32)((VSINT32)iIndex > (VSINT32)iZero)));
    iMask2 = ((VUINT32)(-(VSINT32)((VSINT32)iIndex > (VSINT32)iMaxIdxMask)));
    iIndex = (iIndex & iMask1);
    iIndex = (((~(iMask2)) & (iIndex)) | ((iMask2) & (iMaxIdxMask)));
    // Scale index
    iIndex = ((VUINT32)(iIndex) >> ((17 - 4 - 3)));
    // Load P[0], P[1] polynomial coefficients from lookup table
    (p + 0)[0] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           0 * 8))[iIndex >> 3]);
    (p + 0)[1] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           0 * 8))[(iIndex >> 3) + 1]);
    // Load P[2], P[3] polynomial coefficients from lookup table
    (p + 2)[0] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           2 * 8))[iIndex >> 3]);
    (p + 2)[1] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           2 * 8))[(iIndex >> 3) + 1]);
    // Load P[4], P[5] polynomial coefficients from lookup table
    (p + 4)[0] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           4 * 8))[iIndex >> 3]);
    (p + 4)[1] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           4 * 8))[(iIndex >> 3) + 1]);
    // Load P[6], P[7] polynomial coefficients from lookup table
    (p + 6)[0] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           6 * 8))[iIndex >> 3]);
    (p + 6)[1] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           6 * 8))[(iIndex >> 3) + 1]);
    // Load P[8], P[9] polynomial coefficients from lookup table
    (p + 8)[0] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           8 * 8))[iIndex >> 3]);
    (p + 8)[1] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           8 * 8))[(iIndex >> 3) + 1]);
    // Load P[10], P[11] polynomial coefficients from lookup table
    (p + 10)[0] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           10 * 8))[iIndex >> 3]);
    (p + 10)[1] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           10 * 8))[(iIndex >> 3) + 1]);
    // Load P[12], P[13] polynomial coefficients from lookup table
    (p + 12)[0] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           12 * 8))[iIndex >> 3]);
    (p + 12)[1] = as_double(
        ((const VUINT64 *)((const char *)(&__devicelib_imf_internal_dtanh_data
                                               ._dbP[0]) +
                           12 * 8))[(iIndex >> 3) + 1]);
    // Compute reduced argument as |x|+ P[12]
    dScaledX = (dAbsX + p[12]);
    // Compute polynomial
    poly = __fma(p[11], dScaledX, p[10]);
    poly = __fma(poly, dScaledX, p[9]);
    poly = __fma(poly, dScaledX, p[8]);
    poly = __fma(poly, dScaledX, p[7]);
    poly = __fma(poly, dScaledX, p[6]);
    poly = __fma(poly, dScaledX, p[5]);
    poly = __fma(poly, dScaledX, p[4]);
    poly = __fma(poly, dScaledX, p[3]);
    {
      // hi-lo with p1, p0_hi, p0_lo
      // hint: |p0| > z*p1
      double dbHighMask;
      double p2H;
      double p2L;
      double dxH;
      double dxL;
      double pH;
      double pL;
      double pHl;
      // Second order term + p[0]
      poly = (poly * dScaledX);
      poly = __fma(poly, dScaledX, p[0]);
      // First order term. No high and low splitting when FMA is available
      pH = (p[2] * dScaledX);
      pL = __fma(p[2], dScaledX, -(pH));
      pL = (pL + poly);
      {
        double __VADD2_v1;
        double __VADD2_v2;
        double __VADD2_v3;
        ;
        __VADD2_v1 = (pH + p[1]);
        __VADD2_v2 = (pH - __VADD2_v1);
        __VADD2_v3 = (__VADD2_v1 + __VADD2_v2);
        __VADD2_v2 = (p[1] + __VADD2_v2);
        __VADD2_v3 = (pH - __VADD2_v3);
        __VADD2_v3 = (__VADD2_v2 + __VADD2_v3);
        ;
        pH = __VADD2_v1;
        pHl = __VADD2_v3;
      };
      ;
      pL = (pL + pHl);
      poly = (pH + pL);
    }
    // Set final result sign
    vr1 = as_double((as_ulong(poly) | as_ulong(dSignX)));
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_dtanh(&__cout_a1, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
