#pragma once

#include <Windows.h>
#define _WIN32_DCOM
#include "IStartupCallbackReceiver.hpp"
#include <WbemIdl.h>
#include <comdef.h>

class EventSink final : public IWbemObjectSink {
public:
  explicit EventSink(IStartupCallbackReceiver &callback)
      : mCallback{callback} {}
  virtual ~EventSink() { mDone = true; }
  EventSink(const EventSink &) = delete;

  ULONG __stdcall AddRef() noexcept override;
  ULONG __stdcall Release() noexcept override;
  HRESULT __stdcall QueryInterface(REFIID riid, void **ppv) noexcept override;

  HRESULT __stdcall Indicate(LONG objectCount,
                             IWbemClassObject **objects) noexcept override;

  HRESULT __stdcall SetStatus(LONG flags, HRESULT result, BSTR strParam,
                              IWbemClassObject *objParam) noexcept override;

private:
  IStartupCallbackReceiver &mCallback;
  LONG mReferences{0};
  bool mDone{false};
};
