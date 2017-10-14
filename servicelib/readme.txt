
SERVICE TEMPLATE LIBRARY
Copyright (c) 2001-2014 Michael G. Brehm
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

>> GENERAL DOCUMENTATION HERE
	- why I think this is awesome

>> BRIEF HISTORY COMPARISON WITH OLD SVCTL (WHAT HAS BEEN REMOVED)
	- put svctl code into a subfolder within servicelib for reference
	- note that C++11 / STL was now a goal not a hindrance like in svctl

---------------
SAMPLE SERVICES
---------------

The Service Template Library has a number of sample projects that illustrate it's usage, 
probably much better than this readme file will be able to:

(anticipated samples below; need to describe each)

	- MinimalService		
	- PauseContinueService
	- EventTracingService
	- TriggerEventService
	- SocketService
	- NamedPipeService
	- RpcService (??)
	- ComService (??)

>> SERVICE TABLES 
	- need to illustrate a main() function
	- describe OWN vs SHARED processes
	- how to use multiple OWN services in a single executable (need command line)

>> INSTALL/REMOVE
	- with sc
	- with WiX ?
	- with code (may not be providing this; doesn't make a lot of sense in 2014 thanks to SC and WiX)

>> SERVICE MODULES / AUTOMATING SERVICE TABLES AND INSTALL/REMOVE
	- may or may not make these

>> COM SERVICE (need comservice.h / comservice.cpp?)
>> RPC SERVICE (need rpcservice.h / rpcservice.cpp?)
	- probably not going to bring these forward from svctl, not the goal of servicelib

>> HOW TO DEBUG SERVICES
	- mention ServiceHarness<> as possibly better way for general debugging (below)

>> USING SHARED_PTR SERVICE CLASSES
	- this is now enabled automatically if the service class derives from std::enable_shared_from_this<_derived>:
		class MyService : public Service<MyService>, public std::enable_shared_from_this<MyService>
	- will not activate with any other derivation of enable_shared_from_this, must be _derived
	- caution on managing pointers, must all be released in OnStop() otherwise destructor will never be called
	  and this could be really bad for shared process services

----------------
CONTROL HANDLERS
----------------

STOP/PAUSE/CONTINUE - describe automatic status
	- not supposed to do this; doing it anwyway

	- describe synchronous vs. asynchronous handlers
	- describe when to use DWORD or (DWORD, void*) handlers
	- touch on user controls, note that only void(void) should be used
	- describe what controls are disabled during state changes
	- trigger events?
	- behavior when an exception is thrown from a handler

The following table lists the supported service control codes, whether or not the handler will
be called synchronously or asynchronously, and the suggested handler method signature:

	Service Control                        Model         Suggested Handler Signature
	---------------                        -----         ---------------------------

	ServiceControl::Continue               Asynchronous  void OnContinue(void)
	ServiceControl::HardwareProfileChange  Synchronous   DWORD OnHardwareProfileChange(DWORD eventtype, void* eventdata)
	ServiceControl::ParameterChange        Synchronous   void OnParameterChange(void)
	ServiceControl::Pause                  Asynchronous  void OnPause(void)
	ServiceControl::PreShutdown            Synchronous   void OnPreShutdown(void)
	ServiceControl::PowerEvent             Synchronous   DWORD OnPowerEvent(DWORD eventtype, void* eventdata)
	ServiceControl::SessionChange          Synchronous   void OnSessionChange(DWORD eventtype, void* eventdata)
	ServiceControl::Shutdown               Synchronous   void OnShutdown(void)
	ServiceControl::Stop                   Asynchronous  void OnStop(void)
	ServiceControl::TimeChange             Synchronous   void OnTimeChange(void)
	ServiceControl::TriggerEvent           Synchronous   void OnTriggerEvent(void)
	ServiceControl::UserModeReboot         Synchronous   void OnUserModeReboot(void)
	[Custom: 128-255]                      Synchronous   void OnXxxxxxxxx(void)

--------------------
SERVICE TEST HARNESS
--------------------

Services generated with the service template library can also be run under the context of a provided
test harness class, ServiceHarness<>.  This class provides the ability to start, stop and send control
codes to the service class.  Mimicking control codes that require event data is supported, but it's
left up to the harness application to send in something realistic.

Service command line arguments can be provided when starting the service by appending them to the
Start(servicename) method.  A resource string id can also be used to specify the service name. This 
method is variadic and can accept any fundamental data type supported by std::to_[w]string(), a 
constant generic text string pointer, or a reference to an std::[w]string object:

	Start(L"MyService", std::wstring(L"argumentone"), 11, 123.45, true, L"lastargument");
	Start(IDS_MYSERVICE_NAME);

The simplest pattern for using ServiceHarness<> requires invoking the Start(servicename) method, waiting
for some trigger to occur that will terminate the harness application, and then invoking Stop():

	...
	ServiceHarness<MyService> harness;
	harness.Start(IDS_MYSERVICE_NAME);
	// wait for some trigger here to stop the service test harness
	harness.Stop();
	...

ServiceHarness<> Methods:
-------------------------

void Continue(void)
	- Sends ServiceControl::Continue to the service
	- Waits for service to reach ServiceStatus::Running
	- Throws ServiceException& on error or if service stops prematurely

void Pause(void)
	- Sends ServiceControl::Pause to the service
	- Waits for service to reach ServiceStatus::Paused
	- Throws ServiceException& on error or if service stops prematurely

DWORD SendControl(ServiceControl control)
DWORD SendControl(ServiceControl control, DWORD eventtype, LPVOID eventdata)
	- Sends a control code to the service, optionally specifying event information (this is not common)
	- Returns a status code similar to Win32 API's ControlService() method, should not throw an exception

void Start(const TCHAR* servicename, ...)
void Start(std::[w]string servicename, ...)
void Start(unsigned int servicename, ...)
	- Starts the service with the specified service name and optional command line arguments
	- unsigned int overload accepts a resource string id for the service name
	- If service does not reach ServiceStatus::StartPending in 30 seconds, will raise an exception
	- Waits for the service to reach ServiceStatus::Running
	- Throws ServiceException& on error or if service stops prematurely

void Stop(void)
	- Sends ServiceControl::Stop to the service
	- Waits for service to reach ServiceStatus::Stopped
	- Throws ServiceException& on error or if service stops prematurely

bool WaitForStatus(ServiceStatus status, uint32_t timeout = INFINITE)
	- Optional timeout value is specified in milliseconds
	- Waits for the service to reach the specified status
	- Returns TRUE if service reached the status, false if the operation timed out
	- Throws ServiceException& on error or if service stops prematurely


ServiceHarness<> Properties:
----------------------------

bool CanContinue (read-only)
	- Determines if the service is capable of accepting ServiceControl::Continue

bool CanPause (read-only)
	- Determines if the service is capable of accepting ServiceControl::Pause

bool CanStop (read-only)
	- Determines if the service is capable of accepting ServiceControl::Stop

SERVICE_STATUS Status (read-only)
	- Gets a copy of the current SERVICE_STATUS structure for the service
