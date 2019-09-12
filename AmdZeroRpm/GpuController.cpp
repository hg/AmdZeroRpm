#include "GpuController.hpp"
#include "AdlFunctions.hpp"
#include <stdexcept>

namespace {

ADL_CONTEXT_HANDLE AdlCreateContext() {
  ADL_CONTEXT_HANDLE context;
  if (ADL2_Main_Control_Create(AdlMemoryAllocator, 1, &context) != ADL_OK) {
    throw std::runtime_error("could not initialize ADL");
  };
  return context;
}

void AdlDestroyContext(void *context) {
  if (context) {
    ADL2_Main_Control_Destroy(context);
  }
}

} // namespace

GpuController::GpuController()
    : mContext{AdlCreateContext(), AdlDestroyContext} {}

std::optional<Adapter> GpuController::GetPrimaryAdapter() const noexcept {
  int index = 0;
  if (ADL2_Adapter_Primary_Get(mContext.get(), &index) != ADL_OK) {
    return std::nullopt;
  }
  return Adapter{mContext, index};
}
