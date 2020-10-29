
#include <iostream>
#include <thread>
#include <windows.h>
#include <stdafx.h>
#include <enumser.h>
#include "driver.hpp"
#include "driverlog.h"


using namespace std;
using namespace vr;




class TrackerDeviceDriver : public ITrackedDeviceServerDriver
{
	public:
		TrackerDeviceDriver(string serial)
		{
			m_unObjectId = k_unTrackedDeviceIndexInvalid;
			prop = k_ulInvalidPropertyContainer;
			m_sSerialNumber = serial;
			m_sModelNumber = "AzureTracker";
		}
		
		virtual ~TrackerDeviceDriver(){}
		
		virtual EVRInitError Activate(TrackedDeviceIndex_t unObjectId)
		{
			m_unObjectId = unObjectId;
			prop = VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);
			
			VRProperties()->SetStringProperty(prop, Prop_ModelNumber_String, m_sModelNumber.c_str());
			VRProperties()->SetStringProperty(prop, Prop_RenderModelName_String, m_sModelNumber.c_str());
			VRProperties()->SetStringProperty(prop, Prop_TrackingSystemName_String, "Azure Tracking");
			
			VRProperties()->SetUint64Property(prop, Prop_CurrentUniverseId_Uint64, 2);
			
			VRProperties()->SetInt32Property(prop, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_OptOut);
			
			VRProperties()->SetStringProperty(prop, Prop_NamedIconPathDeviceReady_String, "{AzureTracker}/icons/tracker_ready.png");
			
			VRProperties()->SetStringProperty(prop, Prop_NamedIconPathDeviceOff_String, "{AzureTracker}/icons/tracker_not_ready.png");
			VRProperties()->SetStringProperty(prop, Prop_NamedIconPathDeviceSearching_String, "{AzureTracker}/icons/tracker_not_ready.png");
			VRProperties()->SetStringProperty(prop, Prop_NamedIconPathDeviceSearchingAlert_String, "{AzureTracker}/icons/tracker_not_ready.png");
			VRProperties()->SetStringProperty(prop, Prop_NamedIconPathDeviceReadyAlert_String, "{AzureTracker}/icons/tracker_not_ready.png");
			VRProperties()->SetStringProperty(prop, Prop_NamedIconPathDeviceNotReady_String, "{AzureTracker}/icons/tracker_not_ready.png");
			VRProperties()->SetStringProperty(prop, Prop_NamedIconPathDeviceStandby_String, "{AzureTracker}/icons/tracker_not_ready.png");
			VRProperties()->SetStringProperty(prop, Prop_NamedIconPathDeviceAlertLow_String, "{AzureTracker}/icons/tracker_not_ready.png");
			
			return VRInitError_None;
		}
		
		virtual void Deactivate()
		{
			m_unObjectId = k_unTrackedDeviceIndexInvalid;
		}
		
		virtual void EnterStandby()
		{
		}
		
		virtual void* GetComponent(const char* pchComponentNameAndVersion)
		{
			return nullptr;
		}
		
		virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
		{
			if(unResponseBufferSize >= 1)
				pchResponseBuffer[0] = 0;
		}
		
		string GetSerialNumber() const { return m_sSerialNumber; }
		
		DriverPose_t MakeDefaultPose(bool connected = true, bool tracking = true)
		{
			DriverPose_t out_pose = { 0 };
			
			out_pose.deviceIsConnected = connected;
			out_pose.poseIsValid = tracking;
			out_pose.result = tracking ? ETrackingResult::TrackingResult_Running_OK : ETrackingResult::TrackingResult_Running_OutOfRange;
			out_pose.willDriftInYaw = false;
			out_pose.shouldApplyHeadModel = false;
			out_pose.qDriverFromHeadRotation.w = out_pose.qWorldFromDriverRotation.w = out_pose.qRotation.w = 1.0;
			
			return out_pose;
		}
		
		void RunFrame()
		{
			DriverPose_t pose = MakeDefaultPose();
			
			last_pose = pose;
		}
		
		virtual DriverPose_t GetPose()
		{
			DriverLog("updating pose");
			return last_pose;
		}
			
	private:
		TrackedDeviceIndex_t m_unObjectId;
		PropertyContainerHandle_t prop;
		string m_sSerialNumber;
		string m_sModelNumber;
		DriverPose_t last_pose = MakeDefaultPose();
};




















class ServerDriver_AzureTracker : public IServerTrackedDeviceProvider
{
	public:
		virtual EVRInitError Init(IVRDriverContext *pDriverContext);
		virtual void Cleanup();
		virtual const char * const *GetInterfaceVersions() {return k_InterfaceVersions;}
		virtual void RunFrame();
		virtual bool ShouldBlockStandbyMode() {return false;}
		virtual void EnterStandby() {}
		virtual void LeaveStandby() {}
		void updateTracking();
		void threadTest();
		void startSerial(unsigned int portNum);
	

	private:
		TrackerDeviceDriver *m_pTrackerW = nullptr;
		TrackerDeviceDriver *m_pTrackerL = nullptr;
		TrackerDeviceDriver *m_pTrackerR = nullptr;
		bool isTracking = false;
		HANDLE serialHandle = nullptr;
		thread *th1;

};

