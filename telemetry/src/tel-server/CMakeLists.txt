set(EXECUTABLE_NAME tel-server)

add_executable(${EXECUTABLE_NAME}
    main.c
    uart/impl/uart.c
    ctrl/impl/ctrl.c
    zmqs/impl/zmqs.c
    var.c
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE 
    ${ZMQ_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
    ${ZMQ_LIBRARIES}
    logger
)