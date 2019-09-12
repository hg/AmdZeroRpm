#include "EventSink.hpp"

ULONG __stdcall EventSink::AddRef() noexcept {
  return InterlockedIncrement(&mReferences);
}

ULONG __stdcall EventSink::Release() noexcept {
  const auto ref = InterlockedDecrement(&mReferences);
  if (ref == 0) {
    delete this;
  }
  return ref;
}

HRESULT __stdcall EventSink::QueryInterface(REFIID riid, void **ppv) noexcept {
  if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
    *ppv = static_cast<IWbemObjectSink *>(this);
    AddRef();
    return WBEM_NO_ERROR;
  }
  return E_NOINTERFACE;
}

HRESULT __stdcall EventSink::Indicate(LONG objectCount,
                                      IWbemClassObject **objects) noexcept {
  VARIANT var;
  for (int i = 0; i < objectCount; ++i) {
    const HRESULT result =
        objects[i]->Get(L"ProcessID", 0, &var, nullptr, nullptr);
    if (SUCCEEDED(result)) {
      mCallback.ApplicationStarted(var.ulVal);
    }
  }
  return WBEM_S_NO_ERROR;
}

HRESULT __stdcall EventSink::SetStatus(LONG flags, HRESULT result,
                                       BSTR strParam,
                                       IWbemClassObject *objParam) noexcept {
  UNREFERENCED_PARAMETER(flags);
  UNREFERENCED_PARAMETER(result);
  UNREFERENCED_PARAMETER(strParam);
  UNREFERENCED_PARAMETER(objParam);
  return WBEM_S_NO_ERROR;
}
