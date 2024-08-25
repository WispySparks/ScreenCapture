#ifndef ENCODER_HPP
#define ENCODER_HPP

#include <vector>

#include "wgc.hpp"

void WriteToFile(const std::wstring file, unsigned int outputWidth, unsigned int outputHeight,
                 const unsigned int fps, std::vector<Frame> frames);

#endif