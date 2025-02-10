cmake_minimum_required(VERSION 3.18)

set(MODULE sstv)
project(${MODULE} LANGUAGES C)

set(SRC_DIR src)
file(GLOB SRC_C_LIST "${SRC_DIR}/*.c")
list(FILTER SRC_FILES EXCLUDE REGEX "${SRC_DIR}/${MODULE}.c")

include(FetchContent)
FetchContent_Declare(
    fftw
    URL http://fftw.org/fftw-3.3.10.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(fftw)

add_executable(${MODULE} ${SRC_DIR}/${MODULE}.c ${SRC_C_LIST})
target_compile_options(${MODULE} PRIVATE -Werror -Wextra -Wpedantic)
target_include_directories(${MODULE} PRIVATE ${fftw_SOURCE_DIR}/api)
target_link_libraries(${MODULE} PRIVATE fftw3)
