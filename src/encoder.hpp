#ifndef ENCODER_HPP
#define ENCODER_HPP

#include <vector>

#include "wgc.hpp"

void WriteToFile(const std::wstring file, const unsigned int fps, std::vector<Frame> frames);

#endif