EVRInitError ServerDriver_AzureTracker::Init(IVRDriverContext *pDriverContext)
{
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
	InitDriverLog(VRDriverLog());
	
	DriverLog("Starting AzureTracker driver\n");
	

	m_pTrackerW = new TrackerDeviceDriver("TCKR_w");
	VRServerDriverHost()->TrackedDeviceAdded(m_pTrackerW->GetSerialNumber().c_str(), TrackedDeviceClass_GenericTracker, m_pTrackerW);
	
	m_pTrackerL = new TrackerDeviceDriver("TCKR_l");
	VRServerDriverHost()->TrackedDeviceAdded(m_pTrackerL->GetSerialNumber().c_str(), TrackedDeviceClass_GenericTracker, m_pTrackerL);
	
	m_pTrackerR = new TrackerDeviceDriver("TCKR_r");
	VRServerDriverHost()->TrackedDeviceAdded(m_pTrackerR->GetSerialNumber().c_str(), TrackedDeviceClass_GenericTracker, m_pTrackerR);
	
	//Creates thread of updateTracking, on this object instance.
	//thread th1(&ServerDriver_AzureTracker::updateTracking, this);
	th1 = new thread(&ServerDriver_AzureTracker::updateTracking, this);
	
	
	return VRInitError_None;
}

void ServerDriver_AzureTracker::Cleanup()
{
	isTracking = false;
	if(th1)
	{
		th1->join();
		delete th1;
		th1 = nullptr;
	}
	
	CloseHandle(serialHandle);
	
	CleanupDriverLog();

	delete m_pTrackerW;
	m_pTrackerW = NULL;
	delete m_pTrackerL;
	m_pTrackerL = NULL;
	delete m_pTrackerR;
	m_pTrackerR = NULL;
}

void ServerDriver_AzureTracker::RunFrame()
{
	if(m_pTrackerW)
	{
		m_pTrackerW->RunFrame();
	}
	if(m_pTrackerL)
	{
		m_pTrackerL->RunFrame();
	}
	if(m_pTrackerR)
	{
		m_pTrackerR->RunFrame();
	}
}

void ServerDriver_AzureTracker::threadTest()
{
	int x;
	while(0)
	{
		x = 1;
	}
}

void ServerDriver_AzureTracker::updateTracking()
{
	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr))
	{
		DriverLog("Failed to initialize COM, Error:%x\n", static_cast<unsigned int>(hr));
		return;
	}
	hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
	if (FAILED(hr))
	{
		DriverLog("Failed to initialize COM security, Error:%08X\n", static_cast<unsigned int>(hr));
		CoUninitialize();
		return;
	}
	
	CEnumerateSerial::CPortsArray ports;
	CEnumerateSerial::CNamesArray names;
	CEnumerateSerial::CPortAndNamesArray portAndNames;
	
	hr = CEnumerateSerial::UsingWMI(portAndNames);
	if (SUCCEEDED(hr))
	{
		for (const auto& port : portAndNames)
		{
			DriverLog("COM%u <%s>\n", port.first, port.second.c_str());
			//ex: COM3 <USB Serial Device (COM3)>
			startSerial(port.first);
			if(serialHandle == (HANDLE)-1)
			{
				CloseHandle(serialHandle);
				continue; //failed to open the port, move on
			}
			
			char outBuf[] = "getTrackerInfo";
			DWORD bytesWrite = sizeof(outBuf);
			bool writeStat = WriteFile(serialHandle, outBuf, bytesWrite, &bytesWrite, NULL);
			if(!writeStat)
			{
				CloseHandle(serialHandle);
				continue; //serial write failed
			}
			
			const uint8_t bufSize = 100;
			char inBuf[bufSize];
			DWORD bytesRead;
			bool readStat = ReadFile(serialHandle, inBuf, bufSize, &bytesRead, NULL);
			if(!readStat)
			{
				CloseHandle(serialHandle);
				continue; //serial read failed
			}
			
			//process inBuf code
		}
	}
	else
	{
		DriverLog("CEnumerateSerial::UsingWMI failed, Error:%08X\n", static_cast<unsigned int>(hr));
		return;
	}
	
	isTracking = true;
	while(isTracking)
	{
		
	}
}

void ServerDriver_AzureTracker::startSerial(unsigned int portNum)
{
	string portString = "\\\\.\\COM" + to_string(portNum);
	LPTSTR portName = new TCHAR[portString.size() +1];
	strcpy(portName, portString.c_str());
	serialHandle = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	
	DCB serialParams = { 0 };
	serialParams.DCBlength = sizeof(serialParams);
	
	GetCommState(serialHandle, &serialParams);
	serialParams.BaudRate = 115200;
	serialParams.ByteSize = 8;
	serialParams.StopBits = ONESTOPBIT;
	serialParams.Parity = NOPARITY;
	SetCommState(serialHandle, &serialParams);
	
	COMMTIMEOUTS timeout = {0};
	timeout.ReadIntervalTimeout = 50;
	timeout.ReadTotalTimeoutConstant = 50;
	timeout.WriteTotalTimeoutConstant = 50;
	timeout.WriteTotalTimeoutMultiplier = 10;
	
	SetCommTimeouts(serialHandle, &timeout);
	
	PurgeComm(serialHandle, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
}


ServerDriver_AzureTracker g_serverDriverNull;




















void *HmdDriverFactory (const char *pInterfaceName, int *pReturnCode)
{	
	if (strcmp(pInterfaceName, IServerTrackedDeviceProvider_Version) == 0)
	{
		return &g_serverDriverNull;
	}
	
	if (pReturnCode)
		*pReturnCode = VRInitError_Init_InterfaceNotFound;
	
	return nullptr;
}





















