#include <cstdint>
#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

#define RGBA 4

extern "C" {
  #define STB_IMAGE_IMPLEMENTATION
  #include "stb_image.h"
}

bool load_img(std::vector<uint8_t>& image, const std::string& filename, int& x, int& y)
{
  int n;
  uint8_t* data = stbi_load(filename.c_str(), &x, &y, &n, RGBA);

  if (data != nullptr)
  {
    image = std::vector<uint8_t>(data, data + x * y * RGBA);
  }

  stbi_image_free(data);
  return data != nullptr;
}

std::string get_char(const bool* pixels)
{
  uint16_t uval = 0x2800;
  std::string result;
 
  if (pixels[0]) uval += 0x1;
  if (pixels[1]) uval += 0x8;
  if (pixels[2]) uval += 0x2;
  if (pixels[3]) uval += 0x10;
  if (pixels[4]) uval += 0x4;
  if (pixels[5]) uval += 0x20;
  if (pixels[6]) uval += 0x40;
  if (pixels[7]) uval += 0x80;

  if (uval <= 0x7F)
  {
    result += static_cast<char>(uval);
  }
  else if (uval <= 0x7FF)
  {
    result += static_cast<char>(0xC0 | ((uval >> 6) & 0x1F));
    result += static_cast<char>(0x80 | (uval & 0x3F));
  }
  else
  {
    result += static_cast<char>(0xE0 | ((uval >> 12) & 0x0F));
    result += static_cast<char>(0x80 | ((uval >> 6) & 0x3F));
    result += static_cast<char>(0x80 | (uval & 0x3F));
  }

  return result;
}

int main(int argc, char** argv)
{
  int ascii_s, width, height;
  std::string img_path;
  std::vector<uint8_t> img_data;
  bool success;

  if (argc != 3)
  {
    std::cerr << "You must input an image to convert to ascii." << std::endl;
    return 1;
  }

  img_path = argv[1];
  ascii_s = std::stoi(argv[2]);

  if (ascii_s < 1 || ascii_s > 500 || ascii_s % 4 != 0)
  {
    std::cerr << "The ASCII size should be between 4 and 500 and should be a multiple of 4." << std::endl;
    return 1;
  }

  success = load_img(img_data, img_path, width, height);

  if (!success)
  {
    std::cerr << "Error loading image" << std::endl;
    return 1;
  }

  std::cout << static_cast<int>(img_data[0]) << " "
            << static_cast<int>(img_data[1]) << " "
            << static_cast<int>(img_data[2]) << " "
            << static_cast<int>(img_data[3])
            << std::endl;

  return 0;
}
