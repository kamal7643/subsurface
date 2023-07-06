cmake_minimum_required(VERSION 3.10.2)
project("subsurface")

find_library(GLESv3_LIB GLESv3)
find_library(log-lib log)

# Gather all source files
file(GLOB JNI_SOURCES
        jni/*.cpp
        )

# Gather all header files
file(GLOB JNI_HEADERS
        jni/*h
        )


# Add your project's native library
add_library(native SHARED ${JNI_SOURCES})

# Link against the required libraries
target_include_directories(native PRIVATE ${JNI_HEADERS})
target_link_libraries(native ${log-lib})
target_link_libraries(native ${GLESv3_LIB})