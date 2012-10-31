
#ifndef __ASIODRIVER_FWD_DEFINED__
#define __ASIODRIVER_FWD_DEFINED__

#include "asio.h"

/*****************************************************************************
 * Constants
 *****************************************************************************
 */
DEFINE_GUID(IID_IAsioDriver,
0xf6eb89c1, 0x89bd, 0x11d0, 0xaa, 0x58, 0x08, 0x00, 0x00, 0x17, 0x16, 0x29);

#define ASIOMETHOD_(type,method)	virtual type method
#define ASIOMETHODIMP_(type)		type

/*****************************************************************************
 *//*! @interface IAsioDriver
 *****************************************************************************
 * @brief
 * Interface for ASIO driver object.
 */
DECLARE_INTERFACE_(IAsioDriver, IUnknown)
{
	ASIOMETHOD_(ASIOBool,Init)
	(	THIS_
		IN		PVOID	WndMain
	)	PURE;
	ASIOMETHOD_(VOID,GetDriverName)
	(	THIS_
		OUT		CHAR *	OutDriverName
	)	PURE;	
	ASIOMETHOD_(LONG,GetDriverVersion)
	(	THIS
	)	PURE;
	ASIOMETHOD_(VOID,GetErrorMessage)
	(	THIS_
		OUT		CHAR *	OutString
	)	PURE;	
	ASIOMETHOD_(ASIOError,Start)
	(	THIS
	) PURE;
	ASIOMETHOD_(ASIOError,Stop)
	(	THIS
	)	PURE;
	ASIOMETHOD_(ASIOError,GetChannels)
	(	THIS_
		OUT		LONG *	OutNumInputChannels, 
		OUT		LONG *	OutNumOutputChannels
	)	PURE;
	ASIOMETHOD_(ASIOError,GetLatencies)
	(	THIS_
		OUT		LONG *	OutInputLatency, 
		OUT		LONG *	OutOutputLatency
	)	PURE;
	ASIOMETHOD_(ASIOError,GetBufferSize)
	(	THIS_
		OUT		LONG *	OutMinSize, 
		OUT		LONG *	OutMaxSize,
		OUT		LONG *	OutPreferredSize, 
		OUT		LONG *	OutGranularity
	)	PURE;
	ASIOMETHOD_(ASIOError,CanSampleRate)
	(	THIS_
		IN		ASIOSampleRate	SampleRate
	)	PURE;
	ASIOMETHOD_(ASIOError,GetSampleRate)
	(	THIS_
		OUT		ASIOSampleRate *	OutSampleRate
	)	PURE;
	ASIOMETHOD_(ASIOError,SetSampleRate)
	(	THIS_
		IN		ASIOSampleRate	SampleRate
	)	PURE;
	ASIOMETHOD_(ASIOError,GetClockSources)
	(	THIS_
		OUT		ASIOClockSource *	OutClocks, 
		OUT		LONG *				OutNumSources
	)	PURE;
	ASIOMETHOD_(ASIOError,SetClockSource)
	(	THIS_
		IN		LONG	Reference
	)	PURE;
	ASIOMETHOD_(ASIOError,GetSamplePosition)
	(	THIS_
		OUT		ASIOSamples *	OutSamplePosition, 
		OUT		ASIOTimeStamp *	OutTimeStamp
	)	PURE;
	ASIOMETHOD_(ASIOError,GetChannelInfo)
	(	THIS_
		IN	OUT	ASIOChannelInfo *	Info
	)	PURE;
	ASIOMETHOD_(ASIOError,CreateBuffers)
	(	THIS_
		IN		ASIOBufferInfo *	BufferInfos, 
		IN		LONG				NumChannels,
		IN		LONG				BufferSize, 
		IN		ASIOCallbacks *		Callbacks
	)	PURE;
	ASIOMETHOD_(ASIOError,DisposeBuffers)
	(	THIS
	)	PURE;
	ASIOMETHOD_(ASIOError,ControlPanel)
	(	THIS
	)	PURE;
	ASIOMETHOD_(ASIOError,Future)
	(	THIS_
		IN		LONG	Selector,
		IN		PVOID	Parameter
	)	PURE;
	ASIOMETHOD_(ASIOError,OutputReady)
	(	THIS
	)	PURE;
};

typedef IAsioDriver * PASIODRIVER;

#define IMP_IAsioDriver\
	ASIOMETHODIMP_(ASIOBool) Init\
	(	IN		PVOID	WndMain\
	);\
	ASIOMETHODIMP_(VOID) GetDriverName\
	(	OUT		CHAR *	OutDriverName\
	);\
	ASIOMETHODIMP_(LONG) GetDriverVersion\
	(	void\
	);\
	ASIOMETHODIMP_(VOID) GetErrorMessage\
	(	OUT		CHAR *	OutString\
	);\
	ASIOMETHODIMP_(ASIOError) Start\
	(	void\
	);\
	ASIOMETHODIMP_(ASIOError) Stop\
	(	void\
	);\
	ASIOMETHODIMP_(ASIOError) GetChannels\
	(	OUT		LONG *	OutNumInputChannels,\
		OUT		LONG *	OutNumOutputChannels\
	);\
	ASIOMETHODIMP_(ASIOError) GetLatencies\
	(	OUT		LONG *	OutInputLatency,\
		OUT		LONG *	OutOutputLatency\
	);\
	ASIOMETHODIMP_(ASIOError) GetBufferSize\
	(	OUT		LONG *	OutMinSize,\
		OUT		LONG *	OutMaxSize,\
		OUT		LONG *	OutPreferredSize,\
		OUT		LONG *	OutGranularity\
	);\
	ASIOMETHODIMP_(ASIOError) CanSampleRate\
	(	IN		ASIOSampleRate	SampleRate\
	);\
	ASIOMETHODIMP_(ASIOError) GetSampleRate\
	(	OUT		ASIOSampleRate *	OutSampleRate\
	);\
	ASIOMETHODIMP_(ASIOError) SetSampleRate\
	(	IN		ASIOSampleRate	SampleRate\
	);\
	ASIOMETHODIMP_(ASIOError) GetClockSources\
	(	OUT		ASIOClockSource *	OutClocks,\
		OUT		LONG *				OutNumSources\
	);\
	ASIOMETHODIMP_(ASIOError) SetClockSource\
	(	IN		LONG	Reference\
	);\
	ASIOMETHODIMP_(ASIOError) GetSamplePosition\
	(	OUT		ASIOSamples *	OutSamplePosition,\
		OUT		ASIOTimeStamp *	OutTimeStamp\
	);\
	ASIOMETHODIMP_(ASIOError) GetChannelInfo\
	(	IN	OUT	ASIOChannelInfo *	Info\
	);\
	ASIOMETHODIMP_(ASIOError) CreateBuffers\
	(	IN		ASIOBufferInfo *	BufferInfos,\
		IN		LONG				NumChannels,\
		IN		LONG				BufferSize,\
		IN		ASIOCallbacks *		Callbacks\
	);\
	ASIOMETHODIMP_(ASIOError) DisposeBuffers\
	(	void\
	);\
	ASIOMETHODIMP_(ASIOError) ControlPanel\
	(	void\
	);\
	ASIOMETHODIMP_(ASIOError) Future\
	(	IN		LONG	Selector,\
		IN		PVOID	Parameter\
	);\
	ASIOMETHODIMP_(ASIOError) OutputReady\
	(	void\
	)

#endif 	/* __ASIODRIVER_FWD_DEFINED__ */
