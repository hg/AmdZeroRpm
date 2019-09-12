#pragma once

#include "Adapter.hpp"
#include <memory>
#include <optional>

class GpuController {
public:
  GpuController();

  [[nodiscard]] std::optional<Adapter> GetPrimaryAdapter() const noexcept;

private:
  const AdlContext mContext;
};
