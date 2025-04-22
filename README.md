# abcc-driver-api
The Anybus CompactCom (ABCC) Driver API is the easy-to-use interface for your host application towards the Anybus CompactCom Driver.

To see examples of how it can be used in a project, go to one of the following:
- [abcc-example-starterkit/](https://github.com/hms-networks/abcc-example-starterkit) - Example ported for Anybus CompactCom Starter kit
- [abcc-example-stm32nucleo/](https://github.com/hms-networks/abcc-example-stm32nucleo) - Example ported for STM32 Nucleo
- [abcc-example-raspberrypi](https://github.com/hms-networks/abcc-example-raspberrypi) - Example ported for Raspberry Pi

## Add as submodule (instead of cloning)

### Add this repo as a submodule

*It's suggested to add the repository as a submodule in your projects **lib/** folder.*
```
git submodule add https://github.com/hms-networks/abcc-driver-api.git lib/abcc-driver-api
```
This repository already contains another "nested" submodule ([abcc-driver/](https://github.com/hms-networks/abcc-driver)) that must be initialized. Therefore, go to the path of **abcc-driver-api/** and initialize **abcc-driver/**.
```
cd lib/abcc-driver-api
```
```
git submodule update --init --recursive
```
Go back to your git repository and stage the new submodules, otherwise they will only be linked locally on your machine.
```
cd ../../
```
```
git add .gitmodules lib/abcc-driver-api
```

## Cloning

### Use the flag

This repository contains a submodule ([abcc-driver/](https://github.com/hms-networks/abcc-driver)) that must be initialized. Therefore, pass the flag `--recurse-submodules` when cloning.

*It's suggested to clone the repository into your projects **lib/** folder.*
```
git clone --recurse-submodules https://github.com/hms-networks/abcc-driver-api.git lib/abcc-driver-api
```

#### (In case you missed it...)

If you did not pass the flag `--recurse-submodules` when cloning, the following command can be run:
```
git submodule update --init --recursive
```

## Building

### Configure

The Anybus CompactCom Driver API shall **always** be configured by a file called **abcc_driver_config.h**, created by you, custom to your project. The file shall contain macro definitions to enable, disable, and set values of different features and funtionalities.

The configuration file can either be generated simply with the [Anybus CompactCom Driver Configurator](https://compactcom40driver.online-config.anybus.com/), or created manually (advanced users).

### Alternative 1: CMake

This repository can be included as a library into a CMake target by adding a few sections to your **CMakeLists.txt** file.

1. Add your file **abcc_hardware_abstraction.c** to your target.
```
target_sources(<your_target> PRIVATE <"path/to/abcc_hardware_abstraction.c">)
```

2. Create a variable called `ABCC_API_INCLUDE_DIRS` with directories containing **your** personal include (.h) files related to the CompactCom Driver API, such as **abcc_driver_config.h**.
```
set(ABCC_API_INCLUDE_DIRS
    <${PROJECT_SOURCE_DIR}/path/to/include_directory>
	<...>
)
```

3. Create a variable called `ABCC_API_DIR` with the path to the directory where the CompactCom Driver API repository was cloned.
```
set(ABCC_API_DIR <"${PROJECT_SOURCE_DIR}/path/to/abcc-driver-api">)
```

4. Includes the standard CMake file from the CompactCom Driver API.
```
include(${ABCC_API_DIR}/abcc-driver-api.cmake)
```

5. Add directories containing include (.h) files related to the CompactCom Driver API to your target.
```
target_include_directories(<your_target> PRIVATE ${ABCC_API_INCLUDE_DIRS})
```

6. Add the CompactCom Driver API library to the user host application executable target.
```
target_link_libraries(<your_target> abcc_api)
```
The CompactCom Driver API should now be included into your project when building with CMake!

### Alternative 2: Make

This repository's Makefile, **abcc-driver-api.mk**, can be included into a Make target by adding a few sections to your higher level Makefile.

1. Create `SRCS`, add your source files to it (optional), and create object files from the content of `SRCS`.
```
SRCS  = ./src/main.c
SRCS += ./src/abcc_network_data_parameters.c
SRCS += ./src/abcc_adaptation/abcc_hardware_abstraction.c
...
```
2. Create `INCLUDES`, add your include paths to it (optional), and append the content as compiler flags when compiling. *Make sure to include the folder containing **abcc_driver_config.h** somehow, even if it's not in `INCLUDES` specifically.*.
```
INCLUDES = -I./src
INCLUDES = -I./src/abcc_adaptation
...
```
3. Create `ABCC_API_DIR` containing the path to the Anybus CompactCom Driver API directory.
```
ABCC_API_DIR := ./path/to/abcc-driver-api
```
4. Include **abcc-driver-api.mk**.
```
include $(ABCC_API_DIR)/abcc-driver-api.mk
```
The CompactCom Driver API should now compile together with your target!
