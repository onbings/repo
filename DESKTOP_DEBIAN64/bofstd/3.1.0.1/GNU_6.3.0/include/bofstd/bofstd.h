/*
 * Copyright (c) 2015-2020, Onbings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module defines the bof module interface
 *
 * Name:        bofosd.h
 * Author:      Bernard HARMEL: onbings@gmail.com
 * Web:			    onbings.dscloud.me
 * Revision:    1.0
 *
 * Rem:         Nothing
 *
 * History:
 *
 * V 1.00  Dec 26 2013  BHA : Initial release
 */
#pragma once

/*** Include files ***********************************************************/
#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include <string.h>
#include "boferr.h"

/*** Global variables ********************************************************/
using BofAssertCallback = std::function<BOFERR(const std::string &_rFile_S, uint32_t _Line_U32, const std::string &_rMasg_S)>;

struct BOFSTDPARAM
{
		bool AssertInRelease_B;
		BofAssertCallback AssertCallback;

		BOFSTDPARAM()
		{
			Reset();
		}

		void Reset()
		{
			AssertInRelease_B = false;
			AssertCallback = nullptr;
		}
};

extern BOFSTDPARAM GL_BofStdParam_X;

/*** Definitions *************************************************************/

#if defined __linux__
#if ((__GNUC__ < 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ < 9)))  //For H3XP platform for example
#include<memory>
namespace std {
		template< typename T> struct _Unique_if {
				typedef unique_ptr<T> _Single_object;
		};

		template< typename T> struct _Unique_if<T[]> {
				typedef unique_ptr<T[]> _Unknown_bound;
		};

		template< typename T, size_t N> struct _Unique_if<T[N]> {
				typedef void _Known_bound;
		};

		template< typename T, class... Args>
				typename _Unique_if<T>::_Single_object
				make_unique(Args&&... args) {
						return unique_ptr<T>(new T(std::forward<Args>(args)...));
				}

		template< typename T>
				typename _Unique_if<T>::_Unknown_bound
				make_unique(size_t n) {
						typedef typename remove_extent<T>::type U;
						return unique_ptr<T>(new U[n]());
				}

		template< typename T, class... Args>
				typename _Unique_if<T>::_Known_bound
				make_unique(Args&&...) = delete;
}
#endif
#endif

#define BOF_NAMESPACE               onbings::bof
#define BEGIN_BOF_NAMESPACE()       namespace onbings { namespace bof {
#define END_BOF_NAMESPACE()         } }
#define USE_BOF_NAMESPACE()         using namespace BOF_NAMESPACE;

#if defined( _WIN32 )
#define BOF_LITTLE_ENDIAN
#else
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define BOF_LITTLE_ENDIAN
#endif
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define BOF_BIG_ENDIAN
#endif
#if defined( __BIG_ENDIAN__ )
#define BOF_BIG_ENDIAN
#endif
#if defined( __LITTLE_ENDIAN__ )
#define BOF_LITTLE_ENDIAN
#endif
#endif
#if defined( BOF_BIG_ENDIAN )
#elif defined( BOF_LITTLE_ENDIAN )
#else
#error Define one of BOF_LITTLE_ENDIAN or BOF_BIG_ENDIAN
#endif

/*** Class *******************************************************************/
#if defined (_WIN32)
#define strdup                      _strdup
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#if (_MSC_VER <= 1800)
#define snprintf                    _snprintf
#define unlink                      _unlink
#else
#define HAS_NOEXCEPT
#endif
#else
#endif

#if !defined(HAS_NOEXCEPT)
#if defined(__clang__)
#if __has_feature(cxx_noexcept)
#define HAS_NOEXCEPT
#endif
#else
#if defined(__GXX_EXPERIMENTAL_CXX0X__) && __GNUC__ * 10 + __GNUC_MINOR__ >= 46 || \
    defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023026
#define HAS_NOEXCEPT
#endif
#endif
#endif

#ifdef HAS_NOEXCEPT
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#define constexpr
#endif

BEGIN_BOF_NAMESPACE()

#define BOF_INVALID_HANDLE_VALUE    ( (void *)-1)
const intptr_t BOF_FS_INVALID_HANDLE = -1;

