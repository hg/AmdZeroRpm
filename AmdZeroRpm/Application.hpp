#pragma once

#include "GpuController.hpp"
#include "IStateChangeCallbackReceiver.hpp"
#include "ProcessMonitor.hpp"
#include "WmiEventListener.hpp"
#include <condition_variable>
#include <memory>
#include <thread>

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
  std::thread mGpuWorker;
  std::mutex mZeroRpmMutex;
  bool mZeroRpmStateChanged{false};
  std::condition_variable mZeroRpmChangedCv;
  bool mZeroRpmEnabled;

  [[noreturn]] void UpdateGpuState();
};
