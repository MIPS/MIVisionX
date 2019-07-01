#ifndef __print_helper__ 
#define __print_helper__ 

#ifndef TIME_MEASURE
#define TIME_MEASURE 1
#endif

#if TIME_MEASURE

static int64_t timeStart;
static int64_t timeEnd;
static struct timespec tp;

static unsigned int cycleStart, cycleEnd;

#define CYCLE_START	__asm __volatile(\
	"rdhwr %0,$2\n\t":"=r"(cycleStart)\
	);

#define CYCLE_END     __asm __volatile(\
	"rdhwr %0,$2\n\t":"=r"(cycleEnd)\
	);

#define CYCLE_PRINT	printf("[  CYCLE   ] %s: %u\n", __func__, ((cycleEnd - cycleStart) * 2));

#define TIME_END clock_gettime(CLOCK_MONOTONIC, &tp); \
	timeEnd = (int64_t) tp.tv_sec * 1e9 + tp.tv_nsec;

#define TIME_START clock_gettime(CLOCK_MONOTONIC, &tp); \
	timeStart = (int64_t) tp.tv_sec * 1e9 + tp.tv_nsec;

#define TIME_PRINT printf("[   TIME   ] %s: %.1f ns\n", __func__, (timeEnd - timeStart) * 1.);

#define TIME_PRINT_ms printf("[   TIME	 ] %s: %.1f ms\n", __func__, (timeEnd - timeStart) * 1000. / 1e9);

#else

#define CYCLE_START
#define CYCLE_END
#define CYCLE_PRINT

#define TIME_END
#define TIME_START
#define TIME_PRINT
#define TIME_PRINT_ms

#endif

#ifndef DEBUG
#define DEBUG 1
#endif

#if DEBUG

// print vectors

static int8_t *val8;
static int16_t *val16;
static int32_t *val32;
static int64_t *val64;
static float *valf32;
static uint8_t *valu8;
static uint16_t *valu16;
static uint32_t *valu32;

#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c "
#define PRINTF_BYTE_TO_BINARY_INT8(i)	 \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
    PRINTF_BINARY_PATTERN_INT8		    PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8(i),   PRINTF_BYTE_TO_BINARY_INT8((i) >> 8)
#define PRINTF_BINARY_PATTERN_INT32 \
    PRINTF_BINARY_PATTERN_INT16		    PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
    PRINTF_BYTE_TO_BINARY_INT16(i), PRINTF_BYTE_TO_BINARY_INT16((i) >> 16)
#define PRINTF_BINARY_PATTERN_INT64    \
    PRINTF_BINARY_PATTERN_INT32		    PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
    PRINTF_BYTE_TO_BINARY_INT32(i), PRINTF_BYTE_TO_BINARY_INT32((i) >> 32)


#define PRINT8(val) printf("v16_i8: %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i\n",\
	   val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7],\
	   val[8], val[9], val[10], val[11], val[12], val[13], val[14], val[15]);

#define PRINTu8(val) printf("v16_u8: %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",\
	   val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7],\
	   val[8], val[9], val[10], val[11], val[12], val[13], val[14], val[15]);

#define PRINT16(val) printf("v8_i16: %i %i %i %i %i %i %i %i \n",\
	   val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);

#define PRINTu16(val) printf("v8_u16: %u %u %u %u %u %u %u %u \n",\
	   val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);

#define PRINT32(val) printf("v4_i32: %i %i %i %i \n",\
	   val[0], val[1], val[2], val[3]);

#define PRINTu32(val) printf("v4_u32: %u %u %u %u \n",\
	   val[0], val[1], val[2], val[3]);

#define PRINTf32(val) printf("v4_f32: %f %f %f %f \n",\
	   val[0], val[1], val[2], val[3]);

#define PRINT128(val) printf("v128: " PRINTF_BINARY_PATTERN_INT64 \
				   PRINTF_BINARY_PATTERN_INT64 "\n",\
				   PRINTF_BYTE_TO_BINARY_INT64(val[0]),\
				   PRINTF_BYTE_TO_BINARY_INT64(val[1]));

