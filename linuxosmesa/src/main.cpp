#include <curl/curl.h>
#include <memory>

#include "tangram.h"
#include "data/clientGeoJsonSource.h"
#include "platform_linuxosmesa.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>

#include <iostream>
#include <chrono>

#include <png++/png.hpp>
#include <gif_lib.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <proj_api.h>

namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;

std::shared_ptr<Tangram::ClientGeoJsonSource> data_source;

OSMesaContext osmesa_context = nullptr;
uint8_t *osmesa_buffer = nullptr;
static const int tile_width = 256, tile_height = 256;
static const double mercator_world_size = 40075016.68;
projPJ projection_3785 = nullptr, projection_4326 = nullptr;

int output_func(GifFileType *ptr, const GifByteType *bytes, int len) {
  std::ofstream &out = *((std::ofstream *)ptr->UserData);
  out.write((const char *)bytes, len);
  return len;
}

void cleanup_context() {
    // Destroy old context
    if (osmesa_context != nullptr) {
      OSMesaDestroyContext(osmesa_context);
      osmesa_context = nullptr;
    }

    // Destroy old buffer
    if (osmesa_buffer != nullptr) {
      delete [] osmesa_buffer;
      osmesa_buffer = nullptr;
    }

}

void transform_to_lonlat(int tx, int ty, int metatile_size, int z, double &lon, double &lat) {
  double x = mercator_world_size * ((double(tx) + 0.5 * double(metatile_size)) / double(1 << z) - 0.5);
  double y = mercator_world_size * (0.5 - (double(ty) + 0.5 * double(metatile_size)) / double(1 << z));

  int status = pj_transform(projection_3785, projection_4326, 1, 0, &x, &y, nullptr);
  if (status != 0) {
    std::cerr << "Projection error: " << pj_strerrno(status) << std::endl;
    exit(EXIT_FAILURE);
  }

  lon = x * RAD_TO_DEG;
  lat = y * RAD_TO_DEG;
}

