find_path(_emu_root
          NAMES x86/include/emu_c_utils/memoryweb_x86.h
          HINTS $ENV{EMU_ROOT} $ENV{EMU_DIR} ${EMU_ROOT} ${EMU_DIR}
          )

find_library(_emu_lib
             NAMES libemu_c_utils.a
             HINTS ${_emu_root}/lib ${_emu_root}/lib64)

find_path(_emu_include_dir
          NAMES memoryweb.h
          HINTS ${_emu_root}/include)

if ((NOT ${_emu_root})
        OR (NOT ${_emu_lib})
        OR (NOT ${_emu_include_dir}))
  set(_fail_msg "Could NOT find EMU (set EMU_DIR or EMU_ROOT to point to install)")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EMU ${_fail_msg}
                                  _emu_root
                                  _emu_lib
                                  _emu_include_dir
                                  )

add_library(EMU::emu UNKNOWN IMPORTED)
set_target_properties(EMU::emu PROPERTIES
                      IMPORTED_LOCATION ${_emu_lib}
                      INTERFACE_INCLUDE_DIRECTORIES ${_emu_include_dir}
                      )
set(EMU_DIR ${_emu_root})

mark_as_advanced(
  _emu_library
  _emu_include_dir
)
