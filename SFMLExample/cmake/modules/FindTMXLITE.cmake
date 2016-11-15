include(FindPackageHandleStandardArgs)

# Search for the header file
find_path(TMXLITE_INCLUDE_DIR tmxlite/Config.hpp PATH_SUFFIXES include)

# Search for the library
find_library(TMXLITE_LIBRARIES NAMES libtmxlite PATH_SUFFIXES lib)

# Did we find everything we need?
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libtmxlite DEFAULT_MSG TMXLITE_LIBRARIES TMXLITE_INCLUDE_DIR) 