void init_proj() {
  projection_4326 = pj_init_plus("+init=epsg:4326");
  if (projection_4326 == nullptr) {
    std::cerr << "Unable to initialise latlon projection: " << pj_strerrno(pj_errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  projection_3785 = pj_init_plus("+init=epsg:3785");
  if (projection_3785 == nullptr) {
    std::cerr << "Unable to initialise mercator projection: " << pj_strerrno(pj_errno) << std::endl;
    exit(EXIT_FAILURE);
  }
}

void cleanup_proj() {
  pj_free(projection_3785);
  pj_free(projection_4326);
}

void init_context(const std::string &scene_file, int z, int x, int y, int metatile_size) {
  // Setup tangram
  Tangram::initialize(scene_file.c_str());

  cleanup_context();

  // Create a new context (z, stencil, accum)
  osmesa_context = OSMesaCreateContextExt(OSMESA_RGBA, 16, 0, 0, nullptr);
  if (!osmesa_context) {
    std::cerr << "Failed to create OSMesa context." << std::endl;
    exit(1);
  }

  // Allocate a new offscreen buffer
  int width = tile_width * metatile_size;
  int height = tile_height * metatile_size;
  osmesa_buffer = new uint8_t[4 * width * height];

  // Make the context current and bind to buffer
  GLboolean status = OSMesaMakeCurrent(
    osmesa_context, osmesa_buffer, GL_UNSIGNED_BYTE,
    width, height);
  if (status != GL_TRUE) {
    std::cerr << "Unable to bind offscreen buffer." << std::endl;
    exit(1);
  }

  // find a point in the middle of the screen in lon/lat
  double lon = 0.0, lat = 0.0;
  transform_to_lonlat(x, y, metatile_size, z, lon, lat);

  // Setup graphics
  Tangram::setupGL();
  Tangram::resize(width, height);
  std::cout << "Setting z=" << z << ", center=" << lon << "," << lat << std::endl;
  Tangram::setZoom(z);
  Tangram::setPosition(lon, lat);

  data_source = std::make_shared<Tangram::ClientGeoJsonSource>("touch", "");
  Tangram::addDataSource(data_source);
}

void osmesa_write_png(const std::string &file, int width, int height) {
  Tangram::update(0.0);
  Tangram::render();
  // seems to be necessary for llvmpipe / later versions of OSMesa?
  // TODO: investigate why this might be!
  glFinish();

  png::image<png::rgb_pixel> image(width, height);
  for (int y = 0; y < height; ++y) {
    auto &row = image[height - y - 1];

    for (int x = 0; x < width; ++x) {
      uint8_t *pixel = &osmesa_buffer[4 * (x + width * y)];
      row[x] = png::rgb_pixel(pixel[0], pixel[1], pixel[2]);
    }
  }
  image.write(file);
}

#define CHECK_GIF_ERROR(x) { if ((x) != GIF_OK) { std::cerr << "At " << __FILE__ << ":" << __LINE__ << ": "; PrintGifError(); abort(); } }

enum class disposal_method : uint8_t {
  not_animated = 0,
  draw_on_top = 1,
  restore_background = 2,
  restore_previous = 3 // not widely supported?
};

constexpr uint8_t delay_bitfield(
  disposal_method disposal,
  bool wait_for_user_input,
  bool transparency_flag) {
  // bitfield: [RRRDDDUT]
  //   R = reserved, must be zero
  //   D = disposal method
  //   U = user input flag
  //   T = transparency flag
  return ((uint8_t)(disposal) << 2) |
    (wait_for_user_input ? 2 : 0) |
    (transparency_flag   ? 1 : 0);
}

void osmesa_write_gif(const std::string &file, int width, int height) {
  // see http://www.vurdalakov.net/misc/gif/netscape-looping-application-extension
  // for more detail.
  static uint8_t netscape_app[] = {
    'N', 'E', 'T', 'S', 'C', 'A', 'P' , 'E', // app identifier
    '2', '.', '0'                            // app auth code
  };
  static uint8_t netscape_data[] = {1, 0, 0};
  static uint16_t delay_time = 10;
  static uint8_t delay_data[] = {
    delay_bitfield(disposal_method::draw_on_top, false, false),
    uint8_t(delay_time & 0xff),
    uint8_t(delay_time >> 8),
    0
  };

  int status = 0;
  std::ofstream file_handle(file.c_str());
  GifFileType *gif = EGifOpen(&file_handle, &output_func);

  //status = EGifPutScreenDesc(gif, width, height, 256, 0, nullptr);
  //CHECK_GIF_ERROR(status);
  gif->SWidth = width;
  gif->SHeight = height;
  gif->SColorResolution = 8;
  gif->SBackGroundColor = 0;
  gif->SColorMap = nullptr;

  //status = EGifPutImageDesc(gif, 0, 0, width, height, 0, nullptr);
  //CHECK_GIF_ERROR(status);

  uint8_t *red = (uint8_t *)malloc(sizeof(uint8_t) * width * height);
  uint8_t *green = (uint8_t *)malloc(sizeof(uint8_t) * width * height);
  uint8_t *blue = (uint8_t *)malloc(sizeof(uint8_t) * width * height);

  assert(red != nullptr);
  assert(green != nullptr);
  assert(blue != nullptr);

  int num_frames = 10;
  for (int i = 0; i < num_frames; ++i) {
    std::cout << "rendering frame " << i << "/" << num_frames << std::endl;
    Tangram::update(i == 0 ? 0.001f : (float(delay_time) / 100.0f));
    Tangram::render();

    SavedImage *img = MakeSavedImage(gif, nullptr);
    int color_map_size = 256;
    ColorMapObject *output_color_map = MakeMapObject(color_map_size, nullptr);
    uint8_t *bits = (uint8_t *)malloc(sizeof(uint8_t) * width * height);

    assert(img != nullptr);
    assert(output_color_map != nullptr);
    assert(bits != nullptr);

    img->ImageDesc.Left = 0;
    img->ImageDesc.Top = 0;
    img->ImageDesc.Width = width;
    img->ImageDesc.Height = height;
    img->ImageDesc.Interlace = false;
    img->ImageDesc.ColorMap = output_color_map;
    img->ExtensionBlockCount = 0;
    img->ExtensionBlocks = nullptr;

    if (i == 0) {
      MakeExtension(img, APPLICATION_EXT_FUNC_CODE);
      status = AddExtensionBlock(img, sizeof netscape_app, netscape_app);
      CHECK_GIF_ERROR(status);
      MakeExtension(img, 0);
      status = AddExtensionBlock(img, sizeof netscape_data, netscape_data);
      CHECK_GIF_ERROR(status);
    }

    MakeExtension(img, GRAPHICS_EXT_FUNC_CODE);
    status = AddExtensionBlock(img, sizeof delay_data, delay_data);
    CHECK_GIF_ERROR(status);

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        uint8_t *pixel = &osmesa_buffer[4 * (x + width * (height - y - 1))];
        int offset = x + y * width;
        red[offset] = pixel[0];
        green[offset] = pixel[1];
        blue[offset] = pixel[2];
      }
    }
    // note: we throw this away? seems a little odd... but most of the images
    // will probably use quite a lot of colors anyway.
    int actual_colors_used = color_map_size;
    status = QuantizeBuffer(width, height, &actual_colors_used,
                            red, green, blue, bits, output_color_map->Colors);
    CHECK_GIF_ERROR(status);
    img->RasterBits = bits;

    if (gif->SColorMap == nullptr) {
      gif->SColorMap = MakeMapObject(color_map_size, output_color_map->Colors);
      assert(gif->SColorMap != nullptr);
    }
  }

  status = EGifSpew(gif);
  CHECK_GIF_ERROR(status);

  FreeSavedImages(gif);

  free(red);
  free(green);
  free(blue);
}

struct timer {
  timer() : m_start(std::chrono::steady_clock::now()) {
  }

  double getCurrentTime() {
    auto duration = std::chrono::steady_clock::now() - m_start;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0;
  }

private:
  std::chrono::time_point<std::chrono::steady_clock> m_start;
};

