/*
 * Copyright (c) 2000-2020, Onbings All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module defines the bofsystem interface. It wraps os dependent system call
 *
 * Name:        bofsystem.h
 * Author:      Bernard HARMEL: b.harmel@gmail.com
 * Revision:    1.0
 *
 * Rem:         Nothing
 *
 * History:
 *
 * V 1.00  Jan 19 2017  BHA : Initial release
 */
#pragma once

/*** Include ****************************************************************/

#include <algorithm>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <bofstd/bofstd.h>

#if defined (_WIN32)
char * strptime(const char *buf, const char *fmt, struct tm *tm);
#else
#endif

BEGIN_BOF_NAMESPACE()

#define BOF_MS_TO_S(v)       (static_cast< uint32_t > ( (v) / 1e3) )
#define BOF_NANO_TO_MS(v)    (static_cast< uint64_t > ( (v) / 1e6) )
#define BOF_NANO_TO_S(v)     (static_cast< uint64_t > ( (v) / 1e9) )

#define BOF_S_TO_MS(v)       (static_cast< uint32_t > ( (v) * 1e3) )
#define BOF_MS_TO_NANO(v)    (static_cast< uint64_t > ( (v) * 1e6) )
#define BOF_S_TO_NANO(v)     (static_cast< uint64_t > ( (v) * 1e9) )


enum class BOF_SEEK_METHOD : uint32_t
{
		BOF_SEEK_BEGIN = 0,                    /*! The starting point is zero or the beginning of the file.*/
		BOF_SEEK_CURRENT,                      /*! The starting point is the current value of the file pointer.*/
		BOF_SEEK_END                           /*! The starting point is the current end-of-file position*/
};
enum class BOF_BUFFER_ALLOCATE_ZONE : uint32_t
{
  BOF_BUFFER_ALLOCATE_ZONE_RAM = 0,
  BOF_BUFFER_ALLOCATE_ZONE_HUGE_PAGE,
//	CMA,      ///< Contiguous Memory Allocator
};
struct BOF_BUFFER_ALLOCATE_HEADER
{
	BOF_BUFFER_ALLOCATE_ZONE AllocateZone_E;
	//	uint32_t SizeInByte_U32;
	int Io_i;
  bool Locked_B;
	char pHugePath_c[128];

	BOF_BUFFER_ALLOCATE_HEADER()
	{
		Reset();
	}

	void Reset()
	{
		AllocateZone_E = BOF_BUFFER_ALLOCATE_ZONE::BOF_BUFFER_ALLOCATE_ZONE_RAM;
		//			SizeInByte_U32=0;
		Io_i = -1;
    Locked_B=false;
		pHugePath_c[0] = 0;
	}
};
struct BOF_BUFFER
{
	bool MustBeDeleted_B;
	uint64_t Size_U64;
	uint64_t Capacity_U64;
	void *pUser;		//Used by Bof_AlignedMemAlloc for example
	uint8_t *pData_U8;

	BOF_BUFFER()
	{
		Reset();
	}
	BOF_BUFFER(uint64_t _Capacity_U64, uint64_t _Size_U64, uint8_t *_pData_U8)
	{
		SetStorage(_Capacity_U64, _Size_U64, _pData_U8);
	}
	/*
	~BOF_BUFFER()
	{
		ReleaseStorage();
	}
	*/
	void Reset()
	{
		MustBeDeleted_B = false;
		pUser = nullptr;
		Size_U64 = 0;
		Capacity_U64 = 0;
		pData_U8 = nullptr;
	}
	void Clear()
	{
		Size_U64 = 0;
	}
	uint8_t *SetStorage(uint64_t _Capacity_U64, uint64_t _Size_U64, uint8_t *_pData_U8)
	{
		BOF_ASSERT(_Capacity_U64 < 0x100000000);	//For the moment
    MustBeDeleted_B = false;
		if (_pData_U8)
		{
			pData_U8 = _pData_U8;
		}
		else
		{
			pData_U8 = AllocStorage(_Capacity_U64);
		}
		Capacity_U64 = _Capacity_U64;
		if (_Size_U64 <= _Capacity_U64)
		{
			Size_U64 = _Size_U64;
		}
		else
		{
			Size_U64 = 0;
		}
		return _pData_U8;
	}
	uint8_t *AllocStorage(uint64_t _Capacity_U64)
	{
		BOF_ASSERT(_Capacity_U64 < 0x100000000);	//For the moment
		uint8_t *pRts = new uint8_t[static_cast<uint32_t>(_Capacity_U64)];
		
		if (pRts)
		{
			MustBeDeleted_B = true;
			Capacity_U64 = _Capacity_U64;
			Size_U64 = 0;
			pData_U8 = pRts;
		}
		return pRts;
	}
	void ReleaseStorage()
	{
		if (MustBeDeleted_B)
		{
			BOF_SAFE_DELETE_ARRAY(pData_U8);
		}
		MustBeDeleted_B = false;
		Capacity_U64 = 0;
		Size_U64 = 0;
	}
	uint8_t *MemCpy(uint64_t _Size_U64, const uint8_t *_pData_U8)
	{
		uint8_t *pRts_U8 = nullptr;

		if ((_pData_U8) && (pData_U8))
		{
			if ((_Size_U64 + Size_U64) <= Capacity_U64)
			{
				memcpy(&pData_U8[Size_U64], _pData_U8, static_cast<size_t>(_Size_U64));
				Size_U64 += _Size_U64;
				pRts_U8 = &pData_U8[Size_U64] + _Size_U64;
			}
		}
		return pRts_U8;
	}
};
enum class BOF_ACCESS_SIZE : uint32_t
{
	BOF_ACCESS_SIZE_8 = 0,                 /*! we access the memory zone using 8 bits byte access.*/
	BOF_ACCESS_SIZE_16,                    /*! we access the memory zone using 16 bits word access.*/
	BOF_ACCESS_SIZE_32,                    /*! we access the memory zone using 32 bits long access.*/
	BOF_ACCESS_SIZE_64,                    /*! we access the memory zone using 64 bits long long access.*/
};

