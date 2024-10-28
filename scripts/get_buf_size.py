def count_array_size(filename, array_name):
    with open(filename, 'r') as f:
        content = f.read()
        start = content.find(array_name + '[]')
        if start == -1:
            return 0
        start = content.find('{', start)
        if start == -1:
            return 0
        end = content.find('};', start)
        if end == -1:
            return 0
        subset = content[start:end]
        return subset.count(',') + 1

filename = '/home/jc/Workspace/coralmicro_docker_dev/coralmicro/apps/coral_in_tree_VL53L8_i2c/libs/VL53L8CX_ULD_driver_2.0.0/VL53L8CX_ULD_API/src/vl53l8cx_buffers.c'
buffers = [
    'VL53L8CX_FIRMWARE',
    'VL53L8CX_DEFAULT_CONFIGURATION',
    'VL53L8CX_DEFAULT_XTALK',
    'VL53L8CX_GET_NVM_CMD'
]

print("Buffer sizes:")
for buf in buffers:
    size = count_array_size(filename, buf)
    print(f"#define {buf}_SIZE {size}")