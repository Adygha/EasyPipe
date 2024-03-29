#include "AbsEasyReadPipeObserver.h"
#include "EasyReadPipe.h"
#include <stdexcept>
using std::runtime_error;

namespace ez {
	const char* const EasyReadPipe::_ERR_ALREADY_LSN = "Already listening to pipe.";
	const char* const EasyReadPipe::_ERR_ALREADY_NOLSN = "Already not listening to pipe.";

	EasyReadPipe::EasyReadPipe(const char* pipName, bool isServer) :
		AbsEasyPipe(pipName, isServer ? PipeStatus::SERVER : PipeStatus::CLIENT),
		_meObs(new unordered_set<AbsEasyReadPipeObserver*>()),
		_meReadThrdHdl(INVALID_HANDLE_VALUE) {
	}

	EasyReadPipe::~EasyReadPipe() {
		if ((*this->_meStat & PipeStatus::BUSY) == PipeStatus::BUSY)
			this->stopListening();
		this->_meObs->clear();
		delete this->_meObs;
		//this->_meObs = nullptr;
	}

	void EasyReadPipe::addObserver(AbsEasyReadPipeObserver* newObserver) {
		this->_meObs->insert(newObserver);
	}

	void EasyReadPipe::removeObserver(AbsEasyReadPipeObserver* newObserver) {
		this->_meObs->erase(newObserver);
	}

	void EasyReadPipe::startListening() {
		if ((*this->_meStat & PipeStatus::CONNECTED) != PipeStatus::CONNECTED)
			throw runtime_error(EasyReadPipe::_ERR_ALREADY_DISCONN);
		if ((*this->_meStat & PipeStatus::BUSY) == PipeStatus::BUSY)
			throw runtime_error(EasyReadPipe::_ERR_ALREADY_LSN);
		*this->_meStat = *this->_meStat | PipeStatus::BUSY;
		char tmpBuf[EasyReadPipe::_BUF_SIZE_READ];
		DWORD tmpRead;
		this->_meReadThrdHdl = GetCurrentThread();
		while (((*this->_meStat & PipeStatus::BUSY) == PipeStatus::BUSY) && ReadFile(this->_mePipeHndl, tmpBuf, EasyReadPipe::_BUF_SIZE_READ - 1, &tmpRead, nullptr)) {
			tmpBuf[tmpRead] = '\0';
			for (AbsEasyReadPipeObserver* obs : *this->_meObs)
				obs->informMessageReceived(tmpBuf);
		}
		this->_meReadThrdHdl = INVALID_HANDLE_VALUE;
		if ((*this->_meStat & PipeStatus::BUSY) == PipeStatus::BUSY)
			this->stopListening();
	}

	void EasyReadPipe::stopListening() {
		if ((*this->_meStat & PipeStatus::BUSY) != PipeStatus::BUSY)
			throw runtime_error(EasyReadPipe::_ERR_ALREADY_NOLSN);
		FlushFileBuffers(this->_mePipeHndl);
		if (this->_meReadThrdHdl != INVALID_HANDLE_VALUE)
			CancelSynchronousIo(this->_meReadThrdHdl);
		*this->_meStat = *this->_meStat & ~PipeStatus::BUSY_STATE;
	}

	void EasyReadPipe::stopWorking() {
		this->stopListening();
	}
}
