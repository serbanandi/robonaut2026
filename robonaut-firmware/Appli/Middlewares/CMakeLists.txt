cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

project(robonaut-firmware_Middlewares)

# Create a static library object type
add_library(${PROJECT_NAME} STATIC)

add_compile_definitions(${PROJECT_NAME} PUBLIC
    LL_ATON_PLATFORM=LL_ATON_PLAT_STM32N6
    LL_ATON_OSAL=LL_ATON_OSAL_BARE_METAL
    LL_ATON_RT_MODE=LL_ATON_RT_ASYNC
    LL_ATON_SW_FALLBACK=1
    LL_ATON_DBG_BUFFER_INFO_EXCLUDED=1
    UX_STANDALONE
    UVC_LIB_USE_USBX
    SCR_LIB_USE_UVCL
    USBL_PACKET_PER_MICRO_FRAME=3
    UX_INCLUDE_USER_DEFINE_FILE
)

add_library(NetworkRuntime1010_CM55_GCC STATIC IMPORTED)
set_target_properties(NetworkRuntime1010_CM55_GCC PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/AI_Runtime/Lib/GCC/ARMCortexM55/NetworkRuntime1010_CM55_GCC.a"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    NetworkRuntime1010_CM55_GCC
    STM32_Drivers
)

# Add include paths
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/usbx/common/core/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/usbx/ports/generic/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/usbx/common/usbx_device_classes/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/usbx/common/usbx_stm32_device_controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/AI_Runtime/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/AI_Runtime/Npu/Devices/STM32N6XX
    ${CMAKE_CURRENT_SOURCE_DIR}/lib_vision_models_pp/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/AI_Runtime/Npu/ll_aton
    ${CMAKE_CURRENT_SOURCE_DIR}/screenl/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/screenl/uvcl/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/screenl/uvcl/Inc/usbx
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32_lcd
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/DSP/Include
)

file(GLOB_RECURSE USBX_SOURCES CONFIGURE_DEPENDS 
    "usbx/common/core/*.c"
    "usbx/common/usbx_device_classes/*.c"
    "usbx/common/usbx_network/*.c"
    "usbx/common/usbx_stm32_device_controllers/*.c"
)

file(GLOB_RECURSE AI_SOURCES CONFIGURE_DEPENDS 
    "AI_Runtime/*.c"
)

file(GLOB_RECURSE SCREENL_SOURCES CONFIGURE_DEPENDS 
    "screenl/*.c"
)

file(GLOB_RECURSE STM32_LCD_SOURCES CONFIGURE_DEPENDS 
    "stm32_lcd/*.c"
)

file(GLOB_RECURSE FONT_SOURCES CONFIGURE_DEPENDS 
    "fonts/*.c"
)

file(GLOB_RECURSE LIB_VISION_MODEL_SOURCES CONFIGURE_DEPENDS 
    "lib_vision_models_pp/*.c"
)

# Add sources to executable
target_sources(${PROJECT_NAME} PRIVATE
    ${USBX_SOURCES}
    ${AI_SOURCES}
    ${SCREENL_SOURCES}
    ${STM32_LCD_SOURCES}
    ${FONT_SOURCES}
    ${LIB_VISION_MODEL_SOURCES}
)






