#include "gltf.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#define GLTF_ID 0x46546c67u
#define GLTF_HEADER_SIZE 12u
#define GLTF_CHUNK_INFO_SIZE 8u

typedef enum
{
	GLTF_CHUNK_JSON = 0x4e4f534a,
	GLTF_CHUNK_BINARY = 0x004e4942
} GLTFChunkType;

typedef enum
{
	JSON_ENTITY_NONE,
	JSON_ENTITY_VALUE,
	JSON_ENTITY_STRING,
	JSON_ENTITY_OBJECT,
	JSON_ENTITY_ARRAY
} JSONEntityType;

struct JSONCompound
{
	const char* key;
	JSONEntityType type;
	uint32_t length;
};

struct GLTF
{
	void* buffer;
	uint32_t version;
	uint32_t length;
	struct
	{
		uint32_t length;
		uint8_t* data;
	} json;
	struct
	{
		uint32_t length;
		uint8_t* data;
	} binary;
};

static void jsonPrintCompound(const struct JSONCompound* comp, bool elements)
{
	printf("%s ", (comp->type == JSON_ENTITY_OBJECT) ? "Object" : "Array");
	if (comp->key)
		printf("named %s ", comp->key);
	if (elements)
		printf("with %u members", comp->length);
	printf("\n");
}

static void gltfPreprocessJSON(struct GLTF* gltf)
{
	uint32_t depth = 0;
	static const char* rootKey = "[ROOT]";
	uint8_t* data = gltf->json.data;
	const char* currentKey = rootKey;
	const char* currentValue = NULL;
	struct JSONCompound* tmpStack = gltf->buffer;
	JSONEntityType currentEntity = JSON_ENTITY_NONE;
	for (uint32_t i = 0; i < gltf->json.length; i++)
	{
		switch (data[i])
		{
			case '"':
				if (currentEntity == JSON_ENTITY_STRING)
				{
					data[i] = 0;
					currentEntity = JSON_ENTITY_NONE;
				}
				else
				{
					assert(currentEntity == JSON_ENTITY_NONE);
					currentValue = (const char*)&data[i + 1];
					currentEntity = JSON_ENTITY_STRING;
				}
				break;
			case ':':
				assert(currentEntity == JSON_ENTITY_NONE);
				currentKey = currentValue;
				currentValue = NULL;
				tmpStack->length++;
				break;
			case '[':
				tmpStack += (depth++) ? 1 : 0;
				tmpStack->type = JSON_ENTITY_ARRAY;
				tmpStack->key = currentKey;
				tmpStack->length = 0;
				currentKey = NULL;
				//jsonPrintCompound(tmpStack, false);
				break;
			case '{':
				if (tmpStack->type == JSON_ENTITY_ARRAY)
					tmpStack->length++;
				tmpStack += (depth++) ? 1 : 0;
				tmpStack->type = JSON_ENTITY_OBJECT;
				tmpStack->key = currentKey;
				tmpStack->length = 0;
				currentKey = NULL;
				//jsonPrintCompound(tmpStack, false);
				break;
			case ']':
			case '}':
				data[i] = 0;
				if (currentValue)
				{
					printf("Value: %s\n", currentValue);
					currentValue = NULL;
				}
				//jsonPrintCompound(tmpStack, true);
				tmpStack -= (--depth) ? 1 : 0;
				break;
			case ',':
				data[i] = 0;
				if (currentValue)
				{
					printf("Value: %s\n", currentValue);
					currentValue = NULL;
				}
				currentEntity = JSON_ENTITY_NONE;
				currentKey = NULL;
				break;
			default:
				if (currentEntity == JSON_ENTITY_NONE && !isspace(data[i]))
				{
					currentEntity = JSON_ENTITY_VALUE;
					currentValue = &data[i];
					if (tmpStack->type == JSON_ENTITY_ARRAY)
						tmpStack->length++;
				}
				break;
		}
	}
}

void gltfParse(uint32_t* data, void* buffer)
{
	struct GLTF gltf;
	gltf.buffer = buffer;
	if (*data++ != GLTF_ID)
		return;
	gltf.length = *data++;
	gltf.length = *data++;
	uint32_t chunkOffset = GLTF_HEADER_SIZE;
	while (chunkOffset < gltf.length)
	{
		uint32_t length = *data++;
		GLTFChunkType type = *data++;
		uint8_t* bytes = (uint8_t*)data;
		data = (uint32_t*)(bytes + length);
		chunkOffset += length + GLTF_CHUNK_INFO_SIZE;
		switch (type)
		{
			case GLTF_CHUNK_JSON:
				gltf.json.length = length;
				gltf.json.data = bytes;
				gltfPreprocessJSON(&gltf);
				break;
			case GLTF_CHUNK_BINARY:
				gltf.binary.length = length;
				gltf.binary.data = bytes;
				break;
			default: 
				break;
		}
	}
}

void testGltf()
{
	FILE* fp = NULL;
	fopen_s(&fp, "E:\\blender_test.glb", "rb");
	fseek(fp, 0, SEEK_END);
	uint32_t len = ftell(fp) & UINT32_MAX;
	fseek(fp, 0, SEEK_SET);
	uint32_t aligned = (((len - 1) >> 4) + 1) << 4;
	uint32_t* fData = (uint32_t*)malloc(aligned << 1);
	fread_s(fData, len, len, 1, fp);
	fclose(fp);
	gltfParse(fData, ((uint8_t*)fData) + aligned);
	free(fData);
}

