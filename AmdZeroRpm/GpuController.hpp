#pragma once

#include "AdlFunctions.hpp"

class GpuController {
public:
  GpuController();
  ~GpuController();

  bool ToggleZeroRpm(bool enabled) noexcept;

private:
  ADL_CONTEXT_HANDLE mContext;
};
