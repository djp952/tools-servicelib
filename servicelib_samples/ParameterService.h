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

#ifndef __PARAMETERSERVICE_H_
#define __PARAMETERSERVICE_H_
#pragma once

//-----------------------------------------------------------------------------
// ParameterService Sample
//
// ParameterService illustrates a simple service that uses the parameter support
// built into the service template library
//
class ParameterService : public Service<ParameterService>
{
public:

	// Constructor / Destructor
	ParameterService()=default;
	virtual ~ParameterService()=default;

private:

	ParameterService(const ParameterService&)=delete;
	ParameterService& operator=(const ParameterService&)=delete;

	// CONTROL_HANDLER_MAP
	//
	// Maps service control codes to member function handlers.  The service will not
	// accept any standard controls other than Interrogate and ParameterChange if there 
	// is no handler defined in the CONTROL_HANDLER_MAP.  Failure to define a Stop
	// handler will result in a service that can be started but never stopped.
	//
	BEGIN_CONTROL_HANDLER_MAP(ParameterService)
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

		m_worker = std::move(std::thread([=]() { 
		
			auto waitms = m_messageRate.Value;
			while(WaitForSingleObject(m_signal, waitms) == WAIT_TIMEOUT)
			{
				// do stuff
				auto message = m_message.Value;

				// Parameter values are only changed when a ServiceControl::ParameterChange
				// has been received; re-read the value from cached storage each loop in this case
				waitms = m_messageRate.Value;
			}
		}));
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
		m_signal.Set();
		if(!m_worker.joinable()) throw ServiceException(E_UNEXPECTED);
		m_worker.join();
	}

	// PARAMETER_MAP
	//
	BEGIN_PARAMETER_MAP(MyService)
		PARAMETER_ENTRY(_T("MessageRate"), m_messageRate)
		PARAMETER_ENTRY(_T("Message"), m_message)
	END_PARAMETER_MAP()

	DWordParameter m_messageRate { 1000 };
	StringParameter m_message { _T("Hello from ParameterService\r\n") };

	svctl::signal<svctl::signal_type::AutomaticReset> m_signal;
	std::thread m_worker;
};

#endif	// __PARAMETERSERVICE_H_

