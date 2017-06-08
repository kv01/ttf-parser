# ttf-parser
A bare-bones, single header file ttf font parser written in C++ for fast GPU font rendering.

## How to use
* Define TTF_FONT_PARSER_IMPLEMENTATION in ONE cpp file to enable the implementation in the header file.  
* Use *parse_file* or *parse_data* to get a *FontData* structure with all font metrics and glyph data needed for rendering common fonts.
* Note that *parse_file* is currently synchronous except when compiled with emscripten but will still execute the callback

A glyph is represented as a set of triangles (p_x, p1, p2) where p_x is the center of the glyph and
p1 and p2 are sequential points on the curve. Quadratic splines will have 2 tiangles associated with them,
(p_x, p1, p2) as before and (p1, p_c, p2) where p_c is the spline control point.
