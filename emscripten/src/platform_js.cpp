#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <string>

#include "platform.h"

static std::function<void(std::vector<char>&&, TileID, int)> s_networkCallback;
bool s_continuousRendering = true;

void logMsg(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    emscripten_log(EM_LOG_CONSOLE, args);
    va_end(args);
}

void setContinuousRendering(bool _isContinuous) {
    s_continuousRendering = _isContinuous;
}

bool isContinuousRendering() {
    return s_continuousRendering;
}

std::string stringFromResource(const char* _path) {
    std::string into;

    std::ifstream file;
    std::string buffer;

    file.open(_path);
    if(!file.is_open()) {
        logMsg("Failed to open file at path: %s\n", _path);
        return std::string();
    }

    while(!file.eof()) {
        getline(file, buffer);
        into += buffer + "\n";
    }

    file.close();
    return into;
}

unsigned char* bytesFromResource(const char* _path, unsigned int* _size) {
    std::ifstream resource(_path, std::ifstream::ate | std::ifstream::binary);

    if(!resource.is_open()) {
        logMsg("Failed to read file at path: %s\n", _path);
        *_size = 0;
        return nullptr;
    }

    *_size = resource.tellg();

    resource.seekg(std::ifstream::beg);

    char* cdata = (char*) malloc(sizeof(char) * (*_size));

    resource.read(cdata, *_size);
    resource.close();

    return reinterpret_cast<unsigned char *>(cdata);
}

bool startNetworkRequest(const std::string& _url, const TileID& _tileID, const int _dataSourceID) {
    //emscripten_async_wget_data(NULL, NULL, NULL, NULL);
}

void cancelNetworkRequest(const std::string& _url) {

}

void setNetworkRequestCallback(std::function<void(std::vector<char>&&, TileID, int)>&& _callback) {
    s_networkCallback = _callback;
}

void requestRender() {
    //glfwPostEmptyEvent();
}

