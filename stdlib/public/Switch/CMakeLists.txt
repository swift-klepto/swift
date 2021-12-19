add_swift_target_library(swiftSwitch ${SWIFT_STDLIB_LIBRARY_BUILD_TYPES} IS_SDK_OVERLAY
    "Switch.swift"
    "Pad.swift"
    "Result.swift"

    SWIFT_COMPILE_FLAGS
      ${SWIFT_RUNTIME_SWIFT_COMPILE_FLAGS}
      ${SWIFT_STANDARD_LIBRARY_SWIFT_FLAGS}
    LINK_FLAGS "${SWIFT_RUNTIME_SWIFT_LINK_FLAGS}"
    TARGET_SDKS LIBNX
    INSTALL_IN_COMPONENT sdk-overlay
    DEPENDS switch_modulemap)

set(switch_modulemap_target_list)
foreach(sdk ${SWIFT_SDKS})
  if(NOT "${sdk}" STREQUAL "LIBNX")
    continue()
  endif()

  foreach(arch ${SWIFT_SDK_${sdk}_ARCHITECTURES})
    set(arch_subdir "${SWIFT_SDK_${sdk}_LIB_SUBDIR}/${arch}")
    set(module_dir "${SWIFTLIB_DIR}/${arch_subdir}")
    set(module_dir_static "${SWIFTSTATICLIB_DIR}/${arch_subdir}")

    set(switch_modulemap_source "switch.modulemap.gyb")

    set(switch_modulemap_out "${module_dir}/switch.modulemap")
    set(switch_modulemap_out_static "${module_dir_static}/switch.modulemap")

    handle_gyb_source_single(switch_modulemap_target
        SOURCE "${switch_modulemap_source}"
        OUTPUT "${switch_modulemap_out}"
        FLAGS
            "-DCMAKE_SDK=${sdk}"
            "-DGLIBC_INCLUDE_PATH=${SWIFT_SDK_${sdk}_ARCH_${arch}_LIBC_INCLUDE_DIRECTORY}"
            "-DGLIBC_ARCH_INCLUDE_PATH=${SWIFT_SDK_${sdk}_ARCH_${arch}_LIBC_ARCHITECTURE_INCLUDE_DIRECTORY}"
            "-DLIBNX_INCLUDE_PATH=${SWIFT_LIBNX_DEVKITPRO_PATH}/libnx/include")

    list(APPEND switch_modulemap_target_list ${switch_modulemap_target})

    if(SWIFT_BUILD_STATIC_STDLIB)
        add_custom_command_target(
            copy_switch_modulemap_static
            COMMAND
            "${CMAKE_COMMAND}" "-E" "make_directory" ${module_dir_static}
            COMMAND
            "${CMAKE_COMMAND}" "-E" "copy" ${switch_modulemap_out} ${switch_modulemap_out_static}
            OUTPUT ${switch_modulemap_out_static}
            DEPENDS
            "${switch_modulemap_target}"
            COMMENT "Copying Switch modulemap to static resources")

        list(APPEND switch_modulemap_target_list ${copy_switch_modulemap_static})
    endif()

    # FIXME: When SDK is a cross-compile target (SDK != Host), the generated
    #        modulemap will be relative to the Host, with hardcoded paths.
    #        It is not relocatable to the target platform itself.
    #        This affects any cross-compiled targets that use switch.modulemap.

    swift_install_in_component(FILES "${switch_modulemap_out}"
                               DESTINATION "lib/swift/${arch_subdir}"
                               COMPONENT sdk-overlay)

    if(SWIFT_BUILD_STATIC_STDLIB)
      swift_install_in_component(FILES "${switch_modulemap_out}"
                                 DESTINATION "lib/swift_static/${arch_subdir}"
                                 COMPONENT sdk-overlay)
    endif()
  endforeach()
endforeach()

add_custom_target(switch_modulemap DEPENDS ${switch_modulemap_target_list})
set_property(TARGET switch_modulemap PROPERTY FOLDER "Miscellaneous")
add_dependencies(sdk-overlay switch_modulemap)
