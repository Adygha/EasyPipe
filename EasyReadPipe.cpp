#include "AbsEasyReadPipeObserver.h"
#include "EasyReadPipe.h"

//#include <cstring>
#include <stdexcept>

using std::runtime_error;

namespace ez {
	const char* PIPE_READ_SERVER_PRE = "\\\\.\\pipe\\Upload"; // Server pipe pre-name
	const char* PIPE_READ_CLIENT_PRE = "\\\\.\\pipe\\Download"; // Client pipe pre-name
	const DWORD PIPE_TIMEOUT = 1000; // Pipe's default timeout.
	const DWORD PIPE_SERVER_OPEN_MODE = PIPE_ACCESS_INBOUND;// | FILE_FLAG_FIRST_PIPE_INSTANCE;
	const DWORD PIPE_SERVER_MODE = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT;
	const DWORD PIPE_CLIENT_OPEN_MODE = GENERIC_READ;
	const unsigned BUF_SIZE_READ = 1024;
	const unsigned BUF_SIZE_READ_PIPE = BUF_SIZE_READ * 16;
	const char* ERR_READ_STRCPY = "Error whie copying string data. Please check that none of the provided strings are 'NULL'.";
	const char* ERR_READ_STRCAT = "Error whie concatinating string data. The concatinated string should not be 'NULL'.";
	const char* ERR_READ_PIPE_CREATE = "Could not create the named-pipe. Same pipe name may already exist.";
	//const char* ERR_READ_PIPE_CONN = "Could not connect to the named-pipe.";
	const char* ERR_ALREADY_CONN = "Already connected to pipe.";
	const char* ERR_ALREADY_DISCONN = "Already disconnected from pipe.";
	const char* ERR_ALREADY_LSN = "Already listening to pipe.";
	const char* ERR_ALREADY_NOLSN = "Already not listening to pipe.";
	const char* ERR_ALREADY_CLOSED = "Already closed pipe.";

	EasyReadPipe::EasyReadPipe(const char* pipName, bool isServer) :
		_mePipeName(new char[strlen(isServer ? PIPE_READ_SERVER_PRE : PIPE_READ_CLIENT_PRE) + strlen(pipName) + 1]),
		_meIsServer(isServer),
		_mePipeHndl(nullptr),
		_meIsConnected(false),
		_meIsListening(false),
		_meIsClosed(false),
		_meObs(new vector<AbsEasyReadPipeObserver*>()),
		_meReadThrdHdl(INVALID_HANDLE_VALUE) {

		size_t tmpLen = strlen(isServer ? PIPE_READ_SERVER_PRE : PIPE_READ_CLIENT_PRE) + strlen(pipName) + 1;
		if (strcpy_s(this->_mePipeName, tmpLen, isServer ? PIPE_READ_SERVER_PRE : PIPE_READ_CLIENT_PRE))
			throw runtime_error(ERR_READ_STRCPY);
		if (strcat_s(this->_mePipeName, tmpLen, pipName))
			throw runtime_error(ERR_READ_STRCAT);
		if (this->_meIsServer) {
			this->_mePipeHndl = CreateNamedPipe(TEXT(this->_mePipeName), PIPE_SERVER_OPEN_MODE, PIPE_SERVER_MODE, 1, BUF_SIZE_READ_PIPE, BUF_SIZE_READ_PIPE, PIPE_TIMEOUT, nullptr);
			if (this->_mePipeHndl == INVALID_HANDLE_VALUE)
				throw runtime_error(ERR_READ_PIPE_CREATE);
		}
	}

	EasyReadPipe::~EasyReadPipe() {
		if (!this->_meIsClosed)
			this->close();
		this->_meObs->clear();
		delete this->_meObs;
		this->_meObs = nullptr;
		delete[] this->_mePipeName;
		this->_mePipeName = nullptr;
	}

	void EasyReadPipe::addObserver(AbsEasyReadPipeObserver* newObserver) {
		this->_meObs->push_back(newObserver);
	}

	ConnectResult EasyReadPipe::connect() {
		if (this->_meIsConnected)
			throw runtime_error(ERR_ALREADY_CONN);
		if (this->_meIsServer) {
			if (!ConnectNamedPipe(this->_mePipeHndl, nullptr))
				return ConnectResult::CONN_ERROR;
		} else {
			switch (WaitNamedPipe(this->_mePipeName, PIPE_TIMEOUT)) {
				case 0:
					return ConnectResult::NO_PIPE;
				case ERROR_SEM_TIMEOUT:
					return ConnectResult::CONN_TIMEOUT;
				default:
					this->_mePipeHndl = CreateFile(TEXT(this->_mePipeName), PIPE_CLIENT_OPEN_MODE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
					if (this->_mePipeHndl == INVALID_HANDLE_VALUE)
						return ConnectResult::CONN_ERROR;
			};
		}
		this->_meIsConnected = true;
		return ConnectResult::CONNECTED;
	}

	void EasyReadPipe::startListening() {
		if (!this->_meIsConnected)
			throw runtime_error(ERR_ALREADY_DISCONN);
		if (this->_meIsListening)
			throw runtime_error(ERR_ALREADY_LSN);
		this->_meIsListening = true;
		char tmpBuf[BUF_SIZE_READ];
		DWORD tmpRead;
		this->_meReadThrdHdl = GetCurrentThread();
		while (this->_meIsListening && ReadFile(this->_mePipeHndl, tmpBuf, BUF_SIZE_READ - 1, &tmpRead, nullptr)) {
			tmpBuf[tmpRead] = '\0';
			for (AbsEasyReadPipeObserver* obs : *this->_meObs)
				obs->informMessageReceived(tmpBuf);
		}
		this->_meReadThrdHdl = INVALID_HANDLE_VALUE;
		if (this->_meIsListening)
			this->stopListening();
	}

	void EasyReadPipe::stopListening() {
		if (!this->_meIsConnected)
			throw runtime_error(ERR_ALREADY_DISCONN);
		if (!this->_meIsListening)
			throw runtime_error(ERR_ALREADY_NOLSN);
		FlushFileBuffers(this->_mePipeHndl);
		if (this->_meReadThrdHdl != INVALID_HANDLE_VALUE)
			CancelSynchronousIo(this->_meReadThrdHdl);
		this->_meIsListening = false;
	}

	void EasyReadPipe::disconnect() {
		if (!this->_meIsConnected)
			throw runtime_error(ERR_ALREADY_DISCONN);
		if (this->_meIsListening)
			this->stopListening();
		if (this->_meIsServer) {
			DisconnectNamedPipe(this->_mePipeHndl);
		} else {
			if (this->_mePipeHndl != INVALID_HANDLE_VALUE) {
				CloseHandle(this->_mePipeHndl);
				this->_mePipeHndl = INVALID_HANDLE_VALUE;
			}
		}
		this->_meIsConnected = false;
	}

	void EasyReadPipe::close() {
		if (this->_meIsClosed)
			throw runtime_error(ERR_ALREADY_CLOSED);
		if (this->_meIsConnected)
			this->disconnect();
		if (this->_meIsServer) {
			if (this->_mePipeHndl != INVALID_HANDLE_VALUE) {
				CloseHandle(this->_mePipeHndl);
				this->_mePipeHndl = INVALID_HANDLE_VALUE;
			}
		}
		this->_meIsClosed = true;
	}
}
