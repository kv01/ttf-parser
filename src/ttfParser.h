/*
* ttf-parser
*  A single header ttf parser
*  Reads the minimum information needed to render antialiased glyph geometry as fast as possible
*  Browser support using emscripten
*
*  A glyph is represented as a set of triangles (p_x, p1, p2) where p_x is the center of the glyph and
*  p1 and p2 are sequential points on the curve. Quadratic splines will have 2 tiangles associated with them,
*  (p_x, p1, p2) as before and (p1, p_c, p2) where p_c is the spline control point.
*
*  author: Kaushik Viswanathan <kaushik@ocutex.com>
*  https://github.com/kv01/ttf-parser
*/

#pragma once

#include <stdint.h>
#include <map>
#include <unordered_map>
#include <vector>
#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#include "emscripten/val.h"
#define TTFDEBUG_PRINT(...) {}
#else
#include <fstream>
#ifdef _DEBUG
#include <stdio.h>
#define TTFDEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define TTFDEBUG_PRINT(...) {}
#endif
#endif

namespace TTFFontParser {
	typedef void(*TTF_FONT_MEM_CPY)(void*, const char*);
#ifdef __cplusplus
	extern "C" {
#endif
		extern TTF_FONT_MEM_CPY get2b;
		extern TTF_FONT_MEM_CPY get4b;
		extern TTF_FONT_MEM_CPY get8b;
		extern uint32_t little_endian_test;
		extern bool endian_tested;

		extern void get4b_be(void* dst, const char* src);
		extern void get4b_le(void* dst, const char* src);
		extern void get8b_be(void* dst, const char* src);
		extern void get8b_le(void* dst, const char* src);
		extern void get2b_be(void* dst, const char* src);
		extern void get2b_le(void* dst, const char* src);
		extern void get1b(void* dst, const char* src);
		extern float to_2_14_float(int16_t value);
#ifdef __cplusplus
	}
#endif
		
	struct Flags {
		bool xDual;
		bool yDual;
		bool xShort;
		bool yShort;
		bool repeat;
		bool offCurve;
	};
	enum COMPOUND_GLYPH_FLAGS {
		ARG_1_AND_2_ARE_WORDS = 0x0001,
		ARGS_ARE_XY_VALUES = 0x0002,
		ROUND_XY_TO_GRID = 0x0004,
		WE_HAVE_A_SCALE = 0x0008,
		MORE_COMPONENTS = 0x0020,
		WE_HAVE_AN_X_AND_Y_SCALE = 0x0040,
		WE_HAVE_A_TWO_BY_TWO = 0x0080,
		WE_HAVE_INSTRUCTIONS = 0x0100,
		USE_MY_METRICS = 0x0200,
		OVERLAP_COMPOUND = 0x0400,
		SCALED_COMPONENT_OFFSET = 0x0800,
		UNSCALED_COMPONENT_OFFSET = 0x1000
	};
	struct TTFHeader
	{
		uint32_t version;
		uint16_t numTables;
		uint16_t searchRange;
		uint16_t entrySelector;
		uint16_t rangeShift;

		uint32_t parse(const char* data, uint32_t offset) {
			get4b(&version, data + offset); offset += sizeof(uint32_t);
			get2b(&numTables, data + offset); offset += sizeof(uint16_t);
			/*get2b(&searchRange, data + offset); offset += sizeof(uint16_t);
			get2b(&entrySelector, data + offset); offset += sizeof(uint16_t);
			get2b(&rangeShift, data + offset); offset += sizeof(uint16_t);*/
			offset += sizeof(uint16_t) * 3;
			return offset;
		}
	};
	struct TableEntry
	{
		uint32_t tag;
		char tagstr[5];
		uint32_t checkSum;
		uint32_t offsetPos;
		uint32_t length;

		uint32_t parse(const char* data, uint32_t offset) {
			get4b(&tag, data + offset); memcpy(tagstr, data + offset, sizeof(uint32_t)); tagstr[4] = 0; offset += sizeof(uint32_t);
			get4b(&checkSum, data + offset); offset += sizeof(uint32_t);
			get4b(&offsetPos, data + offset); offset += sizeof(uint32_t);
			get4b(&length, data + offset); offset += sizeof(uint32_t);
			return offset;
		}
	};
	struct HeadTable
	{
		float tableVersion;
		float fontRevision;
		uint32_t checkSumAdjustment;
		uint32_t magicNumber;//0x5F0F3CF5
		uint16_t flags;
		uint16_t unitsPerEm;
		long long createdDate;
		long long modifiedData;
		short xMin;
		short yMin;
		short xMax;
		short yMax;
		uint16_t macStyle;
		uint16_t lowestRecPPEM;
		short fontDirectionHintl;
		short indexToLocFormat;
		short glyphDataFormat;

		uint32_t parse(const char* data, uint32_t offset) {
			get4b(&tableVersion, data + offset); offset += sizeof(uint32_t);
			get4b(&fontRevision, data + offset); offset += sizeof(uint32_t);
			get4b(&checkSumAdjustment, data + offset); offset += sizeof(uint32_t);
			get4b(&magicNumber, data + offset); offset += sizeof(uint32_t);
			get2b(&flags, data + offset); offset += sizeof(uint16_t);
			get2b(&unitsPerEm, data + offset); offset += sizeof(uint16_t);
			get8b(&createdDate, data + offset); offset += sizeof(uint64_t);
			get8b(&modifiedData, data + offset); offset += sizeof(uint64_t);
			get2b(&xMin, data + offset); offset += sizeof(short);
			get2b(&yMin, data + offset); offset += sizeof(short);
			get2b(&xMax, data + offset); offset += sizeof(short);
			get2b(&yMax, data + offset); offset += sizeof(short);
			get2b(&macStyle, data + offset); offset += sizeof(uint16_t);
			get2b(&lowestRecPPEM, data + offset); offset += sizeof(uint16_t);
			get2b(&fontDirectionHintl, data + offset); offset += sizeof(short);
			get2b(&indexToLocFormat, data + offset); offset += sizeof(short);
			get2b(&glyphDataFormat, data + offset); offset += sizeof(short);
			return offset;
		}
	};
	struct MaximumProfile
	{
		float version;
		uint16_t numGlyphs;
		uint16_t maxPoints;
		uint16_t maxContours;
		uint16_t maxCompositePoints;
		uint16_t maxCompositeContours;
		uint16_t maxZones;
		uint16_t maxTwilightPoints;
		uint16_t maxStorage; 
		uint16_t maxFunctionDefs;
		uint16_t maxInstructionDefs;
		uint16_t maxStackElements;
		uint16_t maxSizeOfInstructions;
		uint16_t maxComponentElements;
		uint16_t maxComponentDepth;

