

enum func_proto
{
	PROTO_OTHER = 0,	/** Unsupported (generic) **/
	PROTO_F_F1,			/** F_gentype func(F_gentype) **/
	PROTO_F_F2,			/** F_gentype func(F_gentype,F_gentype) **/
	PROTO_F_F3,			/** F_gentype func(F_gentype,F_gentype,F_gentype) **/
	PROTO_I_I1,			/** I_gentype func(I_gentype) **/
	PROTO_I_I2,			/** I_gentype func(I_gentype,I_gentype) **/
	PROTO_I_I3,			/** I_gentype func(I_gentype,I_gentype,I_gentype) **/
	PROTO_FastInt_2,	/** Int/Uint{1,2,4,8,16} func(Int/Uint{1,2,4,8,16},Int/Uint{1,2,4,8,16}) **/
	PROTO_FastInt_3,	/** Int/Uint{1,2,4,8,16} func(Int/Uint{1,2,4,8,16},Int/Uint{1,2,4,8,16},Int/Uint{1,2,4,8,16}) **/
	PROTO_F_F2_or_Ff,	/** F_gentype func(F_gentype,F_gentype/float) **/
	PROTO_F_F3_or_FFf,	/** F_gentype func(F_gentype,F_gentype,F_gentype/float) **/
	PROTO_I_F,			/** I_gentype func(F_gentype) **/
	PROTO_F_FI_or_Fi,	/** F_gentype func(F_gentype, I_gentype/Int) **/
	PROTO_F_FI,			/** F_gentype func(F_gentype, Int{1,2,4,8,16}) **/
	PROTO_UI_I,			/** UI_gentype func(I_gentype) **/
	PROTO_UI_II,		/** UI_gentype func(I_gentype,I_gentype) **/
	PROTO_F_F3_or_Fff,	/** F_gentype func(F_gentype, F_gentype/Float, F_gentype/Float) **/
	PROTO_ALL_or_Ff,	/** F_gentype/I_gentype func(F_gentype/I_gentype, F_gentype/Float/I_gentype) **/
	PROTO_A_AAU_or_AAI,	/** F_gentype/I_gentype func(F_gentype/I_gentype, F_gentype/I_gentype, U_gentype/I_gentype) **/	
	PROTO_F_F3_or_ffF,	/** F_gentype func(F_gentype/Float,F_gentype/Float,F_gentype) **/
	PROTO_F_F2_or_fF,	/** F_gentype func(F_gentype/Float,F_gentype) **/
	PROTO_Iup_IUI		/** I_gentype+1	func(I_gentype,UI_gentype) **/
};


/* Function generator: ( func name, prototype model, supports vector-of-3 ) */

