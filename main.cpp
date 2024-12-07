#include <cstdint>
#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>

// Global variables
#define BRAILLE_U 0x2800
#define RGBA 4
#define R_HUE 0.2126
#define G_HUE 0.7152
#define B_HUE 0.0722
#define DEFAULT_THRESHOLD 200
#define DEFAULT_ASCII_SIZE 100
#define MAX_ASCII_SIZE 500

// Adding the image reader
extern "C" {
  #define STB_IMAGE_IMPLEMENTATION
  #include "stb_image.h"
}

// Load an image
bool load_img(std::vector<uint8_t>& image, const std::string& filename, int& x, int& y)
{
  int n;
  uint8_t* data = stbi_load(filename.c_str(), &x, &y, &n, RGBA);

  // Setting the image data
  if (data != nullptr)
  {
    image = std::vector<uint8_t>(data, data + x * y * RGBA);
  }

  stbi_image_free(data);
  return data != nullptr;
}

// Getting the global brightness of a pixel by grayscaling it
uint8_t get_pixel_brightness(const std::vector<uint8_t>& img, int x, int y, int width)
{
  uint8_t r = img[(y * width + x) * RGBA];
  uint8_t g = img[(y * width + x) * RGBA + 1];
  uint8_t b = img[(y * width + x) * RGBA + 2];

  return static_cast<uint8_t>(R_HUE * r + G_HUE * g + B_HUE * b);
}

// Telling if a pixel is invisible or not
bool is_pixel_visible(const std::vector<uint8_t>& img, int x, int y, int width)
{
  return img[(y * width + x) * RGBA + 3] > 0;
}

// Turning a 8 characters long array into it's corresponding braille character
std::string get_char(const bool* pixels)
{
  uint16_t uval = BRAILLE_U;
  std::string result;
 
  // Converting using the unicode ASCII Table
  if (pixels[0]) uval += 0x1;
  if (pixels[1]) uval += 0x8;
  if (pixels[2]) uval += 0x2;
  if (pixels[3]) uval += 0x10;
  if (pixels[4]) uval += 0x4;
  if (pixels[5]) uval += 0x20;
  if (pixels[6]) uval += 0x40;
  if (pixels[7]) uval += 0x80;

  // Converting the 16 bits result into a string
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
  // Initializing variables
  int ascii_s = DEFAULT_ASCII_SIZE, width, height, threshold = DEFAULT_THRESHOLD;
  bool success, invert = false;
  std::string img_path = "", output_path = "", result = "";
  std::vector<uint8_t> img_data;

  // Parsing arguments
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-i" || std::string(argv[i]) == "--invert")
    {
      invert = true;
    }
    else if (std::string(argv[i]) == "-s" || std::string(argv[i]) == "--size")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "Input error: You must input an ASCII size" << std::endl;
        return 1;
      }

      try
      {
        ascii_s = std::stoi(argv[++i]);
      }
      catch (const std::invalid_argument& e)
      {
        std::cerr << "Input error: Invalid ASCII size" << std::endl;
        return 1;
      }
    }
    else if (std::string(argv[i]) == "-t" || std::string(argv[i]) == "--threshold")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "Input error: You must input a threshold value" << std::endl;
        return 1;
      }

      try
      {
        threshold = std::stoi(argv[++i]);
      }
      catch (const std::invalid_argument& e)
      {
        std::cerr << "Input error: Invalid threshold value" << std::endl;
        return 1;
      }
    }
    else if (std::string(argv[i]) == "-o" || std::string(argv[i]) == "--output")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "Input error: You must input an output path" << std::endl;
        return 1;
      }

      output_path = argv[++i];
    }
    else if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
    {
      // Displaying the help message
      std::cout << "Usage: " << argv[0] << " <image> [options]" << std::endl;
      std::cout << "Options:" << std::endl;
      std::cout << "  -s, --size       Set the size of the ASCII image (default is " << DEFAULT_ASCII_SIZE << ")" << std::endl;
      std::cout << "  -i, --invert     Invert the image" << std::endl;
      std::cout << "  -t, --threshold  Set the threshold for the image between 0 and 255 (default is " << DEFAULT_THRESHOLD << ")" << std::endl;
      std::cout << "  -h, --help       Display this information" << std::endl;
      std::cout << "  -o, --output     Set the output path for the ASCII image" << std::endl;
      return 0;
    }
    else if (img_path.size() == 0)
    {
      img_path = argv[i];
    }
    else
    {
      try
      {
        ascii_s = std::stoi(argv[i]);
      }
      catch (const std::invalid_argument& e)
      {
        std::cerr << "Input error: Invalid ASCII size" << std::endl;
        return 1;
      }
    }
  }

  // Handling errors
  if (img_path == "")
  {
    std::cerr << "Input error: You must input an image to convert to ascii" << std::endl;
    return 1;
  }

  if (ascii_s < 1 || ascii_s > MAX_ASCII_SIZE || ascii_s % 4 != 0)
  {
    std::cerr << "Input error: The ASCII size should be between 4 and 500 and should be a multiple of 4" << std::endl;
    return 1;
  }

  if (threshold < 0 || threshold > 255)
  {
    std::cerr << "Input error: The threshold should be between 0 and 255" << std::endl;
    return 1;
  }

  // Loading the image
  success = load_img(img_data, img_path, width, height);

  // Returning an error if the image cannot be read
  if (!success)
  {
    std::cerr << "Input error: Error loading image" << std::endl;
    return 1;
  }

  // Initializing the response ASCII matrix
  bool image[ascii_s][ascii_s];

  for (int y = 0; y < ascii_s; y++)
  {
    for (int x = 0; x < ascii_s; x++)
    {
      int img_x = x * width / ascii_s;
      int img_y = y * height / ascii_s;

      bool visible = is_pixel_visible(img_data, img_x, img_y, width);
      uint8_t brightness = get_pixel_brightness(img_data, img_x, img_y, width);
      
      // Choosing if the pixel should be visible or not depending on the threshold and it's visibility
      image[y][x] = visible && (invert ? brightness > threshold : brightness < threshold);
    }
  }

  // Turning the pixel matrix into a printable string
  for (int y = 0; y < ascii_s; y += 4)
  {
    if (y > 0)
    {
      result += "\n";
    }

    for (int x = 0; x < ascii_s; x += 2)
    {
      bool pixels[8];

      for (int py = 0; py < 4; py++)
      {
        for (int px = 0; px < 2; px++)
        {
          pixels[py * 2 + px] = image[y + py][x + px];
        }
      }

      result += get_char(pixels);
    }
  }

  // Outputing the result depending on the '--output' argument
  if (output_path.size() > 0)
  {
    std::ofstream file(output_path);

    if (file.is_open())
    {
      file << result;
      file.close();
    }
    else
    {
      // Handling errors
      std::cerr << "Output error: Error writing to file" << std::endl;
      return 1;
    }
  }
  else
  {
    std::cout << result << std::endl;
  }

  return 0;
}
