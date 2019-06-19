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

namespace ez {
	class EASYPIPE EasyWritePipe : public AbsEasyPipe {
	public:
		EasyWritePipe(const char* pipName, bool isServer);
		~EasyWritePipe();

		void sendMessage(const char* theMsg);

	private:
		HANDLE _meWriteThrdHdl;

		static const char* const _ERR_ALREADY_WRITE;

		virtual void stopWorking();
	};
}
