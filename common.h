#pragma once
#include <string>
#include <vector>

enum class DeviceCommand { IDLE, DATA_RECEIVED, ERROR_REPORT };

struct Event {
    DeviceCommand type;
    std::vector<std::string>  message;
};
enum class WorkerCommand { IDLE, END, ERROR_REPORT };

struct EventToWork {
    WorkerCommand type;
    std::vector<std::string>  message;
};