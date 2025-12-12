set(EXECUTABLE_NAME tel-cli)

add_executable(${EXECUTABLE_NAME} 
    main.c
    commands.c
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${ZMQ_INCLUDE_DIRS})

target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${ZMQ_LIBRARIES})