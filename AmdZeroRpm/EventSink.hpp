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

  ULONG __stdcall AddRef() override;
  ULONG __stdcall Release() override;
  HRESULT __stdcall QueryInterface(REFIID riid, void **ppv) override;

  HRESULT __stdcall Indicate(LONG objectCount,
                             IWbemClassObject **objects) override;

  HRESULT __stdcall SetStatus(LONG flags, HRESULT result, BSTR strParam,
                              IWbemClassObject *objParam) override;

private:
  IStartupCallbackReceiver &mCallback;
  LONG mReferences{0};
  bool mDone{false};
};
