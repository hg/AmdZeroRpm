#include "IStateChangeCallbackReceiver.hpp"
#include <Windows.h>
#include <functional>
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
  bool ScanRunning() noexcept;
  MonitorState CurrentState() const noexcept;

  [[noreturn]] void MonitorLoop(IStateChangeCallbackReceiver &onStateChanged);

private:
  mutable std::mutex mProcessesLock;
  std::vector<DWORD> mProcessPids;
  std::vector<std::wstring> mMonitoredPaths;
  HANDLE mNewProcessEvent;

  bool IsMonitoredPath(const std::wstring &path);
};
