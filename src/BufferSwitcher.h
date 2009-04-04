// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the BUFFERSWITCHER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// BUFFERSWITCHER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef BUFFERSWITCHER_EXPORTS
#define BUFFERSWITCHER_API __declspec(dllexport)
#else
#define BUFFERSWITCHER_API __declspec(dllimport)
#endif

// This class is exported from the BufferSwitcher.dll
class BUFFERSWITCHER_API CBufferSwitcher {
public:
	CBufferSwitcher(void);
	// TODO: add your methods here.
};

extern BUFFERSWITCHER_API int nBufferSwitcher;

BUFFERSWITCHER_API int fnBufferSwitcher(void);
