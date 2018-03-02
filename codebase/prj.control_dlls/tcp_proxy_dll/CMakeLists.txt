project(mozaik_control_dll)

find_package (Eigen3 3.3 REQUIRED NO_MODULE)
find_package (ZeroMQ 4.3 REQUIRED)
include_directories(
	${CMAKE_SOURCE_DIR}/prj.control_dlls/tcp_proxy_dll
	${CMAKE_SOURCE_DIR}/prj.corelibs
)

set(tcp_proxy_dll_HDR
	stdafx.h
)

set(tcp_proxy_dll_SRC
	src/tcp_proxy_dll.cpp
	src/tcp_proxy_dll.def
)

add_library(mozaik_control_dll SHARED ${tcp_proxy_dll_SRC})

target_link_libraries(tcp_proxy_dll robot Eigen3::Eigen libzmq)
set_property(TARGET tcp_proxy_dll PROPERTY FOLDER "prj.control_dlls")