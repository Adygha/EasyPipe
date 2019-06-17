#pragma once

#ifdef EASYPIPE
#undef EASYPIPE
#endif // EASYPIPE

#ifdef EASYPIPE_EXPORTS
#define EASYPIPE __declspec(dllexport)
#else
#define EASYPIPE __declspec(dllimport)
#endif

#include <Windows.h>
#include <thread>

using std::thread;

namespace ez {
	class EASYPIPE EasyWritePipe {
	public:
		EasyWritePipe(const char* pipName, bool isServer);
		~EasyWritePipe();

		void sendMessage(const char* theMsg);
		//void startListening();
		//void stopListening();

	private:
		char* _mePipeName;
		bool _meIsServer;
		HANDLE _mePipeHndl;
		bool _meIsConnected;
		thread* _meConnThrd;
	};
}
