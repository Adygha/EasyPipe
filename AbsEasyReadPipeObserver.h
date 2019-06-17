#pragma once

#ifdef EASYPIPE
#undef EASYPIPE
#endif // EASYPIPE

#ifdef EASYPIPE_EXPORTS
#define EASYPIPE __declspec(dllexport)
#else
#define EASYPIPE __declspec(dllimport)
#endif

namespace ez {
	class EASYPIPE AbsEasyReadPipeObserver {
	public:
		virtual void informMessageReceived(const char* receivedMsg) = 0;
	};
}
