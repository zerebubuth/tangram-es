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

using namespace Tangram;

std::string sceneFile = "scene.yaml";

std::shared_ptr<ClientGeoJsonSource> data_source;

int width = 800;
int height = 600;
OSMesaContext osmesa_context = nullptr;
unsigned char *osmesa_buffer = nullptr;

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

    // Create a new context
    osmesa_context = OSMesaCreateContext(OSMESA_RGBA, nullptr);
    if (!osmesa_context) {
      std::cerr << "Failed to create OSMesa context." << std::endl;
      exit(1);
    }

    // Allocate a new offscreen buffer
    osmesa_buffer = new unsigned char[4 * width * height];

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

  png::image<png::rgb_pixel> image(width, height);
  for (int y = 0; y < height; ++y) {
    auto &row = image[height - y - 1];

    for (int x = 0; x < width; ++x) {
      unsigned char *pixel = &osmesa_buffer[4 * (x + width * y)];
      row[x] = png::rgb_pixel(pixel[0], pixel[1], pixel[2]);
    }
  }
  image.write(file);
}

#define CHECK_GIF_ERROR(x) { if ((x) != GIF_OK) { std::cerr << "At " << __FILE__ << ":" << __LINE__ << ": "; PrintGifError(); abort(); } }

void osmesa_write_gif(const std::string &file) {
  static char netscape_app[] = "NETSCAPE2.0";
  static char netscape_data[] = {1, 0, 0};

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

  unsigned char *red = (unsigned char *)malloc(sizeof(unsigned char) * width * height);
  unsigned char *green = (unsigned char *)malloc(sizeof(unsigned char) * width * height);
  unsigned char *blue = (unsigned char *)malloc(sizeof(unsigned char) * width * height);

  assert(red != nullptr);
  assert(green != nullptr);
  assert(blue != nullptr);

  int num_frames = 10;
  for (int i = 0; i < num_frames; ++i) {
    std::cout << "rendering frame " << i << "/" << num_frames << std::endl;
    Tangram::update(i == 0 ? 0.0 : 1.0);
    Tangram::render();

    SavedImage *img = MakeSavedImage(gif, nullptr);
    int color_map_size = 256;
    ColorMapObject *output_color_map = MakeMapObject(color_map_size, nullptr);
    unsigned char *bits = (unsigned char *)malloc(sizeof(unsigned char) * width * height);

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

    MakeExtension(img, APPLICATION_EXT_FUNC_CODE);
    status = AddExtensionBlock(img, sizeof netscape_app, (unsigned char *)netscape_app);
    CHECK_GIF_ERROR(status);
    status = AddExtensionBlock(img, sizeof netscape_data, (unsigned char *)netscape_data);
    CHECK_GIF_ERROR(status);

    // TODO: see http://giflib.sourceforge.net/whatsinagif/animation_and_transparency.html
    // for information on how to encode the delay.

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        unsigned char *pixel = &osmesa_buffer[4 * (x + width * (height - y - 1))];
        int offset = x + y * width;
        red[offset] = pixel[0];
        green[offset] = pixel[1];
        blue[offset] = pixel[2];
      }
    }
    status = QuantizeBuffer(width, height, &output_color_map->ColorCount,
                            red, green, blue, bits, output_color_map->Colors);
    CHECK_GIF_ERROR(status);
    img->RasterBits = bits;

    if (gif->SColorMap == nullptr) {
      gif->SColorMap = MakeMapObject(output_color_map->ColorCount, output_color_map->Colors);
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
