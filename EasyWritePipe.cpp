#include "EasyWritePipe.h"
#include <stdexcept>
using std::runtime_error;

namespace ez {
	const char* const EasyWritePipe::_ERR_ALREADY_WRITE = "Already writing to pipe";

	EasyWritePipe::EasyWritePipe(const char* pipName, bool isServer) :
		AbsEasyPipe(pipName, (isServer ? PipeStatus::SERVER : PipeStatus::CLIENT) | PipeStatus::WRITE),
		_meWriteThrdHdl(INVALID_HANDLE_VALUE) {
	}

	EasyWritePipe::~EasyWritePipe() {
		if ((*this->_meStat & PipeStatus::BUSY) == PipeStatus::BUSY)
			this->stopWorking();
	}

	void EasyWritePipe::sendMessage(const char* theMsg) {
		//if ((*this->_meStat & PipeStatus::CONNECTED) != PipeStatus::CONNECTED)
		//	throw runtime_error(EasyWritePipe::_ERR_ALREADY_DISCONN);
		if ((*this->_meStat & PipeStatus::CONNECTED) == PipeStatus::CONNECTED) {
			if ((*this->_meStat & PipeStatus::BUSY) == PipeStatus::BUSY)
				throw runtime_error(EasyWritePipe::_ERR_ALREADY_WRITE);
			*this->_meStat = *this->_meStat | PipeStatus::BUSY;
			DWORD tmpWritten;
			this->_meWriteThrdHdl = GetCurrentThread();
			WriteFile(this->_mePipeHndl, theMsg, (DWORD)(strlen(theMsg) + 1), &tmpWritten, nullptr);
			this->_meWriteThrdHdl = INVALID_HANDLE_VALUE;
			*this->_meStat = *this->_meStat & ~PipeStatus::BUSY_STATE;
		}
	}

	void EasyWritePipe::stopWorking() {
		if (this->_meWriteThrdHdl != INVALID_HANDLE_VALUE)
			CancelSynchronousIo(this->_meWriteThrdHdl);
		*this->_meStat = *this->_meStat & ~PipeStatus::BUSY_STATE;
	}
}
