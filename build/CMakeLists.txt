cmake_minimum_required(VERSION 3.12.2)

project(QWHttpServer)

set(CMAKE_VERBOSE_MAKEFILE on)



set(ROOT_DIR "${CMAKE_BINARY_DIR}/..")
set(CMAKE_SOURCE_DIR ${ROOT_DIR}/src)

find_package(ImageMagick COMPONENTS
  MagickWand
  MagickCore)

include_directories(
  ${CMAKE_SOURCE_DIR}/networking/include
  ${CMAKE_SOURCE_DIR}/core/include
  ${CMAKE_SOURCE_DIR}/data_struct/include
  ${CMAKE_SOURCE_DIR}/file_system/include
  ${CMAKE_SOURCE_DIR}/string_utils/include
  ${ImageMagick_INCLUDE_DIRS}
)
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

add_executable (main ${CMAKE_SOURCE_DIR}/main.c
	${CMAKE_SOURCE_DIR}/networking/inet_sockets.c
	${CMAKE_SOURCE_DIR}/networking/connection.c
	${CMAKE_SOURCE_DIR}/networking/http.c
	${CMAKE_SOURCE_DIR}/string_utils/string_utils.c
	${CMAKE_SOURCE_DIR}/file_system/file_system.c
	${CMAKE_SOURCE_DIR}/core/image_processing.c
	${CMAKE_SOURCE_DIR}/core/server.c
	${CMAKE_SOURCE_DIR}/core/threadwork.c
	${CMAKE_SOURCE_DIR}/core/config.c
	)

      # cmake flags
      foreach (_imageMagick_cflags ${PC_MagickWand_CFLAGS})
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_imageMagick_cflags}")
      endforeach()
target_link_libraries(main pthread magic ${ImageMagick_LIBRARIES})
      