#define GEN_ALL_FUNCS(ACTION)			\
/* MATH Functions: */					\
ACTION(acos,PROTO_F_F1,1)				\
ACTION(acosh,PROTO_F_F1,1)				\
ACTION(acospi,PROTO_F_F1,1)				\
ACTION(asin,PROTO_F_F1,1)				\
ACTION(asinh,PROTO_F_F1,1)				\
ACTION(asinpi,PROTO_F_F1,1)				\
ACTION(atan,PROTO_F_F1,1)				\
ACTION(atan2,PROTO_F_F2,1)				\
ACTION(atan2pi,PROTO_F_F2,1)			\
ACTION(atanh,PROTO_F_F1,1)				\
ACTION(atanpi,PROTO_F_F1,1)				\
ACTION(cbrt,PROTO_F_F1,1)				\
ACTION(ceil,PROTO_F_F1,1)				\
ACTION(copysign,PROTO_F_F2,1)			\
ACTION(cos,PROTO_F_F1,1)				\
ACTION(cosh,PROTO_F_F1,1)				\
ACTION(cospi,PROTO_F_F1,1)				\
ACTION(erfc,PROTO_F_F1,1)				\
ACTION(erf,PROTO_F_F1,1)				\
ACTION(exp,PROTO_F_F1,1)				\
ACTION(exp2,PROTO_F_F1,1)				\
ACTION(exp10,PROTO_F_F1,1)				\
ACTION(expm1,PROTO_F_F1,1)				\
ACTION(fabs,PROTO_F_F1,1)				\
ACTION(fdim,PROTO_F_F2,1)				\
ACTION(floor,PROTO_F_F1,1)				\
ACTION(fma,PROTO_F_F3,1)				\
ACTION(fmax,PROTO_F_F2_or_Ff,1)			\
ACTION(fmin,PROTO_F_F2_or_Ff,1)			\
ACTION(fmod,PROTO_F_F2,1)				\
ACTION(hypot,PROTO_F_F2,1)				\
ACTION(ilogb,PROTO_I_F,1)				\
ACTION(ldexp,PROTO_F_FI,1)				\
ACTION(lgamma,PROTO_F_F1,1)				\
ACTION(log,PROTO_F_F1,1)				\
ACTION(log2,PROTO_F_F1,1)				\
ACTION(log10,PROTO_F_F1,1)				\
ACTION(log1p,PROTO_F_F1,1)				\
ACTION(logb,PROTO_F_F1,1)				\
ACTION(mad,PROTO_F_F3,1)				\
ACTION(nextafter,PROTO_F_F2,1)			\
ACTION(pow,PROTO_F_F2,1)				\
ACTION(pown,PROTO_F_FI,1)				\
ACTION(powr,PROTO_F_F2,1)				\
ACTION(remainder,PROTO_F_F2,1)			\
ACTION(rint,PROTO_F_F1,1)				\
ACTION(rootn,PROTO_F_FI,1)				\
ACTION(round,PROTO_F_F1,1)				\
ACTION(rsqrt,PROTO_F_F1,1)				\
ACTION(sin,PROTO_F_F1,1)				\
ACTION(sinh,PROTO_F_F1,1)				\
ACTION(sinpi,PROTO_F_F1,1)				\
ACTION(sqrt,PROTO_F_F1,1)				\
ACTION(tan,PROTO_F_F1,1)				\
ACTION(tanh,PROTO_F_F1,1)				\
ACTION(tanpi,PROTO_F_F1,1)				\
ACTION(tgamma,PROTO_F_F1,1)				\
ACTION(trunc,PROTO_F_F1,1)				\
/* Half and Native math funcs */		\
ACTION(half_cos,PROTO_F_F1,1)			\
ACTION(half_divide,PROTO_F_F2,1)		\
ACTION(half_exp,PROTO_F_F1,1)			\
ACTION(half_exp2,PROTO_F_F1,1)			\
ACTION(half_exp10,PROTO_F_F1,1)			\
ACTION(half_log,PROTO_F_F1,1)			\
ACTION(half_log2,PROTO_F_F1,1)			\
ACTION(half_log10,PROTO_F_F1,1)			\
ACTION(half_powr,PROTO_F_F2,1)			\
ACTION(half_recip,PROTO_F_F1,1)			\
ACTION(half_rsqrt,PROTO_F_F1,1)			\
ACTION(half_sin,PROTO_F_F1,1)			\
ACTION(half_sqrt,PROTO_F_F1,1)			\
ACTION(half_tan,PROTO_F_F1,1)			\
ACTION(native_cos,PROTO_F_F1,1)			\
ACTION(native_divide,PROTO_F_F2,1)		\
ACTION(native_exp,PROTO_F_F1,1)			\
ACTION(native_exp2,PROTO_F_F1,1)		\
ACTION(native_exp10,PROTO_F_F1,1)		\
ACTION(native_log,PROTO_F_F1,1)			\
ACTION(native_log2,PROTO_F_F1,1)		\
ACTION(native_log10,PROTO_F_F1,1)		\
ACTION(native_powr,PROTO_F_F2,1)		\
ACTION(native_recip,PROTO_F_F1,1)		\
ACTION(native_rsqrt,PROTO_F_F1,1)		\
ACTION(native_sin,PROTO_F_F1,1)			\
ACTION(native_sqrt,PROTO_F_F1,1)		\
ACTION(native_tan,PROTO_F_F1,1)			\
/* Integer functions */					\
ACTION(abs,PROTO_UI_I,1)				\
ACTION(abs_diff,PROTO_UI_II,1)			\
ACTION(add_sat,PROTO_I_I2,1)			\
ACTION(clz,PROTO_I_I1,1)				\
ACTION(hadd,PROTO_I_I2,1)				\
ACTION(rhadd,PROTO_I_I2,1)				\
ACTION(mad_hi,PROTO_I_I3,1)				\
ACTION(mad_sat,PROTO_I_I3,1)			\
ACTION(mul_hi,PROTO_I_I2,1)				\
ACTION(rotate,PROTO_I_I2,1)				\
ACTION(sub_sat,PROTO_I_I2,1)			\
ACTION(mad24,PROTO_FastInt_3,1)			\
ACTION(mul24,PROTO_FastInt_2,1)			\
/* Common functions */					\
ACTION(clamp,PROTO_F_F3_or_Fff,1)		\
ACTION(degrees,PROTO_F_F1,1)			\
ACTION(max,PROTO_ALL_or_Ff,1)			\
ACTION(min,PROTO_ALL_or_Ff,1)			\
ACTION(mix,PROTO_F_F3_or_FFf,1)			\
ACTION(radians,PROTO_F_F1,1)			\
ACTION(sign,PROTO_F_F1,1)				\
ACTION(smoothstep,PROTO_F_F3_or_ffF,1)	\
ACTION(step,PROTO_F_F2_or_fF,1)			\
ACTION(upsample,PROTO_Iup_IUI,1)
