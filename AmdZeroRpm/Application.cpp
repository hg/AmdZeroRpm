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

bool RegisterAutostart() noexcept {
  wchar_t pathBuf[MAX_PATH + 1];
  if (!GetModuleFileNameW(nullptr, pathBuf, MAX_PATH)) {
    return false;
  }
  HKEY key;
  constexpr auto path = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
  auto status = RegOpenKeyExW(HKEY_CURRENT_USER, path, 0, KEY_WRITE, &key);
  if (status != ERROR_SUCCESS) {
    return false;
  }
  const auto byteLength = (wcslen(pathBuf) + 1) * sizeof(wchar_t);
  status = RegSetValueExW(key, kApplicationName, 0, REG_SZ,
                          reinterpret_cast<BYTE *>(pathBuf),
                          static_cast<DWORD>(byteLength));
  RegCloseKey(key);
  return status == ERROR_SUCCESS;
}
