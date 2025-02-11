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

FetchContent_Declare(
    libpng
    GIT_REPOSITORY https://github.com/glennrp/libpng.git
    GIT_TAG v1.6.37
)
FetchContent_MakeAvailable(libpng)


add_executable(${MODULE} ${SRC_DIR}/${MODULE}.c ${SRC_C_LIST})
target_compile_options(${MODULE} PRIVATE -Werror -Wextra -Wpedantic)
target_include_directories(${MODULE} PRIVATE
  ${fftw_SOURCE_DIR}/api
  ${libpng_SOURCE_DIR}
  ${libpng_BINARY_DIR}
)
target_link_libraries(${MODULE} PRIVATE fftw3 png)
