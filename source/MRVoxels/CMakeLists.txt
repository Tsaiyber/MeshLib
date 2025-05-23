cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(CMAKE_CXX_STANDARD ${MR_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MRVoxels CXX)

option(MRVOXELS_NO_DICOM "Disable DICOM format support" OFF)
option(MRVOXELS_NO_TIFF "Disable TIFF format support" OFF)

IF(MR_EMSCRIPTEN)
  set(MRVOXELS_NO_TIFF ON)
ENDIF()

file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config_cmake.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config_cmake.h)

set(MRVOXELS_OPTIONAL_DEPENDENCIES "")

IF(NOT MRVOXELS_NO_DICOM)
  find_package(GDCM CONFIG REQUIRED)
  list(APPEND MRVOXELS_OPTIONAL_DEPENDENCIES gdcmIOD gdcmDICT gdcmDSED gdcmMEXD gdcmMSFF)
ENDIF()

IF(NOT MRVOXELS_NO_TIFF)
  IF(WIN32 OR MESHLIB_USE_VCPKG)
    find_package(TIFF REQUIRED)
    list(APPEND MRVOXELS_OPTIONAL_DEPENDENCIES TIFF::TIFF)
  ELSE()
    list(APPEND MRVOXELS_OPTIONAL_DEPENDENCIES tiff)
  ENDIF()
ENDIF()

IF(MR_EMSCRIPTEN)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    MRMesh
    openvdb
    jsoncpp
    spdlog
    tbb
    ${MRVOXELS_OPTIONAL_DEPENDENCIES}
  )
ELSEIF(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    MRMesh
    OpenVDB
    PkgConfig::jsoncpp
    fmt
    spdlog
    tbb
    ${MRVOXELS_OPTIONAL_DEPENDENCIES}
  )
ELSEIF(WIN32 OR MESHLIB_USE_VCPKG)
  find_package(OpenVDB REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    MRMesh
    OpenVDB::openvdb
    fmt::fmt
    spdlog::spdlog
    JsonCpp::JsonCpp
    TBB::tbb
    ${MRVOXELS_OPTIONAL_DEPENDENCIES}
  )
ELSE()
  target_link_libraries(${PROJECT_NAME} PRIVATE
    MRMesh
    openvdb
    fmt::fmt
    spdlog::spdlog
    JsonCpp::JsonCpp
    tbb
    ${MRVOXELS_OPTIONAL_DEPENDENCIES}
  )
ENDIF()

IF(MR_PCH)
  target_precompile_headers(${PROJECT_NAME} REUSE_FROM MRPch)
ENDIF()

set(CMAKE_INSTALL_PREFIX "/Library/Frameworks")
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  LIBRARY DESTINATION "${MR_MAIN_LIB_DIR}"
  ARCHIVE DESTINATION "${MR_MAIN_LIB_DIR}"
  RUNTIME DESTINATION "${MR_BIN_DIR}"
)

install(
  FILES ${HEADERS}
  DESTINATION "${MR_INCLUDE_DIR}/${PROJECT_NAME}"
)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake
  DESTINATION ${MR_CONFIG_DIR}
)

install(
  EXPORT ${PROJECT_NAME}
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE MeshLib::
  DESTINATION ${MR_CONFIG_DIR}
)
