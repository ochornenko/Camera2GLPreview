# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -DVK_USE_PLATFORM_ANDROID_KHR -DVK_NO_PROTOTYPES")

set(SRC_DIR src/main/cpp)
set(WRAPPER_DIR ${SRC_DIR}/vulkan_wrapper)

include_directories(${WRAPPER_DIR})

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             media-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ${SRC_DIR}/VideoRenderer.cpp
             ${SRC_DIR}/VideoRendererContext.cpp
             ${SRC_DIR}/VideoRendererJNI.cpp
             ${SRC_DIR}/CommonUtils.cpp
             ${SRC_DIR}/GLUtils.cpp
             ${SRC_DIR}/GLVideoRendererYUV420.cpp
             ${SRC_DIR}/GLVideoRendererYUV420Filter.cpp
             ${SRC_DIR}/VKUtils.cpp
             ${SRC_DIR}/VKVideoRendererYUV420.cpp
             ${SRC_DIR}/vulkan_wrapper/vulkan_wrapper.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

find_library( # Sets the name of the path variable.
            GLESv2-lib

            # Specifies the name of the NDK library that
            # you want CMake to locate.
            GLESv2 )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       media-lib
                       android
                       ${log-lib}
                       ${GLESv2-lib})

########     CDep package for shaderc    ######################
# pre-requirement:
#    in terminal, cd {project_dir}; ./cdep
# that will generate .cdep/modules/cdep-dependencies-config.cmake
# the next 2 lines will be able to pull it into the project
# shaderc binaries are saved on github:
#      https://github.com/ggfan/shaderc/releases
# CDep fetches the lib with config file in ${project_dir}/cdep.yml
# ******* Most Importantly, CDep full documentation is @:
#  https://github.com/google/cdep
get_filename_component(cdep-dependencies_DIR
        ${SRC_DIR}/../../../../.cdep/modules  ABSOLUTE)
find_package(cdep-dependencies REQUIRED)
add_all_cdep_dependencies(media-lib)
