#ifndef _TYPE_H_
#define _TYPE_H_

#include  <stdbool.h>

#define BOOL bool

#define MAX_VAL(a, b) (((a) > (b)) ? (a) : (b))
#define MIN_VAL(a, b) (((a) < (b)) ? (a) : (b))
#define ABS(a)     (((a) < 0) ? (-(a)) : (a))

#define MHZ           *1000000l
#define KHZ           *1000l
#define HZ            *1l

#ifndef FALSE
#define FALSE (1 == 0)
#endif

#ifndef TRUE
#define TRUE (1==1)
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef double                      FP64;    // Double precision floating point
typedef double                    * pFP64;
typedef float                       FP32;    // Single precision floating point
typedef float                     * pFP32;
typedef signed   long long          INT64S;   // Signed   64 bit quantity
typedef signed   long long        * pINT64S;
typedef unsigned long long          INT64U;   // Unsigned 64 bit quantity
typedef unsigned long long        * pINT64U;
typedef signed   int                INT32S;   // Signed   32 bit quantity
typedef signed   int              * pINT32S;
typedef unsigned int                INT32U;   // Unsigned 32 bit quantity
typedef unsigned int              * pINT32U;
typedef signed   short              INT16S;   // Signed   16 bit quantity
typedef signed   short            * pINT16S;
typedef unsigned short              INT16U;   // Unsigned 16 bit quantity
typedef unsigned short            * pINT16U;
typedef signed   char               INT8S;    // Signed    8 bit quantity
typedef signed   char             * pINT8S;
typedef unsigned char               INT8U;    // Unsigned  8 bit quantity
typedef unsigned char             * pINT8U;
typedef unsigned char               BOOLEAN;  // Boolean
typedef unsigned char             * pBOOLEAN;

typedef unsigned char               bool_t;
typedef unsigned char               Bool_t;

typedef signed char                 int8_t;
typedef unsigned char               uint8_t;
typedef volatile signed char        vint8_t;
typedef volatile unsigned char      vuint8_t;

typedef signed short                int16_t;
typedef unsigned short              uint16_t;
typedef volatile signed short       vint16_t;
typedef volatile unsigned short     vuint16_t;

typedef signed int                  int32_t;
typedef unsigned int                uint32_t;
typedef unsigned int                size_t;
typedef volatile signed int         vint32_t;
typedef volatile unsigned int       vuint32_t;

typedef signed long long            int64_t;
typedef unsigned long long          uint64_t;
typedef volatile signed long long   vint64_t;
typedef volatile unsigned long long vuint64_t;

#endif


