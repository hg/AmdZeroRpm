#pragma once

#include <memory>

using AdlContext = std::shared_ptr<void>;

enum class ZeroRpmStatus {
  On = 1,
  Off = 0,
  Error = -1,
};

class Adapter {
public:
  Adapter(AdlContext context, const int index);

  [[nodiscard]] ZeroRpmStatus GetZeroRpm() const noexcept;
  [[nodiscard]] ZeroRpmStatus SetZeroRpm(bool enabled) const noexcept;

private:
  const AdlContext mContext;
  const int mIndex;
};
