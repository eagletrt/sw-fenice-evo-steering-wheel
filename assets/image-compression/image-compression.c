#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define INPUT_IMAGE_PATH "../shutdown.png"
#define OUTPUT_BIN_PATH "../shutdown_compressed.bin"
#define OUTPUT_IMAGE_PATH "../shutdown_compressed.png"

int main() {
  int width, height, channels;
  uint32_t *img =
      (uint32_t *)stbi_load(INPUT_IMAGE_PATH, &width, &height, &channels, 0);

  if (img == NULL) {
    printf("Error in loading the image\n");
    return 1;
  }

  uint32_t second_image_size = width * height;
  uint32_t *second_image =
      (uint32_t *)malloc(second_image_size * sizeof(uint32_t));

  printf("width: %d\n", width);
  printf("height: %d\n", height);
  printf("channels: %d\n", channels);
  printf("img[0]: %u\n", img[0]);

  for (size_t iindex = 0; iindex < second_image_size; ++iindex) {
    // printf("img[%zu]: %0X\n", iindex, img[iindex]);
    if (img[iindex] <= 0xFFF6F6F0) {
      second_image[iindex] = 0xFF000000;
    } else {
      second_image[iindex] = 0xFFFFFFFF;
    }
  }

  uint32_t compressed_size = second_image_size / 8;
  uint8_t *compressed = (uint8_t *)malloc(compressed_size);

  size_t ccounter = 0;
  size_t ibit = 0;
  size_t ibyte = 0;
  for (size_t iindex = 0; iindex < second_image_size; ++iindex) {
    if (ibit == 8) {
      compressed[ccounter] = ibyte;
      ccounter++;
      ibit = 0;
      ibyte = 0;
    }
    if (second_image[iindex] == 0xFF000000) {
      ibyte |= 1 << ibit;
    }
    ibit++;
  }

  FILE *file = fopen(OUTPUT_BIN_PATH, "wb+");
  fwrite(compressed, sizeof(uint8_t), compressed_size, file);

  uint32_t *image3 = (uint32_t *)malloc(second_image_size * sizeof(uint32_t));
  for (size_t iindex = 0; iindex < compressed_size; ++iindex) {
    for (size_t ibit = 0; ibit < 8; ++ibit) {
      if (compressed[iindex] & (1 << ibit)) {
        image3[iindex * 8 + ibit] = 0xFF000000;
      } else {
        image3[iindex * 8 + ibit] = 0xFFFFFFFF;
      }
    }
  }

  stbi_write_png(OUTPUT_IMAGE_PATH, width, height, channels, image3, 0);

  return 0;
}