#define BOF_COMPUTE_DELTA(start, end, delta)                                            { delta = ( end > start ) ? ( end - start ) : ( start - end ); }
#define BOF_SAFE_FREE(p)                                                                { if ( (p) ) { free ((void *)p); }      (p) = nullptr; }        /*! free a buffer */
#define BOF_SAFE_DELETE(p)                                                              { if ( (p) ) { delete (p); }      (p) = nullptr; }        /*! Delete an object*/
#define BOF_SAFE_DELETE_ARRAY(p)                                                        { if ( (p) ) { delete[] (p); }         (p) = nullptr; } /*! Delete an array of objects*/
#define BOF_NB_ELEM_IN_ARRAY(x)                                                         (sizeof( (x) ) / sizeof( (x) [0]) )
#define BOF_STRUCT_FIELD_OFFSET(Type, Field)                                            (uintptr_t)(&( ( (Type *)0)->Field) )               /*! Get structure field offset in byte*/
#define BOF_STRUCT_FIELD_SIZE(Type, Field)                                              (sizeof( ( (Type *)0)->Field) )                     /*! Get structure field size in byte*/
#define BOF_SETORRESET_BIT_IN_BYTESTREAM(SetOrReset, BitNum, pByteStream)               { if (SetOrReset) { pByteStream [BitNum >> 3] |= (1 << (BitNum & 0x07) ); } else{ pByteStream [BitNum >> 3] &= (0xFF - (1 << (BitNum & 0x07) ) ); } }
#define BOF_ISBIT_IN_BYTESTREAM_SET(Sts, BitNum, pByteStream)                           { Sts = (pByteStream [BitNum >> 3] & (1 << (BitNum & 0x07) ) ) ? 1 : 0; }
//#define BOF_ALIGN_PADDING_MAX(v, a)                                                   ((((v) % (a)) != 0) ? ((a)-((v) % (a))) : a)  //If aligned add a zone of a byte before the next one
#define BOF_ALIGN_ADD_NB_PADDING_BYTE(v, a)                                             ((((v) % (a)) != 0) ? ((a)-((v) % (a))) : 0)  //If aligned add a zone of a byte before the next one
#define BOF_ALIGN_VALUE_ON(v, a)																												(((v) + (a) - 1) & ~((a) - 1))
#define BOF_SNPRINTF_NULL_CLIPPED(pBuffer, MaxBufferSize, Format, ...)                  {snprintf(pBuffer,MaxBufferSize,Format,##__VA_ARGS__);pBuffer[MaxBufferSize-1]=0;}
#define BOF_STRNCPY_NULL_CLIPPED(pDst, pSrc, Count)                                     {strncpy(pDst, pSrc, Count);pDst[Count-1]=0;}
#define BOF_SET_ADDRESS_MAGIC_NUMBER(p, mn)                                             {(p)->MagicNumber_U64=(reinterpret_cast<uint64_t>(p) ^ static_cast<uint64_t>(mn));}
#define BOF_CHECK_ADDRESS_MAGIC_NUMBER(s, p, mn)                                        {if (p) {s=((reinterpret_cast<uint64_t>(p) ^ (p)->MagicNumber_U64)==static_cast<uint64_t>(mn));} else s=0;}
/*https://stackoverflow.com/questions/15832301/understanding-container-of-macro-in-the-linux-kernel
Taking this container for example:
struct container {
	int some_other_data;
	int this_data; 
}
And a pointer int *my_ptr to the this_data member you'd use the macro to get a pointer to struct container *my_container by using:
struct container *my_container;	my_container = container_of(my_ptr, struct container, this_data);
*/
#define BOF_CONTAINER_OF(ptr, type, member)																							({const typeof( ((type *)0)->member ) *__mptr = (ptr);    (type *)( (char *)__mptr - offsetof(type,member) );})

// https://riptutorial.com/cplusplus/example/15335/casting-std--shared-ptr-pointers*/
// Note that std::reinterpret_pointer_cast is not available in C++11 and C++14, as it was only proposed by N3920 and adopted into Library Fundamentals TS in February 2014.
// However, it can be implemented as follows:
template <typename To, typename From>
inline std::shared_ptr<To> reinterpret_pointer_cast(std::shared_ptr<From> const & ptr) noexcept { return std::shared_ptr<To>(ptr, reinterpret_cast<To *>(ptr.get())); }

