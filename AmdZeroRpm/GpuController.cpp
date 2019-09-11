#include "GpuController.hpp"
#include <stdexcept>

GpuController::GpuController() {
  if (ADL2_Main_Control_Create(AdlMemoryAllocator, 1, &mContext) != ADL_OK) {
    throw std::runtime_error("could not initialize ADL");
  }
}

GpuController::~GpuController() {
  if (mContext != nullptr) {
    ADL2_Main_Control_Destroy(mContext);
  }
}

bool GpuController::ToggleZeroRpm(const bool enabled) const noexcept {
  int index = 0;
  if (ADL2_Adapter_Primary_Get(mContext, &index) != ADL_OK) {
    return false;
  }
  int support = 0, current = 0, def = 0;
  const auto status =
      ADL2_OverdriveN_ZeroRPMFan_Get(mContext, index, &support, &current, &def);
  if (status != ADL_OK || !support) {
    return false;
  }
  const auto newValue = static_cast<int>(enabled);
  return current == newValue ||
         ADL2_OverdriveN_ZeroRPMFan_Set(mContext, index, newValue) == ADL_OK;
}
