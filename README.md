# ttf-parser
A bare-bones, single header file ttf font parser for font rendering.

## How to use
* Define TTF_FONT_PARSER_IMPLEMENTATION in ONE cpp file to enable the implementation in the header file.  
* Use *parse_file* or *parse_data* to get a *FontData* structure with all font metrics and glyph data needed for rendering common fonts.
* *parse_file* is currently synchronous except when compiled with emscripten but will still execute the callback

Glyph geometry is a set of lines and quadratic curves
