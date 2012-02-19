cmake_minimum_required(VERSION 2.6)

set(VPVL2_VERSION_MAJOR 0)
set(VPVL2_VERSION_COMPAT 1)
set(VPVL2_VERSION_MINOR 0)

# set library version
set(VPVL2_VERSION "${VPVL2_VERSION_MAJOR}.${VPVL2_VERSION_COMPAT}.${VPVL2_VERSION_MINOR}")
set(VPVL2_VERSION_COMPATIBLE "${VPVL2_VERSION_MAJOR}.${VPVL2_VERSION_COMPAT}.0")

# project configuration
project(libvpvl2)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmx vpvl2_sources)
set(vpvl2_public_headers
    include/vpvl2/Common.h
    include/vpvl2/vpvl2.h
    include/vpvl2/pmx/Bone.h
    include/vpvl2/pmx/Joint.h
    include/vpvl2/pmx/Material.h
    include/vpvl2/pmx/Model.h
    include/vpvl2/pmx/Morph.h
    include/vpvl2/pmx/RigidBody.h
    include/vpvl2/pmx/Vertex.h
)
set(vpvl2_internal_headers
    include/vpvl2/internal/util.h
)

IF(NOT CMAKE_BUILD_TYPE)
 SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

# declarations of function
function(link_bullet target)
  OPTION(FIND_BULLET_BY_PKGCONFIG "Find Bullet Physics by pkg-config" OFF)
  if(FIND_BULLET_BY_PKGCONFIG)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(BULLET REQUIRED bullet)
  endif()
  # exists bullet in pkgconfig?
  if(BULLET_FOUND)
    include_directories(${BULLET_INCLUDE_DIRS})
    target_link_libraries(${target} ${BULLET_LIBRARIES})
    link_directories(${BULLET_LIBRARY_DIRS})
  else()
    get_build_type(BULLET_BUILD_TYPE)
    if(MSVC)
      set(BULLET_LIBRARY_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bullet/msvc-build/lib/${BULLET_BUILD_TYPE}")
    else()
      set(BULLET_LIBRARY_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bullet/${BULLET_BUILD_TYPE}/lib")
    endif()
    # find from MMDAI/bullet or environment variables
    find_path(BULLET_INCLUDE_DIRS btBulletDynamicsCommon.h PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../bullet/src" $ENV{BULLET_INCLUDE_DIR})
    find_library(BULLET_COLLISION_LIB BulletCollision PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_DYNAMICS_LIB BulletDynamics PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_SOFTBODY_LIB BulletSoftBody PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_LINEARMATH_LIB LinearMath PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    if(VPVL2_NO_BULLET)
      if(BULLET_INCLUDE_DIRS AND BULLET_LINEARMATH_LIB)
        include_directories(${BULLET_INCLUDE_DIRS})
        target_link_libraries(${target} ${BULLET_LINEARMATH_LIB})
      else()
        message(FATAL_ERROR "Required LinearMath (included in BulletPhysics) is not found.")
      endif()
    else()
      if(BULLET_INCLUDE_DIRS AND BULLET_COLLISION_LIB AND BULLET_DYNAMICS_LIB AND BULLET_SOFTBODY_LIB AND BULLET_LINEARMATH_LIB)
        include_directories(${BULLET_INCLUDE_DIRS})
        target_link_libraries(${target} ${BULLET_COLLISION_LIB} ${BULLET_DYNAMICS_LIB} ${BULLET_SOFTBODY_LIB} ${BULLET_LINEARMATH_LIB})
      else()
        message(FATAL_ERROR "Required BulletPhysics is not found.")
      endif()
    endif()
  endif()
endfunction()

function(link_assimp target)
  if(MSVC)
    get_build_type(ASSIMP_BUILD_TYPE)
    set(ASSIMP_LOCAL_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../assimp/code/${ASSIMP_BUILD_TYPE}")
  else()
    set(ASSIMP_LOCAL_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../assimp/lib")
  endif()
  set(ASSIMP_LOCAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../assimp/include")
  find_path(ASSIMP_INCLUDE_DIRS assimp.hpp PATHS ${ASSIMP_LOCAL_INCLUDE_DIR} $ENV{ASSIMP_INCLUDE_DIR})
  find_library(ASSIMP_LIBRARY assimp PATHS ${ASSIMP_LOCAL_LIBRARY_DIR} $ENV{ASSIMP_LIBRARY_DIR})
  if (ASSIMP_LIBRARY AND ASSIMP_LIBRARY)
    target_link_libraries(${target} ${ASSIMP_LIBRARY})
    include_directories(${ASSIMP_INCLUDE_DIRS})
  else()
    message(FATAL_ERROR "Required Open Asset Import Library is not found.")
  endif()
endfunction()

function(link_icu_or_iconv target)
  find_library(ICU_LIBRARY_IO icuio)
  find_library(ICU_LIBRARY_UC icuuc)
  find_path(ICU_INCLUDE_DIRS unicode/ucnv.h)
  if(ICU_LIBRARY_IO AND ICU_LIBRARY_UC AND ICU_INCLUDE_DIRS)
    add_definitions(-DVPVL2_HAS_ICU)
    target_link_libraries(${target} ${ICU_LIBRARY_IO} ${ICU_LIBRARY_UC})
    include_directories(${ICU_INCLUDE_DIRS})
  else()
    find_library(ICONV_LIBRARY iconv)
    if(ICONV_LIBRARY)
      add_definitions(-DVPVL2_HAS_ICONV)
      target_link_libraries(${target} ${ICONV_LIBRARY})
    endif()
  endif()
endfunction()

function(get_build_type var)
  string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
  if(MINGW)
    set(${var} "${CMAKE_BUILD_TYPE_TOLOWER}-mingw" PARENT_SCOPE)
  else()
    set(${var} ${CMAKE_BUILD_TYPE_TOLOWER} PARENT_SCOPE)
  endif()
endfunction()

# imported functions from Allegro5's cmake
function(append_lib_type_suffix var)
  string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
  if(CMAKE_BUILD_TYPE_TOLOWER STREQUAL "debug")
    set(${var} "${${var}}_debug" PARENT_SCOPE)
  endif()
  if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "profile")
    set(${var} "${${var}}_profile" PARENT_SCOPE)
  endif()
endfunction()

function(append_lib_linkage_suffix var)
  if(NOT BUILD_SHARED_LIBS)
    #set(${var} "${${var}}_static" PARENT_SCOPE)
  endif()
endfunction()
# end of imported

option(BUILD_SHARED_LIBS "Build Shared Libraries (default is OFF)" OFF)
if(BUILD_SHARED_LIBS)
  set(VPVL2_LIB_TYPE SHARED)
else()
  set(VPVL2_LIB_TYPE STATIC)
endif()

option(VPVL2_COORDINATE_OPENGL "Use OpenGL coordinate system (default is ON)" ON)
option(VPVL2_NO_BULLET "Build libvpvl2 without BulletPhysics except LinearMath (default is OFF)" OFF)

# intercept to add source
option(VPVL2_OPENGL_RENDERER "Include OpenGL renderer class (default is OFF)" OFF)
option(VPVL2_ENABLE_GLSL "Include OpenGL renderer class using GLSL shaders (default is ON)" ON)
option(VPVL2_ENABLE_NVIDIA_CG "Include OpenGL renderer class using NVIDIA Cg (default is OFF)" OFF)
option(VPVL2_ENABLE_OPENCL "Enable OpenCL acceleration (default is OFF)" OFF)
if(VPVL2_OPENGL_RENDERER)
  if(VPVL2_ENABLE_GLSL)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/gl2 vpvl2_gl_sources)
    list(APPEND vpvl2_public_headers include/vpvl2/gl2/Renderer.h)
    list(APPEND vpvl2_internal_headers include/vpvl2/internal/gl2.h)
    if(VPVL2_ENABLE_NVIDIA_CG)
      aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/cg vpvl2_gl_sources)
      list(APPEND vpvl2_public_headers include/vpvl2/cg/Renderer.h)
    endif()
  else()
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/gl vpvl2_gl_sources)
    list(APPEND vpvl2_public_headers include/vpvl2/gl/Renderer.h)
  endif()
  list(APPEND vpvl2_sources ${vpvl2_gl_sources})
  set(VPVL2_COORDINATE_OPENGL ON BOOL)
endif()

option(VPVL2_ENABLE_PROJECT "Include the class of loading and saving project files (default is OFF)" OFF)
if(VPVL2_ENABLE_PROJECT)
  list(APPEND vpvl2_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/project/Project.cc)
  list(APPEND vpvl2_public_headers include/vpvl2/Project.h)
endif()

add_library(vpvl2 ${VPVL2_LIB_TYPE} ${vpvl2_sources} ${vpvl2_public_headers} ${vpvl2_internal_headers})
set_target_properties(vpvl2 PROPERTIES VERSION ${VPVL2_VERSION} SOVERSION ${VPVL2_VERSION_COMPATIBLE})

# project include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# find Bullet Physics
link_bullet(vpvl2)

# find assimp library
option(VPVL2_LINK_ASSIMP "link against Open Asset Import Library (default is OFF)" OFF)
if(VPVL2_LINK_ASSIMP)
  link_assimp(vpvl2)
endif()

set(VPVL2_OUTPUT_NAME "vpvl2")
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
append_lib_type_suffix(VPVL2_OUTPUT_NAME)
append_lib_linkage_suffix(VPVL2_OUTPUT_NAME)
set_target_properties(vpvl2 PROPERTIES OUTPUT_NAME ${VPVL2_OUTPUT_NAME})

if(WIN32 AND BUILD_SHARED_LIBS)
  set_target_properties(vpvl2
      PROPERTIES PREFIX "" SUFFIX .${VPVL2_VERSION}.dll
      IMPORT_SUFFIX ${CMAKE_IMPORT_LIBRARY_SUFFIX})
endif()

# create as a framework if build on darwin environment
if(APPLE)
  if(BUILD_SHARED_LIBS AND FRAMEWORK)
    install(TARGETS vpvl2 DESTINATION .)
    set_target_properties(vpvl2 PROPERTIES FRAMEWORK true)
    set_target_properties(vpvl2 PROPERTIES PUBLIC_HEADER "${vpvl2_public_headers}")
  endif()
  set_target_properties(vpvl2 PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# link against Qt
option(VPVL2_LINK_QT "Link against Qt 4.8 (enabling VPVL2_OPENGL_RENDERER required, default is OFF)" OFF)
option(VPVL2_LINK_QT_WITH_OPENCV "Build a renderer program with recording feature using OpenCV (default is OFF)" OFF)
option(VPVL2_BUILD_QT_RENDERER "Build a renderer program using Qt 4.8 (enabling VPVL2_OPENGL_RENDERER required, default is OFF)" OFF)
if(VPVL2_OPENGL_RENDERER AND (VPVL2_BUILD_QT_RENDERER OR VPVL2_LINK_QT))
  find_package(Qt4 4.8 REQUIRED QtCore QtGui QtOpenGL)
  include(${QT_USE_FILE})
  target_link_libraries(vpvl2 ${QT_LIBRARIES})
  if(VPVL2_LINK_QT_WITH_OPENCV)
    find_package(OpenCV)
    set(vpvl2_qt_sources render/qt/extended.cc)
    add_executable(vpvl2_qt ${vpvl2_qt_sources} ${vpvl2_public_headers} ${vpvl2_internal_headers})
    target_link_libraries(vpvl2_qt ${OpenCV_LIBS})
  elseif(VPVL2_BUILD_QT_RENDERER)
    set(vpvl2_qt_sources render/qt/basic.cc)
    add_executable(vpvl2_qt ${vpvl2_qt_sources} ${vpvl2_public_headers} ${vpvl2_internal_headers})
    target_link_libraries(vpvl2_qt vpvl2 ${QT_LIBRARIES})
  endif()
endif()

# link against OpenGL
if(VPVL2_OPENGL_RENDERER)
  find_package(OpenGL REQUIRED)
  target_link_libraries(vpvl2 ${OPENGL_gl_LIBRARY})
  include_directories(${OPENGL_INCLUDE_DIR})
  if(VPVL2_ENABLE_NVIDIA_CG)
    find_library(Cg_LIBRARY Cg)
    find_path(Cg_INCLUDE_DIR cg.h)
    target_link_libraries(vpvl2 ${Cg_LIBRARY})
    include_directories(${Cg_INCLUDE_DIR})
  endif()
  if(VPVL2_ENABLE_OPENCL)
    find_library(OPENCL_LIBRARY OpenCL)
    if(APPLE)
      find_path(OPENCL_INCLUDE_DIR OpenCL/cl.h)
    else()
      find_path(OPENCL_INCLUDE_DIR CL/cl.h)
    endif()
    target_link_libraries(vpvl2 ${OPENCL_LIBRARY})
    include_directories(${OPENCL_INCLUDE_DIR})
  endif()
endif()

# link against libxml2
if(VPVL2_ENABLE_PROJECT)
  find_package(LibXml2 REQUIRED)
  target_link_libraries(vpvl2 ${LIBXML2_LIBRARIES})
  include_directories(${LIBXML2_INCLUDE_DIR})
endif()

# extra renderer program with SDL
option(VPVL2_BUILD_SDL "Build a renderer program using SDL to test rendering result (enabling VPVL2_OPENGL_RENDERER required, default is OFF)" OFF)
if(VPVL2_OPENGL_RENDERER AND VPVL2_BUILD_SDL)
  find_package(SDL REQUIRED)
  find_package(SDL_image REQUIRED)
  set(vpvl2_sdl_sources render/sdl/main.cc)
  if(APPLE)
    list(APPEND vpvl2_sdl_sources render/sdl/SDLMain.m render/sdl/SDLMain.h)
  endif()
  add_executable(vpvl2_sdl ${vpvl2_sdl_sources} ${vpvl2_public_headers} ${vpvl2_internal_headers})
  target_link_libraries(vpvl2_sdl vpvl2 ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})
  include_directories(${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR})
  link_icu_or_iconv(vpvl2_sdl)
endif()

option(VPVL2_BUILD_IOS "Build a library for iOS with workaround (default is OFF)" OFF)

# generate configuration
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2/config.h")
set(vpvl2_public_headers ${vpvl2_public_headers} ${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
if(NOT WIN32)
  set(PKGCONFIG_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig/" CACHE STRING "Base directory for pkgconfig files")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vpvl2.pc.in ${CMAKE_CURRENT_BINARY_DIR}/vpvl2.pc @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vpvl2.pc DESTINATION ${PKGCONFIG_INSTALL_PREFIX})
endif()

# install configuration
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")
  # disable _CRT_SECURE_NO_WARNINGS for surpressing warnings from vpvl2/Common.h
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
else()
  install(TARGETS vpvl2 DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2" DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
      PATTERN "*.h" PATTERN "config.h.in" EXCLUDE PATTERN "internal" EXCLUDE PATTERN ".svn" EXCLUDE)
endif()
