# EasyPipe
An attempt to make an easy way to use windows named pipes.

## Build
VC++:
```
cl /EHsc /W4 /WX /MD /LD AbsEasyPipe.cpp EasyReadPipe.cpp EasyWritePipe.cpp /I. /DEASYPIPE_EXPORTS /link /OUT:EasyPipe.dll
```
