cmake_minimum_required(VERSION 3.20)

set (CMAKE_CXX_STANDARD 20)

set (BOOST_ROOT "E:/Boost") # Путь к библиотеке Boost
set (Boost_NO_SYSTEM_PATHS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (BOOST_ALL_DYN_LINK OFF)
set(OPENSSL_ROOT_DIR "C:\\Program Files\\OpenSSL-Win64")

set(PostgreSQL_ADDITIONAL_VERSIONS "17")

set(SOURCE_FILES "main.cpp" 
				
				 "spider.h" "spider.cpp"
				 "ini_parser.h" "ini_parser.cpp" "parser_exceptions.h"
				 "data_base.h" "data_base.cpp" 
				 "http_client_async_ssl.cpp" "http_client_async.cpp" "root_certificates.hpp"
				 #"http_req.h" "http_req.cpp"
				 #"https_req.h" "https_req.cpp" "root_certificates.hpp"
				 "tasks_queue.h" "tasks_queue.cpp"				 
				 "html_parser.h" "html_parser.cpp"
								 
				 )

project(Spider)

add_subdirectory("E:/libpqxx-7.9.2" libpqxx-build)

find_package (Boost REQUIRED)

if (Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
endif ()

add_executable(Spider ${SOURCE_FILES})
target_compile_options(Spider PRIVATE /bigobj) #асинхронный вариант ssl не компилируется без поддержки bigobj

target_compile_features(Spider PRIVATE cxx_std_17)

find_package(OpenSSL REQUIRED)
#include_directories("openssl/x64-windows/include")
#include_directories(${OPENSSL_INCLUDE_DIR}) 

#set (READY_OPENSSL_LIBRARIES "openssl/x64-windows/lib") # Путь к библиотеке openssl
#target_link_libraries(Spider  ${READY_OPENSSL_LIBRARIES})
target_link_libraries(Spider PRIVATE ${OPENSSL_LIBRARIES})


#target_link_libraries(Spider PRIVATE pqxx)
target_link_libraries(Spider PRIVATE pqxx)
target_link_libraries(Spider PRIVATE ${Boost_LIBRARIES})


