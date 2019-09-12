#include "Adapter.hpp"
#include "AdlFunctions.hpp"

Adapter::Adapter(AdlContext context, const int index)
    : mContext{std::move(context)}, mIndex{index} {}

ZeroRpmStatus Adapter::GetZeroRpm() const noexcept {
  int support = 0, current = 0, def = 0;
  const auto status = ADL2_OverdriveN_ZeroRPMFan_Get(mContext.get(), mIndex,
                                                     &support, &current, &def);
  if (status != ADL_OK || !support) {
    return ZeroRpmStatus::Error;
  }
  return current ? ZeroRpmStatus::On : ZeroRpmStatus::Off;
}

ZeroRpmStatus Adapter::SetZeroRpm(const bool enabled) const noexcept {
  const auto current = GetZeroRpm();
  if (current == ZeroRpmStatus::Error) {
    return ZeroRpmStatus::Error;
  }
  const auto newStatus = enabled ? ZeroRpmStatus::On : ZeroRpmStatus::Off;
  if (current == newStatus) {
    return current;
  }
  const auto result = ADL2_OverdriveN_ZeroRPMFan_Set(mContext.get(), mIndex,
                                                     static_cast<int>(enabled));
  return result == ADL_OK ? newStatus : ZeroRpmStatus::Error;
}
