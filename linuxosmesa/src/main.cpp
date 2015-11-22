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

using namespace Tangram;

std::string sceneFile = "scene.yaml";

std::shared_ptr<ClientGeoJsonSource> data_source;

int width = 800;
int height = 600;
OSMesaContext osmesa_context = nullptr;
unsigned char *osmesa_buffer = nullptr;

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
    Tangram::update(0.0);

    Tangram::render();

    osmesa_write_png("output.png");

    curl_global_cleanup();
    cleanup_context();
    return 0;
}
