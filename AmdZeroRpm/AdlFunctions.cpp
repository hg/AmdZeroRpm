#include "AdlFunctions.hpp"

void *__stdcall AdlMemoryAllocator(int bytes) { return new char[bytes]; }

void __stdcall AdlMemoryDeallocator(void **buffer) {
  if (buffer != nullptr && *buffer != nullptr) {
    delete *buffer;
    *buffer = nullptr;
  }
}
