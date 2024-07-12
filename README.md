# coral_in_tree_VL53L8_i2c

## Setup

For this build we are using the In-Tree method described [here](https://coral.ai/docs/dev-board-micro/freertos/#freertos-tasks).
Once you have setup the coral micro directory, then you can clone this repo into the `apps` directory.


Oh and you'll need to edit the linker script to increse m_text size. `libs/nxp/rt1176-sdk/MIMXRT1176xxxxx_cm7_ram.ld`
```ld
  m_text                (RX)  : ORIGIN = 0x00000c00, LENGTH = 0x0007F400  /* Increased to about 508KB */
```



## Build the application

Update the CMakeLists.txt file in the `coral_in_tree_VL53L8_i2c` directory to include the necessary libraries:
```cmake
add_executable_m7(coral_in_tree_VL53L8_i2c
    src/main.cc
)

target_link_libraries(coral_in_tree_VL53L8_i2c
    libs_base-m7_freertos
)
```

Then add the project to the `app` directory CMakeLists.txt file:
```cmake
add_subdirectory(coral_in_tree_VL53L8_i2c)
```

Now you should be able to build the application by running the following command:
```bash
bash build.sh
```

## Upload the application

To upload the application to the Coral Dev Board, you can run the following command:

```bash
python3 scripts/flashtool.py --app coral_in_tree_VL53L8_i2c
```

## Run the application

Check the baud with:
```bash
stty -F /dev/ttyACM0 
```


The serial output can be viewed by running the following command:

```bash
screen /dev/ACM0 115200
```

Exit with `Ctrl+A` then `K` then `Y