#define PRINTv16i8(vector) printf("%s ", #vector);\
	   val8 = (int8_t *) &vector;\
	   PRINT8(val8)

#define PRINTv16i8N(vector) \
           val8 = (int8_t*) &vector;\
           printf("%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n",\
           #vector, val8[0], #vector, val8[1], #vector, val8[2], #vector, val8[3], #vector, val8[4], #vector, val8[5], #vector, val8[6], #vector, val8[7],\
           #vector, val8[8], #vector, val8[9], #vector, val8[10], #vector, val8[11], #vector, val8[12], #vector, val8[13], #vector, val8[14], #vector, val8[15]);

#define PRINTv16u8(vector) printf("%s ", #vector);\
	   valu8 = (uint8_t *) &vector;\
	   PRINTu8(valu8)

#define PRINTv16u8N(vector) \
           valu8 = (uint8_t*) &vector;\
           printf("%s: %u\n%s: %u\n%s: %u\n%s: %u\n%s: %u\n%s: %u\n%s: %u\n%s: %u\n",\
           #vector, valu8[0], #vector, valu8[1], #vector, valu8[2], #vector, valu8[3], #vector, valu8[4], #vector, valu8[5], #vector, valu8[6], #vector, valu8[7],\
           #vector, valu8[8], #vector, valu8[9], #vector, valu8[10], #vector, valu8[11], #vector, valu8[12], #vector, valu8[13], #vector, valu8[14], #vector, valu8[15]);

#define PRINTv8i16(vector) printf("%s ", #vector);\
           val16 = (int16_t *) &vector;\
           PRINT16(val16)

#define PRINTv8i16N(vector) \
           val16 = (int16_t*) &vector;\
           printf("%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n",\
           #vector, val16[0], #vector, val16[1], #vector, val16[2], #vector, val16[3], #vector, val16[4], #vector, val16[5], #vector, val16[6], #vector, val16[7]);

#define PRINTv8u16(vector) printf("%s ", #vector);\
           valu16 = (uint16_t *) &vector;\
           PRINTu16(valu16)

#define PRINTv8u16N(vector) \
           valu16 = (uint16_t*) &vector;\
           printf("%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n",\
           #vector, valu16[0], #vector, valu16[1], #vector, valu16[2], #vector, valu16[3], #vector, valu16[4], #vector, valu16[5], #vector, valu16[6], #vector, valu16[7]);

#define PRINTv8u16(vector) printf("%s ", #vector);\
	   valu16 = (uint16_t *) &vector;\
	   PRINTu16(valu16)

#define PRINTv4i32(vector) printf("%s ", #vector);\
	   val32 = (int32_t *) &vector;\
	   PRINT32(val32)

#define PRINTv4u32(vector) printf("%s ", #vector);\
	   valu32 = (uint32_t *) &vector;\
	   PRINTu32(valu32)

#define PRINTv4f32(vector) printf("%s ", #vector);\
	   valf32 = (float *) &vector;\
	   PRINTf32(valf32);

#define PRINTv128(vector) printf("%s ", #vector);\
	   val64 = (int64_t *) &vector;\
	   PRINT128(val64);

// print info, used for function trace

#define PRINT printf

#define DEBUG_INFO printf("MIPS OVX: %s\n", __func__);
//#define DEBUG_INFO printf("MIPS OVX: %s, %s, %d\n", __func__, __FILE__+strlen(__FILE__)-30, __LINE__);

#define DEBUG_INFO_U printf("MIPS OVX UNIMPLEMENTED: %s\n", __func__);
//#define DEBUG_INFO_U printf("MIPS OVX UNIMPLEMENTED: %s, %s, %d\n", __func__, __FILE__+strlen(__FILE__)-30, __LINE__);

#else

#define PRINTv16u8(vector)
#define PRINTv16i8(vector)
#define PRINTv8u16(vector)
#define PRINTv8i16(vector)
#define PRINTv4u32(vector)
#define PRINTv4i32(vector)
#define PRINTv4f32(vector)
#define PRINTv128(vector)

#define PRINT
#define DEBUG_INFO
#define DEBUG_INFO_U

#endif

#endif // __print_helper__
