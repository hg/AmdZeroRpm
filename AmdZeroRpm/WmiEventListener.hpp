#pragma once

#include "EventSink.hpp"

class WmiEventListener {
public:
  WmiEventListener(IUnknown *eventSink);
  ~WmiEventListener();

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