#define BOF_EXTERN_C                extern "C"
#define BOF_EXTERN_C_BEGIN          extern "C" {
#define BOF_EXTERN_C_END            };
extern uint32_t GL_BofDbgPrintfStartTime_U32;
#define BOF_DBG_PRINTF(Format, ...)   {printf("%10d [%08X] " Format, BOF_NAMESPACE::Bof_GetMsTickCount()-BOF_NAMESPACE::GL_BofDbgPrintfStartTime_U32,BOF_NAMESPACE::Bof_CurrentThreadId(),  ##__VA_ARGS__);}

#if defined (DEBUG)
// #define BOF_ASSERT(_Expression)																													assert( (_Expression) )
// This one is better to use with gtest to avoid gtest aborf
//#define BOF_ASSERT(_Expression)                                                         (void)( ( (_Expression) ) || (GL_BofStdParam_X.AssertCallback ? GL_BofStdParam_X.AssertCallback(__FILE__,__LINE__,#_Expression):printf("Assertion failed: %s, file %s, line %d\nPress any key followed by enter to abort the application...",#_Expression, __FILE__, __LINE__), getchar(), abort(), 0) )
#define BOF_ASSERT(_Expression)    (void)( ( (_Expression) ) || (GL_BofStdParam_X.AssertCallback  ? static_cast<int>(GL_BofStdParam_X.AssertCallback(__FILE__,__LINE__,#_Expression)):printf("Assertion failed: %s, file %s, line %d\nPress any key followed by enter to abort the application...",#_Expression, __FILE__, __LINE__), getchar(), abort(), 0) )
#else
// #define BOF_ASSERT(_Expression)																													assert( (_Expression) )	//Void op by default in release
// Microsoft assert.h: #define assert(_Expression) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )
//#define BOF_ASSERT(_Expression)                                                         if (GL_BofStdParam_X.AssertInRelease_B){(void)( ( (_Expression) ) || (GL_BofStdParam_X.AssertCallback ? GL_BofStdParam_X.AssertCallback(__FILE__,__LINE__,#_Expression):printf("Assertion failed: %s, file %s, line %d\nPress any key followed by enter to abort the application...",#_Expression, __FILE__, __LINE__), getchar(), abort(), 0) );}
#define BOF_ASSERT(_Expression)                                                         if (GL_BofStdParam_X.AssertInRelease_B){(void)( ( (_Expression) ) || (GL_BofStdParam_X.AssertCallback ? static_cast<int>(GL_BofStdParam_X.AssertCallback(__FILE__,__LINE__,#_Expression)):printf("Assertion failed: %s, file %s, line %d\nPress any key followed by enter to abort the application...",#_Expression, __FILE__, __LINE__), getchar(), abort(), 0) );}
//#define BOF_ASSERT(_Expression)
#endif

/*
 * 3 steps:
 * 1. Check if bit in first byte of current byte stream position
 * 2. If not found find first byte containing bits
 * 3. Scan byte to find bits
 */
#define BOF_LOOKFORBIT_BIT_IN_BYTESTREAM(SetOrReset, BitNum, MaxBitNum, pByteStream)    { unsigned int i; bool StsBit; U32 FoundBitNum = (U32)-1; uint8_t MaskOfFirstByte, Val; MaskOfFirstByte = 0xFF ^ ( (1 << (BitNum & 0x07) ) - 1); if (SetOrReset) { StsBit = (pByteStream [(BitNum >> 3)] & MaskOfFirstByte); } else{ StsBit = (pByteStream [(BitNum >> 3)] & MaskOfFirstByte) != MaskOfFirstByte; }       if (! StsBit) { Val = SetOrReset ? 0x00 : 0xFF; for (i = (BitNum >> 3) + 1 ; i < (MaxBitNum >> 3) ; i++) { if (pByteStream [i] != Val) { StsBit = true; break; } }     BitNum = i << 3; }   if (StsBit) { for (i = BitNum ; i < BitNum + 8 ; i++) { BOF_ISBIT_IN_BYTESTREAM_SET(StsBit, i, pByteStream); if (StsBit == SetOrReset) { FoundBitNum = i; break; } } }      BitNum = FoundBitNum; }

