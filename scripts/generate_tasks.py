#!/usr/bin/env python3

import yaml
import sys
import os
from datetime import date

def generate_header():
    header = """// AUTO-GENERATED FILE.BY "scripts/generate_tasks.py" 
// EDIT AT YOUR OWN RISK.

#pragma once

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include <cstdio>

namespace coralmicro {

// Task priorities (configMAX_PRIORITIES = 5)
constexpr int TASK_PRIORITY_HIGH   = (configMAX_PRIORITIES - 1);  // 4
constexpr int TASK_PRIORITY_MEDIUM = (configMAX_PRIORITIES - 2);  // 3
constexpr int TASK_PRIORITY_LOW    = (configMAX_PRIORITIES - 3);  // 2

// Stack sizes
constexpr int STACK_SIZE_LARGE  = (configMINIMAL_STACK_SIZE * 4);  // 1440 bytes
constexpr int STACK_SIZE_MEDIUM = (configMINIMAL_STACK_SIZE * 3);  // 1080 bytes
constexpr int STACK_SIZE_SMALL  = (configMINIMAL_STACK_SIZE * 2);  // 720 bytes

// Task interface
enum class TaskErr_t {
    OK = 0,
    CREATE_FAILED,
};

// Function to create all tasks
TaskErr_t CreateAllTasks();

} // namespace coralmicro
"""
    return header

def generate_source(tasks):
    # First collect all unique task entry points to generate includes
    task_headers = set()
    for task in tasks.values():
        task_name = task['TaskEntryPtr'].replace("_task", "")  # strip _task suffix
        task_headers.add(f"#include \"{task_name}_task.hh\"")
    
    # Join headers with newlines
    header_includes = "\n".join(sorted(task_headers))

    config_entries = []
    for task in tasks.values():
        entry = f"""    {{
        {task['TaskEntryPtr']},
        "{task['TaskName']}",
        {task['StackSize']},
        {task['ParametersPtr']},
        {task['TaskPriority']},
        {task['TaskHandle']}
    }}"""
        config_entries.append(entry)

    config_string = ",\n".join(config_entries)

    source = f"""// AUTO-GENERATED FILE FROM "scripts/generate_tasks.py"
// EDIT AT YOUR OWN RISK.

#include "task_config.hh"

// Task implementations
{header_includes}

namespace coralmicro {{
namespace {{

struct TaskConfig {{
    TaskFunction_t taskFunction;
    const char* taskName;
    uint32_t stackSize;
    void* parameters;
    UBaseType_t priority;
    TaskHandle_t* handle;
}};

constexpr TaskConfig kTaskConfigs[] = {{
{config_string}
}};

}} // namespace

TaskErr_t CreateAllTasks() {{
    TaskErr_t status = TaskErr_t::OK;

    for (const auto& config : kTaskConfigs) {{
        BaseType_t ret = xTaskCreate(
            config.taskFunction,
            config.taskName,
            config.stackSize,
            config.parameters,
            config.priority,
            config.handle
        );

        if (ret != pdPASS) {{
            printf("Failed to create task: %s\\r\\n", config.taskName);
            status = TaskErr_t::CREATE_FAILED;
            break;
        }}
    }}

    return status;
}}

}} // namespace coralmicro"""
    return source

def main():
    if len(sys.argv) != 4:
        print("Usage: generate_tasks.py <yaml_file> <output_header> <output_source>")
        sys.exit(1)

    yaml_filepath = sys.argv[1]
    header_filepath = sys.argv[2]
    source_filepath = sys.argv[3]

    try:
        with open(yaml_filepath, 'r') as stream:
            tasks = yaml.safe_load(stream)

        # Create directories if they don't exist
        os.makedirs(os.path.dirname(header_filepath), exist_ok=True)
        os.makedirs(os.path.dirname(source_filepath), exist_ok=True)

        # Generate and write header
        with open(header_filepath, 'w') as f:
            f.write(generate_header())

        # Generate and write source
        with open(source_filepath, 'w') as f:
            f.write(generate_source(tasks))

    except Exception as e:
        print(f"Error generating task configuration: {str(e)}")
        sys.exit(1)

if __name__ == '__main__':
    main()