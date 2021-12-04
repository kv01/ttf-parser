/*
* Example for ttf-parser
* Change font_path to the ttf font to test
*/

#include <stdio.h>
#include <cwchar>
#include <chrono>
#include <thread>

#define TTF_FONT_PARSER_IMPLEMENTATION
#include "../src/ttfParser.h"

//Change to a valid font location
const char* font_path = "test/fonts/cm-unicode/cmunrm.ttf";

void font_parsed(void* args, void* _font_data, int error) {
	if (error) {
		*(uint8_t*)args = error;
		printf("Unable to parse font %s\n", font_path);
	}
	else {
		TTFFontParser::FontData* font_data = (TTFFontParser::FontData*)_font_data;
		printf("Font: %s %s parsed\n", font_data->font_names.begin()->font_family.c_str(), font_data->font_names.begin()->font_style.c_str());
		printf("Number of glyphs: %d\n", uint32_t(font_data->glyphs.size()));

		//step through glyph geometry
		{
			for (const auto& glyph_iterator : font_data->glyphs) {
				uint32_t num_curves = 0, num_lines = 0;
				for (const auto& path_list : glyph_iterator.second.path_list) {
					for (const auto& geometry : path_list.geometry) {
						if (geometry.is_curve)
							num_curves++;
						else
							num_lines++;
					}
				}
				wprintf(L"glyph 0x%x %lc: %d quadratic curves and %d lines\n", glyph_iterator.first, glyph_iterator.first, num_curves, num_lines);
			}
		}

		*(uint8_t*)args = 1;
	}
}

int main() {
	uint8_t condition_variable = 0;

	TTFFontParser::FontData font_data;
	int8_t error = TTFFontParser::parse_file(font_path, &font_data, &font_parsed, &condition_variable);

	while (!condition_variable) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	getchar();

	return 0;
}