		uint32_t parse(const char* data, uint32_t offset) {
			get4b(&version, data + offset); offset += sizeof(uint32_t);
			get2b(&numGlyphs, data + offset); offset += sizeof(uint16_t);
			get2b(&maxPoints, data + offset); offset += sizeof(uint16_t);
			get2b(&maxContours, data + offset); offset += sizeof(uint16_t);
			get2b(&maxCompositePoints, data + offset); offset += sizeof(uint16_t);
			get2b(&maxCompositeContours, data + offset); offset += sizeof(uint16_t);
			get2b(&maxZones, data + offset); offset += sizeof(uint16_t);
			get2b(&maxTwilightPoints, data + offset); offset += sizeof(uint16_t);
			get2b(&maxStorage, data + offset); offset += sizeof(uint16_t);
			get2b(&maxFunctionDefs, data + offset); offset += sizeof(uint16_t);
			get2b(&maxInstructionDefs, data + offset); offset += sizeof(uint16_t);
			get2b(&maxStackElements, data + offset); offset += sizeof(uint16_t);
			get2b(&maxSizeOfInstructions, data + offset); offset += sizeof(uint16_t);
			get2b(&maxComponentElements, data + offset); offset += sizeof(uint16_t);
			get2b(&maxComponentDepth, data + offset); offset += sizeof(uint16_t);
			return offset;
		}
	};
	struct NameValue {
		uint16_t platformID;
		uint16_t encodingID;
		uint16_t languageID;
		uint16_t nameID;
		uint16_t length;
		uint16_t offset_value;

		uint32_t parse(const char* data, uint32_t offset) {
			get2b(&platformID, data + offset); offset += sizeof(uint16_t);
			get2b(&encodingID, data + offset); offset += sizeof(uint16_t);
			get2b(&languageID, data + offset); offset += sizeof(uint16_t);
			get2b(&nameID, data + offset); offset += sizeof(uint16_t);
			get2b(&length, data + offset); offset += sizeof(uint16_t);
			get2b(&offset_value, data + offset); offset += sizeof(uint16_t);
			return offset;
		}
	};
	struct NameTable {
		uint16_t format;
		uint16_t count;
		uint16_t stringOffset;
		std::vector<NameValue> nameRecord;

		uint32_t parse(const char* data, uint32_t offset, std::string* names, uint16_t max_number_of_names = 25) {
			uint32_t offset_start = offset;
			get2b(&format, data + offset); offset += sizeof(uint16_t);
			get2b(&count, data + offset); offset += sizeof(uint16_t);
			get2b(&stringOffset, data + offset); offset += sizeof(uint16_t);
			nameRecord.resize(count);
			for (auto i = 0; i < count; i++) {
				if (nameRecord[i].nameID > max_number_of_names)
					continue;
				offset = nameRecord[i].parse(data, offset);
				char* new_name_string = new char[nameRecord[i].length];
				memcpy(new_name_string, data + offset_start + stringOffset + nameRecord[i].offset_value, sizeof(char) * nameRecord[i].length);
				uint16_t string_length = nameRecord[i].length;
				if (new_name_string[0] == 0) {
					string_length = string_length >> 1;
					for (auto j = 0; j < string_length; j++) {
						new_name_string[j] = new_name_string[j * 2 + 1];
					}
				}
				names[nameRecord[i].nameID].assign(new_name_string, string_length);
				delete[] new_name_string;
			}
			return offset;
		}
	};
	struct HHEATable {
		uint16_t majorVersion;
		uint16_t minorVersion;
		int16_t	Ascender;
		int16_t	Descender;
		int16_t	LineGap;
		uint16_t advanceWidthMax;
		int16_t	minLeftSideBearing;
		int16_t	minRightSideBearing;
		int16_t	xMaxExtent;
		int16_t	caretSlopeRise;
		int16_t	caretSlopeRun;
		int16_t	caretOffset;
		int16_t	metricDataFormat;
		uint16_t numberOfHMetrics;

		uint32_t parse(const char* data, uint32_t offset) {
			get2b(&majorVersion, data + offset); offset += sizeof(uint16_t);
			get2b(&minorVersion, data + offset); offset += sizeof(uint16_t);
			get2b(&Ascender, data + offset); offset += sizeof(int16_t);
			get2b(&Descender, data + offset); offset += sizeof(int16_t);
			get2b(&LineGap, data + offset); offset += sizeof(int16_t);
			get2b(&advanceWidthMax, data + offset); offset += sizeof(uint16_t);
			get2b(&minLeftSideBearing, data + offset); offset += sizeof(int16_t);
			get2b(&minRightSideBearing, data + offset); offset += sizeof(int16_t);
			get2b(&xMaxExtent, data + offset); offset += sizeof(int16_t);
			get2b(&caretSlopeRise, data + offset); offset += sizeof(int16_t);
			get2b(&caretSlopeRun, data + offset); offset += sizeof(int16_t);
			get2b(&caretOffset, data + offset); offset += sizeof(int16_t);
			offset += sizeof(int16_t) * 4;
			get2b(&metricDataFormat, data + offset); offset += sizeof(int16_t);
			get2b(&numberOfHMetrics, data + offset); offset += sizeof(uint16_t);
			return offset;
		}
	};
	
