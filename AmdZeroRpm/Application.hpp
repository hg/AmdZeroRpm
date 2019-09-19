#pragma once

#include "GpuController.hpp"
#include "IStateChangeCallbackReceiver.hpp"
#include "ProcessMonitor.hpp"
#include "WmiEventListener.hpp"
#include <memory>

constexpr auto kApplicationName = L"AmdZeroRpm";

class Application final : public IStartupCallbackReceiver,
                          public IStateChangeCallbackReceiver {
public:
  Application();

  [[noreturn]] void Start();

  void ApplicationStarted(DWORD pid) noexcept override;
  void ProcessStateChanged(MonitorState state) noexcept override;

private:
  GpuController mGpuController;
  ProcessMonitor mMonitor;
  std::unique_ptr<WmiEventListener> mConnection;
};
