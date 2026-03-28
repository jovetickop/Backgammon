
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was Config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set(double-conversion_known_comps static shared)
set(double-conversion_comp_static NO)
set(double-conversion_comp_shared NO)
foreach (double-conversion_comp IN LISTS ${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
    if (double-conversion_comp IN_LIST double-conversion_known_comps)
        set(double-conversion_comp_${double-conversion_comp} YES)
    else ()
        set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
            "double-conversion does not recognize component `${double-conversion_comp}`.")
        set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
        return()
    endif ()
endforeach ()

if (double-conversion_comp_static AND double-conversion_comp_shared)
    set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
        "double-conversion `static` and `shared` components are mutually exclusive.")
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
    return()
endif ()

set(double-conversion_static_targets "${CMAKE_CURRENT_LIST_DIR}/double-conversion-static-targets.cmake")
set(double-conversion_shared_targets "${CMAKE_CURRENT_LIST_DIR}/double-conversion-shared-targets.cmake")

macro(prj_load_targets type)
    if (NOT EXISTS "${double-conversion_${type}_targets}")
        set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
            "double-conversion `${type}` libraries were requested but not found.")
        set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
        return()
    endif ()
    include("${double-conversion_${type}_targets}")
endmacro()

if (double-conversion_comp_static)
    prj_load_targets(static)
elseif (double-conversion_comp_shared)
    prj_load_targets(shared)
elseif (DEFINED double-conversion_SHARED_LIBS AND double-conversion_SHARED_LIBS)
    prj_load_targets(shared)
elseif (DEFINED double-conversion_SHARED_LIBS AND NOT double-conversion_SHARED_LIBS)
    prj_load_targets(static)
elseif (BUILD_SHARED_LIBS)
    if (EXISTS "${double-conversion_shared_targets}")
        prj_load_targets(shared)
    else ()
        prj_load_targets(static)
    endif ()
else ()
    if (EXISTS "${double-conversion_static_targets}")
        prj_load_targets(static)
    else ()
        prj_load_targets(shared)
    endif ()
endif ()

check_required_components("double-conversion")