///@return The ascii printable version of the memory zone.
///@remarks For example calling DumpMemoryZone(60, pMemoryZone_U8, 16, ' ', true, 0x12345678,true,true) will produce the following output
///@remarks Virtual  <-------------- Binary Data ------------------> <--Ascii Data-->
///@remarks 12345678 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ????????????????
///@remarks 12345688 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F ????????????????
///@remarks 12345698 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F  !"#$%&'()*+,-./
///@remarks 123456A8 30 31 32 33 34 35 36 37 38 39 3A 3B             0123456789:;
struct BOF_DUMP_MEMORY_ZONE_PARAM
{
	uint32_t NbItemToDump_U32;	///< Specifies the number of item (uint8_t, uint16_t, uint32_t, uint64_t to dump.
	const volatile void *pMemoryZone;		///< Specifies the address of the first byte of the memory zone to dump
	uint32_t NbItemPerLine_U32;					///< Specifies the number of data item to dump per line. (max MAX_NBBYTEPERLINE=1024/sizeof(item))
	char Separator_c;										///< Specifies the character to use between each binary data
	bool ShowHexaPrefix_B;							///< Specifies if c hexa prefix must be displayed in front of each binary data (virtual offset and binary data value)
	bool GenerateVirtualOffset;					///< Specifies if the virtual offset (below) must be generated.
	int64_t VirtualOffset_S64;					///< Specifies a pseudo starting counter value which will be displayed as "the address" of each dumped row. if -1 use the pMemoryZone_U8 value
	bool GenerateBinaryData_B;					///< Specifies if the binary data must be generated.
	bool GenerateAsciiData_B;						///< Specifies if the ascii data must be generated.
	bool ReverseEndianness_B;						///< If true reverse byte ordering on an AccessSize_E (below) boundary.
	BOF_ACCESS_SIZE AccessSize_E;				///< Specifies how the memory zone must be read
	BOF_DUMP_MEMORY_ZONE_PARAM()
	{
		Reset();
	}
	void Reset()
	{
		NbItemToDump_U32 = 0;
		pMemoryZone=nullptr;
		NbItemPerLine_U32=16;
		Separator_c=' ';
		ShowHexaPrefix_B = false;
		GenerateVirtualOffset=true;
		VirtualOffset_S64=-1;
		GenerateBinaryData_B=true;
		GenerateAsciiData_B=true;
		ReverseEndianness_B=false;
		AccessSize_E= BOF_ACCESS_SIZE::BOF_ACCESS_SIZE_8;
	}
};
const uint32_t BOF_MUTEX_MAGIC = 0x01D654AC;

struct BOF_MUTEX
{
		uint32_t Magic_U32;
		std::string Name_S;
		bool Recursive_B;
		std::recursive_mutex RecursiveMtx;
		std::mutex Mtx;

		BOF_MUTEX()
		{
			Reset();
		}

		void Reset()
		{
			Magic_U32 = 0;
			Name_S = "";
			Recursive_B = true;
		}
};

const uint32_t BOF_EVENT_MAGIC = 0x1F564864;

struct BOF_EVENT
{
		uint32_t Magic_U32;
		std::string Name_S;
		std::mutex Mtx;
		std::condition_variable Cv;
//		bool Canceled_B;
		uint32_t MaxNumberToNotify_U32;
		uint64_t SignaledBitmask_U64;
//		bool NotifyAll_B;
		bool WaitKeepSignaled_B;

		BOF_EVENT()
		{
			Reset();
		}

		void Reset()
		{
			Magic_U32 = 0;
			Name_S = "";
//			Canceled_B = false;
			MaxNumberToNotify_U32=0;
			SignaledBitmask_U64 = 0;
//			NotifyAll_B = false;
			WaitKeepSignaled_B = false;
		}
};

const uint32_t BOF_SEMAPHORE_MAGIC = 0xABFF8974;

struct BOF_SEMAPHORE
{
		uint32_t Magic_U32;
		std::string Name_S;
		std::mutex Mtx;
		std::condition_variable Cv;
		std::atomic<int32_t> Cpt_S32;
		int32_t Max_S32;

		BOF_SEMAPHORE()
		{
			Reset();
		}

		void Reset()
		{
			Magic_U32 = 0;
			Name_S = "";
			Cpt_S32 = 0;
			Max_S32 = 0;
		}
};

const uint32_t BOF__CONDITIONAL_VARIABLE_MAGIC = 0xCBFDE456;

struct BOF_CONDITIONAL_VARIABLE
{
		uint32_t Magic_U32;
		std::string Name_S;
		std::mutex Mtx;
		std::condition_variable Cv;
		bool NotifyAll_B;

		BOF_CONDITIONAL_VARIABLE()
		{
			Reset();
		}

		void Reset()
		{
			Magic_U32 = 0;
			Name_S = "";
			NotifyAll_B = false;
		}
};

const uint32_t BOF_FILEMAPPING_MAGIC = 0x165464DE;

struct BOF_SHARED_MEMORY
{
		uint32_t Magic_U32;
		std::string Name_S;
#if defined(_WIN32)
		void	*pHandle;
#else
#endif
		uint32_t SizeInByte_U32;
		void *pBaseAddress;

		BOF_SHARED_MEMORY()
		{
			Reset();
		}

		void Reset()
		{
			Magic_U32 = 0;
			Name_S = "";
#if defined(_WIN32)
			pHandle=nullptr;
#else
#endif
			SizeInByte_U32 = 0;
			pBaseAddress = nullptr;
		}
};

//const uint32_t BOF_ALIGNEDMALLOC_MAGIC = 0xCEFA8951;


