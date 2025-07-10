cmake_minimum_required(VERSION 3.10)
project(Loongson2K0300)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")

# 查找依赖库
find_package(ALSA REQUIRED)
find_package(Jsoncpp REQUIRED)

# wiringPi需要手动指定路径
set(WIRINGPI_LIB wiringPi)
set(WIRINGPI_INCLUDE /usr/local/include)

# 包含目录
include_directories(
    ${ALSA_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${WIRINGPI_INCLUDE}
    drivers
)

# 可执行文件
add_executable(loongson_app
    main.cpp
    drivers/camera_driver.cpp
    drivers/mic_driver.cpp
    drivers/dht11_driver.cpp
)

# 链接库
target_link_libraries(loongson_app
    ${ALSA_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${WIRINGPI_LIB}
)