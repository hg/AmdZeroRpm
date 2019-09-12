#pragma once

#include "IStateChangeCallbackReceiver.hpp"
#include <Windows.h>
#include <mutex>
#include <string>
#include <vector>

class ProcessMonitor {
public:
  ProcessMonitor();
  ~ProcessMonitor();

  void AddMonitoredPath(const std::wstring &path) noexcept;
  void AddMonitoredPath(std::wstring &&path) noexcept;
  bool AddProcess(DWORD pid) noexcept;

  [[noreturn]] void MonitorLoop(IStateChangeCallbackReceiver &eventHandler);

private:
  mutable std::mutex mProcessesLock;
  std::vector<HANDLE> mNewProcessQueue;
  std::vector<std::wstring> mMonitoredPaths;
  HANDLE mNewProcessEvent;

  bool ScanRunning() noexcept;
  bool IsMonitoredPath(const std::wstring &path) noexcept;
};
