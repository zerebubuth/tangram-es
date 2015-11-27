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
#include <mcheck.h>

using namespace Tangram;

std::string sceneFile = "scene.yaml";

std::shared_ptr<ClientGeoJsonSource> data_source;

int width = 800;
int height = 600;
OSMesaContext osmesa_context = nullptr;
uint8_t *osmesa_buffer = nullptr;

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

void init_context() {

    // Setup tangram
    Tangram::initialize(sceneFile.c_str());

    cleanup_context();

    // Create a new context (z, stencil, accum)
    osmesa_context = OSMesaCreateContextExt(OSMESA_RGBA, 16, 16, 16, nullptr);
    if (!osmesa_context) {
      std::cerr << "Failed to create OSMesa context." << std::endl;
      exit(1);
    }

    // Allocate a new offscreen buffer
    osmesa_buffer = new uint8_t[4 * width * height];

    // Make the context current and bind to buffer
    GLboolean status = OSMesaMakeCurrent(
      osmesa_context, osmesa_buffer, GL_UNSIGNED_BYTE,
      width, height);
    if (status != GL_TRUE) {
      std::cerr << "Unable to bind offscreen buffer." << std::endl;
      exit(1);
    }

    // Setup graphics
    Tangram::setupGL();
    Tangram::resize(width, height);

    data_source = std::make_shared<ClientGeoJsonSource>("touch", "");
    Tangram::addDataSource(data_source);
}

void osmesa_write_png(const std::string &file) {
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

void osmesa_write_gif(const std::string &file) {
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
  //EGifCloseFile(gif);

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

// Main program
// ============

int main(int argc, char* argv[]) {

    int argi = 0;
    while (++argi < argc) {
        if (strcmp(argv[argi - 1], "-f") == 0) {
            sceneFile = std::string(argv[argi]);
            logMsg("File from command line: %s\n", argv[argi]);
            break;
        }
    }

    struct stat sb;
    if (stat(sceneFile.c_str(), &sb) == -1) {
        logMsg("scene file not found!");
        exit(EXIT_FAILURE);
    }

    init_context();

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

    osmesa_write_gif("output.gif");

    curl_global_cleanup();
    cleanup_context();
    return 0;
}
