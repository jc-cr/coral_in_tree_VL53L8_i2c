// AUTO-GENERATED FILE FROM "scripts/generate_tasks.py"
// EDIT AT YOUR OWN RISK.

#include "task_config.hh"

// Task implementations
#include "tof_task.hh"

namespace coralmicro {
namespace {

struct TaskConfig {
    TaskFunction_t taskFunction;
    const char* taskName;
    uint32_t stackSize;
    void* parameters;
    UBaseType_t priority;
    TaskHandle_t* handle;
};

constexpr TaskConfig kTaskConfigs[] = {
    {
        tof_task,
        "TOF_Task",
        32 * 1024,
        0,
        3,
        nullptr
    }
};

} // namespace

TaskErr_t CreateAllTasks() {
    TaskErr_t status = TaskErr_t::OK;

    for (const auto& config : kTaskConfigs) {
        BaseType_t ret = xTaskCreate(
            config.taskFunction,
            config.taskName,
            config.stackSize,
            config.parameters,
            config.priority,
            config.handle
        );

        if (ret != pdPASS) {
            printf("Failed to create task: %s\r\n", config.taskName);
            status = TaskErr_t::CREATE_FAILED;
            break;
        }
    }

    return status;
}

} // namespace coralmicro