#if defined (_WIN32)
enum BOF_THREAD_SCHEDULER_POLICY
{
  BOF_THREAD_SCHEDULER_POLICY_OTHER = 0,	//32,  Need to be different for enum to string convert
  BOF_THREAD_SCHEDULER_POLICY_FIFO = 1,		//32,
  BOF_THREAD_SCHEDULER_POLICY_ROUND_ROBIN = 2,	//32,
  BOF_THREAD_SCHEDULER_POLICY_MAX
};
enum BOF_THREAD_PRIORITY
{
	//THREAD_PRIORITY_TIME_CRITICAL	15	Base priority of 15 for IDLE_PRIORITY_CLASS,
	//THREAD_PRIORITY_HIGHEST				2	Priority 2 points above the priority class.
	//THREAD_PRIORITY_ABOVE_NORMAL	1		Priority 1 point above the priority class.
	//THREAD_PRIORITY_NORMAL				0	Normal priority for the priority class.
	//THREAD_PRIORITY_BELOW_NORMAL	- 1	Priority 1 point below the priority class.
	//THREAD_PRIORITY_LOWEST				- 2	Priority 2 points below the priority class.
	//THREAD_PRIORITY_IDLE					- 15	Base priority of 1 for IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base priority of 16 for REALTIME_PRIORITY_CLASS processes.
	/*
	085->100	15
	068->084	2
	051->067  1
	050->050	0
	033->049  -1
	016->032  -2
	000->015  -15
	*/
  BOF_THREAD_PRIORITY_IDLE = -15,
  BOF_THREAD_PRIORITY_TIME_CRITICAL = 15,
	BOF_THREAD_DEFAULT_PRIORITY = 0x7FFFFFFF,
	BOF_THREAD_NONE = 0x7FFFFFFE,



	BOF_THREAD_PRIORITY_000 = -15, BOF_THREAD_PRIORITY_001 = -15, BOF_THREAD_PRIORITY_002 = -15, BOF_THREAD_PRIORITY_003 = -15, BOF_THREAD_PRIORITY_004 = -15, BOF_THREAD_PRIORITY_005 = -15,
	BOF_THREAD_PRIORITY_006 = -15, BOF_THREAD_PRIORITY_007 = -15, BOF_THREAD_PRIORITY_008 = -15, BOF_THREAD_PRIORITY_009 = -15, BOF_THREAD_PRIORITY_010 = -15, BOF_THREAD_PRIORITY_011 = -15,
	BOF_THREAD_PRIORITY_012 = -15, BOF_THREAD_PRIORITY_013 = -15, BOF_THREAD_PRIORITY_014 = -15, BOF_THREAD_PRIORITY_015 = -15, BOF_THREAD_PRIORITY_016 = -2,  BOF_THREAD_PRIORITY_017 = -2,
	BOF_THREAD_PRIORITY_018 = -2,	 BOF_THREAD_PRIORITY_019 = -2,  BOF_THREAD_PRIORITY_020 = -2,  BOF_THREAD_PRIORITY_021 = -2,  BOF_THREAD_PRIORITY_022 = -2,  BOF_THREAD_PRIORITY_023 = -2,
	BOF_THREAD_PRIORITY_024 = -2,	 BOF_THREAD_PRIORITY_025 = -2,  BOF_THREAD_PRIORITY_026 = -2,  BOF_THREAD_PRIORITY_027 = -2,  BOF_THREAD_PRIORITY_028 = -2,  BOF_THREAD_PRIORITY_029 = -2,
	BOF_THREAD_PRIORITY_030 = -2,	 BOF_THREAD_PRIORITY_031 = -2,  BOF_THREAD_PRIORITY_032 = -2,  BOF_THREAD_PRIORITY_033 = -1,  BOF_THREAD_PRIORITY_034 = -1,  BOF_THREAD_PRIORITY_035 = -1,
	BOF_THREAD_PRIORITY_036 = -1,	 BOF_THREAD_PRIORITY_037 = -1,  BOF_THREAD_PRIORITY_038 = -1,  BOF_THREAD_PRIORITY_039 = -1,  BOF_THREAD_PRIORITY_040 = -1,  BOF_THREAD_PRIORITY_041 = -1,
	BOF_THREAD_PRIORITY_042 = -1,	 BOF_THREAD_PRIORITY_043 = -1,  BOF_THREAD_PRIORITY_044 = -1,  
	BOF_THREAD_PRIORITY_045 = -1,  BOF_THREAD_PRIORITY_046 = -1,  BOF_THREAD_PRIORITY_047 = -1,	 BOF_THREAD_PRIORITY_048 = -1, BOF_THREAD_PRIORITY_049 = -1,
	BOF_THREAD_PRIORITY_050 = 0,	 BOF_THREAD_PRIORITY_051 = 1,	  BOF_THREAD_PRIORITY_052 = 1,   BOF_THREAD_PRIORITY_053 = 1,  BOF_THREAD_PRIORITY_054 = 1, 
	BOF_THREAD_PRIORITY_055 = 1,	 BOF_THREAD_PRIORITY_056 = 1,   BOF_THREAD_PRIORITY_057 = 1,   BOF_THREAD_PRIORITY_058 = 1,  BOF_THREAD_PRIORITY_059 = 1,    BOF_THREAD_PRIORITY_060 = 1, 
	BOF_THREAD_PRIORITY_061 = 1,	 BOF_THREAD_PRIORITY_062 = 1,   BOF_THREAD_PRIORITY_063 = 1,   BOF_THREAD_PRIORITY_064 = 1,  BOF_THREAD_PRIORITY_065 = 1,    BOF_THREAD_PRIORITY_066 = 1, 
	BOF_THREAD_PRIORITY_067 = 1,	 BOF_THREAD_PRIORITY_068 = 2,   BOF_THREAD_PRIORITY_069 = 2,   BOF_THREAD_PRIORITY_070 = 2,  BOF_THREAD_PRIORITY_071 = 2,    BOF_THREAD_PRIORITY_072 = 2, 
	BOF_THREAD_PRIORITY_073 = 2,	 BOF_THREAD_PRIORITY_074 = 2,   BOF_THREAD_PRIORITY_075 = 2,   BOF_THREAD_PRIORITY_076 = 2,  BOF_THREAD_PRIORITY_077 = 2,    BOF_THREAD_PRIORITY_078 = 2, 
	BOF_THREAD_PRIORITY_079 = 2,	 BOF_THREAD_PRIORITY_080 = 2,   BOF_THREAD_PRIORITY_081 = 2,   BOF_THREAD_PRIORITY_082 = 2,  BOF_THREAD_PRIORITY_083 = 2,    BOF_THREAD_PRIORITY_084 = 2, 
	BOF_THREAD_PRIORITY_085 = 15,	 BOF_THREAD_PRIORITY_086 = 15,  BOF_THREAD_PRIORITY_087 = 15,  BOF_THREAD_PRIORITY_088 = 15, BOF_THREAD_PRIORITY_089 = 15,   BOF_THREAD_PRIORITY_090 = 15, 
	BOF_THREAD_PRIORITY_091 = 15,	 BOF_THREAD_PRIORITY_092 = 15,  BOF_THREAD_PRIORITY_093 = 15,	 BOF_THREAD_PRIORITY_094 = 15, BOF_THREAD_PRIORITY_095 = 15,   BOF_THREAD_PRIORITY_096 = 15, 
	BOF_THREAD_PRIORITY_097 = 15,	 BOF_THREAD_PRIORITY_098 = 15,  BOF_THREAD_PRIORITY_099 = 15,
};
#else
enum BOF_THREAD_SCHEDULER_POLICY
{
  BOF_THREAD_SCHEDULER_POLICY_OTHER = 0, // cf linux sched.h SCHED_OTHER/SCHED_FIFO/SCHED_RR
  BOF_THREAD_SCHEDULER_POLICY_FIFO = 1,
  BOF_THREAD_SCHEDULER_POLICY_ROUND_ROBIN = 2,
  BOF_THREAD_SCHEDULER_POLICY_MAX
};
enum BOF_THREAD_PRIORITY
{
  BOF_THREAD_PRIORITY_IDLE = 1,
  BOF_THREAD_PRIORITY_TIME_CRITICAL = 99,
  BOF_THREAD_DEFAULT_PRIORITY = 0x7FFFFFFF,
	BOF_THREAD_NONE = 0x7FFFFFFE,

