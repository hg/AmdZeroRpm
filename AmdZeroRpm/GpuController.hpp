#pragma once

#include "AdlFunctions.hpp"

class GpuController {
public:
  GpuController();
  ~GpuController();
  GpuController(const GpuController &) = delete;

  bool ToggleZeroRpm(bool enabled) const noexcept;

private:
  ADL_CONTEXT_HANDLE mContext{nullptr};
};
