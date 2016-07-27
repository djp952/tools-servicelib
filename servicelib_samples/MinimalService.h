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

#ifndef __MINIMALSERVICE_H_
#define __MINIMALSERVICE_H_
#pragma once

//-----------------------------------------------------------------------------
// MinimalService Sample
//
// MinimalService implements an extremely basic service that does nothing other
// than start and stop
//
// Installation/removal via sc:
//
//	sc create MinimalServiceSample binPath=[path to servicelib_samples.exe] type=share start=demand
//	sc delete MinimalServiceSample
//
// Accepted controls:
//
//	START	- sc start MinimalServiceSample
//	STOP	- sc stop MinimalServiceSample
//
class MinimalService : public Service<MinimalService>
{
public:

	// Constructor / Destructor
	MinimalService()=default;
	virtual ~MinimalService()=default;

private:

	MinimalService(const MinimalService&)=delete;
	MinimalService& operator=(const MinimalService&)=delete;

	// CONTROL_HANDLER_MAP
	//
	// Maps service control codes to member function handlers.  The service will not
	// accept any standard controls other than Interrogate and ParameterChange if there 
	// is no handler defined in the CONTROL_HANDLER_MAP.  Failure to define a Stop
	// handler will result in a service that can be started but never stopped.
	//
	BEGIN_CONTROL_HANDLER_MAP(MinimalService)
		CONTROL_HANDLER_ENTRY(ServiceControl::Stop, OnStop)
	END_CONTROL_HANDLER_MAP()

	// OnStart (Service)
	//
	// Every service that derives from Service<> must define an OnStart(), but
	// it doesn't necessarily have to do anything useful.  argc/argv are the service
	// command line arguments sent from the service control manager.  argv[0] will
	// always contain the service name.
	//
	// OnStart must return for the service to be started, do not perform any operations
	// that will block the thread indefinitely; this will result in your service hanging
	// in a StartPending status until the process is killed
	//
	// Service startup can be aborted by throwing a ServiceException from OnStart():
	//
	//		if(badthing) throw ServiceException(ERROR_NOT_ENOUGH_MEMORY);
	//
	void OnStart(int argc, LPTSTR* argv)
	{
		UNREFERENCED_PARAMETER(argc);
		UNREFERENCED_PARAMETER(argv);

		// Handle your service startup here - do not block the thread indefinitely
	}

	// Service Control Handlers
	//
	// Control handler functions can be declared with any of the following signatures:
	//
	//		void	MyHandler(void)
	//		void	MyHandler(DWORD eventtype, void* eventdata)
	//		DWORD	MyHandler(void)
	//		DWORD	MyHandler(DWORD eventtype, void* eventdata)
	//
	// Control handlers should never throw exceptions; if one is thrown the service
	// will be terminated without invoking any registered Stop handlers
	//
	void OnStop(void)
	{
		// Handle your service shutdown here
	}
};

#endif	// __MINIMALSERVICE_H_

