#include "AbsEasyPipe.h"
//#include <cstring>
#include <stdexcept>
using std::runtime_error;

namespace ez {
#pragma region CONSTs
	const char* const AbsEasyPipe::_PIPE_DOWN_PRE = "\\\\.\\pipe\\Download"; // Download (to server) pipe pre-name
	const char* const AbsEasyPipe::_PIPE_UP_PRE = "\\\\.\\pipe\\Upload"; // Upload (from server) pipe pre-name
	const DWORD AbsEasyPipe::_PIPE_MODE = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT; // The actual pipes' mode
	//const DWORD AbsEasyPipe::_BUF_SIZE_READ = 1024;
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
		_mePipeHndl(INVALID_HANDLE_VALUE),
		_meStat(new atomic<PipeStatus>(pipeStatus)),
		_meConnThrd(nullptr),
		_mePipeName(
			new char[
				strlen(
					(pipeStatus & (PipeStatus::SERVER | PipeStatus::WRITE)) == (PipeStatus::SERVER | PipeStatus::WRITE) ||
					((pipeStatus & PipeStatus::SERVER) != PipeStatus::SERVER && (pipeStatus & PipeStatus::WRITE) != PipeStatus::WRITE)
					? AbsEasyPipe::_PIPE_DOWN_PRE : AbsEasyPipe::_PIPE_UP_PRE
				) + strlen(pipName) + 1
			]
		) {

		size_t tmpLen = strlen(
			(pipeStatus & (PipeStatus::SERVER | PipeStatus::WRITE)) == (PipeStatus::SERVER | PipeStatus::WRITE) ||
			((pipeStatus & PipeStatus::SERVER) != PipeStatus::SERVER && (pipeStatus & PipeStatus::WRITE) != PipeStatus::WRITE)
			? AbsEasyPipe::_PIPE_DOWN_PRE : AbsEasyPipe::_PIPE_UP_PRE
		) + strlen(pipName) + 1;
		if (
			strcpy_s(
				this->_mePipeName,
				tmpLen,
				(pipeStatus & (PipeStatus::SERVER | PipeStatus::WRITE)) == (PipeStatus::SERVER | PipeStatus::WRITE) ||
				((pipeStatus & PipeStatus::SERVER) != PipeStatus::SERVER && (pipeStatus & PipeStatus::WRITE) != PipeStatus::WRITE)
				? AbsEasyPipe::_PIPE_DOWN_PRE : AbsEasyPipe::_PIPE_UP_PRE
			)
		)
			throw runtime_error(AbsEasyPipe::_ERR_READ_STRCPY);
		if (strcat_s(this->_mePipeName, tmpLen, pipName))
			throw runtime_error(AbsEasyPipe::_ERR_READ_STRCAT);
		if ((*this->_meStat & PipeStatus::SERVER) == PipeStatus::SERVER) {
			this->_mePipeHndl = CreateNamedPipe(
				TEXT(this->_mePipeName),
				(*this->_meStat & PipeStatus::WRITE) == PipeStatus::WRITE ? PIPE_ACCESS_OUTBOUND : PIPE_ACCESS_INBOUND, // TODO: Check if 'FILE_FLAG_FIRST_PIPE_INSTANCE' is possible.
				AbsEasyPipe::_PIPE_MODE,
				1, AbsEasyPipe::_BUF_SIZE_READ,
				AbsEasyPipe::_BUF_SIZE_READ,
				AbsEasyPipe::_PIPE_TIMEOUT,
				nullptr
			);
			if (this->_mePipeHndl == INVALID_HANDLE_VALUE)
				throw runtime_error(AbsEasyPipe::_ERR_PIPE_CREATE);
		}
		*this->_meStat = *this->_meStat | PipeStatus::OPEN;
	}

	AbsEasyPipe::~AbsEasyPipe() {
		if ((*this->_meStat & PipeStatus::OPEN) == PipeStatus::OPEN)
			this->close();
		delete this->_meStat;
		delete[] this->_mePipeName;
		//this->_mePipeName = nullptr;
	}

