cmake_minimum_required(VERSION 3.18)
include(CMakePrintHelpers)
# Set app name.
project(bongobs-cat)

set(SDK_ROOT_PATH ${PROJECT_SOURCE_DIR})
set(FRAMEWORK_PATH ${SDK_ROOT_PATH}/CubismSdk/Framework)
set(THIRD_PARTY_PATH ${SDK_ROOT_PATH}/thirdParty)
set(STB_PATH ${THIRD_PARTY_PATH}/stb)
set(GLEW_PATH ${THIRD_PARTY_PATH}/glew)
set(GLFW_PATH ${THIRD_PARTY_PATH}/glfw)
set(RES_PATH ${SDK_ROOT_PATH}/Resources)

set(CORE_PATH ${SDK_ROOT_PATH}/CubismSdk/Core)
# Detect architecture.
set(COMPILER ${MSVC_TOOLSET_VERSION})
set(ARCH x86_64)
# Detect compiler.

if(MSVC_VERSION MATCHES 1800)
  # Visual Studio 2013
  set(COMPILER 120)
elseif(MSVC_VERSION MATCHES 1900)
  # Visual Studio 2015
  set(COMPILER 140)
elseif(MSVC_VERSION GREATER_EQUAL 1910 AND MSVC_VERSION LESS 1920)
  # Visual Studio 2017
  set(COMPILER 141)
elseif(MSVC_VERSION GREATER_EQUAL 1920 AND MSVC_VERSION LESS 1930)
  # Visual Studio 2019
  set(COMPILER 142)
elseif(MSVC)
  message(FATAL_ERROR "[${PROJECT_NAME}] Unsupported Visual C++ compiler used.")
else()
  message(FATAL_ERROR "[${PROJECT_NAME}] Unsupported compiler used.")
endif()
# Detect core crt.
if(CORE_CRL_MD)
  set(CRT MD)
else()
  set(CRT MT)
endif()

# Add Cubism Core.
# Import as static library.
add_library(Live2D::Cubism::Core STATIC IMPORTED)
target_include_directories(Live2D::Cubism::Core
  INTERFACE $<BUILD_INTERFACE:${CORE_PATH}/include>)
# Find library path.
set(CORE_LIB_SUFFIX ${CORE_PATH}/lib/windows/${ARCH}/${COMPILER})
set_target_properties(Live2D::Cubism::Core
  PROPERTIES
    IMPORTED_LOCATION_DEBUG
      ${CORE_LIB_SUFFIX}/Live2DCubismCore_${CRT}d.lib
    IMPORTED_LOCATION_RELWITHDEBINFO
      ${CORE_LIB_SUFFIX}/Live2DCubismCore_${CRT}d.lib
    IMPORTED_LOCATION_RELEASE
      ${CORE_LIB_SUFFIX}/Live2DCubismCore_${CRT}.lib
    IMPORTED_LOCATION_MINSIZEREL
      ${CORE_LIB_SUFFIX}/Live2DCubismCore_${CRT}.lib
)

# Add GLEW ,GLFW.
add_subdirectory(${GLEW_PATH}/build/cmake)
add_subdirectory(${GLFW_PATH})
set_target_properties(glew PROPERTIES FOLDER "plugins/${PROJECT_NAME}")
set_target_properties(glew_s PROPERTIES FOLDER "plugins/${PROJECT_NAME}")

# Specify Cubism Framework rendering.
set(FRAMEWORK_SOURCE OpenGL)
# Add Cubism Native Framework.
add_subdirectory(${FRAMEWORK_PATH} ${CMAKE_CURRENT_BINARY_DIR}/Framework)
# Add rendering definition to framework.
target_compile_definitions(Framework PUBLIC CSM_TARGET_WIN_GL)
# Add include path of GLEW to framework.
target_include_directories(Framework PUBLIC ${GLEW_PATH}/include)
# Link libraries to framework.
target_link_libraries(Framework PUBLIC Live2D::Cubism::Core glew_s)

set_target_properties(Framework PROPERTIES FOLDER "plugins/${PROJECT_NAME}")
# Find opengl libraries.
find_package(OpenGL REQUIRED)

file(GLOB sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/*.cpp)

add_library(${PROJECT_NAME} MODULE ${sources})

# Link libraries to app.
target_link_libraries(${PROJECT_NAME}
  PRIVATE
 	libobs
	${LIBFDK_LIBRARIES}
	Framework
	glfw
	${OPENGL_LIBRARIES}
)

# Specify include directories.
target_include_directories(${PROJECT_NAME} PRIVATE ${STB_PATH})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "plugins/${PROJECT_NAME}")
install_obs_plugin_with_data(${PROJECT_NAME} data)
