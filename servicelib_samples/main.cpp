//-----------------------------------------------------------------------------
// Copyright (c) 2001-2014 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "resource.h"

// Sample services
#include "MinimalService.h"
#include "ParameterService.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// _tWinMain
//
// Application entry point; typically you will want to make your service application
// target the Windows subsystem rather than the Console subsystem so it won't try
// to create a console in the default windowstation
//
// Arguments:
//
//	instance		- Application instance handle (base address)
//	previnstance	- Obsolete; do not use
//	commandline		- Command line used to start the application process
//	show			- Initial window display mode

int APIENTRY _tWinMain(HINSTANCE instance, HINSTANCE previnstance, LPTSTR commandline, int show)
{
	UNREFERENCED_PARAMETER(instance);
	UNREFERENCED_PARAMETER(previnstance);
	UNREFERENCED_PARAMETER(commandline);
	UNREFERENCED_PARAMETER(show);

#ifdef _DEBUG

	// Enable CRT memory leak checking in _DEBUG builds of the application
	int nDbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	nDbgFlags |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(nDbgFlags);

#endif	// _DEBUG

	// Running as a service -- create and dispatch a ServiceTable that references all
	// of the services that will be available as part of this application process
	ServiceTable services = { 
		
		ServiceTableEntry<MinimalService>(IDS_MINIMALSERVICE_NAME), 
		ServiceTableEntry<ParameterService>(IDS_PARAMETERSERVICE_NAME) 
	};

	services.Dispatch();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