	ConnectResult AbsEasyPipe::connect() {
		if ((*this->_meStat & PipeStatus::OPEN) != PipeStatus::OPEN)
			throw runtime_error(AbsEasyPipe::_ERR_ALREADY_CLOSED);
		if ((*this->_meStat & PipeStatus::CONNECTED) == PipeStatus::CONNECTED)
			throw runtime_error(AbsEasyPipe::_ERR_ALREADY_CONN);
		if ((*this->_meStat & (PipeStatus::SERVER | PipeStatus::WRITE)) == (PipeStatus::SERVER | PipeStatus::WRITE)) {
			this->_meConnThrd = new thread([this]() {
				if (ConnectNamedPipe(this->_mePipeHndl, nullptr))
					*this->_meStat = *this->_meStat | PipeStatus::CONNECT_STATE;
			});
			return ConnectResult::CONN_NONBLOCK_SUCC;
		} else if ((*this->_meStat & PipeStatus::SERVER) == PipeStatus::SERVER) {
			if (!ConnectNamedPipe(this->_mePipeHndl, nullptr))
				return ConnectResult::CONN_ERROR;
		} else {
			if (!WaitNamedPipe(this->_mePipeName, AbsEasyPipe::_PIPE_TIMEOUT)) {
				DWORD tmpErr = GetLastError();
				if (tmpErr == ERROR_SEM_TIMEOUT || tmpErr == ERROR_TIMEOUT) {
					return ConnectResult::CONN_TIMEOUT;
				} else {
					return ConnectResult::UNAVAIL_PIPE;
				}
			}
			this->_mePipeHndl = CreateFile(TEXT(this->_mePipeName), (*this->_meStat & PipeStatus::WRITE) == PipeStatus::WRITE ? GENERIC_WRITE : GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
			if (this->_mePipeHndl == INVALID_HANDLE_VALUE)
				return ConnectResult::CONN_ERROR;
		}
		*this->_meStat = *this->_meStat | PipeStatus::CONNECT_STATE;
		return ConnectResult::CONN_SUCC;
	}

	void AbsEasyPipe::disconnect() {
		if ((*this->_meStat & PipeStatus::CONNECTED) != PipeStatus::CONNECTED)
			throw runtime_error(AbsEasyPipe::_ERR_ALREADY_DISCONN);
		if ((*this->_meStat & PipeStatus::BUSY) == PipeStatus::BUSY)
			this->stopWorking();
		if (this->_meConnThrd) {
			CancelSynchronousIo(this->_meConnThrd->native_handle());
			if (this->_meConnThrd->joinable()) {
				this->_meConnThrd->join();
			} else {
				this->_meConnThrd->detach();
			}
			delete this->_meConnThrd;
			this->_meConnThrd = nullptr;
		}
		if ((*this->_meStat & PipeStatus::SERVER) == PipeStatus::SERVER) {
			DisconnectNamedPipe(this->_mePipeHndl);
		} else if (this->_mePipeHndl != INVALID_HANDLE_VALUE) {
			CloseHandle(this->_mePipeHndl);
			this->_mePipeHndl = INVALID_HANDLE_VALUE;
		}
		*this->_meStat = *this->_meStat & ~PipeStatus::CONNECT_STATE;
	}

	void AbsEasyPipe::close() {
		if ((*this->_meStat & PipeStatus::OPEN) != PipeStatus::OPEN)
			throw runtime_error(AbsEasyPipe::_ERR_ALREADY_CLOSED);
		if ((*this->_meStat & PipeStatus::CONNECTED) == PipeStatus::CONNECTED)
			this->disconnect();
		if ((*this->_meStat & PipeStatus::SERVER) == PipeStatus::SERVER && this->_mePipeHndl != INVALID_HANDLE_VALUE) {
			CloseHandle(this->_mePipeHndl);
			this->_mePipeHndl = INVALID_HANDLE_VALUE;
		}
		*this->_meStat = *this->_meStat & ~PipeStatus::OPEN;
	}
}