	BOF_THREAD_PRIORITY_000 = 0, BOF_THREAD_PRIORITY_001, BOF_THREAD_PRIORITY_002, BOF_THREAD_PRIORITY_003, BOF_THREAD_PRIORITY_004, BOF_THREAD_PRIORITY_005,
	BOF_THREAD_PRIORITY_006,     BOF_THREAD_PRIORITY_007, BOF_THREAD_PRIORITY_008, BOF_THREAD_PRIORITY_009, BOF_THREAD_PRIORITY_010, BOF_THREAD_PRIORITY_011,
	BOF_THREAD_PRIORITY_012,     BOF_THREAD_PRIORITY_013, BOF_THREAD_PRIORITY_014, BOF_THREAD_PRIORITY_015, BOF_THREAD_PRIORITY_016, BOF_THREAD_PRIORITY_017,
	BOF_THREAD_PRIORITY_018,     BOF_THREAD_PRIORITY_019, BOF_THREAD_PRIORITY_020, BOF_THREAD_PRIORITY_021, BOF_THREAD_PRIORITY_022, BOF_THREAD_PRIORITY_023,
	BOF_THREAD_PRIORITY_024,     BOF_THREAD_PRIORITY_025, BOF_THREAD_PRIORITY_026, BOF_THREAD_PRIORITY_027, BOF_THREAD_PRIORITY_028, BOF_THREAD_PRIORITY_029,
	BOF_THREAD_PRIORITY_030,     BOF_THREAD_PRIORITY_031, BOF_THREAD_PRIORITY_032, BOF_THREAD_PRIORITY_033, BOF_THREAD_PRIORITY_034, BOF_THREAD_PRIORITY_035,
	BOF_THREAD_PRIORITY_036,     BOF_THREAD_PRIORITY_037, BOF_THREAD_PRIORITY_038, BOF_THREAD_PRIORITY_039, BOF_THREAD_PRIORITY_040, BOF_THREAD_PRIORITY_041,
	BOF_THREAD_PRIORITY_042,     BOF_THREAD_PRIORITY_043, BOF_THREAD_PRIORITY_044,												 
	BOF_THREAD_PRIORITY_045,     BOF_THREAD_PRIORITY_046, BOF_THREAD_PRIORITY_047, BOF_THREAD_PRIORITY_048, BOF_THREAD_PRIORITY_049,
	BOF_THREAD_PRIORITY_050,     BOF_THREAD_PRIORITY_051, BOF_THREAD_PRIORITY_052, BOF_THREAD_PRIORITY_053, BOF_THREAD_PRIORITY_054,
	BOF_THREAD_PRIORITY_055,     BOF_THREAD_PRIORITY_056, BOF_THREAD_PRIORITY_057, BOF_THREAD_PRIORITY_058, BOF_THREAD_PRIORITY_059, BOF_THREAD_PRIORITY_060,
	BOF_THREAD_PRIORITY_061,     BOF_THREAD_PRIORITY_062, BOF_THREAD_PRIORITY_063, BOF_THREAD_PRIORITY_064, BOF_THREAD_PRIORITY_065, BOF_THREAD_PRIORITY_066,
	BOF_THREAD_PRIORITY_067,     BOF_THREAD_PRIORITY_068, BOF_THREAD_PRIORITY_069, BOF_THREAD_PRIORITY_070, BOF_THREAD_PRIORITY_071, BOF_THREAD_PRIORITY_072,
	BOF_THREAD_PRIORITY_073,     BOF_THREAD_PRIORITY_074, BOF_THREAD_PRIORITY_075, BOF_THREAD_PRIORITY_076, BOF_THREAD_PRIORITY_077, BOF_THREAD_PRIORITY_078,
	BOF_THREAD_PRIORITY_079,     BOF_THREAD_PRIORITY_080, BOF_THREAD_PRIORITY_081, BOF_THREAD_PRIORITY_082, BOF_THREAD_PRIORITY_083, BOF_THREAD_PRIORITY_084,
	BOF_THREAD_PRIORITY_085,     BOF_THREAD_PRIORITY_086, BOF_THREAD_PRIORITY_087, BOF_THREAD_PRIORITY_088, BOF_THREAD_PRIORITY_089, BOF_THREAD_PRIORITY_090,
	BOF_THREAD_PRIORITY_091,     BOF_THREAD_PRIORITY_092, BOF_THREAD_PRIORITY_093, BOF_THREAD_PRIORITY_094, BOF_THREAD_PRIORITY_095, BOF_THREAD_PRIORITY_096,
	BOF_THREAD_PRIORITY_097,     BOF_THREAD_PRIORITY_098, BOF_THREAD_PRIORITY_099,
};
#endif


