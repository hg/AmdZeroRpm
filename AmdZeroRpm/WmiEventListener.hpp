#pragma once

#include "EventSink.hpp"

class WmiEventListener {
public:
  explicit WmiEventListener(IUnknown *eventSink);
  ~WmiEventListener();
  WmiEventListener(const WmiEventListener &) = delete;

  void Start();
  void Stop();

private:
  bool mIsStarted{false};
  IWbemLocator *mLocator{nullptr};
  IWbemServices *mServices{nullptr};
  IUnsecuredApartment *mApartment{nullptr};
  IUnknown *mStubUnknown{nullptr};
  IWbemObjectSink *mStubSink{nullptr};
};
