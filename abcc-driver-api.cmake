# The CMake command include_guard is compatible with CMake version 3.10 and greater.
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.10.0")
# Preventing this file to be included more than once.
   include_guard(GLOBAL)
endif()

# Complete list of source (.c) files inside the Anybus CompactCom API.
set(abcc_api_SRCS
   ${ABCC_API_DIR}/src/abcc_api_select_firmware.c
   ${ABCC_API_DIR}/src/abcc_api_handler.c
   ${ABCC_API_DIR}/src/abcc_api_command_handler.c
   ${ABCC_API_DIR}/src/host_objects/application_data_object.c
   ${ABCC_API_DIR}/src/host_objects/assembly_mapping_object.c
   ${ABCC_API_DIR}/src/anybus_objects/anybus_file_system_interface_object.c
)

# Complete list of header (.h) files inside the Anybus CompactCom API. This is
# only used to arrange the files correctly for display in IDEs.
set(abcc_api_INCS
   ${ABCC_API_DIR}/inc/abcc_api.h
   ${ABCC_API_DIR}/inc/abcc_api_network_settings.h
   ${ABCC_API_DIR}/inc/abcc_api_select_firmware.h
   ${ABCC_API_DIR}/inc/host_objects/abcc_api_command_handler_lookup.h
   ${ABCC_API_DIR}/inc/anybus_objects/anybus_file_system_interface_object.h
   ${ABCC_API_DIR}/src/abcc_api_config.h
   ${ABCC_API_DIR}/src/abcc_api_command_handler.h
   ${ABCC_API_DIR}/src/host_objects/application_data_object.h
   ${ABCC_API_DIR}/src/host_objects/assembly_mapping_object.h
)

# Check if ABCC_API_LIB_TYPE is not already defined. ABCC_API_LIB_TYPE can be set in a
# higher level CMake file to the desired library type of the Anybus CompactCom API.
if(NOT DEFINED ABCC_API_LIB_TYPE)
   set(ABCC_API_LIB_TYPE STATIC)
endif()

# Creating a library target containing the Anybus CompactCom API.
# The include files are added only to keep the file and directory tree structure.
add_library(abcc_api ${ABCC_API_LIB_TYPE}
   ${abcc_api_SRCS}
   ${abcc_api_INCS}
)

# Keeping the file and directory tree structure of the Anybus CompactCom API when 
# generating IDE projects.
source_group(TREE ${ABCC_API_DIR} PREFIX "abcc-api" FILES
   ${abcc_api_SRCS}
   ${abcc_api_INCS}
)

# Essentially a renaming of the variable containing include directories.
set(ABCC_DRIVER_INCLUDE_DIRS ${ABCC_API_INCLUDE_DIRS})

# The directory containing the Anybus CompactCom Driver repository.
set(ABCC_DRIVER_DIR ${ABCC_API_DIR}/abcc-driver)

# Including the Anybus CompactCom Driver's CMake module file.
include(${ABCC_DRIVER_DIR}/abcc-driver.cmake)

# Directories inside the Anybus CompactCom API containing include (.h) files to be 
# externally accessible is appended to the list ABCC_API_INCLUDE_DIRS. Not using
# append() command since the 'user unique' include (.h) files previously added is
# included in ABCC_DRIVER_INCLUDE_DIRS from the previous "renaming" operation.
set(ABCC_API_INCLUDE_DIRS
   ${ABCC_DRIVER_INCLUDE_DIRS}
   ${ABCC_API_DIR}/inc
   ${ABCC_API_DIR}/inc/host_objects
   ${ABCC_API_DIR}/inc/anybus_objects
)

# Adding all the Anybus CompactCom API related include (.h) files to the user host 
# Anybus CompactCom API library target.
target_include_directories(abcc_api PRIVATE ${ABCC_API_INCLUDE_DIRS})

# Link the Anybus CompactCom Driver library to the Anybus CompactCom API library.
target_link_libraries(abcc_api PRIVATE abcc_driver)