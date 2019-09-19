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
    mMonitor.AddMonitoredPath(path);
  }
  // This gets deleted later by the COM subsystem.
  const auto eventSink = new EventSink{*this};
  mConnection.reset(new WmiEventListener{eventSink});
}

void Application::Start() {
  mConnection->Start();
  mMonitor.MonitorLoop(*this);
  mConnection->Stop();
}

void Application::ApplicationStarted(DWORD pid) noexcept {
  mMonitor.AddProcess(pid);
}

void Application::ProcessStateChanged(const MonitorState state) noexcept {
  const bool enabled = state == MonitorState::NoProcesses;
  const std::optional<Adapter> adapter = mGpuController.GetPrimaryAdapter();
  if (adapter.has_value()) {
    adapter.value().SetZeroRpm(enabled);
  }
}
