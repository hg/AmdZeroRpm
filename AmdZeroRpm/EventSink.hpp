#pragma once

#include <Windows.h>
#define _WIN32_DCOM
#include "IStartupCallbackReceiver.hpp"
#include <WbemIdl.h>
#include <comdef.h>
#include <functional>

class EventSink : public IWbemObjectSink {
public:
  EventSink(IStartupCallbackReceiver &callback) : mCallback{callback} {}
  virtual ~EventSink() { mDone = true; }

  virtual ULONG __stdcall AddRef();
  virtual ULONG __stdcall Release();
  virtual HRESULT __stdcall QueryInterface(REFIID riid, void **ppv);

  virtual HRESULT __stdcall Indicate(LONG objectCount,
                                     IWbemClassObject **objects);

  virtual HRESULT __stdcall SetStatus(LONG flags, HRESULT result, BSTR strParam,
                                      IWbemClassObject *objParam);

private:
  IStartupCallbackReceiver &mCallback;
  LONG mReferences{0};
  bool mDone{false};
};
