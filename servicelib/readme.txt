
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
	- ParameterService
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

------------------
SERVICE PARAMETERS
------------------

Support for read-only service parameters is provided by the following data types that
can be declared as derived service class member variables.  The defualt implementation
provided accesses the parameter data via the system registry using a "Parameters" key
located underneath the service's entry in HKLM\System\CurrentControlSet\Services.  This
underlying parameter storage medium can be overridden, see below for details.

DWordParameter (uint32_t)
- Registry type: REG_BINARY or REG_DWORD

MultiStringParameter (std::vector<svctl::tstring>)
- Registry type: REG_MULTI_SZ

QWordParameter (uint64_t)
- Registry type: REG_BINARY or REG_QWORD

StringParameter (std::[w]string)
- Registry type: REG_SZ or REG_EXPAND_SZ

BinaryParameter<T> (T)
- Registry type: REG_BINARY
- T must be a trivial data type, see std::is_trivial() for a definition of what qualifies

Declare the parameter(s) as member variable(s) in the service class, optionally specifying
a default value.  If no default value is set at construction, the value will either be set to 
zero (DWordParameter, QWordParameter, BinaryParameter<T>), an empty string (StringParameter) 
or an empty vector<[w]string> (MultiStringParameter):

	private:
		...
		StringParameter m_mysz { _T("defaultvalue") };
		DWordParameter m_mydword;
		// todo: MultiStringParameter initializer is broken right now - see servicelib.h todo
		// MultiStringParameter m_multisz { _T("String1"), _T("String2"), _T("String3")}

In order for the service to locate and bind each parameter, they must be exposed via
the BEGIN_PARAMETER_MAP() set of macros, similar to how control handlers are exposed.
Static constant parameter value names can be used, as can resource string identifiers
provided that the string table is part of the local executable's resources:

	BEGIN_PARAMETER_MAP(MyService)
		PARAMETER_ENTRY(IDS_MYSZ_NAME, m_mysz)
		PARAMETER_ENTRY(_T("MyDWORDParameter"), m_mydword)
	END_PARAMETER_MAP()

At service startup, the PARAMETER_MAP is iterated and each parameter is bound to the 
HLKM\System\CurrentControlSet\Services\{service name}\Parameters registry key.  If this
parent key does not exist, it will be created automatically.  Each parameter will then
be loaded from the parameters key using the name specified in the PARAMETER_MAP.

If any parameters exist in the PARAMETER_MAP, the service will also automatically accept
the SERVICE_CONTROL_PARAMCHANGE control, and will reload each bound parameter in response
to this service control *before* it is passed onto the service class itself.  This makes
it unnecessary for the service to declare a custom handler if no other meaningful work
needs to be done in response to SERVICE_CONTROL_PARAMCHANGE.

Parameter instances are intended to be used as cached read-only values that can be
copied into local variables as needed, they should not be used directly.  A *copy* of the
underlying value are returned each time a parameter is accessed, this behavior should be 
kept in mind, especially for the MultiStringParameter and BinaryParameter<T> versions, as
they potentially store a great deal of data.  This behavior is enforced to allow the parameters
to be thread-safe with the SERVICE_CONTROL_PARAMCHANGE automatic handler.

The auto keyword can be used in conjunction with the .Value property of any parameter
object to simplify the declaration.  This is particularly useful when working with
MultiStringParameters, as the underlying type is an std::vector<svctl::tstring> instance

Declaring and accessing parameters:

	BEGIN_PARAMETER_MAP(MyService)
		PARAMETER_ENTRY(_T("RetryCount"), m_retrycount)
		PARAMETER_ENTRY(_T("TempDirectory"), m_tempdir)
	END_PARAMETER_MAP()

	DWordParameter m_retrycount = 10;
	StringParameter m_tempdir;

	void OnStart(DWORD argc, LPTSTR* argv)
	{
		// auto can be used with the .Value property, recommended for MultiStringParameter
		auto tempdir = m_tempdir.Value;
		DoSomethingInterestingWithTheParameter(tempdir);
			
		// a specific type can also be used to read/copy the parameter value
		uint32_t retrycount = m_retrycount;
		while(--retrycount) { 
			...
		}
	}

Parameters can be manually reloaded from the registry by invoking the Load method, although
if the parameter was not successfully bound to the service's Parameters registry key, this
method will do nothing.  Detection of a successful load from the registry can be accomplished
by checking the IsDefaulted property of the parameter variable.  This will be set to false once
the value has been loaded from the registry at least once

