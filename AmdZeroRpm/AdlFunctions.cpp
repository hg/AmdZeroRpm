#include "AdlFunctions.hpp"

void *__stdcall AdlMemoryAllocator(const int bytes) { return new char[bytes]; }

void __stdcall AdlMemoryDeallocator(void **buffer) {
  if (buffer != nullptr && *buffer != nullptr) {
    delete reinterpret_cast<char *>(*buffer);
    *buffer = nullptr;
  }
}
