#include "streaming.hpp"
#include <iostream>
#include <cstring>

namespace nutc {
namespace streaming {

Streaming::Streaming(const std::string& filePath)
    : filePath(filePath), pipesCreated(false) {}

Streaming::~Streaming() {
    closePipes();
}

bool Streaming::createPipes() {
    if (pipe(pipeFileDescriptors) == -1) {
        std::cerr << "Failed to create pipes." << std::endl;
        return false;
    }
    pipesCreated = true;
    return true;
}

void Streaming::closePipes() {
    if (pipesCreated) {
        close(pipeFileDescriptors[0]);
        close(pipeFileDescriptors[1]);
        pipesCreated = false;
    }
}

bool Streaming::sendMessage(const std::string& message) {
    if (!pipesCreated) {
        std::cerr << "Pipes not created." << std::endl;
        return false;
    }

    if (write(pipeFileDescriptors[1], message.c_str(), message.size()) == -1) {
        std::cerr << "Failed to send message." << std::endl;
        return false;
    }
    return true;
}