	//Font structures
	struct float_v2
	{
		float x, y;
	};
	struct float_v4
	{
		float data[4];
	};
	struct int16_v2
	{
		int16_t x, y;
	};
	struct Curve
	{
		float_v2 p0;
		float_v2 p1;//Bezier control point or random off glyph point
		float_v2 p2;
		bool is_curve;
	};
	struct Path {
		std::vector<Curve> curves;
	};
	struct Glyph {
		uint32_t character;
		int16_t glyph_index;
		int16_t num_contours;
		std::vector<Path> path_list;
		uint16_t advance_width;
		int16_t left_side_bearing;
		int16_t bounding_box[4];
		uint32_t num_triangles;
	};
	struct FontMetaData {
		uint16_t unitsPerEm;
		int16_t Ascender;
		int16_t Descender;
		int16_t LineGap;
	};
	struct FontData {
		uint32_t file_name_hash;
		std::string full_font_name;
		std::string name_table[25];
		std::unordered_map<uint32_t, int16_t> kearning_table;
		std::unordered_map<uint16_t, Glyph> glyphs;
		std::map<uint32_t, uint16_t> glyph_map;
		FontMetaData meta_data;
		uint64_t last_used;
	};
	struct FontLineInfoData {
		uint32_t string_start_index;
		uint32_t string_end_index;
		float_v2 offset_start;
		float_v2 offset_end;
		std::vector<Glyph*> glyph_index;
	};
	struct FontPositioningOutput {
		std::vector<FontLineInfoData> line_positions;
		uint32_t num_triangles;
		//PixelPositioning alignment;
		//BoundingRect bounding_rect;
		uint32_t geometry;
		uint16_t font_size;
	};
	struct FontPositioningOptions {
		bool is_multiline;
		bool is_word_preserve;
		float line_height;
		//PixelPositioning alignment;
		//BoundingRect bounding_rect;
		FontPositioningOptions() {
			is_multiline = true;
			is_word_preserve = true;
			line_height = 1.0f;
		}
	};

	//For async file read
	typedef void(*TTF_FONT_PARSER_CALLBACK)(void*, void*, int);
	struct FileAccessDataPack {
		TTF_FONT_PARSER_CALLBACK callback;
		FontData* font_data;
		void* args;
	};

	//Function definitions
#ifdef __cplusplus
	extern "C" {
#endif
		extern int8_t parse_file(const char* file_name, FontData* font_data, TTF_FONT_PARSER_CALLBACK callback, void* args);
		extern int8_t parse_data(const char* data, FontData* font_data);
		extern int16_t get_kearning_offset(FontData* font_data, uint16_t left_glyph, uint16_t right_glyph);
#ifdef __cplusplus
	}
#endif
};

#ifdef TTF_FONT_PARSER_IMPLEMENTATION
namespace TTFFontParser {
	TTF_FONT_MEM_CPY get2b = get2b_le;
	TTF_FONT_MEM_CPY get4b = get4b_le;
	TTF_FONT_MEM_CPY get8b = get8b_le;
	uint32_t little_endian_test = 0x01234567;
	bool endian_tested = false;
};

void ttfparser_recv_file_async_callback(void* args, void* data, int length) {
	TTFFontParser::FileAccessDataPack* data_pack = (TTFFontParser::FileAccessDataPack*)args;

	if (length <= 0)
		data_pack->callback(data_pack->args, data_pack->font_data, -1);
	else {
		int parse_error = TTFFontParser::parse_data((char*)data, data_pack->font_data);
		data_pack->callback(data_pack->args, data_pack->font_data, parse_error);
	}
	delete data_pack;
}
void ttfparser_recv_file_async_error_callback(void* arg) {
	TTFFontParser::FileAccessDataPack* data_pack = (TTFFontParser::FileAccessDataPack*)arg;
	data_pack->callback(data_pack->args, data_pack->font_data, -1);
	delete data_pack;
}

//Copying functions for big and little endian
void TTFFontParser::get4b_be(void* dst, const char* src) {
	((uint8_t*)dst)[0] = src[0];
	((uint8_t*)dst)[1] = src[1];
	((uint8_t*)dst)[2] = src[2];
	((uint8_t*)dst)[3] = src[3];
}
void TTFFontParser::get4b_le(void* dst, const char* src) {
	((uint8_t*)dst)[0] = src[3];
	((uint8_t*)dst)[1] = src[2];
	((uint8_t*)dst)[2] = src[1];
	((uint8_t*)dst)[3] = src[0];
}
void TTFFontParser::get8b_be(void * dst, const char * src)
{
	((uint8_t*)dst)[0] = src[0];
	((uint8_t*)dst)[1] = src[1];
	((uint8_t*)dst)[2] = src[2];
	((uint8_t*)dst)[3] = src[3];
	((uint8_t*)dst)[4] = src[4];
	((uint8_t*)dst)[5] = src[5];
	((uint8_t*)dst)[6] = src[6];
	((uint8_t*)dst)[7] = src[7];
}
void TTFFontParser::get8b_le(void * dst, const char * src)
{
	((uint8_t*)dst)[0] = src[7];
	((uint8_t*)dst)[1] = src[6];
	((uint8_t*)dst)[2] = src[5];
	((uint8_t*)dst)[3] = src[4];
	((uint8_t*)dst)[4] = src[3];
	((uint8_t*)dst)[5] = src[2];
	((uint8_t*)dst)[6] = src[1];
	((uint8_t*)dst)[7] = src[0];
}
void TTFFontParser::get2b_be(void* dst, const char* src) {
	((uint8_t*)dst)[0] = src[0];
	((uint8_t*)dst)[1] = src[1];
}
void TTFFontParser::get2b_le(void* dst, const char* src) {
	((uint8_t*)dst)[0] = src[1];
	((uint8_t*)dst)[1] = src[0];
}
void TTFFontParser::get1b(void * dst, const char * src)
{
	((uint8_t*)dst)[0] = src[0];
}
float TTFFontParser::to_2_14_float(int16_t value)
{
	return (float(value & 0x3fff) / float(1 << 14)) + (-2 * ((value >> 15) & 0x1) + ((value >> 14) & 0x1));
}

