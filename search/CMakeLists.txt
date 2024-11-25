cmake_minimum_required(VERSION 3.20)

set (CMAKE_CXX_STANDARD 20)

set (BOOST_ROOT "D:/Netology/Libs/boost/boost_1_86_0/boost_1_86_0") # Путь к библиотеке Boost
set (Boost_NO_SYSTEM_PATHS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (BOOST_ALL_DYN_LINK OFF)


set(SOURCE_FILES "main.cpp" 
				
				 #"server.cpp"
				 "ini_parser.h" "ini_parser.cpp" "parser_exceptions.h"
				 "data_base.h" "data_base.cpp" 	
				 #"server_interface.h"  
				 "server_interface.cpp" 
				  )


project(Server)

add_subdirectory("libpqxx-7.8.1" libpqxx-build)


find_package (Boost REQUIRED)

if (Boost_FOUND)    
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
endif ()

add_executable(Server ${SOURCE_FILES})
target_compile_features(Server PRIVATE cxx_std_17)

target_link_libraries(Server PRIVATE pqxx)
target_link_libraries(Server PRIVATE ${Boost_LIBRARIES})


