cmake_minimum_required(VERSION 3.10)

project(micro_behaviortree_cpp)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(TINYXML2_SRC ${CMAKE_SOURCE_DIR}/include/micro_behaviortree_cpp/3rdparty/tinyxml2/tinyxml2.cpp)

# Install behaviortree.hpp
install(FILES ${CMAKE_SOURCE_DIR}/behaviortree.hpp DESTINATION include)

set(EXAMPLE_SRCS
    example_static_type_builder
    example_dynamic_type_builder
    example_xml_text_builder
    example_xml_file_builder
    example_running_action
)

foreach(EXAMPLE_SRC ${EXAMPLE_SRCS})
    add_executable(${EXAMPLE_SRC} examples/${EXAMPLE_SRC}.cpp ${TINYXML2_SRC})
    target_include_directories(${EXAMPLE_SRC} PRIVATE ${INCLUDE_DIR})
endforeach()