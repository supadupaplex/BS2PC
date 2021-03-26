#pragma once
#include "bs2pc_lumps.h"
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#if defined(_MSC_VER) || defined(_WIN32)
#include <malloc.h>
#define bs2pc_alloca _alloca
#define bs2pc_strcasecmp _stricmp
#define bs2pc_strncasecmp _strnicmp
#else
#include <alloca.h>
#define bs2pc_alloca alloca
#define bs2pc_strcasecmp strcasecmp
#define bs2pc_strncasecmp strncasecmp
#endif

// Utility
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void *BS2PC_Alloc(size_t size, bool zeroed);
void BS2PC_AllocReplace(void **memory, size_t size, bool zeroed);
static inline void BS2PC_Free(void *memory) { free(memory); }

void *BS2PC_LoadFile(const char *fileName, unsigned int *fileSize);
void BS2PC_WriteFile(const char *fileName, void *data, unsigned int size);

void BS2PC_Decompress(const void *source, unsigned int sourceSize, void *target, unsigned int targetSize);
void *BS2PC_CompressWithSize(const void *source, unsigned int sourceSize, unsigned int *outTargetSize);

void BS2PC_ResampleTexture(const unsigned char *in, int inwidth, int inheight, unsigned char *out, int outwidth, int outheight);

int BS2PC_CompareTextureNames(const char *name1, const char *name2);

static inline float BS2PC_DotProduct(const float x[3], const float y[3]) {
	return x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
}

// Map

extern unsigned char *bs2pc_idMap, *bs2pc_gbxMap;
extern unsigned int bs2pc_idMapSize, bs2pc_gbxMapSize;
static inline unsigned int BS2PC_IdLumpOffset(unsigned int lump) {
	return ((const dheader_id_t *) bs2pc_idMap)->lumps[lump].fileofs;
}
static inline unsigned char *BS2PC_IdLump(unsigned int lump) {
	return bs2pc_idMap + BS2PC_IdLumpOffset(lump);
}
static inline unsigned int BS2PC_IdLumpSize(unsigned int lump) {
	return ((const dheader_id_t *) bs2pc_idMap)->lumps[lump].filelen;
}
static inline unsigned int BS2PC_GbxLumpOffset(unsigned int lump) {
	return ((const dheader_gbx_t *) bs2pc_gbxMap)->lumpofs[lump];
}
static inline unsigned char *BS2PC_GbxLump(unsigned int lump) {
	return bs2pc_gbxMap + BS2PC_GbxLumpOffset(lump);
}
static inline unsigned int BS2PC_GbxLumpSize(unsigned int lump) {
	return ((const dheader_gbx_t *) bs2pc_gbxMap)->lumplen[lump];
}
static inline unsigned int BS2PC_GbxLumpCount(unsigned int lump) {
	return ((const dheader_gbx_t *) bs2pc_gbxMap)->lumpnum[lump];
}
static inline unsigned int BS2PC_GbxOffsetToIndex(bspoffset_t offset, unsigned int lump, unsigned int lumpSize) {
	return (offset - BS2PC_GbxLumpOffset(lump)) / lumpSize;
}
static inline bspoffset_t BS2PC_GbxIndexToOffset(unsigned int index, unsigned int lump, unsigned int lumpSize) {
	return BS2PC_GbxLumpOffset(lump) + index * lumpSize;
}
void BS2PC_CalcIdSurfaceExtents(const dface_id_t *face, short outTextureMins[2], short outExtents[2]);

// Conversion

void BS2PC_ConvertGbxToId();
void BS2PC_ConvertIdToGbx();
extern const unsigned char bs2pc_nodrawIdTexture[1152];

extern bool bs2pc_errors;

// [SDP] fix #6: config export
extern bool bs2pc_doMergeStrips;
extern bool bs2pc_szReport;
extern bool bs2pc_wadOnly;
extern bool bs2pc_noResize;
extern bool bs2pc_dryRun;

// Polygon subdivision

#pragma pack(push, 4)
typedef struct {
	float xyz[3];
	float st[2];
	unsigned char stoffset[2]; // Multiplies of 16 relative to texturemins.
	unsigned char pad[2];
} bs2pc_polyvert_t;
#pragma pack(pop)

unsigned char *BS2PC_SubdivideIdSurface(unsigned int faceIndex, unsigned int faceFlags, const dmiptex_id_t *texture, unsigned int *outSize);
unsigned char *BS2PC_SubdivideGbxSurface(unsigned int faceIndex, unsigned int *outSize);

// WAD texture management

void BS2PC_AddWadDirectory(const char *path);
void BS2PC_LoadWadsFromEntities(const char *entities, unsigned int entitiesSize);
unsigned char *BS2PC_LoadTextureFromWad(const char *name);