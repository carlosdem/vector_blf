find_program(CCCC_EXECUTABLE
  NAMES cccc cccc.exe
  PATHS
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\C and C++ Code Counter_is1;Inno Setup: App Path]/bin"
    "C:/Program Files/CCCC"
  DOC "C and C++ Code Counter (http://cccc.sourceforge.net)")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CCCC DEFAULT_MSG CCCC_EXECUTABLE)

mark_as_advanced(CCCC_EXECUTABLE)