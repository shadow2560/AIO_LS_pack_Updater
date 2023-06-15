#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <switch.h>

#include "ini_hekate_configs_parse.h"
#include "getdelim.h"

#define MAX_LINE_LENGTH 256

void stripLeadingTrailingSpaces(char* str) {
	if (str == NULL)
		return;

	char* start = str;
	while (*start && isspace((unsigned char)*start))
		++start;

	size_t len = strlen(start);
	char* end = start + len - 1;
	while (end > start && isspace((unsigned char)*end))
		--end;

	*(end + 1) = '\0';

	if (start != str)
		memmove(str, start, len - (start - str) + 1);
}

void parseKeyValue(const char* line, char** key, char** value) {
	char* keyValueSeparator = strchr(line, '=');
	if (keyValueSeparator != NULL) {
		*keyValueSeparator = '\0';
		*key = strdup(line);
		*value = strdup(keyValueSeparator + 1);
		stripLeadingTrailingSpaces(*key);
		stripLeadingTrailingSpaces(*value);
	} else {
		*key = NULL;
		*value = NULL;
	}
}

void parseIniFile(const char* fileName, IniData* iniData, bool is_main_config) {
	FILE* file = fopen(fileName, "r");

	if (file == NULL) {
		// printf("Failed to open file: %s\n", filename);
		return;
	}

	char* line = NULL;
	size_t line_size = 0;
	Section* currentSection = NULL; // Track the current section

	// Comment line that starts with a specific character
	const char* comment_start_chars = "#;";

	Commentary commentaries[] = {
		{'{', '}'},
		{'/', '*'}
	};
	size_t num_commentaries = sizeof(commentaries) / sizeof(commentaries[0]);

	// Comment lines that contain a specific character and the rest of the line is a comment
	const char* comment_char_list = "#";

	int lineCount = 0;
	int lastLineRead = 0;

size_t len;
	while (1) {
		int result = getline(&line, &line_size, file);
		if (result == -1) {
			if (currentSection == NULL && !lastLineRead) {
				lastLineRead = 1;
				continue;
			} else {
				break;
			}
		}
		// Remove trailing newline character
		len = strlen(line);
		if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
			line[len - 1] = '\0';
		} else {
			// Line doesn't end with newline, so it's incomplete
			while (getline(&line, &line_size, file) != -1) {
				len = strlen(line);
				if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
					line[len - 1] = '\0';
					break;
				}
			}
		}

		len = strlen(line);
		if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
			line[len - 1] = '\0';
		}
		stripLeadingTrailingSpaces(line); // Strip leading and trailing spaces
		len = strlen(line);

lineCount++;

		// Skip blank lines
		if (strlen(line) == 0) {
			continue;
		}

		// Skip blank lines or lines consisting only of newline characters or carriage return characters
		int isBlankLine = 1;
		for (size_t i = 0; i < strlen(line); i++) {
			if (line[i] != '\n' && line[i] != '\r') {
				isBlankLine = 0;
				break;
			}
		}
		if (isBlankLine) {
			continue;
		}

		// Skip comment lines
		if (comment_start_chars != NULL && strchr(comment_start_chars, line[0]) != NULL) {
			continue;
		}

		// Check if it's a comment line that starts and ends with specific characters
		int isCommentary = 0;
		if (comment_char_list != NULL) {
			for (size_t i = 0; i < num_commentaries; i++) {
				if (line[0] == commentaries[i].start && line[len - 1] == commentaries[i].end) {
					isCommentary = 1;
					break;
				}
			}
		}

		if (isCommentary) {
			continue;
		}

		// Check if it's a comment line that contains a specific character and the rest of the line is a comment
		int isCommentCharList = 0;
		if (comment_char_list != NULL) {
			for (size_t i = 0; i < strlen(comment_char_list); i++) {
				if (strchr(line, comment_char_list[i]) != NULL) {
					isCommentCharList = 1;
					break;
				}
			}
		}

		// Strip leading spaces and tabulations for the line before the commentary character
		if (isCommentCharList) {
			char* commentCharPosition = strchr(line, comment_char_list[0]);
			if (commentCharPosition != NULL) {
				*commentCharPosition = '\0';  // Terminate the line before the commentary character
				stripLeadingTrailingSpaces(line);  // Strip leading and trailing spaces
				len = strlen(line);
			}
		}

		// Skip blank lines
		if (strlen(line) == 0) {
			continue;
		}

		// Skip blank lines or lines consisting only of newline characters or carriage return characters
		isBlankLine = 1;
		for (size_t i = 0; i < strlen(line); i++) {
			if (line[i] != '\n' && line[i] != '\r') {
				isBlankLine = 0;
				break;
			}
		}
		if (isBlankLine) {
			continue;
		}

		if (line[0] == '[') {
			// New section
			char* sectionNameStart = line + 1; // Skip the '['
			char* sectionNameEnd = strchr(sectionNameStart, ']');
			if (sectionNameEnd != NULL) {
				*sectionNameEnd = '\0'; // Terminate the section name
				char* sectionName = strdup(sectionNameStart);
				iniData->sections = realloc(iniData->sections, (iniData->numSections + 1) * sizeof(Section));
				currentSection = &iniData->sections[iniData->numSections];
				currentSection->section = sectionName;
				currentSection->keyValues = NULL;
				currentSection->lineNumber = lineCount;
				currentSection->numKeyValues = 0;
				currentSection->is_main_config = is_main_config;
				char* sectionFile = strdup(fileName);
				currentSection->filePath = sectionFile;
				
				iniData->numSections++;
			}
		} else {
			// Key-value pair
			if (currentSection != NULL) {
				char* key;
				char* value;
				parseKeyValue(line, &key, &value);
				currentSection->keyValues = realloc(currentSection->keyValues, (currentSection->numKeyValues + 1) * sizeof(KeyValue));
				KeyValue* keyValue = &currentSection->keyValues[currentSection->numKeyValues];
				keyValue->key = key;
				keyValue->value = value;
				keyValue->lineNumber = lineCount;
				currentSection->numKeyValues++;
			}
		}

		if (lastLineRead && currentSection == NULL && lineCount > 2) {
			break;
		}

	}

	if (lastLineRead && currentSection == NULL) {
		// Handle the case where the last line is a section declaration without key-value pairs
		char* sectionNameStart = line + 1; // Skip the '['
		char* sectionNameEnd = strchr(sectionNameStart, ']');
		if (sectionNameEnd != NULL) {
			*sectionNameEnd = '\0'; // Terminate the section name
			char* sectionName = strdup(sectionNameStart);
			stripLeadingTrailingSpaces(sectionName);
			iniData->sections = realloc(iniData->sections, (iniData->numSections + 1) * sizeof(Section));
			currentSection = &iniData->sections[iniData->numSections];
			currentSection->section = sectionName;
			currentSection->keyValues = NULL;
			currentSection->numKeyValues = 0;
			currentSection->is_main_config = is_main_config;
			iniData->numSections++;
		}
	}

	free(line);
	fclose(file);

	return;
}

