#include "Application.hpp"
#include <ShlObj.h>
#include <fstream>
#include <iostream>

namespace {

std::vector<std::wstring> LoadMonitoredPaths() {
  wchar_t *pathPtr;
  if (FAILED(SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &pathPtr))) {
    throw std::runtime_error("could not determine user profile directory");
  }

  std::wstring path{pathPtr};
  CoTaskMemFree(pathPtr);

  path.append(L"\\ZeroRpmDisabledFor.txt");

  std::vector<std::wstring> files;
  std::wifstream fileStream{path};

  for (std::wstring line; std::getline(fileStream, line);) {
    files.push_back(line);
  }

  return files;
}

} // namespace

Application::Application() {
  if (const auto adapter = mGpuController.GetPrimaryAdapter(); adapter) {
    mZeroRpmEnabled = adapter->GetZeroRpm() == ZeroRpmStatus::On;
  } else {
    mZeroRpmEnabled = true;
  }
  for (const auto &path : LoadMonitoredPaths()) {
    mMonitor.AddMonitoredPath(path);
  }
  // This gets deleted later by the COM subsystem.
  const auto eventSink = new EventSink{*this};
  mConnection = std::make_unique<WmiEventListener>(eventSink);
  mGpuWorker = std::thread{&Application::UpdateGpuState, this};
}

void Application::Start() {
  mConnection->Start();
  mMonitor.MonitorLoop(*this);
  mConnection->Stop();
}

void Application::ApplicationStarted(DWORD pid) noexcept {
  mMonitor.AddProcess(pid);
}

void Application::UpdateGpuState() {
  constexpr auto delay = std::chrono::seconds{90};

  while (true) {
    std::unique_lock<std::mutex> lock{mZeroRpmMutex};
    mZeroRpmChangedCv.wait(lock, [this] { return mZeroRpmStateChanged; });
    mZeroRpmStateChanged = false; // Handle spurious wakeups.

    // If fans were disabled, add some delay before we turn them off.
    // This gives the user some time to start another application (e.g. play
    // another video file) and keep them spinning.
    if (mZeroRpmEnabled) {
      mZeroRpmChangedCv.wait_for(lock, delay,
                                 [this] { return !mZeroRpmEnabled; });
    }

    // We have no control over the driver and it can hang for indeterminate
    // amount of time. Release the lock before passing control to the driver.
    const bool enabled = mZeroRpmEnabled;
    lock.unlock();

    if (const auto adapter = mGpuController.GetPrimaryAdapter(); adapter) {
      const auto status = adapter->SetZeroRpm(enabled);
      if (status == ZeroRpmStatus::Error) {
        std::wcerr << L"could not set zero RPM state\n";
      }
    } else {
      std::wcerr << L"could not get primary adapter\n";
    }
  }
}

void Application::ProcessStateChanged(const MonitorState state) noexcept {
  std::lock_guard<std::mutex> guard{mZeroRpmMutex};
  mZeroRpmEnabled = state == MonitorState::NoProcesses;
  mZeroRpmStateChanged = true;
  mZeroRpmChangedCv.notify_one();
}