int8_t TTFFontParser::parse_file(const char* file_name, TTFFontParser::FontData* font_data, TTFFontParser::TTF_FONT_PARSER_CALLBACK callback, void* args) {
#ifdef __EMSCRIPTEN__
	FileAccessDataPack* data_pack = new FileAccessDataPack();
	data_pack->font_data = font_data;
	data_pack->callback = callback;
	data_pack->args = args;
	emscripten_async_wget_data(file_name, data_pack, ttfparser_recv_file_async_callback, ttfparser_recv_file_async_error_callback);
	return 0;
#else
	std::string data_str;
	std::ifstream file(file_name, std::ifstream::binary);
	if (file) {
		file.seekg(0, std::ios::end);
		data_str.reserve(file.tellg());
		file.seekg(0, std::ios::beg);
		data_str.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
	}
	else {
		callback(args, font_data, -1);
		return -1;
	}

	int error = parse_data(data_str.data(), font_data);
	callback(args, font_data, error);
	return error;
#endif
}

/*
* Parse a ttf font and output glyph data into FontData
*/
int8_t TTFFontParser::parse_data(const char* data, TTFFontParser::FontData* font_data) {
	if (endian_tested == false) {
		if (((*((uint8_t*)(&TTFFontParser::little_endian_test))) == 0x67) == true) {
			TTFFontParser::get2b = TTFFontParser::get2b_le;
			TTFFontParser::get4b = TTFFontParser::get4b_le;
			TTFFontParser::get8b = TTFFontParser::get8b_le;
		}
		else {
			TTFFontParser::get2b = TTFFontParser::get2b_be;
			TTFFontParser::get4b = TTFFontParser::get4b_be;
			TTFFontParser::get8b = TTFFontParser::get8b_be;
		}
		endian_tested = true;
	}

	uint32_t ptr = 0;
	TTFHeader header;
	ptr = header.parse(data, ptr);
	std::unordered_map<std::string, TableEntry> table_map;
	for (uint16_t i = 0; i < header.numTables; i++)
	{
		TableEntry te;
		ptr = te.parse(data, ptr);
		table_map[te.tagstr] = te;
	}
	auto head_table_entry = table_map.find("head");
	if (head_table_entry == table_map.end())
		return -2;
	HeadTable head_table;
	ptr = head_table.parse(data, head_table_entry->second.offsetPos);
	auto maxp_table_entry = table_map.find("maxp");
	if (maxp_table_entry == table_map.end())
		return -2;
	MaximumProfile max_profile;
	max_profile.parse(data, maxp_table_entry->second.offsetPos);
	auto name_table_entry = table_map.find("name");
	if (name_table_entry == table_map.end())
		return -2;
	NameTable name_table;
	name_table.parse(data, name_table_entry->second.offsetPos, font_data->name_table);

	font_data->full_font_name = font_data->name_table[1] + std::string(std::move(" ")) + font_data->name_table[2];

	auto loca_table_entry = table_map.find("loca");
	if (loca_table_entry == table_map.end())
		return -2;
	std::vector<uint32_t> glyph_index(max_profile.numGlyphs);
	uint32_t end_of_glyf = 0;
	if (head_table.indexToLocFormat == 0) {
		uint32_t byte_offset = loca_table_entry->second.offsetPos;
		for (uint16_t i = 0; i < max_profile.numGlyphs; i++, byte_offset += sizeof(uint16_t)) {
			get2b(&glyph_index[i], data + byte_offset);
			glyph_index[i] = glyph_index[i] << 1;
		}
		get2b(&end_of_glyf, data + byte_offset);
		end_of_glyf = end_of_glyf << 1;
	}
	else {
		uint32_t byte_offset = loca_table_entry->second.offsetPos;
		for (uint16_t i = 0; i < max_profile.numGlyphs; i++, byte_offset += sizeof(uint32_t)) {
			get4b(&glyph_index[i], data + byte_offset);
		}
		get4b(&end_of_glyf, data + byte_offset);
	}

	auto cmap_table_entry = table_map.find("cmap");
	if (cmap_table_entry == table_map.end())
		return -2;
	uint32_t cmap_offset = cmap_table_entry->second.offsetPos + sizeof(uint16_t); //Skip version
	uint16_t cmap_num_tables;
	get2b(&cmap_num_tables, data + cmap_offset); cmap_offset += sizeof(uint16_t);

	std::map<uint16_t, uint32_t> glyph_reverse_map;

	bool valid_cmap_table = false;
	for (uint16_t i = 0; i < cmap_num_tables; i++) {
		uint16_t platformID, encodingID;
		uint32_t cmap_subtable_offset;
		get2b(&platformID, data + cmap_offset); cmap_offset += sizeof(uint16_t);
		get2b(&encodingID, data + cmap_offset); cmap_offset += sizeof(uint16_t);
		get4b(&cmap_subtable_offset, data + cmap_offset); cmap_offset += sizeof(uint32_t);

		if (!((platformID == 0 && encodingID == 3) || (platformID == 3 && encodingID == 1)))
			continue;

		cmap_subtable_offset += cmap_table_entry->second.offsetPos;
		uint16_t format, length;
		get2b(&format, data + cmap_subtable_offset); cmap_subtable_offset += sizeof(uint16_t);
		get2b(&length, data + cmap_subtable_offset); cmap_subtable_offset += sizeof(uint16_t);

		if (format != 4)
			continue;

		uint16_t language, segCountX2;// , searchRange, entrySelector, rangeShift;
		get2b(&language, data + cmap_subtable_offset); cmap_subtable_offset += sizeof(uint16_t);
		get2b(&segCountX2, data + cmap_subtable_offset); cmap_subtable_offset += sizeof(uint16_t);
		//get2b(&searchRange, data + cmap_subtable_offset); cmap_subtable_offset += sizeof(uint16_t);
		//get2b(&entrySelector, data + cmap_subtable_offset); cmap_subtable_offset += sizeof(uint16_t);
		//get2b(&rangeShift, data + cmap_subtable_offset); cmap_subtable_offset += sizeof(uint16_t);
		cmap_subtable_offset += sizeof(uint16_t) * 3;

		uint16_t segCount = segCountX2 >> 1;
		std::vector<uint16_t> endCount(segCount), startCount(segCount), idRangeOffset(segCount);
		std::vector<int16_t> idDelta(segCount);
		for (uint16_t j = 0; j < segCount; j++) {
			get2b(&endCount[j], data + cmap_subtable_offset); cmap_subtable_offset += sizeof(uint16_t);
		}
		cmap_subtable_offset += sizeof(uint16_t);
		for (uint16_t j = 0; j < segCount; j++) {
			get2b(&startCount[j], data + cmap_subtable_offset);
			get2b(&idDelta[j], data + cmap_subtable_offset + sizeof(uint16_t) * segCount);
			get2b(&idRangeOffset[j], data + cmap_subtable_offset + sizeof(uint16_t) * segCount * 2);
			if (idRangeOffset[j] == 0) {
				for (uint32_t k = startCount[j]; k <= endCount[j]; k++) {
					font_data->glyph_map[k] = k + idDelta[j];
					glyph_reverse_map[k + idDelta[j]] = k;
				}
			}
			else {
				uint32_t glyph_address_offset = cmap_subtable_offset + sizeof(uint16_t) * segCount * 2; //idRangeOffset_ptr
				for (uint32_t k = startCount[j]; k <= endCount[j]; k++) {
					uint32_t glyph_address_index_offset = idRangeOffset[j] + 2 * (k - startCount[j]) + glyph_address_offset;
					uint16_t& glyph_map_value = font_data->glyph_map[k];
					get2b(&glyph_map_value, data + glyph_address_index_offset);
					glyph_reverse_map[glyph_map_value] = k;
					glyph_map_value += idDelta[j];
				}
			}
			cmap_subtable_offset += sizeof(uint16_t);
		}
		valid_cmap_table = true;
		break;
	}
	if (!valid_cmap_table)
		TTFDEBUG_PRINT("ttf-parser: No valid cmap table found\n");

	HHEATable hhea_table;
	auto hhea_table_entry = table_map.find("hhea");
	if (hhea_table_entry == table_map.end())
		return -2;
	uint32_t hhea_offset = hhea_table.parse(data, hhea_table_entry->second.offsetPos);

	auto glyf_table_entry = table_map.find("glyf");
	if (glyf_table_entry == table_map.end())
		return -2;
	uint32_t glyf_offset = glyf_table_entry->second.offsetPos;

	auto kern_table_entry = table_map.find("kern");
	uint32_t kern_offset = 0;
	if (kern_table_entry != table_map.end())
		kern_offset = kern_table_entry->second.offsetPos;

	auto hmtx_table_entry = table_map.find("hmtx");
	if (hmtx_table_entry == table_map.end())
		return -2;
	uint32_t hmtx_offset = hmtx_table_entry->second.offsetPos;
	uint16_t last_glyph_advance_width = 0;

	std::vector<std::vector<uint16_t>> point_index((max_profile.maxContours < 4096) ? max_profile.maxContours : 4096);
	std::vector<uint16_t> points_per_contour((max_profile.maxContours < 4096) ? max_profile.maxContours : 4096);

	if (!max_profile.numGlyphs)
		return -1;

	bool* glyph_loaded = new bool[max_profile.numGlyphs];
	memset(glyph_loaded, 0, sizeof(bool) * max_profile.numGlyphs);

	auto parse_glyph = [&](uint16_t i, auto&& self) -> int8_t {
		if (glyph_loaded[i] == true)
			return 1;

		Glyph& current_glyph = font_data->glyphs[i];
		current_glyph.glyph_index = i;
		current_glyph.character = glyph_reverse_map[i];
		current_glyph.num_triangles = 0;

		if (i < hhea_table.numberOfHMetrics) {
			get2b(&current_glyph.advance_width, data + hmtx_offset + i * sizeof(uint32_t));
			last_glyph_advance_width = current_glyph.advance_width;
			get2b(&current_glyph.left_side_bearing, data + hmtx_offset + i * sizeof(uint32_t) + sizeof(uint16_t));
		}
		else
			current_glyph.advance_width = last_glyph_advance_width;

		if (i != max_profile.numGlyphs - 1 && glyph_index[i] == glyph_index[i + 1]) {
			glyph_loaded[i] = true;
			return -1;
		}
		if (glyph_index[i] >= end_of_glyf)
			return -1;

		uint32_t current_offset = glyf_offset + glyph_index[i];

		get2b(&current_glyph.num_contours, data + current_offset); current_offset += sizeof(int16_t);
		get2b(&current_glyph.bounding_box[0], data + current_offset); current_offset += sizeof(int16_t);
		get2b(&current_glyph.bounding_box[1], data + current_offset); current_offset += sizeof(int16_t);
		get2b(&current_glyph.bounding_box[2], data + current_offset); current_offset += sizeof(int16_t);
		get2b(&current_glyph.bounding_box[3], data + current_offset); current_offset += sizeof(int16_t);

		float_v2 glyph_center;
		glyph_center.x = (current_glyph.bounding_box[0] + current_glyph.bounding_box[2]) / 2.0f;
		glyph_center.y = (current_glyph.bounding_box[1] + current_glyph.bounding_box[3]) / 2.0f;

		if (current_glyph.num_contours > 0) { //Simple glyph
			std::vector<uint16_t> contour_end(current_glyph.num_contours);
			current_glyph.path_list.resize(current_glyph.num_contours);
			for (uint16_t j = 0; j < current_glyph.num_contours; j++) {
				get2b(&contour_end[j], data + current_offset); current_offset += sizeof(uint16_t);
			}
			for (uint16_t j = 0; j < current_glyph.num_contours; j++) {
				uint16_t num_points = contour_end[j] - (j ? contour_end[j - 1] : -1);
				if (point_index[j].size() < num_points)
					point_index[j].resize(num_points);
				points_per_contour[j] = num_points;
			}

			//Skip instructions
			uint16_t num_instructions;
			get2b(&num_instructions, data + current_offset); current_offset += sizeof(uint16_t);
			current_offset += sizeof(uint8_t) * num_instructions;

			uint16_t num_points = contour_end[current_glyph.num_contours - 1] + 1;
			std::vector<uint8_t> flags(num_points);
			std::vector<Flags> flagsEnum(num_points);
			std::vector<uint16_t> contour_index(num_points);
			uint16_t current_contour_index = 0;
			int16_t repeat = 0;
			uint16_t coutour_count_first_point = 0;
			for (uint16_t j = 0; j < num_points; j++, coutour_count_first_point++) {
				if (repeat == 0) {
					get1b(&flags[j], data + current_offset); current_offset += sizeof(uint8_t);
					if (flags[j] & 0x8) {
						get1b(&repeat, data + current_offset); current_offset += sizeof(uint8_t);
					}
				}
				else {
					flags[j] = flags[j - 1];
					repeat--;
				}
				flagsEnum[j].offCurve = (!(flags[j] & 0b00000001)) != 0;
				flagsEnum[j].xShort = (flags[j] & 0b00000010) != 0;
				flagsEnum[j].yShort = (flags[j] & 0b00000100) != 0;
				flagsEnum[j].repeat = (flags[j] & 0b00001000) != 0;
				flagsEnum[j].xDual = (flags[j] & 0b00010000) != 0;
				flagsEnum[j].yDual = (flags[j] & 0b00100000) != 0;
				if (j > contour_end[current_contour_index]) {
					current_contour_index++;
					coutour_count_first_point = 0;
				}
				contour_index[j] = current_contour_index;
				point_index[current_contour_index][coutour_count_first_point] = j;
			}
			uint16_t current_flag_size = 1;
			std::vector<int16_v2> points;
			points.resize(num_points);
			for (uint16_t j = 0; j < num_points; j++) {
				if (flagsEnum[j].xDual && !flagsEnum[j].xShort)
					points[j].x = j ? points[j - 1].x : 0;
				else {
					if (flagsEnum[j].xShort) {
						get1b(&points[j].x, data + current_offset); current_offset += 1;
					}
					else {
						get2b(&points[j].x, data + current_offset); current_offset += 2;
					}
					if (flagsEnum[j].xShort && !flagsEnum[j].xDual)
						points[j].x *= -1;
					if (j != 0)
						points[j].x += points[j - 1].x;
				}
			}
			for (uint16_t j = 0; j < num_points; j++) {
				if (flagsEnum[j].yDual && !flagsEnum[j].yShort)
					points[j].y = j ? points[j - 1].y : 0;
				else {
					if (flagsEnum[j].yShort) {
						get1b(&points[j].y, data + current_offset); current_offset += 1;
					}
					else {
						get2b(&points[j].y, data + current_offset); current_offset += 2;
					}
					if (flagsEnum[j].yShort && !flagsEnum[j].yDual)
						points[j].y *= -1;
					if (j != 0)
						points[j].y += points[j - 1].y;
				}
			}

			//Generate contours
			for (uint16_t j = 0; j < current_glyph.num_contours; j++) {
				const uint16_t& num_points_per_contour = points_per_contour[j];
				float_v2 prev_point;
				const uint16_t& point_index_0 = point_index[j][0];
				const Flags& flags_0 = flagsEnum[point_index_0];
				//If the first point is off curve
				if (flags_0.offCurve) {
					const uint16_t& point_index_m1 = point_index[j][num_points_per_contour - 1];
					const Flags& flags_m1 = flagsEnum[point_index_m1];
					const int16_v2& p0 = points[point_index_0];
					const int16_v2& pm1 = points[point_index_m1];
					if (flags_m1.offCurve) {
						prev_point.x = (p0.x + pm1.x) / 2.0f;
						prev_point.y = (p0.y + pm1.y) / 2.0f;
					}
					else {
						prev_point.x = pm1.x;
						prev_point.y = pm1.y;
					}
				}
				for (uint16_t k = 0; k < num_points_per_contour; k++) {
					const uint16_t& point_index0 = point_index[j][k % num_points_per_contour];
					const uint16_t& point_index1 = point_index[j][(k + 1) % num_points_per_contour];
					const Flags& flags0 = flagsEnum[point_index0];
					const Flags& flags1 = flagsEnum[point_index1];
					const int16_v2& p0 = points[point_index0];
					const int16_v2& p1 = points[point_index1];
					Curve curve;
					if (flags0.offCurve) {
						curve.p0.x = prev_point.x;
						curve.p0.y = prev_point.y;
						curve.p1.x = p0.x;
						curve.p1.y = p0.y;
						if (flags1.offCurve) {
							curve.p2.x = (p0.x + p1.x) / 2.0f;
							curve.p2.y = (p0.y + p1.y) / 2.0f;

							prev_point = curve.p2;
						}
						else {
							curve.p2.x = p1.x;
							curve.p2.y = p1.y;
							//No change to prev_point
						}
					}
					else if (!flags1.offCurve) {
						curve.p0.x = p0.x;
						curve.p0.y = p0.y;
						curve.p1.x = p1.x;
						curve.p1.y = p1.y;
						curve.p2.x = glyph_center.x + 0.5f;
						curve.p2.y = glyph_center.y + 0.5f;

						prev_point.x = p0.x;
						prev_point.y = p0.y;
					}
					else {
						const uint16_t& point_index2 = point_index[j][(k + 2) % num_points_per_contour];
						const Flags& flags2 = flagsEnum[point_index2];
						const int16_v2& p2 = points[point_index2];
						if (flags2.offCurve) {
							curve.p0.x = p0.x;
							curve.p0.y = p0.y;
							curve.p1.x = p1.x;
							curve.p1.y = p1.y;
							curve.p2.x = (p1.x + p2.x) / 2.0f;
							curve.p2.y = (p1.y + p2.y) / 2.0f;

							prev_point = curve.p2;

						}
						else {
							curve.p0.x = p0.x;
							curve.p0.y = p0.y;
							curve.p1.x = p1.x;
							curve.p1.y = p1.y;
							curve.p2.x = p2.x;
							curve.p2.y = p2.y;

							prev_point.x = p0.x;
							prev_point.y = p0.y;
						}
					}
					if (flags0.offCurve || flags1.offCurve) {
						curve.is_curve = true;
						Curve line_curve;
						line_curve.is_curve = false;
						line_curve.p0.x = curve.p0.x;
						line_curve.p0.y = curve.p0.y;
						line_curve.p1.x = curve.p2.x;
						line_curve.p1.y = curve.p2.y;
						line_curve.p2.x = glyph_center.x + 0.5f;
						line_curve.p2.y = glyph_center.y + 0.5f;
						current_glyph.path_list[j].curves.push_back(std::move(line_curve));
						if (flags0.offCurve == false)
							k++;
					}
					else
						curve.is_curve = false;
					current_glyph.path_list[j].curves.push_back(std::move(curve));
				}
				current_glyph.num_triangles += (uint32_t)current_glyph.path_list[j].curves.size();
			}
		}

		else { //Composite glyph
			for (auto compound_glyph_index = 0; compound_glyph_index < -current_glyph.num_contours; compound_glyph_index++) {
				uint16_t glyf_flags, glyphIndex;
				do {
					get2b(&glyf_flags, data + current_offset); current_offset += sizeof(uint16_t);
					get2b(&glyphIndex, data + current_offset); current_offset += sizeof(uint16_t);

					int16_t glyf_args1, glyf_args2;
					int8_t glyf_args1_u8, glyf_args2_u8;
					bool is_word = false;
					if (glyf_flags & ARG_1_AND_2_ARE_WORDS) {
						get2b(&glyf_args1, data + current_offset); current_offset += sizeof(int16_t);
						get2b(&glyf_args2, data + current_offset); current_offset += sizeof(int16_t);
						is_word = true;
					}
					else {
						get1b(&glyf_args1_u8, data + current_offset); current_offset += sizeof(int8_t);
						get1b(&glyf_args2_u8, data + current_offset); current_offset += sizeof(int8_t);
					}

					float composite_glyph_element_transformation[6] = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };

					if (glyf_flags & WE_HAVE_A_SCALE) {
						int16_t xy_value;
						get2b(&xy_value, data + current_offset); current_offset += sizeof(int16_t);
						composite_glyph_element_transformation[0] = to_2_14_float(xy_value);
						composite_glyph_element_transformation[3] = to_2_14_float(xy_value);
					}
					else if (glyf_flags & WE_HAVE_AN_X_AND_Y_SCALE) {
						int16_t xy_values[2];
						get2b(&xy_values[0], data + current_offset); current_offset += sizeof(int16_t);
						get2b(&xy_values[1], data + current_offset); current_offset += sizeof(int16_t);
						composite_glyph_element_transformation[0] = to_2_14_float(xy_values[0]);
						composite_glyph_element_transformation[3] = to_2_14_float(xy_values[1]);
					}
					else if (glyf_flags & WE_HAVE_A_TWO_BY_TWO) {
						int16_t xy_values[4];
						get2b(&xy_values[0], data + current_offset); current_offset += sizeof(int16_t);
						get2b(&xy_values[1], data + current_offset); current_offset += sizeof(int16_t);
						get2b(&xy_values[2], data + current_offset); current_offset += sizeof(int16_t);
						get2b(&xy_values[3], data + current_offset); current_offset += sizeof(int16_t);
						composite_glyph_element_transformation[0] = to_2_14_float(xy_values[0]);
						composite_glyph_element_transformation[1] = to_2_14_float(xy_values[1]);
						composite_glyph_element_transformation[2] = to_2_14_float(xy_values[2]);
						composite_glyph_element_transformation[3] = to_2_14_float(xy_values[3]);
					}

					bool matched_points = false;
					if (glyf_flags & ARGS_ARE_XY_VALUES) {
						composite_glyph_element_transformation[4] = is_word ? glyf_args1 : glyf_args1_u8;
						composite_glyph_element_transformation[5] = is_word ? glyf_args2 : glyf_args2_u8;
						if (glyf_flags & SCALED_COMPONENT_OFFSET) {
							composite_glyph_element_transformation[4] *= composite_glyph_element_transformation[0];
							composite_glyph_element_transformation[5] *= composite_glyph_element_transformation[3];
						}
					}
					else {
						matched_points = true;
					}

					//Skip instructions
					if (glyf_flags & WE_HAVE_INSTRUCTIONS) {
						uint16_t num_instructions = 0;
						get2b(&num_instructions, data + current_offset); current_offset += sizeof(uint16_t);
						current_offset += sizeof(uint8_t) * num_instructions;
					}

					if (glyph_loaded[glyphIndex] == false) {
						if (self(glyphIndex, self) < 0) {
							TTFDEBUG_PRINT("ttf-parser: bad glyph index %d in composite glyph\n", glyphIndex);
							continue;
						}
					}
					Glyph& composite_glyph_element = font_data->glyphs[glyphIndex];

					auto transform_curve = [&composite_glyph_element_transformation](Curve& _in) -> Curve {
						Curve out;
						out.p0.x = _in.p0.x * composite_glyph_element_transformation[0] + _in.p0.y * composite_glyph_element_transformation[1] + composite_glyph_element_transformation[4];
						out.p0.y = _in.p0.x * composite_glyph_element_transformation[2] + _in.p0.y * composite_glyph_element_transformation[3] + composite_glyph_element_transformation[5];
						out.p1.x = _in.p1.x * composite_glyph_element_transformation[0] + _in.p1.y * composite_glyph_element_transformation[1] + composite_glyph_element_transformation[4];
						out.p1.y = _in.p1.x * composite_glyph_element_transformation[2] + _in.p1.y * composite_glyph_element_transformation[3] + composite_glyph_element_transformation[5];
						out.p2.x = _in.p2.x * composite_glyph_element_transformation[0] + _in.p2.y * composite_glyph_element_transformation[1] + composite_glyph_element_transformation[4];
						out.p2.y = _in.p2.x * composite_glyph_element_transformation[2] + _in.p2.y * composite_glyph_element_transformation[3] + composite_glyph_element_transformation[5];
						return out;
					};

					uint32_t composite_glyph_path_count = composite_glyph_element.path_list.size();
					for (uint32_t glyph_point_index = 0; glyph_point_index < composite_glyph_path_count; glyph_point_index++) {
						std::vector<Curve>& current_curves_list = composite_glyph_element.path_list[glyph_point_index].curves;
						uint32_t composite_glyph_path_curves_count = current_curves_list.size();
						Path new_path;
						if (matched_points == false) {
							for (uint32_t glyph_curves_point_index = 0; glyph_curves_point_index < composite_glyph_path_curves_count; glyph_curves_point_index++) {
								new_path.curves.emplace_back(transform_curve(current_curves_list[glyph_curves_point_index]));
							}
						}
						else {
							TTFDEBUG_PRINT("ttf-parser: unsupported matched points in ttf composite glyph\n");
							continue;
						}
						current_glyph.path_list.emplace_back(new_path);
					}
					current_glyph.num_triangles += composite_glyph_element.num_triangles;
				} while (glyf_flags & MORE_COMPONENTS);
			}
		}
		glyph_loaded[i] = true;
		return 0;
	};

	for (uint16_t i = 0; i < max_profile.numGlyphs; i++) {
		parse_glyph(i, parse_glyph);
	}

	delete[] glyph_loaded;

	//Kearning table
	if (kern_offset) {
		uint32_t current_offset = kern_offset;
		uint16_t kern_table_version, num_kern_subtables;
		get2b(&kern_table_version, data + current_offset); current_offset += sizeof(uint16_t);
		get2b(&num_kern_subtables, data + current_offset); current_offset += sizeof(uint16_t);
		uint16_t kern_length = 0;
		uint32_t kern_start_offset = current_offset;
		for (uint16_t kern_subtable_index = 0; kern_subtable_index < num_kern_subtables; kern_subtable_index++) {
			uint16_t kern_version, kern_converage;
			current_offset = kern_start_offset + kern_length;
			kern_start_offset = current_offset;
			get2b(&kern_version, data + current_offset); current_offset += sizeof(uint16_t);
			get2b(&kern_length, data + current_offset); current_offset += sizeof(uint16_t);
			if (kern_version != 0) {
				current_offset += kern_length - sizeof(uint16_t) * 3;
				continue;
			}
			get2b(&kern_converage, data + current_offset); current_offset += sizeof(uint16_t);

			uint16_t num_kern_pairs;
			get2b(&num_kern_pairs, data + current_offset); current_offset += sizeof(uint16_t);
			current_offset += sizeof(uint16_t) * 3;
			for (uint16_t kern_index = 0; kern_index < num_kern_pairs; kern_index++) {
				uint16_t kern_left, kern_right;
				int16_t kern_value;
				get2b(&kern_left, data + current_offset); current_offset += sizeof(uint16_t);
				get2b(&kern_right, data + current_offset); current_offset += sizeof(uint16_t);
				get2b(&kern_value, data + current_offset); current_offset += sizeof(int16_t);

				font_data->kearning_table[(kern_left << 16) | kern_right] = kern_value;
			}
		}
	}

	font_data->meta_data.unitsPerEm = head_table.unitsPerEm;
	font_data->meta_data.Ascender = hhea_table.Ascender;
	font_data->meta_data.Descender = hhea_table.Descender;
	font_data->meta_data.LineGap = hhea_table.LineGap;

	return 0;
}

int16_t TTFFontParser::get_kearning_offset(FontData* font_data, uint16_t left_glyph, uint16_t right_glyph)
{
	auto kern_data = font_data->kearning_table.find((left_glyph << 16) | right_glyph);
	return (kern_data == font_data->kearning_table.end()) ? 0 : kern_data->second;
}
#endif