using BofThreadFunction = std::function<void *(const std::atomic<bool> &_rIsThreadLoopMustExit_B, void *_pContext)>;
const uint32_t BOF_THREAD_MAGIC = 0xCBE89448;

struct BOF_THREAD
{
		uint32_t Magic_U32;
		std::string Name_S;
		uint32_t StartStopTimeoutInMs_U32;
		uint32_t StackSize_U32;
		uint32_t ThreadCpuCoreAffinity_U32;
    BOF_THREAD_SCHEDULER_POLICY ThreadSchedulerPolicy_E;
    BOF_THREAD_PRIORITY ThreadPriority_E;
		BofThreadFunction ThreadFunction;
		void *pUserContext;

#if defined (_WIN32)
		void                   *pThread;          /*! Thread windows handle*/
		uint32_t               ThreadId_U32;      /*! Thread windows Id*/
#else
		pthread_t ThreadId;
#endif
		std::atomic<bool> ThreadLoopMustExit_B;
		std::atomic<bool> ThreadRunning_B;

		BOF_THREAD()
		{
			Reset();
		}

		void Reset()
		{
			Magic_U32 = 0;
			Name_S = "";
			StartStopTimeoutInMs_U32 = 1000;
			StackSize_U32 = 0;
			ThreadCpuCoreAffinity_U32 = 0;
			ThreadSchedulerPolicy_E = BOF_THREAD_SCHEDULER_POLICY_OTHER;
			ThreadPriority_E = BOF_THREAD_DEFAULT_PRIORITY;
			ThreadFunction = nullptr;
			pUserContext = nullptr;

			ThreadLoopMustExit_B = false;
			ThreadRunning_B = false;
#if defined (_WIN32)
			pThread=nullptr;
			ThreadId_U32=0;
#else
			ThreadId = 0;
#endif
		}
};

struct BOF_DATE_TIME;

BOFERR Bof_DiffDateTime(const BOF_DATE_TIME &_rFirstDateTime_X, const BOF_DATE_TIME &_rSecondDateTime_X, BOF_DATE_TIME &_rDiffTime_X, uint32_t &_rDiffDay_U32);

BOFERR Bof_ComputeDayOfWeek(const BOF_DATE_TIME &_rDateTime_X, uint8_t &_DayOfWeek_U8);  //0 is sunday

#define BOF_SET_DATE_TIME(datetime, day, month, year, hour, minute, second, ms) {datetime.Day_U8 = day;datetime.Month_U8 = month;datetime.Year_U16 = year;datetime.DayOfWeek_U8 = 0;datetime.Hour_U8 = hour;datetime.Minute_U8 = minute;datetime.Second_U8 = second;datetime.Millisecond_U16=ms;}

// Get month name by strftime.
struct BOF_DATE_TIME
{
		uint16_t Year_U16;                     // 2014
		uint8_t Month_U8;                     // 1-12
		uint8_t DayOfWeek_U8;                 // 0-6 0:sunday
		uint8_t Day_U8;                       // 1-31
		uint8_t Hour_U8;                      // 0-23
		uint8_t Minute_U8;                    // 0-59
		uint8_t Second_U8;                    // 0-59
		uint16_t Millisecond_U16;              // 0-999


		BOF_DATE_TIME()
		{
			Reset();
		}

		void Reset()
		{
			Year_U16 = 1970;
			Month_U8 = 1;
			DayOfWeek_U8 = 4;  //Thursday
			Day_U8 = 1;
			Hour_U8 = 0;
			Minute_U8 = 0;
			Second_U8 = 0;
			Millisecond_U16 = 0;
		}

		BOF_DATE_TIME(uint8_t _Day_U8, uint8_t _Month_U8, uint16_t _Year_U16, uint8_t _Hour_U8, uint8_t _Minute_U8, uint8_t _Second_U8, uint16_t _Millisecond_U16)
		{
			Year_U16 = _Year_U16;
			Month_U8 = _Month_U8;
			Day_U8 = _Day_U8;
			Hour_U8 = _Hour_U8;
			Minute_U8 = _Minute_U8;
			Second_U8 = _Second_U8;
			Millisecond_U16 = _Millisecond_U16;
			Bof_ComputeDayOfWeek(*this, DayOfWeek_U8);
		}

		BOF_DATE_TIME(const std::tm &_rRm_X)
		{
			Year_U16 = static_cast<uint16_t>(_rRm_X.tm_year + 1900);
			Month_U8 = static_cast<uint8_t>(_rRm_X.tm_mon + 1);
			Day_U8 = static_cast<uint8_t>(_rRm_X.tm_mday);
			Hour_U8 = static_cast<uint8_t>(_rRm_X.tm_hour);
			Minute_U8 = static_cast<uint8_t>(_rRm_X.tm_min);
			Second_U8 = static_cast<uint8_t>(_rRm_X.tm_sec);
			Millisecond_U16 = 0;
			Bof_ComputeDayOfWeek(*this, DayOfWeek_U8);
		}

