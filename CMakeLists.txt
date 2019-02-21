cmake_minimum_required(VERSION 3.3.0) 

project(shared_ptr_with_log)
set(${PROJECT_NAME}_VERSION 0.0.1)

set("src_dir" ${CMAKE_CURRENT_LIST_DIR}/src)
set("${PROJECT_NAME}_HDR" "${src_dir}/shared_ptr_with_log.hpp")

install(FILES ${${PROJECT_NAME}_HDR} INCLUDES DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)

set("test_dir" ${CMAKE_CURRENT_LIST_DIR}/test)
set("${PROJECT_NAME}_TEST_SRC" "${test_dir}/test.cpp")

include_directories(${src_dir})
find_package(Boost)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
add_executable(${PROJECT_NAME}_test ${${PROJECT_NAME}_TEST_SRC})
