#pragma once

#include "IStateChangeCallbackReceiver.hpp"
#include <Windows.h>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

class ProcessMonitor {
public:
  void AddMonitoredPath(const std::wstring &path) noexcept;
  void AddMonitoredPath(std::wstring &&path) noexcept;
  bool AddProcess(DWORD pid) noexcept;

  [[noreturn]] void MonitorLoop(IStateChangeCallbackReceiver &eventHandler);

private:
  const size_t kMaxProcesses = MAXIMUM_WAIT_OBJECTS;

  std::mutex mProcessesLock;
  std::vector<HANDLE> mNewProcessQueue;
  std::vector<std::wstring> mMonitoredPaths;
  std::condition_variable mProcessCreated;

  bool ScanRunning() noexcept;
  void DrainNewProcessQueue(std::vector<HANDLE> &handles) noexcept;
  bool IsMonitoredPath(const std::wstring &path) noexcept;
};
