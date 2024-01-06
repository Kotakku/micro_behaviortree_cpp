cmake_minimum_required(VERSION 3.10)

project(micro_behaviortree_cpp)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-O2 -Wall -Wextra -Wpedantic -Wno-unused-variable -Wno-unused-function)
endif()

option(BUILD_EXAMPLES "build examples" OFF)

add_library(${PROJECT_NAME} 
    include/micro_behaviortree_cpp/3rdparty/tinyxml2/tinyxml2.cpp
)
target_include_directories(${PROJECT_NAME}  PRIVATE include)

if(BUILD_EXAMPLES)
set(EXAMPLE_SRCS
    example_static_type_builder
    example_dynamic_type_builder
    example_xml_text_builder
    example_xml_file_builder
    example_running_action
)

foreach(EXAMPLE_SRC ${EXAMPLE_SRCS})
    add_executable(${EXAMPLE_SRC} examples/${EXAMPLE_SRC}.cpp)
    target_include_directories(${EXAMPLE_SRC} PRIVATE include)
    target_link_libraries(${EXAMPLE_SRC} ${PROJECT_NAME})
endforeach()
endif()

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-config
)

install(EXPORT ${PROJECT_NAME}-config
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
)