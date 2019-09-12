#include "ProcessMonitor.hpp"
#include <algorithm>
#include <array>
#include <psapi.h>

using MutexGuard = std::lock_guard<std::mutex>;

namespace {

std::wstring GetProcessExePath(const DWORD pid) noexcept {
  wchar_t buffer[MAX_PATH + 1];

  const auto process =
      OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
  if (!process) {
    return std::wstring{};
  }

  const auto valid = GetModuleFileNameExW(process, nullptr, buffer, MAX_PATH);
  CloseHandle(process);

  return valid ? buffer : std::wstring{};
}

} // namespace

void ProcessMonitor::AddMonitoredPath(const std::wstring &path) noexcept {
  mMonitoredPaths.push_back(path);
}

void ProcessMonitor::AddMonitoredPath(std::wstring &&path) noexcept {
  mMonitoredPaths.push_back(path);
}

bool ProcessMonitor::AddProcess(const DWORD pid) noexcept {
  const auto path = GetProcessExePath(pid);

  if (IsMonitoredPath(path)) {
    const auto process = OpenProcess(SYNCHRONIZE, false, pid);
    if (process) {
      MutexGuard guard{mProcessesLock};
      mNewProcessQueue.push_back(process);
      mProcessCreated.notify_one();
      return true;
    }
  }

  return false;
}

void ProcessMonitor::DrainNewProcessQueue(
    std::vector<HANDLE> &handles) noexcept {
  const auto count = std::min<size_t>(kMaxProcesses, mNewProcessQueue.size());
  std::copy_n(mNewProcessQueue.cbegin(), count, std::back_inserter(handles));
  mNewProcessQueue.erase(mNewProcessQueue.cbegin(),
                         mNewProcessQueue.cbegin() + count);
}

bool ProcessMonitor::ScanRunning() noexcept {
  DWORD pids[3072];
  DWORD bytesReturned = 0;

  if (EnumProcesses(pids, sizeof(pids), &bytesReturned)) {
    MutexGuard guard{mProcessesLock};
    mNewProcessQueue.clear();
    for (DWORD i = 0; i < bytesReturned / sizeof(DWORD); ++i) {
      const auto pid = pids[i];
      const auto path = GetProcessExePath(pid);
      if (IsMonitoredPath(path)) {
        const auto process = OpenProcess(SYNCHRONIZE, false, pid);
        if (process) {
          mNewProcessQueue.push_back(process);
        }
      }
    }
    if (!mNewProcessQueue.empty()) {
      mProcessCreated.notify_one();
    }
    return true;
  }

  return false;
}

void ProcessMonitor::MonitorLoop(IStateChangeCallbackReceiver &eventHandler) {
  std::vector<HANDLE> handles;
  handles.reserve(kMaxProcesses);

  ScanRunning();

  while (true) {
    if (handles.empty()) {
      eventHandler.ProcessStateChanged(MonitorState::NoProcesses);
      std::unique_lock<std::mutex> lock{mProcessesLock};
      mProcessCreated.wait(lock);
      DrainNewProcessQueue(handles);
      lock.unlock();
    } else {
      eventHandler.ProcessStateChanged(MonitorState::Active);
      if (WaitForMultipleObjects(static_cast<DWORD>(handles.size()),
                                 &handles[0], true,
                                 INFINITE) != WAIT_OBJECT_0) {
        throw std::runtime_error{"WaitForMultipleObjects() failed"};
      }
      std::for_each(handles.cbegin(), handles.cend(), CloseHandle);
      handles.clear();
      MutexGuard guard{mProcessesLock};
      DrainNewProcessQueue(handles);
    }
  }
}

bool ProcessMonitor::IsMonitoredPath(const std::wstring &path) noexcept {
  if (path.empty()) {
    return false;
  }
  for (const auto &monitoredPath : mMonitoredPaths) {
    const auto count = std::min<size_t>(monitoredPath.length(), path.length());
    if (_wcsnicmp(monitoredPath.c_str(), path.c_str(), count) == 0) {
      return true;
    }
  }
  return false;
}
