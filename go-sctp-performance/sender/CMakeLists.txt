CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT("preformance test")

SET(CMAKE_C_FLAGS "-g -std=c99 -pthread -Wall")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

ADD_EXECUTABLE(sender sender.c)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  TARGET_LINK_LIBRARIES(sender /usr/lib/libsctp.dylib)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