The default registry-based implementation for parameter storage can be overriden by the service
class, perhaps to load parameters from a file or a database.  This is accomplished by overloading
three protected methods (all three should be overridden):

	void* OpenParameterStore(const TCHAR* servicename)
		- Opens the storage medium (file, database, registry, etc) where the parameters are stored
		- Must return a non-NULL opaque handle as a void pointer.  If NULL is returned, the parameters will not be bound/loaded
		- This function can throw a ServiceException, but doing so will terminate the service prior to it starting

	size_t LoadParameter(void* handle, const TCHAR* name, ServiceParameterFormat format, void* buffer, size_t length)
		- Loads a parameter value from the opened parameter storage
		- Returns the number of bytes written to the buffer, or the number of bytes required if length is insufficient
		- name indicates the name of the parameter
		- format is the required output format for the parameter value data; these match up with registry value data types
		- buffer is a fixed-length buffer to receive the raw parameter value data
		- length is the length, in bytes, of the output buffer
		- If a nullptr is specified for buffer, the function should return the number of bytes required to hold the value
		- If a non-nullptr is specified for buffer and length is insufficient, this function should throw a ServiceException

	void CloseParameterStore(void* handle)
		- Closes the storage medium opened by OpenParameterStore() and associated with the opaque handle
		- This function should not throw an exception


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

When using ServiceHarness<>, the parameters implementation of the service class is disabled and
replaced with a harness-specific implementation.  Other than requiring they be set at runtime (see
below), the only limitation as compared with the registry-based service class implementation is that
the REG_EXPAND_SZ registry type is not supported; environment variables must be expanded prior to 
setting a string parameter's value.

ServiceHarness<> parameters can be set any time and are persistent within the lifetime of the harness
instance, they are not cleared when the service is stopped.  After a service has been started, a change
in a parameter's value will not become visible to the service class until a ServiceControl::ParameterChange
control has been sent, the same behavior exhibited by a service running normally:
	
The SetParameter() method can accept a constant generic text string pointer, an std::[w]string reference,
or a resource string id for the parameter name, and will infer the actual data type of the parameter value
based on what is being provided.  If the inferred format does not match the member variable type in the
service class, the value will not be propogated to the service at runtime, much like how an invalid
registry value type would behave.

	<type>                                 Inferred format                      Service<> declaration
	------                                 ---------------                      ---------------------
	8-bit integer                          ServiceParameterFormat::DWord        DWordParameter
	16-bit integer                         ServiceParameterFormat::DWord        DWordParameter
	32-bit integer                         ServiceParameterFormat::DWord        DWordParameter
	64-bit integer                         ServiceParameterFormat::QWord        QWordParameter
	const TCHAR*                           ServiceParameterFormat::String       StringParameter
	const std::[w]string&                  ServiceParameterFormat::String       StringParameter
	const TCHAR*[]                         ServiceParameterFormat::MultiString  MultiStringParameter
	std::[w]string[]                       ServiceParameterFormat::MultiString  MultiStringParameter
	std::initializer_list<const TCHAR*>    ServiceParameterFormat::MultiString  MultiStringParameter
	std::initializer_list<std::[w]string>  ServiceParameterFormat::MultiString  MultiStringParameter
	iterator<const TCHAR*> (1)             ServiceParameterFormat::MultiString  MultiStringParameter
	iterator<std::[w]string> (1)           ServiceParameterFormat::MultiString  MultiStringParameter
	[any other trivial data type]          ServiceParameterFormat::Binary       BinaryParameter

	(1) - This method requires 2 <type> parameters be specified, a begin() iterator and an end() iterator

	...
	ServiceHarness<MyService> harness;
	harness.SetParameter(_T("MyStringParameter"), _T("PathToAnInterestingFile"));
	harness.SetParameter(IDS_MYDWORDPARAMETER, 0x12345678);
	harness.Start(_T("MyServiceName"));

	// parameters can be set at any time, but if the service is running, a ServiceControl::ParameterChange 
	// control must be sent to reload them
	harness.SetParameter(_T("MyMultiStringValue"), { _T("String1"), _T("String2"), _T("String3") });
	harness.SetParameter(IDS_MYDWORDPARAMETER, 0);
	harness.SendControl(ServiceControl::ParameterChange);
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

void SetParameter(const TCHAR* name, <type> value)
void SetParameter(std::[w]string name, <type> value)
void SetParameter(unsigned int name, <type> value)
	- Sets a local service parameter key/value pair with a format derived from <type>; see table above
	- unsigned int overload accepts a resource string id for the parameter name
	- Throws ServiceException& on error

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
