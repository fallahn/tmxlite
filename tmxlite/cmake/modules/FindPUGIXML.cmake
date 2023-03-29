find_path(PUGIXML_INCLUDE_DIR NAMES pugixml.hpp)
find_library(PUGIXML_LIBRARY NAMES pugixml)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PUGIXML DEFAULT_MSG
                                  PUGIXML_LIBRARY PUGIXML_INCLUDE_DIR)

mark_as_advanced(PUGIXML_INCLUDE_DIR
                 PUGIXML_LIBRARY)

