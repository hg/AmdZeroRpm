#include "Application.hpp"
#include <cstdlib>

int WINAPI wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance,
                    _In_ LPWSTR cmdLine, _In_ int showCmd) {
  UNREFERENCED_PARAMETER(instance);
  UNREFERENCED_PARAMETER(prevInstance);
  UNREFERENCED_PARAMETER(cmdLine);
  UNREFERENCED_PARAMETER(showCmd);

  try {
    Application app;
    app.Start();
    return EXIT_SUCCESS;
  } catch (const std::exception &) {
    // TODO: log error message
    return EXIT_FAILURE;
  }
}