#define BOF_SWAP16(ValToSwap)                                            \
  { ( (uint8_t *)&(ValToSwap) ) [0] ^= ( (uint8_t *)&(ValToSwap) ) [1];  \
    ( (uint8_t *)&(ValToSwap) ) [1] ^= ( (uint8_t *)&(ValToSwap) ) [0];  \
    ( (uint8_t *)&(ValToSwap) ) [0] ^= ( (uint8_t *)&(ValToSwap) ) [1]; }

#define BOF_SWAP16PTR(pValToSwap)                                        \
  { ( (uint8_t *)(pValToSwap) ) [0] ^= ( (uint8_t *)(pValToSwap) ) [1];  \
    ( (uint8_t *)(pValToSwap) ) [1] ^= ( (uint8_t *)(pValToSwap) ) [0];  \
    ( (uint8_t *)(pValToSwap) ) [0] ^= ( (uint8_t *)(pValToSwap) ) [1]; }

#define BOF_SWAP32(ValToSwap)                                            \
  { ( (uint8_t *)&(ValToSwap) ) [0] ^= ( (uint8_t *)&(ValToSwap) ) [3];  \
    ( (uint8_t *)&(ValToSwap) ) [3] ^= ( (uint8_t *)&(ValToSwap) ) [0];  \
    ( (uint8_t *)&(ValToSwap) ) [0] ^= ( (uint8_t *)&(ValToSwap) ) [3];  \
    ( (uint8_t *)&(ValToSwap) ) [1] ^= ( (uint8_t *)&(ValToSwap) ) [2];  \
    ( (uint8_t *)&(ValToSwap) ) [2] ^= ( (uint8_t *)&(ValToSwap) ) [1];  \
    ( (uint8_t *)&(ValToSwap) ) [1] ^= ( (uint8_t *)&(ValToSwap) ) [2]; }

#define BOF_SWAP32PTR(pValToSwap)                                        \
  { ( (uint8_t *)(pValToSwap) ) [0] ^= ( (uint8_t *)(pValToSwap) ) [3];  \
    ( (uint8_t *)(pValToSwap) ) [3] ^= ( (uint8_t *)(pValToSwap) ) [0];  \
    ( (uint8_t *)(pValToSwap) ) [0] ^= ( (uint8_t *)(pValToSwap) ) [3];  \
    ( (uint8_t *)(pValToSwap) ) [1] ^= ( (uint8_t *)(pValToSwap) ) [2];  \
    ( (uint8_t *)(pValToSwap) ) [2] ^= ( (uint8_t *)(pValToSwap) ) [1];  \
    ( (uint8_t *)(pValToSwap) ) [1] ^= ( (uint8_t *)(pValToSwap) ) [2]; }

#define BOF_SWAP64(ValToSwap)                                            \
  { ( (uint8_t *)&(ValToSwap) ) [0] ^= ( (uint8_t *)&(ValToSwap) ) [7];  \
    ( (uint8_t *)&(ValToSwap) ) [7] ^= ( (uint8_t *)&(ValToSwap) ) [0];  \
    ( (uint8_t *)&(ValToSwap) ) [0] ^= ( (uint8_t *)&(ValToSwap) ) [7];  \
    ( (uint8_t *)&(ValToSwap) ) [1] ^= ( (uint8_t *)&(ValToSwap) ) [6];  \
    ( (uint8_t *)&(ValToSwap) ) [6] ^= ( (uint8_t *)&(ValToSwap) ) [1];  \
    ( (uint8_t *)&(ValToSwap) ) [1] ^= ( (uint8_t *)&(ValToSwap) ) [6];  \
    ( (uint8_t *)&(ValToSwap) ) [2] ^= ( (uint8_t *)&(ValToSwap) ) [5];  \
    ( (uint8_t *)&(ValToSwap) ) [5] ^= ( (uint8_t *)&(ValToSwap) ) [2];  \
    ( (uint8_t *)&(ValToSwap) ) [2] ^= ( (uint8_t *)&(ValToSwap) ) [5];  \
    ( (uint8_t *)&(ValToSwap) ) [3] ^= ( (uint8_t *)&(ValToSwap) ) [4];  \
    ( (uint8_t *)&(ValToSwap) ) [4] ^= ( (uint8_t *)&(ValToSwap) ) [3];  \
    ( (uint8_t *)&(ValToSwap) ) [3] ^= ( (uint8_t *)&(ValToSwap) ) [4]; }

