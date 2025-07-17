cmake_minimum_required(VERSION 3.22)
project(typeracer C)

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)


include(FindPackageHandleStandardArgs)

set(_sdl_components SDL3 SDL3_ttf SDL3_image SDL3_mixer SDL3_net)
foreach(_comp IN LISTS _sdl_components)
    find_package(${_comp} CONFIG QUIET)
endforeach()

if(UNIX)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        foreach(_pc sdl3 sdl3-ttf sdl3-image sdl3-mixer sdl3-net)
            string(TOUPPER ${_pc} _pc_upper)
            if(NOT TARGET PkgConfig::${_pc_upper})
                pkg_check_modules(${_pc_upper} QUIET IMPORTED_TARGET ${_pc})
            endif()
        endforeach()
    endif()
endif()

if(APPLE)
    if(NOT TARGET SDL3::SDL3 AND EXISTS /opt/homebrew/include/SDL3)
        message(STATUS "Falling back to Homebrew SDL3 in /opt/homebrew")
        include_directories(/opt/homebrew/include)
        link_directories(/opt/homebrew/lib)
        set(_sdl_homebrew 1)
    endif()
endif()

# Gather any libraries we found
set(SDL_LIBS)
foreach(_comp IN LISTS _sdl_components)
    if(TARGET ${_comp}::${_comp})
        list(APPEND SDL_LIBS ${_comp}::${_comp})
    elseif(TARGET PkgConfig::${_comp})
        list(APPEND SDL_LIBS PkgConfig::${_comp})
    elseif(_sdl_homebrew)
        list(APPEND SDL_LIBS ${_comp})
    endif()
endforeach()

if(NOT SDL_LIBS)
    message(FATAL_ERROR "Could not locate SDL3 and the required extension libs. Please install them or set CMAKE_PREFIX_PATH.")
endif()


file(GLOB SRC_FILES
     src/*.c
     lib/src/*.c)

add_executable(app ${SRC_FILES})


target_link_libraries(app PRIVATE ${SDL_LIBS})


if(APPLE AND _sdl_homebrew)
    set_target_properties(app PROPERTIES
        INSTALL_RPATH "/opt/homebrew/lib")
endif() 