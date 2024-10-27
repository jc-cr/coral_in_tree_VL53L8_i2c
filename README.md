# coral_in_tree_VL53L8_i2c

## Setup

For this build we are using the In-Tree method described [here](https://coral.ai/docs/dev-board-micro/freertos/#freertos-tasks).
Once you have setup the coral micro directory, then you can clone this repo into the `apps` directory with all submodules.
```bash
cd coralmicro/apps
git clone https://github.com/jc-cr/coral_in_tree_VL53L8_i2c.git --recurse-submodules
```


## Build the application

Update the project to the `app` directory CMakeLists.txt file:
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

I recommend using a USB to Serial adapter to connect to the Coral Dev Board. 
Using one your can view the serial output with the following command:

```bash
picocom -b 115200 /dev/ttyUSB0
```

Exit with:
```
Ctrl-a Ctrl-x
```


## Tips

You may need to edit the linker script to increase m_text size.
`libs/nxp/rt1176-sdk/MIMXRT1176xxxxx_cm7_ram.ld`

```ld
  m_text                (RX)  : ORIGIN = 0x00000c00, LENGTH = 0x0007F400  /* Increased to about 508KB */
```
