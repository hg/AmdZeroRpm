#include "Application.hpp"
#include <ShlObj.h>
#include <fstream>

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
  for (const auto &path : LoadMonitoredPaths()) {
    monitor.AddMonitoredPath(path);
  }
  // This gets deleted later by the COM subsystem.
  auto eventSink = new EventSink{*this};
  connection.reset(new WmiEventListener{eventSink});
}

void Application::Start() {
  connection->Start();

  if (!monitor.ScanRunning()) {
    throw std::runtime_error("could not scan running processes");
  }

  ProcessStateChanged(monitor.CurrentState());
  monitor.MonitorLoop(*this);

  connection->Stop();
}

void Application::ApplicationStarted(DWORD pid) noexcept {
  monitor.AddProcess(pid);
}

void Application::ProcessStateChanged(MonitorState state) noexcept {
  bool enabled = state == MonitorState::NoProcesses;
  gpuController.ToggleZeroRpm(enabled);
}
