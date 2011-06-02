#pragma once
#include "../CommonInc/common.h"
#include "../CommonInc/MightyThread.h"
#include <limits>
#include "BaseMightyServerInterface.h"

class win_iocp_io_service
{
	class operation
		: public OVERLAPPED
	{
	public:
		typedef void (*invoke_func_type)(operation*, DWORD, size_t);
		typedef void (*destroy_func_type)(operation*);

		operation(win_iocp_io_service& iocp_service,invoke_func_type invoke_func, destroy_func_type destroy_func)
			: outstanding_operations_(&iocp_service.outstanding_operations_),
			invoke_func_(invoke_func),
			destroy_func_(destroy_func)
		{
			Internal = 0;
			InternalHigh = 0;
			Offset = 0;
			OffsetHigh = 0;
			hEvent = 0;

			::InterlockedIncrement(outstanding_operations_);
		}

		void do_completion(DWORD last_error, size_t bytes_transferred)
		{
			invoke_func_(this, last_error, bytes_transferred);
		}

		void destroy()
		{
			destroy_func_(this);
		}

	protected:
		// Prevent deletion through this type.
		~operation()
		{
			::InterlockedDecrement(outstanding_operations_);
		}

	private:
		long* outstanding_operations_;
		invoke_func_type invoke_func_;
		destroy_func_type destroy_func_;
	};
public:
	// Constructor.
	win_iocp_io_service()
		:iocp_(),
		outstanding_work_(0),
		outstanding_operations_(0),
		stopped_(0),
		shutdown_(0)
		//timer_thread_(0),
		//timer_interrupt_issued_(false)
	{
	}

	void init(size_t concurrency_hint)
	{
		iocp_.handle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0,
			static_cast<DWORD>((std::min<size_t>)(concurrency_hint, DWORD(~0))));
		if (!iocp_.handle)
		{
			DWORD last_error = ::GetLastError();

		}
	}

	// Destroy all user-defined handler objects owned by the service.
	void shutdown_service()
	{
		::InterlockedExchange(&shutdown_, 1);

		while (::InterlockedExchangeAdd(&outstanding_operations_, 0) > 0)
		{
			DWORD bytes_transferred = 0;
#if (WINVER < 0x0500)
			DWORD completion_key = 0;
#else
			DWORD_PTR completion_key = 0;
#endif
			LPOVERLAPPED overlapped = 0;
			::GetQueuedCompletionStatus(iocp_.handle, &bytes_transferred,
				&completion_key, &overlapped, INFINITE);
			if (overlapped)
				static_cast<operation*>(overlapped)->destroy();
		}

		
	}



	// Register a handle with the IO completion port.
	void register_handle(HANDLE handle)
	{
		if (::CreateIoCompletionPort(handle, iocp_.handle, 0, 0) == 0)
		{
			DWORD last_error = ::GetLastError();

		}
	}

	// Run the event loop until stopped or no more work.
	size_t run()
	{
		if (::InterlockedExchangeAdd(&outstanding_work_, 0) == 0)
		{
			return 0;
		}

		size_t n = 0;
		while (do_one(true))
			if (n != (std::numeric_limits<size_t>::max)())
				++n;
		return n;
	}

	// Run until stopped or one operation is performed.
	size_t run_one()
	{
		if (::InterlockedExchangeAdd(&outstanding_work_, 0) == 0)
		{
			
			return 0;
		}
		return do_one(true);
	}

	// Poll for operations without blocking.
	size_t poll()
	{
		if (::InterlockedExchangeAdd(&outstanding_work_, 0) == 0)
		{
			return 0;
		}

		size_t n = 0;
		while (do_one(false))
			if (n != (std::numeric_limits<size_t>::max)())
				++n;
		return n;
	}

	// Poll for one operation without blocking.
	size_t poll_one()
	{
		if (::InterlockedExchangeAdd(&outstanding_work_, 0) == 0)
		{
			return 0;
		}
		return do_one(false);
	}

	// Stop the event processing loop.
	void stop()
	{
		if (::InterlockedExchange(&stopped_, 1) == 0)
		{
			if (!::PostQueuedCompletionStatus(iocp_.handle, 0, 0, 0))
			{
				DWORD last_error = ::GetLastError();
				
			}
		}
	}

	// Reset in preparation for a subsequent run invocation.
	void reset()
	{
		::InterlockedExchange(&stopped_, 0);
	}

	// Notify that some work has started.
	void work_started()
	{
		::InterlockedIncrement(&outstanding_work_);
	}

	// Notify that some work has finished.
	void work_finished()
	{
		if (::InterlockedDecrement(&outstanding_work_) == 0)
			stop();
	}

	// Request invocation of the given handler.
	/*template <typename Handler>
		void dispatch(Handler handler)
	{
		if (call_stack<win_iocp_io_service>::contains(this))
			asio_handler_invoke_helpers::invoke(handler, &handler);
		else
			post(handler);
	}*/

	// Request invocation of the given handler and return immediately.
	/*template <typename Handler>
		void post(Handler handler)
	{
		// If the service has been shut down we silently discard the handler.
		if (::InterlockedExchangeAdd(&shutdown_, 0) != 0)
			return;

		// Allocate and construct an operation to wrap the handler.
		//typedef handler_operation<Handler> value_type;
		//typedef handler_alloc_traits<Handler, value_type> alloc_traits;
		//raw_handler_ptr<alloc_traits> raw_ptr(handler);
		//handler_ptr<alloc_traits> ptr(raw_ptr, *this, handler);

		// Enqueue the operation on the I/O completion port.
		if (!::PostQueuedCompletionStatus(iocp_.handle, 0, 0, ptr.get()))
		{
			DWORD last_error = ::GetLastError();
			
		}

		// Operation has been successfully posted.
		//ptr.release();
	}*/

	// Request invocation of the given OVERLAPPED-derived operation.
	void post_completion(operation* op, DWORD op_last_error,
		DWORD bytes_transferred)
	{
		// Enqueue the operation on the I/O completion port.
		if (!::PostQueuedCompletionStatus(iocp_.handle,
			bytes_transferred, op_last_error, op))
		{
			DWORD last_error = ::GetLastError();
			
		}
	}

	

	

