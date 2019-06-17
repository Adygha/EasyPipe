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
#include <vector>

using std::vector;

namespace ez {
	enum ConnectResult : char { NO_PIPE, CONN_ERROR, CONN_TIMEOUT, CONNECTED };

	class EASYPIPE EasyReadPipe {
	public:
		EasyReadPipe(const char* pipName, bool isServer);
		~EasyReadPipe();

		void addObserver(AbsEasyReadPipeObserver* newObserver);
		ConnectResult connect();
		void startListening();
		void stopListening();
		void disconnect();
		void close();

	private:
		char* _mePipeName;
		bool _meIsServer;
		HANDLE _mePipeHndl;
		bool _meIsConnected;
		bool _meIsListening;
		bool _meIsClosed;
		vector<AbsEasyReadPipeObserver*>* _meObs;
		HANDLE _meReadThrdHdl;
	};
}
