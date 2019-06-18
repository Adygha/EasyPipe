#include "AbsEasyPipe.h"
#include <winnt.h>
#include <WinBase.h>
#include <cstring>
#include <stdexcept>
using std::runtime_error;

namespace ez {
#pragma region CONSTs
	const char* const AbsEasyPipe::_PIPE_READ_SERVER_PRE = "\\\\.\\pipe\\Upload"; // Server pipe pre-name
	const char* const AbsEasyPipe::_PIPE_READ_CLIENT_PRE = "\\\\.\\pipe\\Download"; // Client pipe pre-name
	const DWORD AbsEasyPipe::_PIPE_SERVER_MODE = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT; // The actual pipes' mode
	const DWORD AbsEasyPipe::_BUF_SIZE_READ = 1024;
	const DWORD AbsEasyPipe::_BUF_SIZE_PIPE = AbsEasyPipe::_BUF_SIZE_READ * 16;
	const DWORD AbsEasyPipe::_PIPE_TIMEOUT = 1000; // Pipe's default timeout.
	const char* const AbsEasyPipe::_ERR_READ_STRCPY = "Error whie copying string data. Please check that none of the provided strings are 'NULL'.";
	const char* const AbsEasyPipe::_ERR_READ_STRCAT = "Error whie concatinating string data. The concatinated string should not be 'NULL'.";
	const char* const AbsEasyPipe::_ERR_PIPE_CREATE = "Could not create the named-pipe. Same pipe name may already exist.";
	const char* const AbsEasyPipe::_ERR_ALREADY_CONN = "Already connected to pipe.";
	const char* const AbsEasyPipe::_ERR_ALREADY_DISCONN = "Already disconnected from pipe.";
	const char* const AbsEasyPipe::_ERR_ALREADY_CLOSED = "Already closed pipe.";
#pragma endregion

	AbsEasyPipe::AbsEasyPipe(const char* pipName, PipeStatus pipeStatus) :
		_mePipeName(new char[strlen(pipeStatus & PipeStatus::SERVER ? AbsEasyPipe::_PIPE_READ_SERVER_PRE : AbsEasyPipe::_PIPE_READ_CLIENT_PRE) + strlen(pipName) + 1]),
		_meStat(pipeStatus) {

		size_t tmpLen = strlen(this->_meStat & PipeStatus::SERVER ? AbsEasyPipe::_PIPE_READ_SERVER_PRE : AbsEasyPipe::_PIPE_READ_CLIENT_PRE) + strlen(pipName) + 1;
		if (strcpy_s(this->_mePipeName, tmpLen, this->_meStat & PipeStatus::SERVER ? AbsEasyPipe::_PIPE_READ_SERVER_PRE : AbsEasyPipe::_PIPE_READ_CLIENT_PRE))
			throw runtime_error(AbsEasyPipe::_ERR_READ_STRCPY);
		if (strcat_s(this->_mePipeName, tmpLen, pipName))
			throw runtime_error(AbsEasyPipe::_ERR_READ_STRCAT);
		if (this->_meStat & PipeStatus::SERVER) {
			this->_mePipeHndl = CreateNamedPipe(
				TEXT(this->_mePipeName),
				this->_meStat & PipeStatus::WRITE ? PIPE_ACCESS_OUTBOUND : PIPE_ACCESS_INBOUND,
				AbsEasyPipe::_PIPE_SERVER_MODE,
				1, AbsEasyPipe::_BUF_SIZE_READ,
				AbsEasyPipe::_BUF_SIZE_READ,
				AbsEasyPipe::_PIPE_TIMEOUT,
				nullptr
			);
			if (this->_mePipeHndl == INVALID_HANDLE_VALUE)
				throw runtime_error(AbsEasyPipe::_ERR_PIPE_CREATE);
		}
		this->_meStat |= PipeStatus::OPEN;
	}

	AbsEasyPipe::~AbsEasyPipe() {
		delete[] this->_mePipeName;
		//this->_mePipeName = nullptr;
	}

	ConnectResult AbsEasyPipe::connect() {
		if (this->_meStat & PipeStatus::OPEN != PipeStatus::OPEN)
			throw runtime_error(AbsEasyPipe::_ERR_ALREADY_CLOSED);
		if (this->_meStat & PipeStatus::CONNECTED == PipeStatus::CONNECTED)
			throw runtime_error(AbsEasyPipe::_ERR_ALREADY_CONN);
		if (this->_meStat & PipeStatus::SERVER) {
			if (!ConnectNamedPipe(this->_mePipeHndl, nullptr))
				return ConnectResult::CONN_ERROR;
		} else {
			if (WaitNamedPipe(this->_mePipeName, AbsEasyPipe::_PIPE_TIMEOUT)) {
				DWORD tmpErr = GetLastError();
				if (tmpErr == ERROR_SEM_TIMEOUT || tmpErr == ERROR_TIMEOUT) {
					return ConnectResult::CONN_TIMEOUT;
				} else {
					return ConnectResult::UNAVAIL_PIPE;
				}
				this->_mePipeHndl = CreateFile(TEXT(this->_mePipeName), this->_meStat & PipeStatus::WRITE ? GENERIC_WRITE : GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
				if (this->_mePipeHndl == INVALID_HANDLE_VALUE)
					return ConnectResult::CONN_ERROR;
			}
		}
		this->_meStat |= PipeStatus::CONNECT_STATE;
		return ConnectResult::CONNECTED;
	}

	void AbsEasyPipe::disconnect() {
		if (this->_meStat & PipeStatus::CONNECTED != PipeStatus::CONNECTED)
			throw runtime_error(AbsEasyPipe::_ERR_ALREADY_DISCONN);
		if (this->_meStat & PipeStatus::BUSY != PipeStatus::BUSY)
			this->stopWorking();
	}

	void AbsEasyPipe::close() {

	}
}
