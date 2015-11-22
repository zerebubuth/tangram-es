# find PngPP, and also libpng

if(PngPP_FIND_QUIETLY)
  set(_FIND_PNG_ARG QUIET)
endif()

find_package(PNG ${_FIND_PNG_ARG} REQUIRED)

find_path(PngPP_INCLUDE_DIR
  NAMES  png++/color.hpp  png++/config.hpp  png++/consumer.hpp  png++/convert_color_space.hpp
  png++/end_info.hpp  png++/error.hpp  png++/ga_pixel.hpp  png++/generator.hpp
  png++/gray_pixel.hpp  png++/image.hpp  png++/image_info.hpp  png++/index_pixel.hpp
  png++/info.hpp  png++/info_base.hpp  png++/io_base.hpp  png++/packed_pixel.hpp
  png++/palette.hpp  png++/pixel_buffer.hpp  png++/pixel_traits.hpp  png++/png.hpp
  png++/reader.hpp  png++/require_color_space.hpp  png++/rgb_pixel.hpp  png++/rgba_pixel.hpp
  png++/streaming_base.hpp  png++/tRNS.hpp  png++/types.hpp  png++/writer.hpp)

set(PngPP_INCLUDE_DIRS ${PngPP_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})
# PngPP is a header-only program, so no libraries of its own.
set(PngPP_LIBRARIES ${PNG_LIBRARIES})

find_package_handle_standard_args(PngPP DEFAULT_MSG PngPP_INCLUDE_DIR)

mark_as_advanced(PngPP_LIBRARY PngPP_INCLUDE_DIR)