private:
	// Dequeues at most one operation from the I/O completion port, and then
	// executes it. Returns the number of operations that were dequeued (i.e.
	// either 0 or 1).
	size_t do_one(bool block)
	{
		long this_thread_id = static_cast<long>(::GetCurrentThreadId());

		for (;;)
		{
			// Try to acquire responsibility for dispatching timers.
			/*bool dispatching_timers = (::InterlockedCompareExchange(
				&timer_thread_, this_thread_id, 0) == 0);

			// Calculate timeout for GetQueuedCompletionStatus call.
			DWORD timeout = max_timeout;
			if (dispatching_timers)
			{
				asio::detail::mutex::scoped_lock lock(timer_mutex_);
				timer_interrupt_issued_ = false;
				timeout = get_timeout();
			}*/
			DWORD timeout = max_timeout;
			// Get the next operation from the queue.
			DWORD bytes_transferred = 0;
#if (WINVER < 0x0500)
			DWORD completion_key = 0;
#else
			DWORD_PTR completion_key = 0;
#endif
			LPOVERLAPPED overlapped = 0;
			::SetLastError(0);
			BOOL ok = ::GetQueuedCompletionStatus(iocp_.handle, &bytes_transferred,
				&completion_key, &overlapped, block ? timeout : 0);
			DWORD last_error = ::GetLastError();

			// Dispatch any pending timers.
			/*if (dispatching_timers)
			{
				try
				{
					asio::detail::mutex::scoped_lock lock(timer_mutex_);
					timer_queues_copy_ = timer_queues_;
					for (std::size_t i = 0; i < timer_queues_copy_.size(); ++i)
					{
						timer_queues_copy_[i]->dispatch_timers();
						timer_queues_copy_[i]->dispatch_cancellations();
						timer_queues_copy_[i]->complete_timers();
					}
				}
				catch (...)
				{
					// Transfer responsibility for dispatching timers to another thread.
					if (::InterlockedCompareExchange(&timer_thread_,
						0, this_thread_id) == this_thread_id)
					{
						::PostQueuedCompletionStatus(iocp_.handle,
							0, transfer_timer_dispatching, 0);
					}

					throw;
				}
			}*/

			if (!ok && overlapped == 0)
			{
				if (block && last_error == WAIT_TIMEOUT)
				{
					// Relinquish responsibility for dispatching timers.
					//if (dispatching_timers)
					//{
						//::InterlockedCompareExchange(&timer_thread_, 0, this_thread_id);
					//}

					continue;
				}

				// Transfer responsibility for dispatching timers to another thread.
				//if (dispatching_timers && ::InterlockedCompareExchange(
					//&timer_thread_, 0, this_thread_id) == this_thread_id)
				//{
					//::PostQueuedCompletionStatus(iocp_.handle,
						//0, transfer_timer_dispatching, 0);
				//}

				//ec = asio::error_code();
				//return 0;
			}
			else if (overlapped)
			{
				// We may have been passed a last_error value in the completion_key.
				if (last_error == 0)
				{
					last_error = completion_key;
				}

				// Transfer responsibility for dispatching timers to another thread.
				//if (dispatching_timers && ::InterlockedCompareExchange(
					//&timer_thread_, 0, this_thread_id) == this_thread_id)
				//{
					//::PostQueuedCompletionStatus(iocp_.handle,
						//0, transfer_timer_dispatching, 0);
				//}

				// Ensure that the io_service does not exit due to running out of work
				// while we make the upcall.
				//auto_work work(*this);

				// Dispatch the operation.
				operation* op = static_cast<operation*>(overlapped);
				op->do_completion(last_error, bytes_transferred);

				
				return 1;
			}
			//else if (completion_key == transfer_timer_dispatching)
			//{
				// Woken up to try to acquire responsibility for dispatching timers.
			//	::InterlockedCompareExchange(&timer_thread_, 0, this_thread_id);
			//}
			//else if (completion_key == steal_timer_dispatching)
			//{
				// Woken up to steal responsibility for dispatching timers.
				//::InterlockedExchange(&timer_thread_, 0);
			//}
			else
			{
				// Relinquish responsibility for dispatching timers. If the io_service
				// is not being stopped then the thread will get an opportunity to
				// reacquire timer responsibility on the next loop iteration.
				//if (dispatching_timers)
				//{
					//::InterlockedCompareExchange(&timer_thread_, 0, this_thread_id);
				//}

				// The stopped_ flag is always checked to ensure that any leftover
				// interrupts from a previous run invocation are ignored.
				if (::InterlockedExchangeAdd(&stopped_, 0) != 0)
				{
					// Wake up next thread that is blocked on GetQueuedCompletionStatus.
					if (!::PostQueuedCompletionStatus(iocp_.handle, 0, 0, 0))
					{
						last_error = ::GetLastError();
						
						return 0;
					}

					
					return 0;
				}
			}
		}
	}



	// The IO completion port used for queueing operations.
	struct iocp_holder
	{
		HANDLE handle;
		iocp_holder() : handle(0) {}
		~iocp_holder() { if (handle) ::CloseHandle(handle); }
	} iocp_;

	// The count of unfinished work.
	long outstanding_work_;

	// The count of unfinished operations.
	long outstanding_operations_;
	friend class operation;

	// Flag to indicate whether the event loop has been stopped.
	long stopped_;

	// Flag to indicate whether the service has been shut down.
	long shutdown_;

	enum
	{
		// Maximum GetQueuedCompletionStatus timeout, in milliseconds.
		max_timeout = 500,

		// Completion key value to indicate that responsibility for dispatching
		// timers is being cooperatively transferred from one thread to another.
		transfer_timer_dispatching = 1,

		// Completion key value to indicate that responsibility for dispatching
		// timers should be stolen from another thread.
		steal_timer_dispatching = 2
	};

	// The thread that's currently in charge of dispatching timers.
	long timer_thread_;

	// Mutex for protecting access to the timer queues.
	//mutex timer_mutex_;

	// Whether a thread has been interrupted to process a new timeout.
	bool timer_interrupt_issued_;

	// The timer queues.
	//std::vector<timer_queue_base*> timer_queues_;

	// A copy of the timer queues, used when dispatching, cancelling and cleaning
	// up timers. The copy is stored as a class data member to avoid unnecessary
	// memory allocation.
	//std::vector<timer_queue_base*> timer_queues_copy_;
};