		std::tm ToStdTm()
		{
			std::tm Rts_X;
			//uint8_t DayOfWeek_U8;
			uint32_t DiffDay_U32;
			BOF_DATE_TIME FirstDayOfYear(1, 1, Year_U16, 0, 0, 0, 0);
			BOF_DATE_TIME DiffTime_X;
			//BOFERR  Sts_E=
			Bof_DiffDateTime(*this, FirstDayOfYear, DiffTime_X, DiffDay_U32);

			Rts_X.tm_sec = Second_U8;   // seconds after the minute - [0, 60] including leap second
			Rts_X.tm_min = Minute_U8;   // minutes after the hour - [0, 59]
			Rts_X.tm_hour = Hour_U8;  // hours since midnight - [0, 23]
			Rts_X.tm_mday = Day_U8;  // day of the month - [1, 31]
			Rts_X.tm_mon = Month_U8 - 1;   // months since January - [0, 11]
			Rts_X.tm_year = Year_U16 - 1900;  // years since 1900
			//Sts_E=Bof_ComputeDayOfWeek(*this,DayOfWeek_U8);
			Rts_X.tm_wday = DayOfWeek_U8;  // days since Sunday - [0, 6]
			Rts_X.tm_yday = DiffDay_U32 + 1;  // days since January 1 - [0, 365]
			Rts_X.tm_isdst = 0; // daylight savings time flag
			return Rts_X;
		}

		bool operator==(const BOF_DATE_TIME &_Other) const
		{
			return ((Year_U16 == _Other.Year_U16) && (Month_U8 == _Other.Month_U8) && (DayOfWeek_U8 == _Other.DayOfWeek_U8) && (Day_U8 == _Other.Day_U8) && (Hour_U8 == _Other.Hour_U8) &&
			        (Minute_U8 == _Other.Minute_U8) && (Second_U8 == _Other.Second_U8) && (Millisecond_U16 == _Other.Millisecond_U16));
		}

		bool operator!=(const BOF_DATE_TIME &_Other) const
		{
			return !(*this == _Other);
		}
};


typedef struct
{
  struct
  {
    float    UserCpuUsedInSec_f; /* user CPU time used */
    float    SystemCpuUsedInSec_f; /* system CPU time used */

    uint64_t UpTimeInSec_U64;             /* Seconds since boot */
  } TIME;

  struct
  {
    uint64_t NbSoftPageFault_U64;        /* page reclaims (soft page faults) */
    uint64_t NbHardPageFault_U64;        /* page faults (hard page faults) */
    uint64_t NbBlkInputOp_U64;       /* block input operations */
    uint64_t NbBlkOutputOp_U64;       /* block output operations */
    uint64_t NbVoluntaryContextSwitch_U64;         /* voluntary context switches */
    uint64_t NbInvoluntaryContextSwitch_U64;        /* involuntary context switches */

    float pLoad_f[3];  /* 1, 5, and 15 minute load averages */
    uint64_t NbProcess_U64;    /* Number of current processes */
  } OS;

  struct
  {
    uint64_t MaxRssInKB_U64;        /* maximum resident set size */

    uint64_t TotalRamInKB_U64;  /* Total usable main memory size */
    uint64_t FreeRamInKB_U64;   /* Available memory size */
    uint64_t SharedRamInKB_U64; /* Amount of shared memory */
    uint64_t BufferRamInKB_U64; /* Memory used by buffers */
    uint64_t TotalSwapInKB_U64; /* Total swap space size */
    uint64_t FreeSwapInKB_U64;  /* Swap space still available */
    uint64_t TotaHighInKB_U64; /* Total high memory size */
    uint64_t FreeHighInKB_U64;  /* Available high memory size */
  } MEMORY;
}  BOF_SYSTEM_USAGE_INFO;
// !!! Millisecond_U16;  !! http://h30499.www3.hp.com/t5/Languages-and-Scripting/migration-to-64-bit-mode-semctl/td-p/3204127#.VUCj4XhV2zl !!!!
typedef union semsetgetval
{
		int val;                               /* Value for SETVAL */
}
	BOF_SEM_SETGETVAL;

BOFERR Bof_OpenSharedMemory(const std::string &_rName_S, uint32_t _SizeInByte_U32, BOF_SHARED_MEMORY &_rSharedMemory_X);

bool Bof_IsSharedMemoryValid(BOF_SHARED_MEMORY &_rSharedMemory_X);

BOFERR Bof_CloseSharedMemory(BOF_SHARED_MEMORY &_rSharedMemory_X);

BOFERR Bof_DestroySharedMemory(const std::string &_rName_S);

BOFERR Bof_CreateSemaphore(const std::string &_rName_S, int32_t _InitialCount_S32, BOF_SEMAPHORE &_rSem_X);

bool Bof_IsSemaphoreValid(BOF_SEMAPHORE &_rSem_X);

BOFERR Bof_SignalSemaphore(BOF_SEMAPHORE &_rSem_X);

BOFERR Bof_WaitForSemaphore(BOF_SEMAPHORE &_rSem_X, uint32_t _TimeoutInMs_U32);

BOFERR Bof_DestroySemaphore(BOF_SEMAPHORE &_rSem_X);

BOFERR Bof_CreateMutex(const std::string &_rName_S, bool _Recursive_B, bool _PriorityInversionAware_B, BOF_MUTEX &_rMtx_X);

bool Bof_IsMutexValid(BOF_MUTEX &_rMtx_X);

BOFERR Bof_LockMutex(BOF_MUTEX &_rMtx_X);

BOFERR Bof_UnlockMutex(BOF_MUTEX &_rMtx_X);

BOFERR Bof_DestroyMutex(BOF_MUTEX &_rMtx_X);

BOFERR Bof_CreateEvent(const std::string &_rName_S, bool _InitialState_B, /*bool _NotifyAll_B*/ uint32_t _MaxNumberToNotify_U32, bool _WaitKeepSignaled_B, BOF_EVENT &_rEvent_X);

bool Bof_IsEventValid(BOF_EVENT &_rEvent_X);

BOFERR Bof_SignalEvent(BOF_EVENT &_rEvent_X, /*bool _CancelIt_B*/ uint32_t _Instance_U32);

