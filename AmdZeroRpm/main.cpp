#include "Application.hpp"
#include <PathCch.h>
#include <Shlwapi.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace {

[[nodiscard]] std::wstring GetExeDirectory() {
  wchar_t path[MAX_PATH + 1];
  GetModuleFileNameW(nullptr, path, MAX_PATH);
  PathRemoveFileSpecW(path);
  return {path};
}

[[nodiscard]] std::wstring GetExeDirFilePath(const std::wstring_view fileName) {
  wchar_t filePath[MAX_PATH + 1];
  const auto exeDir = GetExeDirectory();
  const auto status =
      PathCchCombine(filePath, MAX_PATH, exeDir.c_str(), fileName.data());
  return status == S_OK ? filePath : L"";
}

} // namespace

int WINAPI wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance,
                    _In_ LPWSTR cmdLine, _In_ int showCmd) {
  UNREFERENCED_PARAMETER(instance);
  UNREFERENCED_PARAMETER(prevInstance);
  UNREFERENCED_PARAMETER(cmdLine);
  UNREFERENCED_PARAMETER(showCmd);

  const auto outFile = GetExeDirFilePath(L"out.log");
  if (outFile.empty()) {
    std::wcerr << L"could not redirect output to log file\n";
    return EXIT_FAILURE;
  }
  const std::wofstream output{outFile};
  std::wcout.rdbuf(output.rdbuf());
  std::wcerr.rdbuf(output.rdbuf());

  try {
    Application app;
    app.Start();
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::wcerr << L"error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}
