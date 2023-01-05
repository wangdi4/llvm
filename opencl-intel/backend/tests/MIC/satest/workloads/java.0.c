typedef struct WeightedValue {
  float duration;
  uint leftIdx;
  float leftWeight;
} WeightedValue;
typedef struct FraParams {
  float tradeDate;
  float maturityDate;
  float contractRate;
  float contractDuration;
  float notional;
  float contractDuration_mul_notional;
  float fixingRate;
} FraParams;
typedef struct FraStepParams {
  float stepDate;
  uint fixingStepIndex;
  WeightedValue discountRate;
  WeightedValue fixingRate1;
  WeightedValue fixingRate2;
} FraStepParams;
float getWeightedValue(const WeightedValue w, const global float *values,
                       const uint offset) {
  uint i = w.leftIdx + offset;
  // Try to remove conditional (we don't really need it here!)
  // if (w.leftWeight == 1.0f) {
  //  return values[i];
  // }
  return values[i] * w.leftWeight + values[i + 1] * (1.0f - w.leftWeight);
}
float getWeightedFwdRate(const WeightedValue from, const WeightedValue to,
                         global float *rates, const uint rateOffset) {
  float fromRate = getWeightedValue(from, rates, rateOffset);
  // do we really need this check here?
  /// if (from.duration == to.duration)
  ///  return fromRate;
  float toRate = getWeightedValue(to, rates, rateOffset);
  // (to.duration - from.duration) is a constant (throughout the NDRange). If we
  // pass it as constant, then the compiler may optimize on special cases (e.g.
  // divide by one). Another option to optimize it is by calculating
  // rec_duration_diff = 1/(to.duration - from.duration) on the host (once per
  // NDRange) and change this line to: return (toRate * to.duration - fromRate *
  // from.duration) * rec_duration_diff;
  /// return (toRate * to.duration - fromRate * from.duration) / (to.duration -
  /// from.duration);
  // Let's try hardcoded value.  to.duration - from.duration = 0.02f - 0.01f =
  // 0.1f fixed on the host ; 1/(to.duration - from.duration) = 100.0f
  return (toRate * to.duration - fromRate * from.duration) * (100.0f);
  // accurate alternative with select
  /// return select((getWeightedValue(to, rates, rateOffset)*to.duration -
  /// fromRate * from.duration) / (to.duration - from.duration), fromRate,
  /// (from.duration == to.duration));
}
kernel void fraKernel(
    global float *restrict result, global float *restrict dcRates,
    uint scenarioCount, uint tenorCount,
    global FraStepParams *restrict stepParamsPtr,
    global FraParams *restrict fraParams, global float *restrict fxRates,
    uint fxRatesOffset, float res,
    uint stepDates) { //(Dima: try 1D NDRange - stepDates the new argument)
  // Array offsets
  const uint stepIdx =
      get_group_id(1); //(Dima: try 1D NDRange - replace with loop)
  /// prefetch((const __global float*)&stepParamsPtr[stepIdx+1],11);
  /// prefetch((const __global float*)fraParams,7);
  /// for(uint stepIdx = 0; stepIdx < stepDates; stepIdx++) //(Dima: try 1D
  /// NDRange - without loop; global size increased multiplied by: stepDates)
  {
    uint scenarioOffset;
    uint irScenarioOffset;
    {
      // Group index (Why we do it here? It's equivalent to get_global_id)
      /// const uint scenarioBlockSize = get_local_size(0);
      /// const uint scenarioBlockIdx = get_group_id(0);
      // Thread index
      const uint scenarioIdx =
          get_global_id(0); /// get_global_id(0) = scenarioBlockIdx *
                            /// scenarioBlockSize + get_local_id(0);
      /// const uint scenarioIdx = scenarioBlockIdx * scenarioBlockSize +
      /// get_local_id(0);
      scenarioOffset = scenarioIdx + stepIdx * scenarioCount;
      /// prefetch(&result[scenarioOffset],1);
      /// prefetch(&fxRates[fxRatesOffset + scenarioOffset],1);
      /// scenarioOffset = scenarioIdx;  ///Dima: try 1D NDRange - without loop;
      irScenarioOffset = scenarioOffset * tenorCount;
    }
    /// uint stepIdx = scenarioOffset/scenarioCount; //Dima: try 1D NDRange -
    /// without loop;
    // Pricing parameters for this step date
    FraStepParams stepParams = stepParamsPtr[stepIdx];
    /// prefetch((const __global float*)&stepParamsPtr[stepIdx+1],11);
    // Actual pricing
    float contractRate = fraParams->contractRate;
    float fixingRate = fraParams->fixingRate;
    /// if (fixingRate <= 0.0f) //do we really need this check? Try to avoid
    /// conditionals (it's always 0.0f)
    {
      const uint fixingIrOffset =
          irScenarioOffset -
          (stepIdx - stepParams.fixingStepIndex) * scenarioCount * tenorCount;
      fixingRate =
          getWeightedFwdRate(stepParams.fixingRate1, stepParams.fixingRate2,
                             dcRates, fixingIrOffset);
    }
    // fraParams->contractDuration * (fraParams->notional) is loop invariant.
    // We need to change the order of calculation to help compiler understand
    // it:
    // Original
    /// float payoff = fraParams->contractDuration * (fixingRate - contractRate)
    /// * (fraParams->notional);
    // Change the order
    /// float payoff = fraParams->contractDuration * (fraParams->notional) *
    /// (fixingRate - contractRate);
    // Loop invariant precalculated on the host
    float payoff =
        fraParams->contractDuration_mul_notional * (fixingRate - contractRate);
    float dcRate =
        getWeightedValue(stepParams.discountRate, dcRates, irScenarioOffset);
    /// float stepValuation = payoff * exp(-dcRate *
    /// stepParams.discountRate.duration); Try to use fast native_
    /// implementation for math. (or relaxed math program build options )
    float stepValuation =
        payoff * native_exp(-dcRate * stepParams.discountRate.duration);
    float scenarioValue =
        stepValuation * fxRates[fxRatesOffset + scenarioOffset];
    // Add this scenario's value to the result accumulator
    result[scenarioOffset] = scenarioValue;
  }
}
