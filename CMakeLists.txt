cmake_minimum_required(VERSION 3.14)
project(dcqcn_sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/headers)

add_executable(dcqcn_sim
    main.cpp
    source/sender.cpp
    source/switch_node.cpp
    source/receiver.cpp
    source/metrics.cpp
    source/network.cpp
)