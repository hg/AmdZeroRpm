#pragma once

enum class MonitorState { Active, NoProcesses };

struct IStateChangeCallbackReceiver {
  virtual void ProcessStateChanged(MonitorState state) noexcept = 0;

protected:
  ~IStateChangeCallbackReceiver() = default;
};
