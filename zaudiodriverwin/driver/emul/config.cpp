/*
   This file is part of the EMU CA0189 USB Audio Driver.

   Copyright (C) 2008 EMU Systems/Creative Technology Ltd. 

   This driver is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This driver is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library.   If not, a copy of the GNU Lesser General Public 
   License can be found at <http://www.gnu.org/licenses/>.
*/
/*
 *****************************************************************************
 *//*!
 * @file	   config.cpp
 * @brief	   USB configuration implementation.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "config.h"
#include "device.h"

#define STR_MODULENAME "config: "


#pragma code_seg("PAGE")

/*****************************************************************************
 * CUsbConfiguration::~CUsbConfiguration()
 *****************************************************************************
 */
CUsbConfiguration::
~CUsbConfiguration
(	void
)
{
	PAGED_CODE();

	if (m_ConfigurationDescriptor)
	{
		ExFreePool(m_ConfigurationDescriptor);
	}
}

/*****************************************************************************
 * CUsbConfiguration::Init()
 *****************************************************************************
 */
NTSTATUS
CUsbConfiguration::
Init
(
	IN		CUsbDevice *	UsbDevice,
	IN		UCHAR			ConfigurationIndex
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;

	m_UsbDevice = UsbDevice;

	m_ConfigurationIndex = ConfigurationIndex;

	m_ConfigurationDescriptor = m_UsbDevice->GetConfigurationDescriptor(ConfigurationIndex);

	if (m_ConfigurationDescriptor)
	{
		for (UCHAR InterfaceNumber=0; InterfaceNumber<m_ConfigurationDescriptor->bNumInterfaces; InterfaceNumber++)
		{
			CUsbInterface * UsbInterface = new(NonPagedPool) CUsbInterface();

			if (UsbInterface)
			{
				ntStatus = UsbInterface->Init(this, m_UsbDevice, InterfaceNumber);

				if (NT_SUCCESS(ntStatus))
				{
					m_InterfaceList.Put(UsbInterface);
				}
				else
				{
					delete UsbInterface;
					break;
				}
			}
		}
	}
	else
	{
		ntStatus = STATUS_INVALID_PARAMETER;
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbConfiguration::GetAudioControlInterface()
 *****************************************************************************
 */
NTSTATUS 
CUsbConfiguration::
GetAudioControlInterface
(
	OUT		CAudioControlInterface **	OutAudioControlInterface
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_NOT_FOUND;

	for (CUsbInterface * Interface = m_InterfaceList.First(); Interface; Interface = m_InterfaceList.Next(Interface))
	{
		if (Interface->InterfaceSubClass() == USB_AUDIO_10_SUBCLASS_AUDIOCONTROL)
		{
			*OutAudioControlInterface = (CAudioControlInterface*)Interface->GetAlternateSetting(0);

			ntStatus = STATUS_SUCCESS;
			break;
		}
	}

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CUsbConfiguration::GetInterface()
 *****************************************************************************
 */
NTSTATUS 
CUsbConfiguration::
GetInterface
(
	IN		UCHAR				InterfaceNumber,
	OUT		CUsbInterface **	OutUsbInterface
)
{
	NTSTATUS ntStatus = STATUS_NOT_FOUND;

	for (CUsbInterface * Interface = m_InterfaceList.First(); Interface; Interface = m_InterfaceList.Next(Interface))
	{
		if (Interface->InterfaceNumber() == InterfaceNumber)
		{
			*OutUsbInterface = Interface;

			ntStatus = STATUS_SUCCESS;
			break;
		}
	}

	return ntStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CUsbConfiguration::GetOtherUsbAudioDescriptorSize()
 *****************************************************************************
 */
ULONG 
CUsbConfiguration::
GetOtherUsbAudioDescriptorSize
(	void
)
{
	PAGED_CODE();

	// The configuration descriptor.
	ULONG TotalLength = sizeof(USB_CONFIGURATION_DESCRIPTOR);

	// The other speed configuration descriptor.
	// Device is not capable of other speed.

	// The interface association descriptor.
	TotalLength += sizeof(USB_INTERFACE_ASSOCIATION_DESCRIPTOR);

	// The other interfaces.
	for (CUsbInterface * Interface = m_InterfaceList.First(); Interface; Interface = m_InterfaceList.Next(Interface))
	{
		TotalLength += Interface->GetOtherUsbAudioDescriptorSize();
	}

	return TotalLength;
}

/*****************************************************************************
 * CUsbConfiguration::GetOtherUsbAudioDescriptor()
 *****************************************************************************
 */
ULONG 
CUsbConfiguration::
GetOtherUsbAudioDescriptor
(
	IN		PUCHAR	Buffer,
	IN		ULONG	BufferSize
)
{
	PAGED_CODE();

	ULONG TotalLength = 0;

	// The configuration descriptor.
	PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor = PUSB_CONFIGURATION_DESCRIPTOR(Buffer);

	ConfigurationDescriptor->bLength = sizeof(USB_CONFIGURATION_DESCRIPTOR);
	ConfigurationDescriptor->bDescriptorType = USB_CONFIGURATION_DESCRIPTOR_TYPE;
	//ConfigurationDescriptor->wTotalLength = 0;
	ConfigurationDescriptor->bNumInterfaces = (UCHAR)m_InterfaceList.Count();
	ConfigurationDescriptor->bConfigurationValue = m_ConfigurationDescriptor->bConfigurationValue;
	ConfigurationDescriptor->iConfiguration = m_ConfigurationDescriptor->iConfiguration;
	ConfigurationDescriptor->bmAttributes = m_ConfigurationDescriptor->bmAttributes;
	ConfigurationDescriptor->MaxPower = m_ConfigurationDescriptor->MaxPower;

	Buffer += sizeof(USB_CONFIGURATION_DESCRIPTOR);
	TotalLength += sizeof(USB_CONFIGURATION_DESCRIPTOR);

	if (BufferSize >= GetOtherUsbAudioDescriptorSize())
	{
		// The other speed configuration descriptor.
		// Device is not capable of other speed.

		// The interface association descriptor.
		PUSB_INTERFACE_ASSOCIATION_DESCRIPTOR InterfaceAssociationDescriptor = PUSB_INTERFACE_ASSOCIATION_DESCRIPTOR(Buffer);

		InterfaceAssociationDescriptor->bLength = sizeof(USB_INTERFACE_ASSOCIATION_DESCRIPTOR);
		InterfaceAssociationDescriptor->bDescriptorType = USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE;
		InterfaceAssociationDescriptor->bFirstInterface = 0; // FIXME: assume it starts at 0 for now.
		InterfaceAssociationDescriptor->bInterfaceCount = (UCHAR)m_InterfaceList.Count();
		InterfaceAssociationDescriptor->bFunctionClass = USB_CLASS_CODE_AUDIO;
		InterfaceAssociationDescriptor->bFunctionSubClass = USB_AUDIO_20_SUBCLASS_UNDEFINED;
		InterfaceAssociationDescriptor->bFunctionProtocol = USB_AUDIO_20_PROTOCOL_VERSION_02_00;
		InterfaceAssociationDescriptor->iFunction = 0; // no name.

		Buffer += sizeof(USB_INTERFACE_ASSOCIATION_DESCRIPTOR);
		TotalLength += sizeof(USB_INTERFACE_ASSOCIATION_DESCRIPTOR);

		// The other interfaces.
		for (CUsbInterface * Interface = m_InterfaceList.First(); Interface; Interface = m_InterfaceList.Next(Interface))
		{
			ULONG DescriptorSize = Interface->GetOtherUsbAudioDescriptor(Buffer);

			Buffer += DescriptorSize;
			TotalLength += DescriptorSize;
		}

		// Update the total length in the configuration descriptor.
		ConfigurationDescriptor->wTotalLength = (USHORT)TotalLength;
	}
	else
	{
		// Update the total length in the configuration descriptor.
		ConfigurationDescriptor->wTotalLength = (USHORT)GetOtherUsbAudioDescriptorSize();
	}

	return TotalLength;
}

/*****************************************************************************
 * ParseConfigurationDescriptorEx()
 *****************************************************************************
 */
PUSB_INTERFACE_DESCRIPTOR
ParseConfigurationDescriptorEx
(
	IN		PUSB_CONFIGURATION_DESCRIPTOR	ConfigurationDescriptor,
	IN		PUSB_CONFIGURATION_DESCRIPTOR	StartPosition,
    IN		LONG							InterfaceNumber,
    IN		LONG							AlternateSetting,
	IN		LONG							InterfaceClass,
	IN		LONG							InterfaceSubClass,
	IN		LONG							InterfaceProtocol
)
{
	PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

	if (ConfigurationDescriptor)
	{
		PUCHAR DescriptorEnd = PUCHAR(ConfigurationDescriptor) + ConfigurationDescriptor->wTotalLength;

		PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)StartPosition;

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
			   ((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			switch (CommonDescriptor->bDescriptorType)
			{
				case USB_INTERFACE_DESCRIPTOR_TYPE:
					if ((CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)) &&
						(CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)+2))
					{
						InterfaceDescriptor = NULL;
					}
					else
					{
						InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)CommonDescriptor;

						if (((InterfaceNumber == -1) || (InterfaceDescriptor->bInterfaceNumber == InterfaceNumber)) &&
							((AlternateSetting == -1) || (InterfaceDescriptor->bAlternateSetting == AlternateSetting)) &&
							((InterfaceClass == -1) || (InterfaceDescriptor->bInterfaceClass == InterfaceClass)) &&
							((InterfaceSubClass == -1) || (InterfaceDescriptor->bInterfaceSubClass == InterfaceSubClass)) &&
							((InterfaceProtocol == -1) || (InterfaceDescriptor->bInterfaceProtocol == InterfaceProtocol)))
						{
							return InterfaceDescriptor;
						}
						else
						{
							InterfaceDescriptor = NULL;
						}
					}
					break;

			} // switch

			CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)CommonDescriptor + CommonDescriptor->bLength);
		} // while
	}

    return InterfaceDescriptor;
}

