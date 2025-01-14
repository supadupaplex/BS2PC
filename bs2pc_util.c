#pragma warning(disable : 4996)

#include "bs2pc.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zlib.h"
#ifdef _WIN32
#include <Windows.h>
#endif

void *BS2PC_Alloc(size_t size, bool zeroed) {
	void *memory = malloc(size);
	if (memory == NULL) {
		fprintf(stderr, "Couldn't allocate %zu bytes.\n", size);
		exit(EXIT_FAILURE);
	}
	if (zeroed) {
		memset(memory, 0, size);
	}
	return memory;
}

void BS2PC_AllocReplace(void **memory, size_t size, bool zeroed) {
	if (*memory != NULL) {
		BS2PC_Free(*memory);
	}
	*memory = BS2PC_Alloc(size, zeroed);
}

void *BS2PC_LoadFile(const char *fileName, unsigned int *fileSize) {
	FILE *file;
	long fileEnd;
	void *data;

	file = fopen(fileName, "rb");
	if (file == NULL) {
		fprintf(stderr, "Couldn't open %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		fprintf(stderr, "Couldn't seek to the end of %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	fileEnd = ftell(file);
	if (fileEnd <= 0) {
		fprintf(stderr, "Couldn't get the length of %s or it's empty.\n", fileName);
		exit(EXIT_FAILURE);
	}

	if (fseek(file, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Couldn't seek to the beginning of %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	data = BS2PC_Alloc(fileEnd, false);
	if (fread(data, fileEnd, 1, file) == 0) {
		fprintf(stderr, "Couldn't read %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	fclose(file);

	if (fileSize != NULL) {
		*fileSize = fileEnd;
	}
	return data;
}

void BS2PC_WriteFile(const char *fileName, void *data, unsigned int size) {
	FILE *file;

	file = fopen(fileName, "wb");
	if (file == NULL) {
		fprintf(stderr, "Couldn't open %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	if (fwrite(data, size, 1, file) == 0) {
		fprintf(stderr, "Couldn't write to %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	fclose(file);
}

// Zlib compression.

#ifdef ZLIB_STATIC // [SDP] Static/dynamic link switch

typedef int (*bs2pc_zlib_inflateInit__t)(z_streamp strm, const char *version, int stream_size);
typedef int (*bs2pc_zlib_inflate_t)(z_streamp strm, int flush);
typedef int (*bs2pc_zlib_inflateEnd_t)(z_streamp strm);
typedef int (*bs2pc_zlib_deflateInit__t)(z_streamp strm, int level, const char *version, int stream_size);
typedef uLong (*bs2pc_zlib_deflateBound_t)(z_streamp strm, uLong sourceLen);
typedef int (*bs2pc_zlib_deflate_t)(z_streamp strm, int flush);
typedef int (*bs2pc_zlib_deflateEnd_t)(z_streamp strm);

static bs2pc_zlib_inflateInit__t bs2pc_zlib_inflateInit_ = inflateInit_;
static bs2pc_zlib_inflate_t bs2pc_zlib_inflate = inflate;
static bs2pc_zlib_inflateEnd_t bs2pc_zlib_inflateEnd = inflateEnd;
static bs2pc_zlib_deflateInit__t bs2pc_zlib_deflateInit_ = deflateInit_;
static bs2pc_zlib_deflateBound_t bs2pc_zlib_deflateBound = deflateBound;
static bs2pc_zlib_deflate_t bs2pc_zlib_deflate = deflate;
static bs2pc_zlib_deflateEnd_t bs2pc_zlib_deflateEnd = deflateEnd;

void BS2PC_InitializeZlib() {
	return;
}

#else // ZLIB_STATIC

static bool bs2pc_zlib_initialized = false;

#ifdef _WIN32
#define BS2PC_ZLIB_IMPORT WINAPI
#else
#define BS2PC_ZLIB_IMPORT
#endif
typedef int (BS2PC_ZLIB_IMPORT *bs2pc_zlib_inflateInit__t)(z_streamp strm, const char *version, int stream_size);
static bs2pc_zlib_inflateInit__t bs2pc_zlib_inflateInit_;
typedef int (BS2PC_ZLIB_IMPORT *bs2pc_zlib_inflate_t)(z_streamp strm, int flush);
static bs2pc_zlib_inflate_t bs2pc_zlib_inflate;
typedef int (BS2PC_ZLIB_IMPORT *bs2pc_zlib_inflateEnd_t)(z_streamp strm);
static bs2pc_zlib_inflateEnd_t bs2pc_zlib_inflateEnd;
typedef int (BS2PC_ZLIB_IMPORT *bs2pc_zlib_deflateInit__t)(z_streamp strm, int level, const char *version, int stream_size);
static bs2pc_zlib_deflateInit__t bs2pc_zlib_deflateInit_;
typedef uLong (BS2PC_ZLIB_IMPORT *bs2pc_zlib_deflateBound_t)(z_streamp strm, uLong sourceLen);
static bs2pc_zlib_deflateBound_t bs2pc_zlib_deflateBound;
typedef int (BS2PC_ZLIB_IMPORT *bs2pc_zlib_deflate_t)(z_streamp strm, int flush);
static bs2pc_zlib_deflate_t bs2pc_zlib_deflate;
typedef int (BS2PC_ZLIB_IMPORT *bs2pc_zlib_deflateEnd_t)(z_streamp strm);
static bs2pc_zlib_deflateEnd_t bs2pc_zlib_deflateEnd;

void BS2PC_InitializeZlib() {
	#ifdef _WIN32
	HMODULE module;
	#endif

	if (bs2pc_zlib_initialized) {
		return;
	}
	
	#ifdef _WIN32
	module = LoadLibrary(TEXT("zlibwapi.dll"));
	if (module == NULL) {
		fputs("Couldn't open zlibwapi.dll.\n", stderr);
		exit(EXIT_FAILURE);
	}
	bs2pc_zlib_inflateInit_ = (bs2pc_zlib_inflateInit__t) GetProcAddress(module, "inflateInit_");
	bs2pc_zlib_inflate = (bs2pc_zlib_inflate_t) GetProcAddress(module, "inflate");
	bs2pc_zlib_inflateEnd = (bs2pc_zlib_inflateEnd_t) GetProcAddress(module, "inflateEnd");
	bs2pc_zlib_deflateInit_ = (bs2pc_zlib_deflateInit__t) GetProcAddress(module, "deflateInit_");
	bs2pc_zlib_deflateBound = (bs2pc_zlib_deflateBound_t) GetProcAddress(module, "deflateBound");
	bs2pc_zlib_deflate = (bs2pc_zlib_deflate_t) GetProcAddress(module, "deflate");
	bs2pc_zlib_deflateEnd = (bs2pc_zlib_deflateEnd_t) GetProcAddress(module, "deflateEnd");
	#else
	#error No zlib loading code for this platform.
	#endif

	if (bs2pc_zlib_inflateInit_ == NULL || bs2pc_zlib_inflate == NULL || bs2pc_zlib_inflateEnd == NULL ||
			bs2pc_zlib_deflateInit_ == NULL || bs2pc_zlib_deflateBound == NULL ||
			bs2pc_zlib_deflate == NULL || bs2pc_zlib_deflateEnd == NULL) {
		fputs("Couldn't get a zlib function.\n", stderr);
		exit(EXIT_FAILURE);
	}

	bs2pc_zlib_initialized = true;
}
#endif // ZLIB_STATIC

void BS2PC_Decompress(const void *source, unsigned int sourceSize, void *target, unsigned int targetSize) {
	z_stream stream;

	BS2PC_InitializeZlib();

	memset(&stream, 0, sizeof(stream));
	stream.avail_in = sourceSize;
	stream.next_in = (Bytef *) source;
	stream.avail_out = targetSize;
	stream.next_out = (Bytef *) target;
	if (bs2pc_zlib_inflateInit_(&stream, ZLIB_VERSION, sizeof(stream)) != Z_OK) {
		fputs("Couldn't initialize decompression.\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (bs2pc_zlib_inflate(&stream, Z_FINISH) != Z_STREAM_END) {
		fputs("Couldn't decompress the data.\n", stderr);
		exit(EXIT_FAILURE);
	}
	bs2pc_zlib_inflateEnd(&stream);
}

void *BS2PC_CompressWithSize(const void *source, unsigned int sourceSize, unsigned int *outTargetSize) {
	z_stream stream;
	unsigned int targetSize;
	void *target;

	BS2PC_InitializeZlib();
	
	memset(&stream, 0, sizeof(stream));
	stream.avail_in = sourceSize;
	stream.next_in = (Bytef *) source;
	if (bs2pc_zlib_deflateInit_(&stream, Z_BEST_COMPRESSION, ZLIB_VERSION, sizeof(stream)) != Z_OK) {
		fputs("Couldn't initialize compression.\n", stderr);
		exit(EXIT_FAILURE);
	}
	targetSize = bs2pc_zlib_deflateBound(&stream, sourceSize);
	target = BS2PC_Alloc(targetSize + sizeof(unsigned int), false);
	*((unsigned int *) target) = sourceSize;
	stream.avail_out = targetSize;
	stream.next_out = (Bytef *) ((char *) target + sizeof(unsigned int));
	if (bs2pc_zlib_deflate(&stream, Z_FINISH) != Z_STREAM_END) {
		fputs("Couldn't compress the data.\n", stderr);
		exit(EXIT_FAILURE);
	}
	targetSize -= stream.avail_out;
	bs2pc_zlib_deflateEnd(&stream);
	if (outTargetSize != NULL) {
		*outTargetSize = targetSize + sizeof(unsigned int);
	}

	// [SDP] fix #7: give warning if resulting map file is too big
	#define MiB	(float)(1024*1024)
	float DecomprSize = sourceSize / MiB;
	float ComprSize = *outTargetSize / MiB;
	float TotalSize = DecomprSize + ComprSize;
	fprintf(stderr, "\tDecompressed map size:\t %.2f MiB \n", DecomprSize);
	fprintf(stderr, "\tCompressed map size:\t %.2f MiB \n", ComprSize);
//	fprintf(stderr, "\tTotal map size:\t\t %.2f MiB \n", TotalSize);
	if (DecomprSize > 8.0f || TotalSize > 12.0f)
		fputs("[WARNING] map file is too big and can cause crashes\n", stderr);

#if 1 // Evaluate PS2 RAM that will be used for map
	dmodel_id_t *mdls = (dmodel_id_t *) BS2PC_IdLump(LUMP_ID_MODELS);
	dface_id_t *faces = (dface_id_t *) BS2PC_IdLump(LUMP_ID_FACES);
	int nummdl = BS2PC_IdLumpSize(LUMP_ID_MODELS) / sizeof(dmodel_id_t);
	int numedgerefs, m, f;

	for (numedgerefs = 0, m = 0; m < nummdl; m++)
		for (f = mdls[m].firstface; f < (mdls[m].firstface + mdls[m].numfaces); f++)
			numedgerefs += faces[f].numedges;

	// Purely empirical, may be inaccurate
	int mem_surf = (numedgerefs * 2 * 0x10 + 0x10000) & ~0xFFFF;
	int mem_lm = (BS2PC_IdLumpSize(LUMP_ID_LIGHTING) * 4 / 3 + 0x10000) & ~0xFFFF;
	int mem_total = sourceSize + mem_surf + mem_lm;
	fprintf(stderr, "\tEstimated PS2 RAM usage (experimental):\n");
	fprintf(stderr, "\t\tBS2 map:            %.2f MiB (%d b)\n", DecomprSize, sourceSize);
	fprintf(stderr, "\t\tLigtmaps:           %.2f MiB (%d b)\n", (float) mem_lm / MiB, mem_lm);
	fprintf(stderr, "\t\tSurfaceDisplayList: %.2f MiB (%d b)\n", (float) mem_surf / MiB, mem_surf);
	fprintf(stderr, "\t\tTotal:              %.2f MiB (%d b)\n", (float) mem_total / MiB, mem_total);
#endif

#if 0 // Further assumptions on PS2 RAM usage
	#define PS2_AVAIL_MEM	0x011F5600
	#define PS2_MEM_OTHER	(0x3cd3c0 /* valve.pak */ + \
		540864 /* edicts */ + 104128 /* client */ + \
		131136 /* zone */ + (67648 * 6) /* skybox */)
	int mem_left = PS2_AVAIL_MEM - mem_total - PS2_MEM_OTHER;
	fprintf(stderr, "\t\t==========================================\n");
	fprintf(stderr, "\t\tPS2 mem pool:       %.2f MiB (%d b)\n", (float) PS2_AVAIL_MEM / MiB, PS2_AVAIL_MEM);
	fprintf(stderr, "\t\tMap file:           %.2f MiB (%d b)\n", (float) mem_total / MiB, mem_total);
	fprintf(stderr, "\t\tOther:              %.2f MiB (%d b)\n", (float) PS2_MEM_OTHER / MiB, PS2_MEM_OTHER);
	fprintf(stderr, "\t\tMem left for cache: %.2f MiB (%d b)\n", (float) mem_left / MiB, mem_left);
#endif

	return target;
}

// GL_Resample8BitTexture from Quake.
void BS2PC_ResampleTexture(const unsigned char *in, int inwidth, int inheight, unsigned char *out, int outwidth, int outheight) {
	int i, j;
	const unsigned char *inrow;
	unsigned frac, fracstep;

	fracstep = inwidth * 0x10000 / outwidth;
	for (i = 0; i < outheight; ++i, out += outwidth)
	{
		inrow = in + inwidth * (i * inheight / outheight);
		frac = fracstep >> 1;
		for (j = 0; j < outwidth; j += 4)
		{
			out[j] = inrow[frac >> 16];
			frac += fracstep;
			out[j + 1] = inrow[frac >> 16];
			frac += fracstep;
			out[j + 2] = inrow[frac >> 16];
			frac += fracstep;
			out[j + 3] = inrow[frac >> 16];
			frac += fracstep;
		}
	}
}

int BS2PC_CompareTextureNames(const char *name1, const char *name2) {
	unsigned int index;
	for (index = 0; index < 15; ++index) {
		int char1 = name1[index], char2 = name2[index];
		if (char1 >= 'a' && char1 <= 'z') {
			char1 -= 'a' - 'A';
		}
		if (char2 >= 'a' && char2 <= 'z') {
			char2 -= 'a' - 'A';
		}
		if (char1 != char2) {
			return char1 - char2;
		}
		if (char1 == '\0') {
			break;
		}
	}
	return 0;
}