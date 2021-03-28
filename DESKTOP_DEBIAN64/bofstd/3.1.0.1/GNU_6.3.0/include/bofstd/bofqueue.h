/*
 * Copyright (c) 2015-2020, Onbings. All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 * This module provide an implementation of the classic multiple producer, multiple consumer thread-safe queue concept
 *
 * Name:        bofqueue.h
 * Author:      Bernard HARMEL: onbings@dscloud.me
 * Web:			    onbings.dscloud.me
 * Revision:    1.0
 *
 * Rem:          based on https://juanchopanzacpp.wordpress.com/2013/02/26/concurrent-queue-c11/
 * and https://github.com/rigtorp/MPMCQueue
 * 
 *
 * History:
 *
 * V 1.00  Dec 26 2013  BHA : Initial release
 */

#pragma once

/*** Include files ***********************************************************/
#include <bofstd/bofstd.h>
#include <bofstd/bofsystem.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <atomic>

BEGIN_BOF_NAMESPACE()

/*** Global variables ********************************************************/

/*** Definitions *************************************************************/

/*** Class *******************************************************************/


template<typename T>
class BofQueue
{
private:
		//	std::queue<T> mQueue;
		std::queue<T> mQueue;
		std::mutex mMtx;
		std::condition_variable mCvNotEmpty;
		std::condition_variable mCvNotFull;
		uint32_t mMaxSize_U32;
public:
/*
 * Max size 0 for ever growing queue
 */
		BofQueue(uint32_t _MaxSize_U32) : mMaxSize_U32(_MaxSize_U32)
		{
			//Pre allocate memory
			if (_MaxSize_U32)
			{
				//		mQueue.resize(_MaxSize_U32);
			}
		}

		virtual ~BofQueue()
		{}

		uint32_t Capacity()
		{ return mMaxSize_U32; }

		uint32_t Size()
		{ return static_cast<uint32_t>(mQueue.size()); }

		bool IsEmpty()
		{ return mQueue.empty(); }

		bool IsFull()
		{ return mMaxSize_U32 ? (mQueue.size() == mMaxSize_U32) : false; }

		/*
		* timeout 0 for ever waiting queue
		*/
		BOFERR Pop(uint32_t _TimeoutInMs_U32, T &_rItem)
		{
			BOFERR Rts_E = BOF_ERR_NO_ERROR;

			std::unique_lock<std::mutex> WaitingLock(mMtx);
			if (_TimeoutInMs_U32)
			{
				Rts_E = mCvNotEmpty.wait_for(WaitingLock, std::chrono::milliseconds(_TimeoutInMs_U32), [&]() { return !mQueue.empty(); }) ? BOF_ERR_NO_ERROR : BOF_ERR_ETIMEDOUT;
			}
			else
			{
				while (mQueue.empty())
				{
					mCvNotEmpty.wait(WaitingLock);
				}
			}
			if (Rts_E == BOF_ERR_NO_ERROR)
			{
				_rItem = mQueue.front();
				mQueue.pop();
				mCvNotFull.notify_one();
			}
			return Rts_E;
		}

		BOFERR Push(uint32_t _TimeoutInMs_U32, const T &_rItem)
		{
			BOFERR Rts_E = BOF_ERR_NO_ERROR;

			std::unique_lock<std::mutex> WaitingLock(mMtx);
			if (mMaxSize_U32)
			{
				if (_TimeoutInMs_U32)
				{
					Rts_E = mCvNotFull.wait_for(WaitingLock, std::chrono::milliseconds(_TimeoutInMs_U32), [&]() { return (mQueue.size() < mMaxSize_U32); }) ? BOF_ERR_NO_ERROR : BOF_ERR_ETIMEDOUT;
				}
				else
				{
					while (mQueue.size() == mMaxSize_U32)
					{
						mCvNotFull.wait(WaitingLock);
					}
				}
			}
			if (Rts_E == BOF_ERR_NO_ERROR)
			{
				mQueue.push(_rItem);
				WaitingLock.unlock();    //Avoid mutex contention
				mCvNotEmpty.notify_one();
			}
			return Rts_E;
		}

		BOFERR Push(uint32_t _TimeoutInMs_U32, T &&_rrItem)
		{
			BOFERR Rts_E = BOF_ERR_NO_ERROR;

			std::unique_lock<std::mutex> WaitingLock(mMtx);
			if (mMaxSize_U32)
			{
				if (_TimeoutInMs_U32)
				{
					Rts_E = mCvNotFull.wait_for(WaitingLock, std::chrono::milliseconds(_TimeoutInMs_U32), [&]() { return (mQueue.size() < mMaxSize_U32); }) ? BOF_ERR_NO_ERROR : BOF_ERR_ETIMEDOUT;
				}
				else
				{
					while (mQueue.size() == mMaxSize_U32)
					{
						mCvNotFull.wait(WaitingLock);
					}
				}
			}
			if (Rts_E == BOF_ERR_NO_ERROR)
			{
				mQueue.Push(std::move(_rrItem));
				WaitingLock.unlock();
				mCvNotEmpty.notify_one();
			}
			return Rts_E;
		}
};

END_BOF_NAMESPACE()