bool parse_zxy(const std::string &tile_coord, int &z, int &x, int &y) {
  int num_matched = ::sscanf(tile_coord.c_str(), "%d/%d/%d", &z, &x, &y);
  return num_matched == 3;
}

void osmesa_render_metatile() {
  Tangram::update(0.0);
  Tangram::render();
  // seems to be necessary for llvmpipe / later versions of OSMesa?
  // TODO: investigate why this might be!
  glFinish();
}

void write_single_tile_png(const std::string &file_name,
                           int tx, int ty, int metatile_size) {
  const int x_offset = tx * tile_width;
  const int y_offset = ty * tile_height;
  const int buffer_width = metatile_size * tile_width;

  png::image<png::rgb_pixel> image(tile_width, tile_height);
  for (int y = 0; y < tile_height; ++y) {
    auto &row = image[tile_height - y - 1];

    for (int x = 0; x < tile_width; ++x) {
      uint8_t *pixel = &osmesa_buffer[4 * (x + x_offset + buffer_width * (y + y_offset))];
      row[x] = png::rgb_pixel(pixel[0], pixel[1], pixel[2]);
    }
  }
  image.write(file_name);
}

void write_metatile_to_disk(const std::string &output,
                            int z, int origin_x, int origin_y,
                            int metatile_size) {
  for (int delta_y = 0; delta_y < metatile_size; ++delta_y) {
    for (int delta_x = 0; delta_x < metatile_size; ++delta_x) {
      int x = origin_x + delta_x;
      int y = origin_y + (metatile_size - delta_y - 1);

      std::ostringstream sdir, sfile;
      sdir << output << "/" << z << "/" << x;
      sfile << y << ".png";

      bfs::path dir(sdir.str()), file(sfile.str());
      bfs::create_directories(dir);

      write_single_tile_png((dir / file).native(), delta_x, delta_y, metatile_size);
    }
  }
}

// Main program
// ============

int main(int argc, char* argv[]) {
  std::string scene_file, tile_coord, output;
  int metatile_size = 8;

  bpo::options_description desc("Giflantro options");
  desc.add_options()
    ("help", "Show usage information")
    ("scene-file,f", bpo::value<std::string>(&scene_file)->default_value("scene.yaml"),
     "Scene file to load")
    ("coord,c", bpo::value<std::string>(&tile_coord),
     "Tile coordinate to render, should be 'z/x/y' with z, x & y all integers")
    ("metatile-size,m", bpo::value<int>(&metatile_size)->default_value(8),
     "Size of a metatile. Set to 1 to render only a single tile. Defaults to "
     "8, which is a good value for batch renders")
    ("output,o", bpo::value<std::string>(&output)->default_value("."),
     "Directory root to output tiles to.")
    ;

  bpo::variables_map vm;
  bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
  bpo::notify(vm);

  if (vm.count("help") > 0) {
    std::cout << desc << "\n";
    return EXIT_FAILURE;
  }

  if (!bfs::exists(scene_file)) {
    std::cerr << "Scene file \"" << scene_file << "\" not found." << std::endl;
    return EXIT_FAILURE;
  }

  int z, x, y;
  if (!parse_zxy(tile_coord, z, x, y)) {
    std::cerr << "Unable to parse \"" << tile_coord << "\" as 'z/x/y'." << std::endl;
    return EXIT_FAILURE;
  }
  if (z > 30) {
    std::cerr << "Zoom levels > 30 are not supported." << std::endl;
    return EXIT_FAILURE;
  }
  int max_coord = 1 << z;
  if ((x < 0) || (x >= max_coord)) {
    std::cerr << "X coordinate out of range, must be 0 <= x < "
              << max_coord << " at zoom " << z << "." << std::endl;
    return EXIT_FAILURE;
  }
  if ((y < 0) || (y >= max_coord)) {
    std::cerr << "Y coordinate out of range, must be 0 <= y < "
              << max_coord << " at zoom " << z << "." << std::endl;
    return EXIT_FAILURE;
  }

  // sanity check
  if (((x % metatile_size) > 0) ||
      ((y % metatile_size) > 0)) {
    std::cerr << "X & Y coordinates must be aligned to the upper left corner "
              << "of the metatile." << std::endl;
    return EXIT_FAILURE;
  }

  init_proj();

  init_context(scene_file, z, x, y, metatile_size);

  // Initialize cURL
  curl_global_init(CURL_GLOBAL_DEFAULT);

  setContinuousRendering(false);
  timer time;
  double last_time = time.getCurrentTime();

  do {
    processNetworkQueue();

    double current_time = time.getCurrentTime();
    double delta = current_time - last_time;
    last_time = current_time;

    Tangram::update(delta);
    usleep(100000);

  } while (tilesLoading());

  processNetworkQueue();

  //osmesa_write_gif("output.gif");
  //osmesa_write_png("output.png");

  osmesa_render_metatile();
  write_metatile_to_disk(output, z, x, y, metatile_size);

  curl_global_cleanup();
  cleanup_context();
  cleanup_proj();

  return 0;
}
