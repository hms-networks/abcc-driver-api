# abcc-api
The Anybus CompactCom (ABCC) API is the easy-to-use interface for your host application towards the CompactCom Driver.

Test

To see examples of how it can be used in a project, go to [abcc-example-starterkit/](https://github.com/hms-networks/abcc-example-starterkit).

## Add as submodule (instead of cloning)

### Add this repo as a submodule

*It's suggested to add the repository as a submodule in your projects `lib/` folder.*
```
git submodule add https://github.com/hms-networks/abcc-api.git lib/abcc-api
```
This repository already contains another "nested" submodule ([abcc-driver/](https://github.com/hms-networks/abcc-driver)) that must be initialized. Therefore, go to the path of abcc-api/ and initialize abcc-driver/.
```
cd lib/abcc-api
```
```
git submodule update --init --recursive
```
Go back to your git repository and stage the new submodules, otherwise they will only be linked locally on your machine.
```
cd ../../
```
```
git add .gitmodules lib/abcc-api
```

## Cloning

### Use the flag

This repository contains a submodule ([abcc-driver/](https://github.com/hms-networks/abcc-driver)) that must be initialized. Therefore, pass the flag `--recurse-submodules` when cloning.

*It's suggested to clone the repository into your projects `lib/` folder.*
```
git clone --recurse-submodules https://github.com/hms-networks/abcc-api.git lib/abcc-api
```

#### (In case you missed it...)

If you did not pass the flag `--recurse-submodules` when cloning, the following command can be run:
```
git submodule update --init --recursive
```

## Building

### Configure

The Anybus CompactCom API shall **always** be configured by a file called `abcc_driver_config.h`, created by you, custom to your project. The file shall contain macro definitions to enable, disable, and set values of different features and funtionalities.

The configuration file can either be generated simply with the [Anybus CompactCom Driver Configurator](https://compactcom40driver.online-config.anybus.com/), or created manually (advanced users).

### CMake

This repository can be included as a library into a CMake target by adding a few sections to your CMakeLists.txt file.

1. Add your file abcc_hardware_abstraction.c to your target.
```
target_sources(<your_target> PRIVATE <"path/to/abcc_hardware_abstraction.c">)
```

2. Create a variable called ABCC_API_INCLUDE_DIRS with directories containing **your** personal include (.h) files related to the CompactCom API, such as `abcc_driver_config.h`.
```
set(ABCC_API_INCLUDE_DIRS
    <${PROJECT_SOURCE_DIR}/path/to/include_directory>
	<...>
)
```

3. Create a variable called ABCC_API_DIR with the path to the directory where the CompactCom API repository was cloned.
```
set(ABCC_API_DIR <"${PROJECT_SOURCE_DIR}/path/to/abcc-api">)
```

4. Includes the standard CMake file from the CompactCom API.
```
include(${ABCC_API_DIR}/abcc-api.cmake)
```

5. Add directories containing include (.h) files related to the CompactCom API to your target.
```
target_include_directories(host_application_exec PRIVATE ${ABCC_API_INCLUDE_DIRS})
```

6. Add the CompactCom API library to the user host application executable target.
```
target_link_libraries(<your_target> abcc_api)
```
The CompactCom API should now be included into your project when building with CMake!
