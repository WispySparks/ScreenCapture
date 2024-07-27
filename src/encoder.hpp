#ifndef ENCODER_HPP
#define ENCODER_HPP

#include <vector>

#include "wgc.hpp"

void WriteToFile(std::wstring file, int fps, std::vector<Frame> frames);

#endif