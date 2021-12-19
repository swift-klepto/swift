function(swift_libnx_tools_path path_var_name)
  set(${path_var_name} "${SWIFT_LIBNX_DEVKITPRO_PATH}/devkitA64/bin" PARENT_SCOPE)
endfunction ()

function (swift_libnx_includes var)
  execute_process(COMMAND
    bash -c "echo | ${SWIFT_LIBNX_DEVKITPRO_PATH}/devkitA64/bin/aarch64-none-elf-gcc -xc++ -E -Wp,-v - 2>&1 1>/dev/null \
      | grep '^ ' | sed 's/^ //g' | paste -sd\; -"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE includes)

  set(${var} ${includes} PARENT_SCOPE)
endfunction ()

function(swift_libnx_libgcc_cross_compile var)
  execute_process(COMMAND
    bash -c "${SWIFT_LIBNX_DEVKITPRO_PATH}/devkitA64/bin/aarch64-none-elf-gcc -print-search-dirs | grep libraries | sed 's/libraries: =//g' | sed 's/:/;/g'"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE paths)
  set(${var} ${paths} PARENT_SCOPE)
endfunction()
