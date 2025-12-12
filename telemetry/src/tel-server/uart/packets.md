# Nucleo <--> Rpi UART communication

## Generic communication description
The UART communication is completely in binary. There are 4 types of frames: _log_ frames which contain text and a timestamp, _request_ frames which are mostly sent by the Rpi to request some action from the Nucleo, _reply_ frames which are sent by the Nucleo in response to a request frame, and _stream_ frames which are periodically sent by the Nucleo about the current value of different parameters.

## Frame format
Each frame starts with a start-of-frame (`SOF = 10'42`) character, after which there is a frame type byte. This byte contains the following bits: **2'CCDDDDDD**, where **CC** marks the frame category (0 - request, 1 - reply, 2 - stream), and **DDDDDD** marks the ID of this frame type. In case of the _request_ and _reply_ categories, another byte follows which marks the sequence number of the request and the reply respectively.

Following this header there will be some data bytes. After these data bytes the next to last byte is the checksum byte, which contains the sum product of all bytes in the frame (excluding the `SOF` and `EOF` characters). Finally, the last byte in the frame is the end-of-frame (`EOF = 10'69`) character, which marks the end of the given frame.

### Escape character
The `SOF` and `EOF` characters must not appear anywhere else in the transmitted frame, so they are always "escaped" using the escape character (`ESC = 10'123`).
If the encoded character is the following:
- `SOF` -> `ESC` and a `1` is transmitted
- `ESC` -> `ESC` and a `2` is transmitted
- `EOF` -> `ESC` and a `3` is transmitted

## Frame types
The following are the currently supported frame types.

### Currently registered variable (that can be read or written) list request frame (2'01000001):
This frame is used to request the list of currently registered variables that can be read or written via telemetry. There is no payload for this frame.

### Variable list reply frame (2'10000001):
This frame is the reply to the "currently registered variable list request" frame and has a payload that consists of:
- 1 byte: number of registered variables `N`
- `N` times:
    - 1 byte: variable ID
    - 1 byte: variable type (0 - uint8_t, 1 - int8_t, 2 - uint16_t, 3 - int16_t, 4 - uint32_t, 5 - int32_t, 6 - float)
    - 1 byte: variable access type (0 - read-only, 1 - read-write)
    - 1 byte: variable name length `L`
    - `L` bytes: variable name string (not null-terminated)

### Variable write request frame (2'01000010):
This frame is used to write a value to a registered variable. The payload consists of:
- 1 byte: variable ID
- N bytes: variable value (size depends on variable type)

### Variable write reply frame (2'10000010):
This frame is the reply to the "variable write request" frame and has no payload.

### Log frame (Nucleo --> Rpi) (0'10000001):
This frame is used to transmit log messages from the Nucleo to the Rpi. The payload consists of:
- 4 bytes: timestamp in milliseconds (uint32_t)
- 1 byte: log level (0 - DEBUG, 1 - INFO, 2 - WARN, 3 - ERROR)
- N bytes: log message string (not null-terminated)

### Text input frame (Rpi --> Nucleo) (0'10000010):
This frame is used to send text input from the Rpi to the Nucleo. The payload consists of:
- N bytes: text input string (not null-terminated)

### Variable value stream frame (2'10000011):
This is a frame that the Nucleo streams at a certain frequency for each registered variable. The payload consists of:
- 1 byte: variable ID
- N bytes: variable value (size depends on variable type)
