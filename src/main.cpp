#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <vector>
#include <algorithm>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cerr << "Failed to initialize FreeType library" << std::endl;
		return 1;
	}

	FT_Face face;
	if (FT_New_Face(ft, "/usr/share/fonts/TTF/Hack-Regular.ttf", 0, &face)) {
		std::cerr << "Failed to load font" << std::endl;
		return 1;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);  // font size (48px)

	// RENDERING
	// atlas dimensions
	const int WIDTH = 512;
	const int HEIGHT = 512;

	std::vector<unsigned char> atlas(WIDTH * HEIGHT, 0);

	int xOffset = 0;
	int yOffset = 0;
	int maxRowHeight = 0;

	// render each character
	for (unsigned char c = 32; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cerr << "Failed to laod glyph" << std::endl;
			continue;
		}

		FT_GlyphSlot g = face->glyph;

		if (xOffset + g->bitmap.width >= WIDTH) {
			xOffset = 0;
			yOffset += maxRowHeight;
			maxRowHeight = 0;
		}

		if (yOffset + g->bitmap.rows >= HEIGHT) {
			std::cerr << "Not enough space in the atlas" << std::endl;
			break;
		}

		for (int y = 0; y < g->bitmap.rows; y++) {
			for (int x = 0; x < g->bitmap.width; x++) {
				// write to atlas
				atlas[(y+yOffset) * WIDTH + (x+xOffset)] = g->bitmap.buffer[y * g->bitmap.width + x];
			}
		}

		xOffset += g->bitmap.width;
		maxRowHeight = std::max(maxRowHeight, static_cast<int>(g->bitmap.rows));
		std::cout << static_cast<int>(c) << " ";
	}
	std::cout << std::endl;

	// write image to PNG
	stbi_write_png("glyph-atlas.png", WIDTH, HEIGHT, 1, atlas.data(), WIDTH);


	// CLEANUP
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return 0;
}