void removeSection(IniData* iniData, size_t sectionIndex) {
	if (iniData == NULL || sectionIndex >= iniData->numSections)
		return;

	Section* section = &iniData->sections[sectionIndex];
	free(section->section);

	for (size_t i = 0; i < section->numKeyValues; i++) {
		KeyValue* keyValue = &section->keyValues[i];
		free(keyValue->key);
		free(keyValue->value);
	}

	free(section->keyValues);
	free(section->filePath);

	// Shift remaining sections
	for (size_t i = sectionIndex + 1; i < iniData->numSections; i++) {
		iniData->sections[i - 1] = iniData->sections[i];
	}

	iniData->numSections--;
	iniData->sections = realloc(iniData->sections, iniData->numSections * sizeof(Section));
}

void removeKeyInSection(IniData* iniData, size_t sectionIndex, size_t keyIndex) {
	if (iniData == NULL || sectionIndex >= iniData->numSections)
		return;

	Section* section = &iniData->sections[sectionIndex];
	if (keyIndex >= section->numKeyValues)
		return;

	KeyValue* keyValue = &section->keyValues[keyIndex];
	free(keyValue->key);
	free(keyValue->value);

	// Shift remaining keys
	for (size_t i = keyIndex + 1; i < section->numKeyValues; i++) {
		section->keyValues[i - 1] = section->keyValues[i];
	}

	section->numKeyValues--;
	section->keyValues = realloc(section->keyValues, section->numKeyValues * sizeof(KeyValue));
}

void freeIniData(IniData* iniData) {
	if (iniData == NULL)
		return;

	for (size_t i = 0; i < iniData->numSections; i++) {
		Section* section = &(iniData->sections[i]);
		free(section->section);
		for (size_t j = 0; j < section->numKeyValues; j++) {
			KeyValue* keyValue = &(section->keyValues[j]);
			free(keyValue->key);
			free(keyValue->value);
		}
		free(section->keyValues);
		free(section->filePath);
	}
	free(iniData->sections);
	iniData->sections = NULL;
	iniData->numSections = 0;
}

/*
void printIniData(const IniData* iniData) {
	for (size_t i = 0; i < iniData->numSections; i++) {
		Section* section = &(iniData->sections[i]);
		printf("Section: %s\n", section->section);

		for (size_t j = 0; j < section->numKeyValues; j++) {
			printf("Key: %s, Value: %s\n", section->keyValues[j].key, section->keyValues[j].value);
		}

		printf("\n");
	}
}

int test() {
	IniData iniData = {NULL, 0};
	parseIniFile("hekate_ipl.ini", &iniData);
	parseIniFile("unbrick.ini", &iniData);
	printIniData(&iniData);

	// Clean up the allocated memory
	freeIniData(&iniData);

	return 0;
}
*/