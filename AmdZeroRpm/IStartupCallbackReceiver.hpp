#include <minwindef.h>

struct IStartupCallbackReceiver {
  virtual void ApplicationStarted(DWORD pid) noexcept = 0;
};
