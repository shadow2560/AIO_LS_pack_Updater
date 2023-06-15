#ifndef _INI_HEKATE_CONFIGS_PARSE_H_
#define _INI_HEKATE_CONFIGS_PARSE_H_

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char* key;
	char* value;
	int  lineNumber;
} KeyValue;

typedef struct {
	char* section;
	KeyValue* keyValues;
	int lineNumber;
	bool is_main_config;
	size_t numKeyValues;
	char* filePath;
} Section;

typedef struct {
	Section* sections;
	size_t numSections;
} IniData;

// Comment lines that start and end with specific characters
typedef struct {
	char start;
	char end;
} Commentary;

void parseIniFile(const char* filename, IniData* iniData, bool  is_main_config);
void removeSection(IniData* iniData, size_t sectionIndex);
void removeKeyInSection(IniData* iniData, size_t sectionIndex, size_t keyIndex);
void freeIniData(IniData* iniData);

#ifdef __cplusplus
}
#endif

#endif