cmake_minimum_required(VERSION 3.10)

project("$CACHE{FSR_VERSION}_unity_plugin_$CACHE{FSR_BACKEND}")

set(FSR_UNITY_PLUGIN "$CACHE{FSR_VERSION}_unity_plugin_$CACHE{FSR_BACKEND}")

file(GLOB src
${CMAKE_CURRENT_SOURCE_DIR}/fsrunityplugin.h
${CMAKE_CURRENT_SOURCE_DIR}/fsrunityplugin.cpp
${CMAKE_CURRENT_SOURCE_DIR}/device.h
${CMAKE_CURRENT_SOURCE_DIR}/device.cpp
${CMAKE_CURRENT_SOURCE_DIR}/device_$CACHE{FSR_BACKEND}.h
${CMAKE_CURRENT_SOURCE_DIR}/device_$CACHE{FSR_BACKEND}.cpp
${CMAKE_CURRENT_SOURCE_DIR}/$CACHE{FSR_VERSION}.h
${CMAKE_CURRENT_SOURCE_DIR}/$CACHE{FSR_VERSION}.cpp
)

add_library(${FSR_UNITY_PLUGIN} SHARED ${src})

target_include_directories(${FSR_UNITY_PLUGIN} PRIVATE 
$CACHE{UNITY_PLUGINAPI_INCLUDE_DIR}
$CACHE{FFX_FSR_API_INCLUDE_DIR}
)

target_link_directories(${FSR_UNITY_PLUGIN} PRIVATE
$CACHE{FFX_FSR_API_LIB_DIR}
)

set(FSR_BACKEND_DEF)
if(FSR_BACKEND STREQUAL "dx11")
	set(FSR_BACKEND_DEF "FSR_BACKEND_DX11")
elseif(FSR_BACKEND STREQUAL "dx12")
	set(FSR_BACKEND_DEF "FSR_BACKEND_DX12")
else()
	message(FATAL_ERROR "Unsupported FSR backend, must be one of: [dx11, dx12]")
endif()

set(FSR_VERSION_DEF)
if(FSR_VERSION STREQUAL "fsr2")
	set(FSR_VERSION_DEF "FSR_2")
	target_link_libraries(${FSR_UNITY_PLUGIN} PRIVATE
	dxguid
	debug "ffx_fsr2_api_x64d"
	debug "ffx_fsr2_api_$CACHE{FSR_BACKEND}_x64d"
	optimized "ffx_fsr2_api_x64"
	optimized "ffx_fsr2_api_$CACHE{FSR_BACKEND}_x64"
	)
elseif(FSR_VERSION STREQUAL "fsr3")
	set(FSR_VERSION_DEF "FSR_3")
	if(NOT FSR_BACKEND STREQUAL "dx12")
		message(FATAL_ERROR "Unsupported fsr3 backend, must be: dx12")
	endif()
	target_link_libraries(${FSR_UNITY_PLUGIN} PRIVATE
	dxguid
	debug "ffx_fsr3_x64d"
	debug "ffx_fsr3upscaler_x64d"
	debug "ffx_frameinterpolation_x64d"
	debug "ffx_opticalflow_x64d"
	debug "ffx_backend_$CACHE{FSR_BACKEND}_x64d"
	optimized "ffx_fsr3_x64"
	optimized "ffx_fsr3upscaler_x64"
	optimized "ffx_frameinterpolation_x64"
	optimized "ffx_opticalflow_x64"
	optimized "ffx_backend_$CACHE{FSR_BACKEND}_x64"
	)
elseif(FSR_VERSION STREQUAL "fsrapi")
	set(FSR_VERSION_DEF "FSR_API")
	if(NOT FSR_BACKEND STREQUAL "dx12")
		message(FATAL_ERROR "Unsupported fsrapi backend, must be: dx12")
	endif()
	target_link_libraries(${FSR_UNITY_PLUGIN} PRIVATE
	dxguid
	debug "amd_fidelityfx_$CACHE{FSR_BACKEND}d"
	optimized "amd_fidelityfx_$CACHE{FSR_BACKEND}"
	)
else()
	message(FATAL_ERROR "Unsupported FSR version")
endif()

target_compile_definitions(${FSR_UNITY_PLUGIN} PRIVATE
${FSR_BACKEND_DEF} ${FSR_VERSION_DEF}
)

if(NOT FSR_UNITY_PLUGIN_DST_DIR STREQUAL "")
add_custom_command(TARGET ${FSR_UNITY_PLUGIN} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${FSR_UNITY_PLUGIN}> ${FSR_UNITY_PLUGIN_DST_DIR}
)
endif()
