cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message("Build type: " ${CMAKE_BUILD_TYPE})

set(BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE} CACHE PATH "Binary directory for build files")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Compiler options
set(STM32_MCU_FLAGS  "-mcpu=cortex-m55 -mfpu=fpv5-d16 -mfloat-abi=hard -mcmse " )

#Linker options
set(STM32_LINKER_SCRIPT STM32N657X0HXQ_ROMxspi2.ld)
set(STM32_LINKER_OPTION  )

# Set the project name
set(CMAKE_PROJECT_NAME robonaut-firmware_Appli)
project(${CMAKE_PROJECT_NAME})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Include mx-generated
include("mx-generated.cmake")

# Enable float support in printf and scanf functions
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u _printf_float -u _scanf_float")


# In order to use CMake for cross-compiling
set(CMAKE_CXX_COMPILER_FORCED "true")

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # ${RobonAut_AI}
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers
    ${CMAKE_CURRENT_SOURCE_DIR}/RobonAUT
    ${CMAKE_CURRENT_SOURCE_DIR}/Control
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
)

set(CONTROL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Control/LineController/impl/lc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Control/LineProcessor/impl/line.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Control/System/impl/sys_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Control/System/impl/sys_main.c
)

set(UTIL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/IntHandler/impl/int_intHandler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/IntHandler/impl/int_listManager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/Telemetry/impl/tel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/MicroTimer/impl/mt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/RadioControl/impl/rc.c
)

set(HW_DRIVER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/Drive/impl/drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/Drive/impl/drv_encoder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/Drive/impl/drv_motor.c
    #${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/Imu/impl/imu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/Servo/impl/servo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/SSD1306/impl/ssd1306.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/SSD1306/impl/ssd1306_fonts.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/Uart/impl/uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/UserInput/impl/ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/AnalogInputs/impl/anlg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HwDrivers/LineSensor/impl/ls.c
)

set(ROBONAUT_UTIL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/RobonAUT/SimpleLogger/SimpleLogger.c
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CONTROL_SOURCES}
    ${UTIL_SOURCES}
    ${HW_DRIVER_SOURCES}
    ${ROBONAUT_UTIL_SOURCES}
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined libraries
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()

# # Convert to bin file and create a trusted binary using STM32SignTool
# add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
#     COMMENT "Generating binary and trusted binary files"
# )