/*****************************************************************************
 * CUsbConfiguration::GetConfigurationDescriptor()
 *****************************************************************************
 */
NTSTATUS
CUsbConfiguration::
GetConfigurationDescriptor
(	
    OUT		PUSB_CONFIGURATION_DESCRIPTOR *		OutConfigurationDescriptor
)
{
	ASSERT(OutConfigurationDescriptor);

    *OutConfigurationDescriptor = m_ConfigurationDescriptor;

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CUsbConfiguration::GetInterfaceDescriptor()
 *****************************************************************************
 */
NTSTATUS
CUsbConfiguration::
GetInterfaceDescriptor
(
    IN		LONG							InterfaceNumber,
    IN		LONG							AlternateSetting,
	IN		LONG							InterfaceClass,
	IN		LONG							InterfaceSubClass,
	IN		LONG							InterfaceProtocol,
    OUT		PUSB_INTERFACE_DESCRIPTOR *		OutInterfaceDescriptor
)
{
	ASSERT(OutInterfaceDescriptor);

	NTSTATUS ntStatus = STATUS_SUCCESS;

    if (m_ConfigurationDescriptor)
	{
		if ((InterfaceNumber != -1) && (m_ConfigurationDescriptor->bNumInterfaces < InterfaceNumber))
		{
			ntStatus = STATUS_INVALID_PARAMETER;
		}

		if (NT_SUCCESS(ntStatus))
		{
			// parse the config descriptor for the interface and
			// alternate setting we want
			PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = ParseConfigurationDescriptorEx
															(
																m_ConfigurationDescriptor,
																m_ConfigurationDescriptor,
																InterfaceNumber,
																AlternateSetting,
																InterfaceClass,
																InterfaceSubClass,
																InterfaceProtocol
															);

			if (InterfaceDescriptor)
			{
				*OutInterfaceDescriptor = InterfaceDescriptor;
			}
			else
			{
				*OutInterfaceDescriptor = NULL;

				ntStatus = STATUS_UNSUCCESSFUL;
			}
		}
	}
	else
	{
		ntStatus = STATUS_UNSUCCESSFUL;
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbConfiguration::GetEndpointDescriptor()
 *****************************************************************************
 */
NTSTATUS
CUsbConfiguration::
GetEndpointDescriptor
(
    IN		UCHAR							InterfaceNumber,
    IN		UCHAR							AlternateSetting,
    IN		UCHAR							EndpointAddress,
    OUT		PUSB_ENDPOINT_DESCRIPTOR *		OutEndpointDescriptor
)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

	if (m_ConfigurationDescriptor)
	{
		UCHAR CurrentPipeIndex;
		BOOL CorrectInterface = FALSE;
		PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

		PUCHAR DescriptorEnd = PUCHAR(m_ConfigurationDescriptor) + m_ConfigurationDescriptor->wTotalLength;

		PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)m_ConfigurationDescriptor;

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
			((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			switch (CommonDescriptor->bDescriptorType)
			{
				case USB_CONFIGURATION_DESCRIPTOR_TYPE:
					if (CommonDescriptor->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;

				case USB_INTERFACE_DESCRIPTOR_TYPE:
					if ((CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)) &&
						(CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)+2))
					{
						return STATUS_UNSUCCESSFUL;
					}
					else
					{
						InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)CommonDescriptor;

						if ((InterfaceDescriptor->bInterfaceNumber == InterfaceNumber) &&
							(InterfaceDescriptor->bAlternateSetting == AlternateSetting))
						{
							CorrectInterface = TRUE;
							CurrentPipeIndex = 0;
						}
						else
						{
							CorrectInterface = FALSE;
						}
					}
					break;

				case USB_ENDPOINT_DESCRIPTOR_TYPE:
					if (((CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)) &&
						 (CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)+2)) ||
						(InterfaceDescriptor == NULL))
					{
						return STATUS_UNSUCCESSFUL;
					}

					if (CorrectInterface)
					{
						PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR)CommonDescriptor;

						if (EndpointDescriptor->bEndpointAddress == EndpointAddress)
						{
							*OutEndpointDescriptor = EndpointDescriptor;

							return STATUS_SUCCESS;
						}
						else
						{
							CurrentPipeIndex++;
						}
					}
					break;

				default:

					if (InterfaceDescriptor == NULL)
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;
			} // switch

			CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)CommonDescriptor + CommonDescriptor->bLength);
		} // while
	}

    return STATUS_UNSUCCESSFUL;
}

