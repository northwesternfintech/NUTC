#pragma once
// keep track of active users and account information
#include "actions.hpp"

#include <glaze/glaze.hpp>

#include <optional>
#include <string>
#include <fstream>
#include <unistd.h>



namespace nutc {
namespace streaming {

class Streaming {

public:
    Streaming(const std::string& filePath);
    ~Streaming();

    bool createPipes();
    void closePipes();
    bool sendMessage(const std::string& message);

private:
    std::string filePath;
    int pipeFileDescriptors[2]; // [0] for reading, [1] for writing
    bool pipesCreated;

    bool openFileDescriptor();
};

} // namespace streaming
} // namespace nutc