BOFERR Bof_ResetEvent(BOF_EVENT &_rEvent_X, uint32_t _Instance_U32);

bool Bof_IsEventSignaled(BOF_EVENT &_rEvent_X, uint32_t _Instance_U32);

//bool Bof_IsEventCanceled(BOF_EVENT &_rEvent_X);

BOFERR Bof_WaitForEvent(BOF_EVENT &_rEvent_X, uint32_t _TimeoutInMs_U32, uint32_t _Instance_U32);

BOFERR Bof_DestroyEvent(BOF_EVENT &_rEvent_X);

BOF_THREAD_PRIORITY Bof_ThreadPriorityFromValue(int32_t _Priority_S32);
int32_t Bof_ValueFromThreadPriority(BOF_THREAD_PRIORITY _Priority_E);

BOFERR Bof_GetThreadPriorityRange(BOF_THREAD_SCHEDULER_POLICY _ThreadSchedulerPolicy_E, BOF_THREAD_PRIORITY &_rMin_E, BOF_THREAD_PRIORITY &_rMax_E);

BOFERR Bof_GetThreadPriorityLevel(BOF_THREAD &_rThread_X, BOF_THREAD_SCHEDULER_POLICY &_rPolicy_E, BOF_THREAD_PRIORITY &_rPriority_E);

BOFERR Bof_SetThreadPriorityLevel(BOF_THREAD &_rThread_X, BOF_THREAD_SCHEDULER_POLICY _ThreadSchedulerPolicy_E, BOF_THREAD_PRIORITY _ThreadPriority_E);

BOFERR Bof_CreateThread(const std::string &_rName_S, BofThreadFunction _ThreadFunction, void *_pUserContext, BOF_THREAD &_rThread_X);

bool Bof_IsThreadValid(BOF_THREAD &_rThread_X);

BOFERR Bof_LaunchThread(BOF_THREAD &_rThread_X, uint32_t _StackSize_U32, uint32_t _ThreadCpuCoreAffinity_U32, BOF_THREAD_SCHEDULER_POLICY _ThreadSchedulerPolicy_E, BOF_THREAD_PRIORITY _ThreadPriority_E, uint32_t _StartStopTimeoutInMs_U32);

BOFERR Bof_DestroyThread(BOF_THREAD &_rThread_X);

uint32_t Bof_CurrentThreadId();

BOFERR Bof_GetMemoryState(uint64_t &_rAvailableFreeMemory_U64, uint64_t &_rTotalMemorySize_U64);

uint32_t Bof_InterlockedCompareExchange(uint32_t volatile *_pDestination_U32, uint32_t _ValueToSetIfEqual_U32, uint32_t _CheckIfEqualToThis_U32);

bool Bof_IsPidRunning(uint32_t _Pid_U32);

uint32_t Bof_GetCurrentPid();

BOFERR Bof_GetLastError(bool _NetError_B, int32_t *_pNativeErrorCode_S32=nullptr);

bool Bof_PatternCompare(const char *_pString_c, const char *_pPattern_c);

BOFERR Bof_FileTimeToSystemTime(uint64_t _FileTime_U64, BOF_DATE_TIME &_rDateTime_X);

BOFERR Bof_TimeInSecSince1970_To_BofDateTime(time_t _TimeInSecSice1970, BOF_DATE_TIME &_rDateTime_X);

BOFERR Bof_DateInDaySince1970_To_BofDateTime(time_t _DateInDaySince1970, BOF_DATE_TIME &_rDateTime_X);

BOFERR Bof_BofDateTime_To_DateInDaySince1970(const BOF_DATE_TIME &_rDateTime_X, time_t &_rDateInDaySince1970);

BOFERR Bof_Now(BOF_DATE_TIME &_rDateTime_X);

BOFERR Bof_SetDateTime(const BOF_DATE_TIME &_rDateTime_X);

bool Bof_IsLeapYear(uint16_t _Year_U16);

BOFERR Bof_DateTimeToNumber(const BOF_DATE_TIME &_pDateTime_X, double &_rDayNumber_lf);

BOFERR Bof_ValidateDateTime(const BOF_DATE_TIME &_rDateTime_X);

BOFERR Bof_DiffDateTime(const BOF_DATE_TIME &_rFirstDateTime_X, const BOF_DATE_TIME &_rSecondDateTime_X, BOF_DATE_TIME &_rDiffTime_X, uint32_t &_rDiffDay_U32);

BOFERR Bof_ComputeDayOfWeek(const BOF_DATE_TIME &_rDateTime_X, uint8_t &_DayOfWeek_U8);  //0 is sunday
const std::string Bof_DateTimeToString(const BOF_DATE_TIME &_rDateTime_X, const std::string &_rFormat_S = "%Y-%m-%d %H:%M:%S");

BOF_DATE_TIME Bof_DateTimeFromString(const std::string &_rDateTime_S, const std::string &_rFormat_S = "%Y-%m-%d %H:%M:%S");

BOFERR Bof_DeltaMsToHms(uint32_t _DeltaInMs_U32, uint32_t &_rDay_U32, uint32_t &_rHour_U32, uint32_t &_rMinute_U32, uint32_t &_rSecond_U32,uint32_t &_rMs_U32);

BOFERR Bof_Exec(const std::string &_rCommand_S, std::string *_pCapturedOutput_S, int32_t &_rExitCode_S32);

const char *Bof_GetEnvVar(const char *_pName_c);

int Bof_SetEnvVar(const char *_pName_c, const char *_pValue_c, int _Overwrite_i);

BOFERR Bof_LockMem(uint64_t _SizeInByte_U64, void *_pData);
BOFERR Bof_UnlockMem(uint64_t _SizeInByte_U64, void *_pData);
bool Bof_AlignedMemCpy8(volatile void *_pDst, const volatile void *_pSrc, uint32_t _SizeInByte_U32);
bool Bof_AlignedMemCpy16(volatile void *_pDst, const volatile void *_pSrc, uint32_t _SizeInByte_U32);
bool Bof_AlignedMemCpy32(volatile void *_pDst, const volatile void *_pSrc, uint32_t _SizeInByte_U32);
BOFERR Bof_AlignedMemAlloc(BOF_BUFFER_ALLOCATE_ZONE _AllocateZone_E, uint32_t _AligmentInByte_U32, uint32_t _SizeInByte_U32, bool _LockIt_B, bool _ClearIt_B, BOF_BUFFER &_rAllocatedBuffer_X);
BOFERR Bof_AlignedMemFree(BOF_NAMESPACE::BOF_BUFFER &_rBuffer_X);

