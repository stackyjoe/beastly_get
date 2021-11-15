cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

project(beastly_get)

set(SOURCE_FILES main.cpp url_parser.cpp getter.cpp beastly_connection.cpp string_functions.cpp)
set(HEADER_FILES url_parser.hpp getter.hpp beastly_connection.hpp string_functions.hpp egg_timer.hpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set(SHARED_CXX_FLAGS "-DBOOST_ASIO_DISABLE_CONCEPTS")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SHARED_CXX_FLAGS} -g -Og -Wall -Werror -pedantic-errors ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${SHARED_CXX_FLAGS} -O2")

target_include_directories(${PROJECT_NAME} PUBLIC "/usr/local/include")
target_link_directories(${PROJECT_NAME} PUBLIC "/usr/local/lib")


#use Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.75.0 COMPONENTS)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} boost_context boost_coroutine boost_system)
endif()
#end Boost

#use pthreads
find_package (Threads)
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
#end pthreads

#use ssl, tls
find_package(OpenSSL REQUIRED)
target_link_libraries (${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
#end ssl, tls

#use fmt
find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} fmt::fmt)
#end fmt
