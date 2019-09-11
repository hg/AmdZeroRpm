#include "ProcessMonitor.hpp"
#include <array>
#include <psapi.h>

using MutexGuard = std::lock_guard<std::mutex>;

namespace {

std::wstring GetProcessExePath(const DWORD pid) {
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

ProcessMonitor::ProcessMonitor() {
  mNewProcessEvent = CreateEventW(nullptr, false, false, nullptr);
  if (!mNewProcessEvent) {
    throw std::runtime_error("could not create thread notification event");
  }
}

ProcessMonitor::~ProcessMonitor() { CloseHandle(mNewProcessEvent); }

void ProcessMonitor::AddMonitoredPath(const std::wstring &path) noexcept {
  mMonitoredPaths.push_back(path);
}

void ProcessMonitor::AddMonitoredPath(std::wstring &&path) noexcept {
  mMonitoredPaths.push_back(path);
}

bool ProcessMonitor::AddProcess(const DWORD pid) noexcept {
  const auto path = GetProcessExePath(pid);

  if (!IsMonitoredPath(path)) {
    return false;
  }

  {
    MutexGuard guard{mProcessesLock};
    mProcessPids.push_back(pid);
  }

  if (!SetEvent(mNewProcessEvent)) {
    MutexGuard guard{mProcessesLock};
    if (mProcessPids.back() == pid) {
      mProcessPids.pop_back();
    }
  }

  return true;
}

bool ProcessMonitor::ScanRunning() noexcept {
  DWORD pids[3072];
  DWORD bytesReturned = 0;

  if (!EnumProcesses(pids, sizeof(pids), &bytesReturned)) {
    return false;
  }

  for (DWORD i = 0; i < bytesReturned / sizeof(DWORD); ++i) {
    AddProcess(pids[i]);
  }

  return true;
}

MonitorState ProcessMonitor::CurrentState() const noexcept {
  MutexGuard guard{mProcessesLock};
  return mProcessPids.size() > 1 ? MonitorState::Active
                                 : MonitorState::NoProcesses;
}

void ProcessMonitor::MonitorLoop(IStateChangeCallbackReceiver &onStateChanged) {
  constexpr auto kEventIndex = 0;
  constexpr auto kMaxHandles = MAXIMUM_WAIT_OBJECTS;

  std::vector<HANDLE> handles{mNewProcessEvent};
  handles.reserve(kMaxHandles);
  bool newProcessesInQueue = true;

  while (true) {
    // Checking for incoming queue being not empty not safe without
    // locking the mutex, so we're using a separate flag here.
    if (newProcessesInQueue) {
      bool addedNewProcesses = false;
      {
        MutexGuard guard{mProcessesLock};
        while (!mProcessPids.empty() && handles.size() < kMaxHandles) {
          const DWORD pid = mProcessPids.back();
          mProcessPids.pop_back();

          HANDLE process = OpenProcess(SYNCHRONIZE, false, pid);
          if (process) {
            handles.push_back(process);
            addedNewProcesses = true;
          }
        }
        mProcessPids.clear();
      }
      if (addedNewProcesses) {
        onStateChanged.ProcessStateChanged(MonitorState::Active);
      }
      newProcessesInQueue = false;
    }

    const auto count = static_cast<DWORD>(handles.size());
    DWORD index = WaitForMultipleObjects(count, &handles[0], false, INFINITE);
    if (index >= WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS) {
      throw std::runtime_error("unexpected WaitForMultipleObjects() result");
    }
    index -= WAIT_OBJECT_0;

    if (index == kEventIndex) {
      newProcessesInQueue = true;
      continue;
    }

    CloseHandle(handles[index]);
    handles.erase(handles.cbegin() + index);

    if (handles.size() == 1) {
      onStateChanged.ProcessStateChanged(MonitorState::NoProcesses);
    }
  }
}

bool ProcessMonitor::IsMonitoredPath(const std::wstring &path) {
  if (path.empty()) {
    return false;
  }
  for (const auto &monitoredPath : mMonitoredPaths) {
    const int comp =
        _wcsnicmp(monitoredPath.c_str(), path.c_str(), path.length());
    if (comp == 0) {
      return true;
    }
  }
  return false;
}