std::string Bof_DumpMemoryZone(const BOF_DUMP_MEMORY_ZONE_PARAM &_rDumpMemoryZoneParam_X);

//void Bof_SpinYieldOrSleep(const std::chrono::time_point<std::chrono::system_clock> &_Now, const std::chrono::time_point<std::chrono::system_clock> &_LastOpTime, uint32_t _SpinLimitInMicro_U32 = 50, uint32_t _YieldLimitInMicro_U32 = 150, uint32_t _SleepLimitInMicro_U32 = 20000);

BOFERR Bof_ReEvaluateTimeout(uint32_t _Start_U32, uint32_t &_rNewTimeOut_U32);

void Bof_MsSleep(uint32_t _Ms_U32);
void Bof_UsSleep(uint32_t _Us_U32);

uint32_t Bof_GetMsTickCount();
uint64_t Bof_GetUsTickCount();
uint64_t Bof_GetNsTickCount();

uint32_t Bof_ElapsedMsTime(uint32_t _StartInMs_U32);
uint64_t Bof_ElapsedUsTime(uint64_t _StartInUs_U64);
uint64_t Bof_ElapsedNsTime(uint64_t _StartInNs_U64);

bool Bof_IsElapsedTimeInMs(uint32_t _Start_U32, uint32_t _TimeoutInMs_U32);

//const char        *Bof_Eol();
//char              Bof_FilenameSeparator();
#if defined (_WIN32)
inline const char        *Bof_Eol() { return "\r\n";  }
inline char        Bof_FilenameSeparator() { return '\\'; }
#else

inline const char *Bof_Eol()
{ return "\n"; }

inline char Bof_FilenameSeparator()
{ return '/'; }
void Bof_LockRam_ShowNewPagefaultCount(const char *logtext,  const char *allowed_maj, const char *allowed_min);
BOFERR Bof_LockRam(uint32_t _StackSizeInByte_U32,uint64_t _ReserveProcessMemoryInByte_U64);
#endif

int32_t Bof_Random(bool _Reset_B, int32_t _MinValue_S32, int32_t _MaxValue_S32);

std::string Bof_Random(bool _Reset_B, uint32_t _Size_U32, char _MinValue_c, char _MaxValue_c);

template<typename Container, typename SearchFunc>
auto Bof_EraseWhere(Container &_Container, SearchFunc &&_Func) -> decltype(_Container.end())
{
	return _Container.erase(std::remove_if(_Container.begin(), _Container.end(), std::forward<SearchFunc>(_Func)), _Container.end());
}

template<typename ...Args>
using BofCvPredicateAndReset = std::function<bool(Args...)>;
template<typename ...Args>
using BofCvSetter = std::function<void(Args...)>;

BOFERR Bof_CreateConditionalVariable(const std::string &_rName_S, bool _NotifyAll_B, BOF_CONDITIONAL_VARIABLE &_rCv_X);

template<typename ...Args>
BOFERR Bof_SignalConditionalVariable(BOF_CONDITIONAL_VARIABLE &_rCv_X, BofCvSetter<Args...> _CvSetter, const Args &... _Args)
{
	BOFERR Rts_E = BOF_ERR_INIT;

	if (_rCv_X.Magic_U32 == BOF__CONDITIONAL_VARIABLE_MAGIC)
	{
		std::unique_lock<std::mutex> WaitLock_O(_rCv_X.Mtx);
		_CvSetter(_Args...);
		_rCv_X.NotifyAll_B ? _rCv_X.Cv.notify_all() : _rCv_X.Cv.notify_one();
		Rts_E = BOF_ERR_NO_ERROR;
	}
	return Rts_E;
}

template<typename ...Args>
BOFERR Bof_WaitForConditionalVariable(BOF_CONDITIONAL_VARIABLE &_rCv_X, uint32_t _TimeoutInMs_U32, BofCvPredicateAndReset<Args...> _CvPredicateAndReset, const Args &... _Args)
{
	BOFERR Rts_E = BOF_ERR_INIT;

	if (_rCv_X.Magic_U32 == BOF__CONDITIONAL_VARIABLE_MAGIC)
	{
		std::unique_lock<std::mutex> WaitLock_O(_rCv_X.Mtx);
		//if (_rCv_X.Cv.wait_for(WaitLock_O, std::chrono::milliseconds(_TimeoutInMs_U32), _CvPredicate(_Args...)))
		Rts_E = BOF_ERR_NO_ERROR;
		std::chrono::system_clock::time_point End = std::chrono::system_clock::now() + std::chrono::milliseconds(_TimeoutInMs_U32);
		while (!_CvPredicateAndReset(_Args...))
		{
			if (_rCv_X.Cv.wait_until(WaitLock_O, End) == std::cv_status::timeout)
			{
				Rts_E = _CvPredicateAndReset(_Args...) ? BOF_ERR_NO_ERROR : BOF_ERR_ETIMEDOUT;
				break;
			}
		}
	}
	return Rts_E;
}

BOFERR Bof_DestroyConditionalVariable(BOF_CONDITIONAL_VARIABLE &_rCv_X);

BOFERR Bof_SystemUsageInfo(BOF_SYSTEM_USAGE_INFO &_rSystemUsageInfo_X);
std::string Bof_SystemUsageInfoToString(const BOF_SYSTEM_USAGE_INFO &_rSystemUsageInfo_X, const BOF_SYSTEM_USAGE_INFO *_pPreviousSystemUsageInfo_X);

END_BOF_NAMESPACE()