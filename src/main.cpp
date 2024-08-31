#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <vector>
#include <algorithm>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>

struct Metadata {
	unsigned char codepoint;
	int width;
	int height;
	int x;
	int y;
	int advance;
	float u0, v0;
	float u1, v1;
};

void generateAtlas(FT_Face face, const std::string& jsonPath) {
	// metadata vector
	std::vector<Metadata> allMetadata;

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
			std::cerr << "Failed to load glyph" << std::endl;
			continue;
		}

		FT_GlyphSlot g = face->glyph;

		Metadata metadata;
		metadata.codepoint = c;
		metadata.width = g->bitmap.width;
		metadata.height = g->bitmap.rows;
		metadata.x = g->bitmap_left;
		metadata.y = g->bitmap_top;
		metadata.advance = g->advance.x >> 6;

		if (xOffset + g->bitmap.width >= WIDTH) {
			xOffset = 0;
			yOffset += maxRowHeight;
			yOffset += 4;
			maxRowHeight = 0;
		}

		if (yOffset + g->bitmap.rows >= HEIGHT) {
			std::cerr << "Not enough space in the atlas" << std::endl;
			break;
		}

		metadata.u0 = static_cast<float>(xOffset) / WIDTH;
		metadata.u1 = static_cast<float>(xOffset + metadata.width) / WIDTH;
		metadata.v0 = static_cast<float>(yOffset) / WIDTH;
		metadata.v1 = static_cast<float>(yOffset + metadata.height) / WIDTH;

		allMetadata.push_back(metadata);

		for (int y = 0; y < g->bitmap.rows; y++) {
			for (int x = 0; x < g->bitmap.width; x++) {
				// write to atlas
				atlas[(y+yOffset) * WIDTH + (x+xOffset)] = g->bitmap.buffer[y * g->bitmap.width + x];
			}
		}

		xOffset += g->bitmap.width;
		xOffset += 4;
		maxRowHeight = std::max(maxRowHeight, static_cast<int>(g->bitmap.rows));
	}

	// write atlas to PNG
	stbi_write_png("glyphAtlas.png", WIDTH, HEIGHT, 1, atlas.data(), WIDTH);

	// write metadata to JSON
	nlohmann::json jsonOutput;
	for (const auto& glyph : allMetadata) {
		std::string letter;
		letter.push_back(glyph.codepoint);
		jsonOutput[letter] = {
			{"width", glyph.width},
			{"height", glyph.height},
			{"x", glyph.x},
			{"y", glyph.y},
			{"advance", glyph.advance},
			{"u0", glyph.u0},
			{"u1", glyph.u1},
			{"v0", glyph.v0},
			{"v1", glyph.v1}
		};
	}

	std::ofstream outFile(jsonPath);
	outFile << jsonOutput.dump(4);
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Required arguments: font, size" <<std::endl;
		return 1;
	}

	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cerr << "Failed to initialize FreeType library" << std::endl;
		return 1;
	}

	FT_Face face;
	if (FT_New_Face(ft, argv[1], 0, &face)) {
		std::cerr << "Failed to load font" << std::endl;
		return 1;
	}

	int pixelSize = std::stoi(argv[2]);

	FT_Set_Pixel_Sizes(face, 0, pixelSize);

	generateAtlas(face, "metadata.json");

	// CLEANUP
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return 0;
}
