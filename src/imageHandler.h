#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int width, height, nrChannels;
unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);