#define BOF_SWAP64PTR(pValToSwap)                                        \
  { ( (uint8_t *)(pValToSwap) ) [0] ^= ( (uint8_t *)(pValToSwap) ) [7];  \
    ( (uint8_t *)(pValToSwap) ) [7] ^= ( (uint8_t *)(pValToSwap) ) [0];  \
    ( (uint8_t *)(pValToSwap) ) [0] ^= ( (uint8_t *)(pValToSwap) ) [7];  \
    ( (uint8_t *)(pValToSwap) ) [1] ^= ( (uint8_t *)(pValToSwap) ) [6];  \
    ( (uint8_t *)(pValToSwap) ) [6] ^= ( (uint8_t *)(pValToSwap) ) [1];  \
    ( (uint8_t *)(pValToSwap) ) [1] ^= ( (uint8_t *)(pValToSwap) ) [6];  \
    ( (uint8_t *)(pValToSwap) ) [2] ^= ( (uint8_t *)(pValToSwap) ) [5];  \
    ( (uint8_t *)(pValToSwap) ) [5] ^= ( (uint8_t *)(pValToSwap) ) [2];  \
    ( (uint8_t *)(pValToSwap) ) [2] ^= ( (uint8_t *)(pValToSwap) ) [5];  \
    ( (uint8_t *)(pValToSwap) ) [3] ^= ( (uint8_t *)(pValToSwap) ) [4];  \
    ( (uint8_t *)(pValToSwap) ) [4] ^= ( (uint8_t *)(pValToSwap) ) [3];  \
    ( (uint8_t *)(pValToSwap) ) [3] ^= ( (uint8_t *)(pValToSwap) ) [4]; }

#define BOF_BIND_0_ARG_TO_METHOD(Class_Method)                                          std::bind(&Class_Method, this)
#define BOF_BIND_1_ARG_TO_METHOD(Class_Method)                                          std::bind(&Class_Method, this, std::placeholders::_1)
#define BOF_BIND_2_ARG_TO_METHOD(Class_Method)                                          std::bind(&Class_Method, this, std::placeholders::_1, std::placeholders::_2)
#define BOF_BIND_3_ARG_TO_METHOD(Class_Method)                                          std::bind(&Class_Method, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
#define BOF_BIND_4_ARG_TO_METHOD(Class_Method)                                          std::bind(&Class_Method, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
#define BOF_BIND_5_ARG_TO_METHOD(Class_Method)                                          std::bind(&Class_Method, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)
#define BOF_BIND_6_ARG_TO_METHOD(Class_Method)                                          std::bind(&Class_Method, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)
#define BOF_BIND_7_ARG_TO_METHOD(Class_Method)                                          std::bind(&Class_Method, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7)
#define BOF_BIND_8_ARG_TO_METHOD(Class_Method)                                          std::bind(&Class_Method, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8)


BOFERR Bof_Initialize(const BOFSTDPARAM &_rStdParam_X);

BOFERR Bof_Shutdown();

std::string Bof_GetVersion();

const char *Bof_ErrorCode(BOFERR _ErrorCode_E);

bool Bof_IsCpuLittleEndian();

template<typename T>
BOFERR Bof_StringToBin(uint32_t _Base_U32, const char *_pAsciiNumber_c, T &_rConvertedValue)
{
	BOFERR Rts_E = BOF_ERR_EINVAL;
	char * p_c;
	int64_t Nb_S64;

	_rConvertedValue = 0;
	if ((_pAsciiNumber_c) && ((_Base_U32 == 0) || ((_Base_U32 >= 2) && (_Base_U32 <= 36))))
	{
		Nb_S64 = strtol(_pAsciiNumber_c, &p_c, _Base_U32);
		if (*p_c == 0) //else not a number
		{
			Rts_E = BOF_ERR_NO_ERROR;
			_rConvertedValue = static_cast<T>(Nb_S64);
		}
		else
		{
			Rts_E = BOF_ERR_ENOKEY;
		}
	}
	return Rts_E;
}

END_BOF_NAMESPACE()