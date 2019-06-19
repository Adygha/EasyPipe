#pragma once

#ifdef EASYPIPE
#undef EASYPIPE
#endif // EASYPIPE

#ifdef EASYPIPE_EXPORTS
#define EASYPIPE __declspec(dllexport)
#else
#define EASYPIPE __declspec(dllimport)
#endif

#include"AbsEasyPipe.h"
#include <unordered_set>
using std::unordered_set;

namespace ez {
	class EASYPIPE EasyReadPipe : public AbsEasyPipe {
	public:
		EasyReadPipe(const char* pipName, bool isServer);
		virtual ~EasyReadPipe() override;

		virtual void addObserver(AbsEasyReadPipeObserver* newObserver);
		virtual void removeObserver(AbsEasyReadPipeObserver* newObserver);
		virtual void startListening();
		virtual void stopListening();

	private:
		unordered_set<AbsEasyReadPipeObserver*>* _meObs;
		HANDLE _meReadThrdHdl;

		static const char* const _ERR_ALREADY_LSN;
		static const char* const _ERR_ALREADY_NOLSN;

		virtual void stopWorking() override;
	};
}