/*****************************************************************************
 * CUsbConfiguration::GetEndpointDescriptorByIndex()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbConfiguration::
GetEndpointDescriptorByIndex
(
    IN		UCHAR							InterfaceNumber,
    IN		UCHAR							AlternateSetting,
    IN		UCHAR							PipeIndex,
    OUT		PUSB_ENDPOINT_DESCRIPTOR *		OutEndpointDescriptor
)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

	if (m_ConfigurationDescriptor)
	{
		UCHAR CurrentPipeIndex;
		BOOL CorrectInterface = FALSE;
		PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

		PUCHAR DescriptorEnd = PUCHAR(m_ConfigurationDescriptor) + m_ConfigurationDescriptor->wTotalLength;

		PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)m_ConfigurationDescriptor;

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
			((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			switch (CommonDescriptor->bDescriptorType)
			{
				case USB_CONFIGURATION_DESCRIPTOR_TYPE:
					if (CommonDescriptor->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;

				case USB_INTERFACE_DESCRIPTOR_TYPE:
					if ((CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)) &&
						(CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)+2))
					{
						return STATUS_UNSUCCESSFUL;
					}
					else
					{
						InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)CommonDescriptor;

						if ((InterfaceDescriptor->bInterfaceNumber == InterfaceNumber) &&
							(InterfaceDescriptor->bAlternateSetting == AlternateSetting))
						{
							if (InterfaceDescriptor->bNumEndpoints <= PipeIndex)
							{
								return STATUS_UNSUCCESSFUL;
							}

							CorrectInterface = TRUE;
							CurrentPipeIndex = 0;
						}
						else
						{
							CorrectInterface = FALSE;
						}
					}
					break;

				case USB_ENDPOINT_DESCRIPTOR_TYPE:
					if (((CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)) &&
						 (CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)+2)) ||
						(InterfaceDescriptor == NULL))
					{
						return STATUS_UNSUCCESSFUL;
					}

					if (CorrectInterface)
					{
						PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR)CommonDescriptor;

						if (CurrentPipeIndex == PipeIndex)
						{
							*OutEndpointDescriptor = EndpointDescriptor;

							return STATUS_SUCCESS;
						}
						else
						{
							CurrentPipeIndex++;
						}
					}
					break;

				default:

					if (InterfaceDescriptor == NULL)
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;
			} // switch

			CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)CommonDescriptor + CommonDescriptor->bLength);
		} // while
	}

    return STATUS_UNSUCCESSFUL;

}

/*****************************************************************************
 * CUsbConfiguration::GetClassInterfaceDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbConfiguration::
GetClassInterfaceDescriptor
(
    IN		UCHAR							InterfaceNumber,
    IN		UCHAR							AlternateSetting,
	IN		UCHAR							ClassSpecificDescriptorType,
    OUT		PUSB_INTERFACE_DESCRIPTOR *		OutInterfaceDescriptor
)
{
	ASSERT(OutInterfaceDescriptor);

	NTSTATUS ntStatus = STATUS_SUCCESS;

    if (m_ConfigurationDescriptor)
	{
		if (m_ConfigurationDescriptor->bNumInterfaces < InterfaceNumber)
		{
			ntStatus = STATUS_INVALID_PARAMETER;
		}

		if (NT_SUCCESS(ntStatus))
		{
			// parse the config descriptor for the interface and
			// alternate setting we want
			PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = ParseConfigurationDescriptorEx
															(
																m_ConfigurationDescriptor,
																m_ConfigurationDescriptor,
																InterfaceNumber,
																AlternateSetting,
																-1,
																-1,
																-1
															);

			if (InterfaceDescriptor)
			{
				PUCHAR DescriptorEnd = PUCHAR(m_ConfigurationDescriptor) + m_ConfigurationDescriptor->wTotalLength;

				PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)(PUCHAR(InterfaceDescriptor) + InterfaceDescriptor->bLength);

				if (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  				    ((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
				{
					if (CommonDescriptor->bDescriptorType == ClassSpecificDescriptorType)
					{
						*OutInterfaceDescriptor = PUSB_INTERFACE_DESCRIPTOR(CommonDescriptor);
					}
					else
					{
						ntStatus = STATUS_UNSUCCESSFUL;
					}
				}
				else
				{
					ntStatus = STATUS_UNSUCCESSFUL;
				}
			}
			else
			{
				ntStatus = STATUS_UNSUCCESSFUL;
			}
		}
	}
	else
	{
		ntStatus = STATUS_UNSUCCESSFUL;
	}

	if (!NT_SUCCESS(ntStatus))
	{
		*OutInterfaceDescriptor = NULL;
	}

	return ntStatus;
}

/*****************************************************************************
 * CUsbConfiguration::GetClassEndpointDescriptor()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS
CUsbConfiguration::
GetClassEndpointDescriptor
(
    IN		UCHAR							InterfaceNumber,
    IN		UCHAR							AlternateSetting,
    IN		UCHAR							EndpointAddress,
	IN		UCHAR							ClassSpecificDescriptorType,
    OUT		PUSB_ENDPOINT_DESCRIPTOR *		OutEndpointDescriptor
)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

	if (m_ConfigurationDescriptor)
	{
		BOOL CorrectInterface = FALSE;

		PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

		PUCHAR DescriptorEnd = PUCHAR(m_ConfigurationDescriptor) + m_ConfigurationDescriptor->wTotalLength;

		PUSB_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)m_ConfigurationDescriptor;

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  			   ((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			switch (CommonDescriptor->bDescriptorType)
			{
				case USB_CONFIGURATION_DESCRIPTOR_TYPE:
					if (CommonDescriptor->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;

				case USB_INTERFACE_DESCRIPTOR_TYPE:
					if ((CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)) &&
						(CommonDescriptor->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)+2))
					{
						return STATUS_UNSUCCESSFUL;
					}
					else
					{
						InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)CommonDescriptor;

						if ((InterfaceDescriptor->bInterfaceNumber == InterfaceNumber) &&
							(InterfaceDescriptor->bAlternateSetting == AlternateSetting))
						{
							CorrectInterface = TRUE;
						}
						else
						{
							CorrectInterface = FALSE;
						}
					}
					break;

				case USB_ENDPOINT_DESCRIPTOR_TYPE:
					if (((CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)) &&
						 (CommonDescriptor->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)+2)) ||
						(InterfaceDescriptor == NULL))
					{
						return STATUS_UNSUCCESSFUL;
					}

					if (CorrectInterface)
					{
						PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR)CommonDescriptor;

						if (EndpointDescriptor->bEndpointAddress == EndpointAddress)
						{
							CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)(PUCHAR(EndpointDescriptor) + EndpointDescriptor->bLength);

							if (((PUCHAR(CommonDescriptor) + sizeof(USB_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  								((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
							{
								if (CommonDescriptor->bDescriptorType == ClassSpecificDescriptorType)
								{
									*OutEndpointDescriptor = PUSB_ENDPOINT_DESCRIPTOR(CommonDescriptor);

									ntStatus = STATUS_SUCCESS;
								}
								else
								{
									ntStatus = STATUS_UNSUCCESSFUL;
								}
							}
							else
							{
								ntStatus = STATUS_UNSUCCESSFUL;
							}

							return ntStatus;
						}
					}
					break;

				default:

					if (InterfaceDescriptor == NULL)
					{
						return STATUS_UNSUCCESSFUL;
					}
					break;
			} // switch

			CommonDescriptor = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)CommonDescriptor + CommonDescriptor->bLength);
		} // while
	}

    return STATUS_UNSUCCESSFUL;
}

#pragma code_seg()
