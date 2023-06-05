cmake_minimum_required(VERSION 3.20)

project(gimg)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB sourcefiles ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(gimg_e ${sourcefiles})

include(FetchContent)


if (NOT TARGET vec3_static)
  FetchContent_Declare(vec3_static
    URL /home/ianertson/workspace/vec3
    )

  FetchContent_MakeAvailable(vec3_static)
endif()


  if (NOT TARGET webp)
  FetchContent_Declare(
    webp
    GIT_REPOSITORY https://chromium.googlesource.com/webm/libwebp
    GIT_TAG 0aa2736ac1ec97068fc41f2bdace65baa0be8c65
  )
  FetchContent_MakeAvailable(webp)
  include_directories(${webp_SOURCE_DIR}/src)
  endif()


set(gimg_deps
  webp
  webpdecoder
  jpeg
  png
  vec3_static
  cglm
  )





add_library(gimg SHARED ${sourcefiles})
add_library(gimg_static STATIC ${sourcefiles})

target_compile_options(gimg_e PRIVATE -g -Wall)
target_compile_options(gimg PRIVATE -Wall)
target_compile_options(gimg_static PRIVATE -Wall)

target_link_libraries(gimg PUBLIC ${gimg_deps})
target_link_libraries(gimg_e PUBLIC ${gimg_deps})
target_link_libraries(gimg_static PUBLIC ${gimg_deps})




target_include_directories(
  gimg_e
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_include_directories(
  gimg
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)



target_include_directories(
  gimg_static
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)
