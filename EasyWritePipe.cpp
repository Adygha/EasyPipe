#include "EasyWritePipe.h"

//#include <cstring>
#include <stdexcept>

using std::runtime_error;

namespace ez {
	const char* PIPE_SERVER_PRE = "\\\\.\\pipe\\Download"; // Server pipe pre-name
	const char* PIPE_CLIENT_PRE = "\\\\.\\pipe\\Upload"; // Client pipe pre-name
	const DWORD PIPE_SERVER_OPEN_MODE = PIPE_ACCESS_OUTBOUND;// | FILE_FLAG_FIRST_PIPE_INSTANCE;
	const DWORD PIPE_SERVER_MODE = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT;
	const DWORD PIPE_CLIENT_OPEN_MODE = GENERIC_WRITE;
	const unsigned BUF_SIZE_PIPE = 16384;
	const char* ERR_STRCPY = "Error whie copying string data. Please check that none of the provided strings are 'NULL'.";
	const char* ERR_STRCAT = "Error whie concatinating string data. The concatinated string should not be 'NULL'.";
	const char* ERR_PIPE_CREATE = "Could not create the named-pipe. Same pipe name may already exist.";
	const char* ERR_PIPE_CONN = "Could not connect to the named-pipe.";

	EasyWritePipe::EasyWritePipe(const char* pipName, bool isServer) :
		_mePipeName(new char[strlen(isServer ? PIPE_SERVER_PRE : PIPE_CLIENT_PRE) + strlen(pipName) + 1]),
		_meIsServer(isServer),
		_mePipeHndl(nullptr),
		_meIsConnected(isServer ? false : true),
		_meConnThrd(nullptr) {

		size_t tmpLen = strlen(isServer ? PIPE_SERVER_PRE : PIPE_CLIENT_PRE) + strlen(pipName) + 1;
		if (strcpy_s(this->_mePipeName, tmpLen, isServer ? PIPE_SERVER_PRE : PIPE_CLIENT_PRE))
			throw runtime_error(ERR_STRCPY);
		if (strcat_s(this->_mePipeName, tmpLen, pipName))
			throw runtime_error(ERR_STRCAT);
		if (this->_meIsServer) {
			this->_mePipeHndl = CreateNamedPipe(TEXT(this->_mePipeName), PIPE_SERVER_OPEN_MODE, PIPE_SERVER_MODE, 1, BUF_SIZE_PIPE, BUF_SIZE_PIPE, NMPWAIT_USE_DEFAULT_WAIT, nullptr);
			if (this->_mePipeHndl == INVALID_HANDLE_VALUE)
				throw runtime_error(ERR_PIPE_CREATE);
			this->_meConnThrd = new thread([this]() {
				if (!ConnectNamedPipe(this->_mePipeHndl, nullptr))
					throw runtime_error(ERR_PIPE_CONN);
				this->_meIsConnected = true;
			});
		} else {
			this->_mePipeHndl = CreateFile(TEXT(this->_mePipeName), PIPE_CLIENT_OPEN_MODE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
			if (this->_mePipeHndl == INVALID_HANDLE_VALUE)
				throw runtime_error(ERR_PIPE_CONN);
		}
	}

	EasyWritePipe::~EasyWritePipe() {
		if (!this->_meIsConnected)
			CancelSynchronousIo(this->_meConnThrd->native_handle());
		if (this->_meConnThrd->joinable()) {
			this->_meConnThrd->join();
		} else {
			this->_meConnThrd->detach();
		}
		FlushFileBuffers(this->_mePipeHndl);
		if (this->_meIsServer)
			DisconnectNamedPipe(this->_mePipeHndl);
		if (this->_mePipeHndl != INVALID_HANDLE_VALUE)
			CloseHandle(this->_mePipeHndl);
		delete this->_meConnThrd;
		this->_meConnThrd = nullptr;
		delete[] this->_mePipeName;
		this->_mePipeName = nullptr;
	}

	void EasyWritePipe::sendMessage(const char* theMsg) {
		if (this->_meIsConnected) {
			DWORD tmpWritten;
			WriteFile(this->_mePipeHndl, theMsg, (DWORD)(strlen(theMsg) + 1), &tmpWritten, nullptr);
		}
	}

	//void EasyWritePipe::startListening() {
	//}

	//void EasyWritePipe::stopListening() {
	//}
}
