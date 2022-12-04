#include "StdAfx.h"
#include <iostream>
#include "SerialLink.h"


SerialLink::SerialLink(const char* pszPort, int bps)
	: dwToWrite(0)
{

	hComm = ::CreateFileA( pszPort,  
                    GENERIC_READ | GENERIC_WRITE, 
                    0, 
                    0, 
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED,
                    0);
	if (hComm == INVALID_HANDLE_VALUE) {
		std::cerr << "Unable to open serial port " << pszPort << std::endl;
		exit(3);
	}


	DCB dcb;
	::memset(&dcb, 0, sizeof(dcb));

	dcb.DCBlength = sizeof(dcb);
   if (!BuildCommDCBA("4800,n,8,1", &dcb)) {   
		std::cerr << "Unable to build device control block " << std::endl;
		exit(3);
   }
   dcb.BaudRate = bps;
   dcb.fOutxCtsFlow = 0;
   dcb.fOutxDsrFlow = 0;
   dcb.fDtrControl = DTR_CONTROL_ENABLE; // just enable it
   dcb.fDsrSensitivity = 0;
   dcb.fOutX = 0;
   dcb.fInX = 0;
   dcb.fRtsControl = RTS_CONTROL_ENABLE; // just enable it.
   dcb.fAbortOnError = 0;


   if (!SetCommState(hComm, &dcb)) {
		std::cerr << "Unable to set comms state " << std::endl;
		exit(4);
   }

   // Create event for use by overlapped IO
   hEvent = ::CreateEvent(NULL, TRUE, TRUE, NULL); // Manual reset, initially signalled.
}


SerialLink::~SerialLink(void)
{
	::CloseHandle(hEvent);
	::CloseHandle(hComm);
}

bool SerialLink::canSend() {
	DWORD dwWritten;
	DWORD dw = ::WaitForSingleObject(hEvent,0); // sample but don't wait.
	std::cout << "WFSO returned " << dw << std::endl;
	switch(dw) {
	case WAIT_OBJECT_0:	// event is signalled so write should have completed.

		// If bytes have been written then check completed.
		if(dwToWrite > 0) {
			if (!::GetOverlappedResult(hComm, &osWrite, &dwWritten, FALSE)) {
				std::cerr << "GetOverlappedResult failed" << std::endl;
			} else { // success
			   if (dwWritten != dwToWrite) {
				   std::cerr << "Only wrote " << dwWritten << " of " << dwToWrite << " bytes" << std::endl;
			   }
			}
		}

		::ResetEvent(osWrite.hEvent);
		return true;

	case WAIT_TIMEOUT:  // not yet signalled, still pending.
		std::cout << "Waiting..." << std::endl;
		return false;

	default:
		std::cerr << "Unknown status " << dw << " from WaitForSingleObject" << std::endl;
		break;
	}
	return false;
}


void SerialLink::send(std::string& str) {

   
   DWORD dwWritten;

    // reset OVERLAPPED structure for this write
   	::memset(&osWrite, 0, sizeof(osWrite));
    osWrite.hEvent = hEvent;

   const char * lpBuf = str.c_str();
   dwToWrite = (DWORD)str.length();

    // Issue write
   if (!WriteFile(hComm, lpBuf, dwToWrite, &dwWritten, &osWrite)) {
      if (GetLastError() != ERROR_IO_PENDING) { 
         std::cerr << "Write file failed" << std::endl;
      }
   }
}


