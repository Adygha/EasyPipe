#pragma once

#ifdef EASYPIPE
#undef EASYPIPE
#endif // EASYPIPE

#ifdef EASYPIPE_EXPORTS
#define EASYPIPE __declspec(dllexport)
#else
#define EASYPIPE __declspec(dllimport)
#endif

#include <Windows.h>
#include <thread>
#include <atomic>
using std::thread;
using std::atomic;

namespace ez {
	enum class ConnectResult : char { UNAVAIL_PIPE, CONN_ERROR, CONN_TIMEOUT, CONN_NONBLOCK_SUCC, CONN_SUCC };

	enum class PipeStatus : char {
		CLIENT,
		SERVER,
		WRITE,
		OPEN = 4,
		CONNECT_STATE = 8,
		CONNECTED = OPEN | CONNECT_STATE,
		BUSY_STATE = 16,
		BUSY = CONNECTED | BUSY_STATE
	};

	inline PipeStatus operator~ (PipeStatus a) { return (PipeStatus)~(char)a; }
	inline PipeStatus operator| (PipeStatus a, PipeStatus b) { return (PipeStatus)((char)a | (char)b); }
	inline PipeStatus operator& (PipeStatus a, PipeStatus b) { return (PipeStatus)((char)a & (char)b); }
	//inline PipeStatus operator^ (PipeStatus a, PipeStatus b) { return (PipeStatus)((char)a ^ (char)b); }
	//inline PipeStatus& operator|= (PipeStatus& a, PipeStatus b) { return (PipeStatus&)((char&)a |= (char)b); }
	//inline PipeStatus& operator&= (PipeStatus& a, PipeStatus b) { return (PipeStatus&)((char&)a &= (char)b); }
	//inline PipeStatus& operator^= (PipeStatus& a, PipeStatus b) { return (PipeStatus&)((char&)a ^= (char)b); }

	class EASYPIPE AbsEasyPipe {
	public:
		AbsEasyPipe(const char* pipName, PipeStatus pipeStatus);
		virtual ~AbsEasyPipe();

		virtual ConnectResult connect() final;
		virtual void disconnect() final;
		virtual void close() final;

	protected:
		HANDLE _mePipeHndl;
		atomic<PipeStatus> *_meStat;

		static const DWORD _BUF_SIZE_READ = 1024; // Size of read buffer
		static const char* const _ERR_ALREADY_DISCONN;

	private:
		thread* _meConnThrd;
		char* _mePipeName;

		static const char* const _PIPE_DOWN_PRE; // Server pipe pre-name
		static const char* const _PIPE_UP_PRE; // Client pipe pre-name
		static const DWORD _PIPE_MODE; // The actual pipes' mode
		static const DWORD _BUF_SIZE_PIPE; // Size of pipe's buffers
		static const DWORD _PIPE_TIMEOUT; // Pipe's default timeout.
		static const char* const _ERR_READ_STRCPY;
		static const char* const _ERR_READ_STRCAT;
		static const char* const _ERR_PIPE_CREATE;
		static const char* const _ERR_ALREADY_CONN;
		static const char* const _ERR_ALREADY_CLOSED;

		//virtual void startWorking() = 0;
		virtual void stopWorking() = 0;
	};
}
