#include "WmiEventListener.hpp"
#include <sstream>

namespace {

void CheckResult(const HRESULT result) {
  if (FAILED(result)) {
    std::stringstream ss;
    ss << "error establishing WMI connection: 0x" << std::hex << GetLastError();
    throw std::runtime_error(ss.str());
  }
}

} // namespace

WmiEventListener::WmiEventListener(IUnknown *eventSink) {
  CheckResult(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

  CheckResult(CoInitializeSecurity(
      nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
      RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr));

  CheckResult(CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                               IID_IWbemLocator,
                               reinterpret_cast<LPVOID *>(&mLocator)));

  CheckResult(mLocator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr,
                                      nullptr, 0, nullptr, nullptr,
                                      &mServices));

  CheckResult(CoSetProxyBlanket(
      mServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
      RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE));

  CheckResult(CoCreateInstance(CLSID_UnsecuredApartment, nullptr,
                               CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
                               reinterpret_cast<void **>(&mApartment)));

  eventSink->AddRef();

  CheckResult(mApartment->CreateObjectStub(eventSink, &mStubUnknown));

  CheckResult(mStubUnknown->QueryInterface(
      IID_IWbemObjectSink, reinterpret_cast<void **>(&mStubSink)));
}

void WmiEventListener::Start() {
  if (mIsStarted) {
    throw std::runtime_error("the listener has already been started");
  }
  CheckResult(mServices->ExecNotificationQueryAsync(
      _bstr_t(L"WQL"),
      _bstr_t(L"SELECT ProcessID FROM Win32_ProcessStartTrace"), 0, nullptr,
      mStubSink));
  mIsStarted = true;
}

void WmiEventListener::Stop() {
  if (mIsStarted) {
    mIsStarted = false;
    CheckResult(mServices->CancelAsyncCall(mStubSink));
  }
}

WmiEventListener::~WmiEventListener() {
  if (mStubSink != nullptr) {
    mStubSink->Release();
  }
  if (mStubUnknown != nullptr) {
    mStubUnknown->Release();
  }
  if (mApartment != nullptr) {
    mApartment->Release();
  }
  if (mServices != nullptr) {
    mServices->Release();
  }
  if (mLocator != nullptr) {
    mLocator->Release();
  }
  CoUninitialize();
}
