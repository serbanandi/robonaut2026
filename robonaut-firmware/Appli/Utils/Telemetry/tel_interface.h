#ifndef TEL_INTERFACE_H_
#define TEL_INTERFACE_H_

#include <stdint.h>

// Variable Types
typedef enum {
    TEL_UINT8  = 0,
    TEL_INT8   = 1,
    TEL_UINT16 = 2,
    TEL_INT16  = 3,
    TEL_UINT32 = 4,
    TEL_INT32  = 5,
    TEL_FLOAT  = 6
} tel_VarTypeType;

// Log Levels
typedef enum {
    TEL_LOG_DEBUG = 0,
    TEL_LOG_INFO  = 1,
    TEL_LOG_WARN  = 2,
    TEL_LOG_ERROR = 3
} tel_LogLevelType;

/**
 * @brief Initialize the telemetry system.
 * @return 1 on success, 0 on failure.
 */
uint8_t tel_Init(void);

/**
 * @brief Register a Read-Only variable to be streamed.
 * @param data Pointer to the variable in memory.
 * @param type Type of the variable (use tel_VarTypeType).
 * @param name String name of the variable.
 * @param updateRateMs How often to stream this variable (0 = never stream, only on request).
 * @return 1 on success, 0 if registry is full.
 */
uint8_t tel_RegisterR(void* data, tel_VarTypeType type, const char* name, uint32_t updateRateMs);

/**
 * @brief Register a Read-Write variable.
 * @param data Pointer to the variable in memory.
 * @param type Type of the variable (use tel_VarTypeType).
 * @param name String name of the variable.
 * @param updateRateMs How often to stream this variable.
 * @return 1 on success, 0 if registry is full.
 */
uint8_t tel_RegisterRW(void* data, tel_VarTypeType type, const char* name, uint32_t updateRateMs);

/**
 * @brief Send a log message to the RPi.
 * @param level Log level (tel_LogLevelType).
 * @param fmt Format string (printf style).
 * @return 1 on success, 0 on failure.
 */
uint8_t tel_Log(tel_LogLevelType level, const char* fmt, ...);

/**
 * @brief Get text input from the RPi. Non-blocking.
 * @param buffer Buffer to store the received text.
 * @param maxLen Maximum length of the buffer.
 * @return Number of bytes received, 0 if no new data.
 */
uint32_t tel_GetTextInput(char* buffer, uint32_t maxLen);

/**
 * @brief Main telemetry process loop. Call this frequently in your main while(1).
 */
void tel_Process(void);

#endif /* TEL_INTERFACE_H_ */
