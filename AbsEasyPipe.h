#pragma once

namespace ez {
	enum ConnectResult : char { UNAVAIL_PIPE, CONN_ERROR, CONN_TIMEOUT, CONNECTED };

	enum PipeStatus : char {
		CLIENT,
		SERVER,
		WRITE,
		OPEN = 4,
		CONNECT_STATE = 8,
		CONNECTED = OPEN | CONNECT_STATE,
		BUSY_STATE = 16,
		BUSY = CONNECTED | BUSY_STATE
	};

	//inline PipeStatus operator~ (PipeStatus a) { return (PipeStatus)~a; }
	inline PipeStatus operator| (PipeStatus a, PipeStatus b) { return (PipeStatus)(a | b); }
	//inline PipeStatus operator& (PipeStatus a, PipeStatus b) { return (PipeStatus)(a & b); }
	//inline PipeStatus operator^ (PipeStatus a, PipeStatus b) { return (PipeStatus)(a ^ b); }
	inline PipeStatus& operator|= (PipeStatus& a, PipeStatus b) { return (PipeStatus&)(a |= b); }
	//inline PipeStatus& operator&= (PipeStatus& a, PipeStatus b) { return (PipeStatus&)(a &= b); }
	//inline PipeStatus& operator^= (PipeStatus& a, PipeStatus b) { return (PipeStatus&)(a ^= b); }

	class AbsEasyPipe {
	public:
		AbsEasyPipe(const char* pipName, PipeStatus pipeStatus);
		virtual ~AbsEasyPipe();

		virtual ConnectResult connect() final;
		virtual void disconnect() final;
		virtual void close() final;

	private:
		char* _mePipeName;
		PipeStatus _meStat;
		HANDLE _mePipeHndl;

		static const char* const _PIPE_READ_SERVER_PRE; // Server pipe pre-name
		static const char* const _PIPE_READ_CLIENT_PRE; // Client pipe pre-name
		static const DWORD _PIPE_SERVER_MODE; // The actual pipes' mode
		static const DWORD _BUF_SIZE_READ; // Size of read buffer
		static const DWORD _BUF_SIZE_PIPE; // Size of pipe's buffers
		static const DWORD _PIPE_TIMEOUT; // Pipe's default timeout.
		static const char* const _ERR_READ_STRCPY;
		static const char* const _ERR_READ_STRCAT;
		static const char* const _ERR_PIPE_CREATE;
		static const char* const _ERR_ALREADY_CONN;
		static const char* const _ERR_ALREADY_DISCONN;
		static const char* const _ERR_ALREADY_CLOSED;

		//virtual void startWorking() = 0;
		virtual void stopWorking() = 0;
	};
}
