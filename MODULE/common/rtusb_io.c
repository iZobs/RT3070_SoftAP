/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
 
 	Module Name:
	rtusb_io.c

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
	Paul Lin    06-25-2004  created
*/

#ifdef RTMP_MAC_USB


#include	"rt_config.h"

#define MAX_VENDOR_REQ_RETRY_COUNT  10

/*
	========================================================================
	
	Routine Description: NIC initialization complete

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/

static NTSTATUS	RTUSBFirmwareRun(
	IN	PRTMP_ADAPTER	pAd)
{
	NTSTATUS	Status;

	Status = RTUSB_VendorRequest(
		pAd,
		USBD_TRANSFER_DIRECTION_OUT,
		DEVICE_VENDOR_REQUEST_OUT,
		0x01,
		0x8,
		0,
		NULL,
		0);
	
	return Status;
}



/*
	========================================================================
	
	Routine Description: Write Firmware to NIC.

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS RTUSBFirmwareWrite(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR		pFwImage,
	IN ULONG		FwLen)
{
	UINT32		MacReg;
	NTSTATUS 	Status;
//	ULONG 		i;
	USHORT		writeLen;
	
	Status = RTUSBReadMACRegister(pAd, MAC_CSR0, &MacReg);


	writeLen = FwLen;
	RTUSBMultiWrite(pAd, FIRMWARE_IMAGE_BASE, pFwImage, writeLen);
	
	Status = RTUSBWriteMACRegister(pAd, 0x7014, 0xffffffff);
	Status = RTUSBWriteMACRegister(pAd, 0x701c, 0xffffffff);
	Status = RTUSBFirmwareRun(pAd);

	
	return Status;
}


NTSTATUS	RTUSBVenderReset(
	IN	PRTMP_ADAPTER	pAd)
{
	NTSTATUS	Status;
	DBGPRINT_RAW(RT_DEBUG_ERROR, ("-->RTUSBVenderReset\n"));
	Status = RTUSB_VendorRequest(
		pAd,
		USBD_TRANSFER_DIRECTION_OUT,
		DEVICE_VENDOR_REQUEST_OUT,
		0x01,
		0x1,
		0,
		NULL,
		0);

	DBGPRINT_RAW(RT_DEBUG_ERROR, ("<--RTUSBVenderReset\n"));
	return Status;
}
/*
	========================================================================
	
	Routine Description: Read various length data from RT2573

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS	RTUSBMultiRead(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUCHAR			pData,
	IN	USHORT			length)
{
	NTSTATUS	Status;

	Status = RTUSB_VendorRequest(
		pAd,
		(USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK),
		DEVICE_VENDOR_REQUEST_IN,
		0x7,
		0,
		Offset,
		pData,
		length);
	
	return Status;
}

/*
	========================================================================
	
	Routine Description: Write various length data to RT2573

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS	RTUSBMultiWrite_OneByte(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData)
{
	NTSTATUS	Status;

	// TODO: In 2870, use this funciton carefully cause it's not stable.
	Status = RTUSB_VendorRequest(
		pAd,
		USBD_TRANSFER_DIRECTION_OUT,
		DEVICE_VENDOR_REQUEST_OUT,
		0x6,
		0,
		Offset,
		pData,
		1);

	return Status;
}

NTSTATUS	RTUSBMultiWrite(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData,
	IN	USHORT			length)
{
	NTSTATUS	Status;


	USHORT          index = 0,Value;
	PUCHAR          pSrc = pData;
	USHORT          resude = 0;
		
	resude = length % 2;
	length  += resude;
	do
	{
			Value =(USHORT)( *pSrc  | (*(pSrc + 1) << 8));
		Status = RTUSBSingleWrite(pAd,Offset + index,Value);
            index +=2;
            length -= 2;
            pSrc = pSrc + 2;
        }while(length > 0);

	return Status;
}


NTSTATUS RTUSBSingleWrite(
	IN 	RTMP_ADAPTER 	*pAd,
	IN	USHORT			Offset,
	IN	USHORT			Value)
{
	NTSTATUS	Status;

	Status = RTUSB_VendorRequest(
		pAd,
		USBD_TRANSFER_DIRECTION_OUT,
		DEVICE_VENDOR_REQUEST_OUT,
		0x2,
		Value,
		Offset,
		NULL,
		0);
	
	return Status;

}


/*
	========================================================================
	
	Routine Description: Read 32-bit MAC register

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS	RTUSBReadMACRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUINT32			pValue)
{
	NTSTATUS	Status = 0;
	UINT32		localVal;

	Status = RTUSB_VendorRequest(
		pAd,
		(USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK),
		DEVICE_VENDOR_REQUEST_IN,
		0x7,
		0,
		Offset,
		&localVal,
		4);
	
	*pValue = le2cpu32(localVal);


	if (Status < 0)
		*pValue = 0xffffffff;
	
	return Status;
}


/*
	========================================================================
	
	Routine Description: Write 32-bit MAC register

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS	RTUSBWriteMACRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value)
{
	NTSTATUS Status;
	UINT32  localVal;
	INT   loopIdx;

	localVal = Value;

	Status = RTUSBSingleWrite(pAd, Offset, (USHORT)(localVal & 0xffff));
	Status = RTUSBSingleWrite(pAd, Offset + 2, (USHORT)((localVal & 0xffff0000) >> 16));

	return Status;
}



/*
	========================================================================
	
	Routine Description: Read 8-bit BBP register

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS	RTUSBReadBBPRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Id,
	IN	PUCHAR			pValue)
{
	BBP_CSR_CFG_STRUC	BbpCsr;
	UINT			i = 0;
	NTSTATUS		status;
	
	// Verify the busy condition
	do
	{
		status = RTUSBReadMACRegister(pAd, BBP_CSR_CFG, &BbpCsr.word);
		if(status >= 0)
		{
		if (!(BbpCsr.field.Busy == BUSY))
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("RTUSBReadBBPRegister(BBP_CSR_CFG_1):retry count=%d!\n", i));
		i++;
	}while ((i < RETRY_LIMIT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
	
	if ((i == RETRY_LIMIT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		//
		// Read failed then Return Default value.
		//
		*pValue = pAd->BbpWriteLatch[Id];
	
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
		return STATUS_UNSUCCESSFUL;
	}

	// Prepare for write material
	BbpCsr.word 				= 0;
	BbpCsr.field.fRead			= 1;
	BbpCsr.field.Busy			= 1;
	BbpCsr.field.RegNum 		= Id;
	RTUSBWriteMACRegister(pAd, BBP_CSR_CFG, BbpCsr.word);

	i = 0;	
	// Verify the busy condition
	do
	{
		status = RTUSBReadMACRegister(pAd, BBP_CSR_CFG, &BbpCsr.word);
		if (status >= 0)
		{
		if (!(BbpCsr.field.Busy == BUSY))
		{
			*pValue = (UCHAR)BbpCsr.field.Value;
			break;
		}
		}
		DBGPRINT(RT_DEBUG_TRACE, ("RTUSBReadBBPRegister(BBP_CSR_CFG_2):retry count=%d!\n", i));
		i++;
	}while ((i < RETRY_LIMIT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
	
	if ((i == RETRY_LIMIT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		//
		// Read failed then Return Default value.
		//
		*pValue = pAd->BbpWriteLatch[Id];

		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
		return STATUS_UNSUCCESSFUL;
	}
	
	return STATUS_SUCCESS;
}


/*
	========================================================================
	
	Routine Description: Write 8-bit BBP register

	Arguments:

	Return Value:
	
	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS	RTUSBWriteBBPRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Id,
	IN	UCHAR			Value)
{
	BBP_CSR_CFG_STRUC	BbpCsr;
	UINT			i = 0;
	NTSTATUS		status;
	// Verify the busy condition
	do
	{
		status = RTUSBReadMACRegister(pAd, BBP_CSR_CFG, &BbpCsr.word);
		if (status >= 0)
		{
		if (!(BbpCsr.field.Busy == BUSY))
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("RTUSBWriteBBPRegister(BBP_CSR_CFG):retry count=%d!\n", i));
		i++;
	}
	while ((i < RETRY_LIMIT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
	
	if ((i == RETRY_LIMIT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
		return STATUS_UNSUCCESSFUL;
	}

	// Prepare for write material
	BbpCsr.word 				= 0;
	BbpCsr.field.fRead			= 0;
	BbpCsr.field.Value			= Value;
	BbpCsr.field.Busy			= 1;
	BbpCsr.field.RegNum 		= Id;
	RTUSBWriteMACRegister(pAd, BBP_CSR_CFG, BbpCsr.word);
	
	pAd->BbpWriteLatch[Id] = Value;

	return STATUS_SUCCESS;
}
/*
	========================================================================
	
	Routine Description: Write RF register through MAC

	Arguments:

	Return Value:
	
	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS	RTUSBWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT32			Value)
{
	PHY_CSR4_STRUC	PhyCsr4;
	UINT			i = 0;
	NTSTATUS		status;

	NdisZeroMemory(&PhyCsr4, sizeof(PHY_CSR4_STRUC));
	do
	{
		status = RTUSBReadMACRegister(pAd, RF_CSR_CFG0, &PhyCsr4.word);
		if (status >= 0)
		{
		if (!(PhyCsr4.field.Busy))
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("RTUSBWriteRFRegister(RF_CSR_CFG0):retry count=%d!\n", i));
		i++;
	}
	while ((i < RETRY_LIMIT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));

	if ((i == RETRY_LIMIT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
		return STATUS_UNSUCCESSFUL;
	}

	RTUSBWriteMACRegister(pAd, RF_CSR_CFG0, Value);
	
	return STATUS_SUCCESS;
}


/*
	========================================================================
	
	Routine Description:

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS RTUSBReadEEPROM(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUCHAR			pData,
	IN	USHORT			length)
{
	NTSTATUS	Status = STATUS_SUCCESS;

		Status = RTUSB_VendorRequest(
			pAd,
			(USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK),
			DEVICE_VENDOR_REQUEST_IN,
			0x9,
			0,
			Offset,
			pData,
			length);

	return Status;
}

/*
	========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS RTUSBWriteEEPROM(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData,
	IN	USHORT			length)
{
	NTSTATUS	Status = STATUS_SUCCESS;

		Status = RTUSB_VendorRequest(
			pAd,
			USBD_TRANSFER_DIRECTION_OUT,
			DEVICE_VENDOR_REQUEST_OUT,
			0x8,
			0,
			Offset,
			pData,
			length);
	
	return Status;
}


NTSTATUS RTUSBReadEEPROM16(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			offset,
	OUT	PUSHORT			pData)
{
	NTSTATUS status;
	USHORT  localData;
	
	status = RTUSBReadEEPROM(pAd, offset, (PUCHAR)(&localData), 2);
	if (status == STATUS_SUCCESS)
		*pData = le2cpu16(localData);
	
	return status;

}

NTSTATUS RTUSBWriteEEPROM16(
	IN RTMP_ADAPTER *pAd, 
	IN USHORT offset, 
	IN USHORT value)
{
	USHORT tmpVal;
	
	tmpVal = cpu2le16(value);
	return RTUSBWriteEEPROM(pAd, offset, (PUCHAR)&(tmpVal), 2);
}
	
/*
	========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	IRQL = 
	
	Note:
	
	========================================================================
*/
VOID RTUSBPutToSleep(
	IN	PRTMP_ADAPTER	pAd)
{
	UINT32		value;
	
	// Timeout 0x40 x 50us
	value = (SLEEPCID<<16)+(OWNERMCU<<24)+ (0x40<<8)+1;
	RTUSBWriteMACRegister(pAd, 0x7010, value);
	RTUSBWriteMACRegister(pAd, 0x404, 0x30);
	//RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);			
	DBGPRINT_RAW(RT_DEBUG_ERROR, ("Sleep Mailbox testvalue %x\n", value));
	
}

/*
	========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	IRQL = 
	
	Note:
	
	========================================================================
*/
NTSTATUS RTUSBWakeUp(
	IN	PRTMP_ADAPTER	pAd)
{
	NTSTATUS	Status;
	
	Status = RTUSB_VendorRequest(
		pAd,
		USBD_TRANSFER_DIRECTION_OUT,
		DEVICE_VENDOR_REQUEST_OUT,
		0x01,
		0x09,
		0,
		NULL,
		0);
	
	return Status;
}

/*
	========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	IRQL = 
	
	Note:
	
	========================================================================
*/
NDIS_STATUS	RTUSBEnqueueCmdFromNdis(
	IN	PRTMP_ADAPTER	pAd,
	IN	NDIS_OID		Oid,
	IN	BOOLEAN			SetInformation,
	IN	PVOID			pInformationBuffer,
	IN	UINT32			InformationBufferLength)
{
	NDIS_STATUS	status;
	PCmdQElmt	cmdqelmt = NULL;
	RTMP_OS_TASK	*pTask = &pAd->cmdQTask;
	
#ifdef KTHREAD_SUPPORT
	if (pTask->kthread_task == NULL)
#else
	CHECK_PID_LEGALITY(pTask->taskPID) 
		;
	else
#endif
		return (NDIS_STATUS_RESOURCES);

	status = os_alloc_mem(pAd, (PUCHAR *)(&cmdqelmt), sizeof(CmdQElmt));
	if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt == NULL))
		return (NDIS_STATUS_RESOURCES);

		cmdqelmt->buffer = NULL;
		if (pInformationBuffer != NULL)
		{	
			status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt->buffer, InformationBufferLength);
			if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt->buffer == NULL))
			{       
				kfree(cmdqelmt);
				return (NDIS_STATUS_RESOURCES);
			}
			else
			{
				NdisMoveMemory(cmdqelmt->buffer, pInformationBuffer, InformationBufferLength);
				cmdqelmt->bufferlength = InformationBufferLength;
			}
		}
		else
			cmdqelmt->bufferlength = 0;
	
	cmdqelmt->command = Oid;
	cmdqelmt->CmdFromNdis = TRUE;
	if (SetInformation == TRUE)
		cmdqelmt->SetOperation = TRUE;
	else
		cmdqelmt->SetOperation = FALSE;

	NdisAcquireSpinLock(&pAd->CmdQLock);
	if (pAd->CmdQ.CmdQState & RTMP_TASK_CAN_DO_INSERT)
	{
		EnqueueCmd((&pAd->CmdQ), cmdqelmt);
		status = NDIS_STATUS_SUCCESS;
	}
	else
	{
		status = NDIS_STATUS_FAILURE;
	}
	NdisReleaseSpinLock(&pAd->CmdQLock);
	
	if (status == NDIS_STATUS_FAILURE)
	{
		if (cmdqelmt->buffer)
			os_free_mem(pAd, cmdqelmt->buffer);
		os_free_mem(pAd, cmdqelmt);
	}
	else
	RTCMDUp(pAd);


    return(NDIS_STATUS_SUCCESS);
}


/*
    ========================================================================
 	Routine Description:
		RTUSB_VendorRequest - Builds a ralink specific request, sends it off to USB endpoint zero and waits for completion

	Arguments:
		@pAd:
	  	@TransferFlags:
	  	@RequestType: USB message request type value
	  	@Request: USB message request value
	  	@Value: USB message value
	  	@Index: USB message index value
	  	@TransferBuffer: USB data to be sent
	  	@TransferBufferLength: Lengths in bytes of the data to be sent

	Context: ! in atomic context

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE
	
	Note:
		This function sends a simple control message to endpoint zero
		and waits for the message to complete, or CONTROL_TIMEOUT_JIFFIES timeout.
		Because it is synchronous transfer, so don't use this function within an atomic context, 
		otherwise system will hang, do be careful.

		TransferBuffer may located in stack region which may not in DMA'able region in some embedded platforms, 
		so need to copy TransferBuffer to UsbVendorReqBuf allocated by kmalloc to do DMA transfer.
		Use UsbVendorReq_semaphore to protect this region which may be accessed by multi task.
		Normally, coherent issue is resloved by low-level HC driver, so do not flush this zone by RTUSB_VendorRequest.
	
	========================================================================
*/
NTSTATUS    RTUSB_VendorRequest(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT32			TransferFlags,
	IN	UCHAR			RequestType,
	IN	UCHAR			Request,
	IN	USHORT			Value,
	IN	USHORT			Index,
	IN	PVOID			TransferBuffer,
	IN	UINT32			TransferBufferLength)
{
	int				RET = 0;
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	if(in_interrupt())
	{
		DBGPRINT(RT_DEBUG_ERROR, ("BUG: RTUSB_VendorRequest is called from invalid context\n"));
		return NDIS_STATUS_FAILURE;
	}

	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("WIFI device has been disconnected\n"));
		return NDIS_STATUS_FAILURE;
	}
	else
	{

		int RetryCount = 0; // RTUSB_CONTROL_MSG retry counts
		ASSERT(TransferBufferLength <MAX_PARAM_BUFFER_SIZE);
		
		RTMP_SEM_EVENT_WAIT(&(pAd->UsbVendorReq_semaphore), RET);
		   
		if ((TransferBufferLength > 0) && (RequestType == DEVICE_VENDOR_REQUEST_OUT))
				NdisMoveMemory(pAd->UsbVendorReqBuf, TransferBuffer, TransferBufferLength);

		do {
			RTUSB_CONTROL_MSG(pObj->pUsb_Dev, 0, Request, RequestType, Value, Index, pAd->UsbVendorReqBuf, TransferBufferLength, CONTROL_TIMEOUT_JIFFIES, RET);
			
			if (RET < 0) {
				DBGPRINT(RT_DEBUG_OFF, ("#\n"));
				RetryCount++;
				RTMPusecDelay(5000); // wait for 5ms
			}
		} while((RET < 0) && (RetryCount < MAX_VENDOR_REQ_RETRY_COUNT));
		
	  	if ( (!(RET < 0)) && (TransferBufferLength > 0) && (RequestType == DEVICE_VENDOR_REQUEST_IN))
			NdisMoveMemory(TransferBuffer, pAd->UsbVendorReqBuf, TransferBufferLength);
		
	  	RTMP_SEM_EVENT_UP(&(pAd->UsbVendorReq_semaphore));

        	if (RET < 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("RTUSB_VendorRequest failed(%d),TxFlags=0x%x, ReqType=%s, Req=0x%x, Index=0x%x\n",
						RET, TransferFlags, (RequestType == DEVICE_VENDOR_REQUEST_OUT ? "OUT" : "IN"), Request, Index));
			if (Request == 0x2)
				DBGPRINT(RT_DEBUG_ERROR, ("\tRequest Value=0x%04x!\n", Value));
			
			if ((!TransferBuffer) && (TransferBufferLength > 0))
				hex_dump("Failed TransferBuffer value", TransferBuffer, TransferBufferLength);

			if (RET == -ENODEV)
					RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			
		}

	}

	if (RET < 0)
		return NDIS_STATUS_FAILURE;
	else
		return NDIS_STATUS_SUCCESS;
	
}


/*
	========================================================================
	
	Routine Description:
	  Creates an IRP to submite an IOCTL_INTERNAL_USB_RESET_PORT
	  synchronously. Callers of this function must be running at
	  PASSIVE LEVEL.

	Arguments:

	Return Value:

	Note:
	
	========================================================================
*/
NTSTATUS RTUSB_ResetDevice(
	IN	PRTMP_ADAPTER	pAd)
{
	NTSTATUS		Status = TRUE;

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->USB_ResetDevice\n"));
	//RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);
	return Status;
}


NTSTATUS CheckGPIOHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{

#ifdef RALINK_ATE			
       if (ATE_ON(pAd))
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("The driver is in ATE mode now\n"));
		return NDIS_STATUS_SUCCESS;
	}	
#endif // RALINK_ATE //


	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS ResetBulkOutHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{

	INT32 MACValue = 0;
	UCHAR Index = 0;
	int ret=0;
	PHT_TX_CONTEXT	pHTTXContext;
//	RTMP_TX_RING *pTxRing;
	unsigned long IrqFlags;
						
	DBGPRINT(RT_DEBUG_TRACE, ("CMDTHREAD_RESET_BULK_OUT(ResetPipeid=0x%0x)===>\n", pAd->bulkResetPipeid));
	
	// All transfers must be aborted or cancelled before attempting to reset the pipe.						
	//RTUSBCancelPendingBulkOutIRP(pAd);
	// Wait 10ms to let previous packet that are already in HW FIFO to clear. by MAXLEE 12-25-2007
	do 
	{
		RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &MACValue);
		if ((MACValue & 0xf00000/*0x800000*/) == 0)
			break;
		
		Index++;
		RTMPusecDelay(10000);
	}while(Index < 100);
	
	RTUSBReadMACRegister(pAd, USB_DMA_CFG, &MACValue);
	
	// 2nd, to prevent Read Register error, we check the validity.
	if ((MACValue & 0xc00000) == 0)
		RTUSBReadMACRegister(pAd, USB_DMA_CFG, &MACValue);
	
	// 3rd, to prevent Read Register error, we check the validity.
	if ((MACValue & 0xc00000) == 0)
		RTUSBReadMACRegister(pAd, USB_DMA_CFG, &MACValue);
	
	MACValue |= 0x80000;
	RTUSBWriteMACRegister(pAd, USB_DMA_CFG, MACValue);

	// Wait 1ms to prevent next URB to bulkout before HW reset. by MAXLEE 12-25-2007
	RTMPusecDelay(1000);

	MACValue &= (~0x80000);
	RTUSBWriteMACRegister(pAd, USB_DMA_CFG, MACValue);
	DBGPRINT(RT_DEBUG_TRACE, ("\tSet 0x2a0 bit19. Clear USB DMA TX path\n"));
						
	// Wait 5ms to prevent next URB to bulkout before HW reset. by MAXLEE 12-25-2007
	//RTMPusecDelay(5000);

	if ((pAd->bulkResetPipeid & BULKOUT_MGMT_RESET_FLAG) == BULKOUT_MGMT_RESET_FLAG)
	{
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
		
		if (pAd->MgmtRing.TxSwFreeIdx < MGMT_RING_SIZE /* pMLMEContext->bWaitingBulkOut == TRUE */)
			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);
		
		RTUSBKickBulkOut(pAd);	
		DBGPRINT(RT_DEBUG_TRACE, ("\tTX MGMT RECOVER Done!\n"));
	}
	else
	{
		pHTTXContext = &(pAd->TxContext[pAd->bulkResetPipeid]);
		
		//NdisAcquireSpinLock(&pAd->BulkOutLock[pAd->bulkResetPipeid]);
		RTMP_INT_LOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
		if ( pAd->BulkOutPending[pAd->bulkResetPipeid] == FALSE)
		{
			pAd->BulkOutPending[pAd->bulkResetPipeid] = TRUE;
			pHTTXContext->IRPPending = TRUE;
			pAd->watchDogTxPendingCnt[pAd->bulkResetPipeid] = 1;
								
			// no matter what, clean the flag
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
								
			//NdisReleaseSpinLock(&pAd->BulkOutLock[pAd->bulkResetPipeid]);
			RTMP_INT_UNLOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
			
#ifdef RALINK_ATE
			if (ATE_ON(pAd))				
				ret = ATEResetBulkOut(pAd);
			else
#endif // RALINK_ATE //
			{
				RTUSBInitHTTxDesc(pAd, pHTTXContext, pAd->bulkResetPipeid, 
													pHTTXContext->BulkOutSize, 
													(usb_complete_t)RTUSBBulkOutDataPacketComplete);
				
				if ((ret = RTUSB_SUBMIT_URB(pHTTXContext->pUrb))!=0)
				{
						RTMP_INT_LOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
						pAd->BulkOutPending[pAd->bulkResetPipeid] = FALSE;
						pHTTXContext->IRPPending = FALSE;
						pAd->watchDogTxPendingCnt[pAd->bulkResetPipeid] = 0;
						RTMP_INT_UNLOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

						DBGPRINT(RT_DEBUG_ERROR, ("CMDTHREAD_RESET_BULK_OUT:Submit Tx URB failed %d\n", ret));
				} 
				else
				{
						RTMP_INT_LOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
						
						DBGPRINT(RT_DEBUG_TRACE,("\tCMDTHREAD_RESET_BULK_OUT: TxContext[%d]:CWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d, pending=%d!\n", 
											pAd->bulkResetPipeid, pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, 
											pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad, 
											pAd->BulkOutPending[pAd->bulkResetPipeid]));
						DBGPRINT(RT_DEBUG_TRACE,("\t\tBulkOut Req=0x%lx, Complete=0x%lx, Other=0x%lx\n", 
											pAd->BulkOutReq, pAd->BulkOutComplete, pAd->BulkOutCompleteOther));
						
						RTMP_INT_UNLOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
						
						DBGPRINT(RT_DEBUG_TRACE, ("\tCMDTHREAD_RESET_BULK_OUT: Submit Tx DATA URB for failed BulkReq(0x%lx) Done, status=%d!\n", 
											pAd->bulkResetReq[pAd->bulkResetPipeid], 
											pHTTXContext->pUrb->rtusb_urb_status));
				}
			}
		}
		else
		{
			//NdisReleaseSpinLock(&pAd->BulkOutLock[pAd->bulkResetPipeid]);
			//RTMP_INT_UNLOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
								
			DBGPRINT(RT_DEBUG_ERROR, ("CmdThread : TX DATA RECOVER FAIL for BulkReq(0x%lx) because BulkOutPending[%d] is TRUE!\n", 
								pAd->bulkResetReq[pAd->bulkResetPipeid], pAd->bulkResetPipeid));
			
			if (pAd->bulkResetPipeid == 0)
			{
				UCHAR	pendingContext = 0;
				PHT_TX_CONTEXT pHTTXContext = (PHT_TX_CONTEXT)(&pAd->TxContext[pAd->bulkResetPipeid ]);
				PTX_CONTEXT pMLMEContext = (PTX_CONTEXT)(pAd->MgmtRing.Cell[pAd->MgmtRing.TxDmaIdx].AllocVa);
				PTX_CONTEXT pNULLContext = (PTX_CONTEXT)(&pAd->PsPollContext);
				PTX_CONTEXT pPsPollContext = (PTX_CONTEXT)(&pAd->NullContext);

				if (pHTTXContext->IRPPending)
					pendingContext |= 1;
				else if (pMLMEContext->IRPPending)
					pendingContext |= 2;
				else if (pNULLContext->IRPPending)
					pendingContext |= 4;
				else if (pPsPollContext->IRPPending)
					pendingContext |= 8;
				else
					pendingContext = 0;
									
				DBGPRINT(RT_DEBUG_ERROR, ("\tTX Occupied by %d!\n", pendingContext));
			}

			// no matter what, clean the flag
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);

			RTMP_INT_UNLOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
								
			RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << pAd->bulkResetPipeid));
		}

		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
		//RTUSBKickBulkOut(pAd);
	}
/*

	// Don't cancel BULKIN.	
	while ((atomic_read(&pAd->PendingRx) > 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) 
	{
		if (atomic_read(&pAd->PendingRx) > 0)
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("BulkIn IRP Pending!!cancel it!\n"));
			RTUSBCancelPendingBulkInIRP(pAd);
		}
			RTMPusecDelay(100000);
	}
						
	if ((atomic_read(&pAd->PendingRx) == 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
	{
		UCHAR	i;
		RTUSBRxPacket(pAd);
		pAd->NextRxBulkInReadIndex = 0;	// Next Rx Read index
		pAd->NextRxBulkInIndex		= 0;	// Rx Bulk pointer
		for (i = 0; i < (RX_RING_SIZE); i++)
		{
			PRX_CONTEXT  pRxContext = &(pAd->RxContext[i]);

			pRxContext->pAd	= pAd;
			pRxContext->InUse		= FALSE;
			pRxContext->IRPPending	= FALSE;
			pRxContext->Readable	= FALSE;
			pRxContext->ReorderInUse = FALSE;
							
		}
			RTUSBBulkReceive(pAd);
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("RTUSBBulkReceive\n"));
	}*/
	
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_OUT<===\n"));
	return NDIS_STATUS_SUCCESS;


}


// All transfers must be aborted or cancelled before attempting to reset the pipe.
static NTSTATUS ResetBulkInHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UINT32 MACValue;
	NTSTATUS ntStatus;
	
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_IN === >\n"));
	
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		ATEResetBulkIn(pAd);
	else
#endif // RALINK_ATE //
	{
		//while ((atomic_read(&pAd->PendingRx) > 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) 
		if((pAd->PendingRx > 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("BulkIn IRP Pending!!!\n"));
			RTUSBCancelPendingBulkInIRP(pAd);
			RTMPusecDelay(100000);
			pAd->PendingRx = 0;
		}
	}						
						
	// Wait 10ms before reading register.
	RTMPusecDelay(10000);
	ntStatus = RTUSBReadMACRegister(pAd, MAC_CSR0, &MACValue);

	if ((NT_SUCCESS(ntStatus) == TRUE) && 
				(!(RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_RADIO_OFF | 
												fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))))
	{		
		UCHAR	i;

		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_RADIO_OFF | 
									fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))
			return NDIS_STATUS_SUCCESS;
		
		pAd->NextRxBulkInPosition = pAd->RxContext[pAd->NextRxBulkInIndex].BulkInOffset;
		
		DBGPRINT(RT_DEBUG_TRACE, ("BULK_IN_RESET: NBIIdx=0x%x,NBIRIdx=0x%x, BIRPos=0x%lx. BIReq=x%lx, BIComplete=0x%lx, BICFail0x%lx\n", 
					pAd->NextRxBulkInIndex,  pAd->NextRxBulkInReadIndex, pAd->NextRxBulkInPosition, pAd->BulkInReq, pAd->BulkInComplete, pAd->BulkInCompleteFail));
		
		for (i = 0; i < RX_RING_SIZE; i++)
		{
 			DBGPRINT(RT_DEBUG_TRACE, ("\tRxContext[%d]: IRPPending=%d, InUse=%d, Readable=%d!\n"
							, i, pAd->RxContext[i].IRPPending, pAd->RxContext[i].InUse, pAd->RxContext[i].Readable));
		}
		
 		/*
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("==========================================\n"));

			pAd->NextRxBulkInReadIndex = 0;	// Next Rx Read index
			pAd->NextRxBulkInIndex		= 0;	// Rx Bulk pointer
			for (i = 0; i < (RX_RING_SIZE); i++)
			{
				PRX_CONTEXT  pRxContext = &(pAd->RxContext[i]);

				pRxContext->pAd	= pAd;
				pRxContext->InUse		= FALSE;
				pRxContext->IRPPending	= FALSE;
				pRxContext->Readable	= FALSE;
				pRxContext->ReorderInUse = FALSE;
									
			}*/
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
		
		for (i = 0; i < pAd->CommonCfg.NumOfBulkInIRP; i++)
		{
			//RTUSBBulkReceive(pAd);
			PRX_CONTEXT		pRxContext;
			PURB			pUrb;
			int				ret = 0;
			unsigned long	IrqFlags;

			RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
			pRxContext = &(pAd->RxContext[pAd->NextRxBulkInIndex]);
			
			if ((pAd->PendingRx > 0) || (pRxContext->Readable == TRUE) || (pRxContext->InUse == TRUE))
			{
				RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
				return NDIS_STATUS_SUCCESS;
			}
			
			pRxContext->InUse = TRUE;
			pRxContext->IRPPending = TRUE;
			pAd->PendingRx++;
			pAd->BulkInReq++;
			RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);

			// Init Rx context descriptor
			RTUSBInitRxDesc(pAd, pRxContext);
			pUrb = pRxContext->pUrb;
			if ((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
			{	// fail
				RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
				pRxContext->InUse = FALSE;
				pRxContext->IRPPending = FALSE;
				pAd->PendingRx--;
				pAd->BulkInReq--;
				RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
				DBGPRINT(RT_DEBUG_ERROR, ("CMDTHREAD_RESET_BULK_IN: Submit Rx URB failed(%d), status=%d\n", ret, pUrb->rtusb_urb_status));
			}
			else
			{	// success
				//DBGPRINT(RT_DEBUG_TRACE, ("BIDone, Pend=%d,BIIdx=%d,BIRIdx=%d!\n", 
				//							pAd->PendingRx, pAd->NextRxBulkInIndex, pAd->NextRxBulkInReadIndex));
				DBGPRINT_RAW(RT_DEBUG_TRACE, ("CMDTHREAD_RESET_BULK_IN: Submit Rx URB Done, status=%d!\n", pUrb->rtusb_urb_status));
				ASSERT((pRxContext->InUse == pRxContext->IRPPending));
			}
		}					

	}
	else
	{
		// Card must be removed
		if (NT_SUCCESS(ntStatus) != TRUE)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("CMDTHREAD_RESET_BULK_IN: Read Register Failed!Card must be removed!!\n\n"));
		}
		else
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("CMDTHREAD_RESET_BULK_IN: Cannot do bulk in because flags(0x%lx) on !\n", pAd->Flags));
	}

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_IN <===\n"));
	return NDIS_STATUS_SUCCESS;
}                                                                                                                                                   


static NTSTATUS SetAsicWcidHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	RT_SET_ASIC_WCID	SetAsicWcid;
	USHORT		offset;
	UINT32		MACValue, MACRValue = 0;
	SetAsicWcid = *((PRT_SET_ASIC_WCID)(CMDQelmt->buffer));
						
	if (SetAsicWcid.WCID >= MAX_LEN_OF_MAC_TABLE)
		return NDIS_STATUS_FAILURE;
						
	offset = MAC_WCID_BASE + ((UCHAR)SetAsicWcid.WCID)*HW_WCID_ENTRY_SIZE;

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("CmdThread : CMDTHREAD_SET_ASIC_WCID : WCID = %ld, SetTid  = %lx, DeleteTid = %lx.\n", 
						SetAsicWcid.WCID, SetAsicWcid.SetTid, SetAsicWcid.DeleteTid));
	
	MACValue = (pAd->MacTab.Content[SetAsicWcid.WCID].Addr[3]<<24)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[2]<<16)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[1]<<8)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[0]);

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("1-MACValue= %x,\n", MACValue));
	RTUSBWriteMACRegister(pAd, offset, MACValue);
	// Read bitmask
	RTUSBReadMACRegister(pAd, offset+4, &MACRValue);
	if ( SetAsicWcid.DeleteTid != 0xffffffff)
		MACRValue &= (~SetAsicWcid.DeleteTid);
	if (SetAsicWcid.SetTid != 0xffffffff)
		MACRValue |= (SetAsicWcid.SetTid);
	
	MACRValue &= 0xffff0000;	
	MACValue = (pAd->MacTab.Content[SetAsicWcid.WCID].Addr[5]<<8)+pAd->MacTab.Content[SetAsicWcid.WCID].Addr[4];
	MACValue |= MACRValue;
	RTUSBWriteMACRegister(pAd, offset+4, MACValue);
						
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("2-MACValue= %x,\n", MACValue));
	
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS SetWcidSecInfoHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_SEC_INFO pInfo;

	pInfo = (PRT_ASIC_WCID_SEC_INFO)CMDQelmt->buffer;
	RTMPSetWcidSecurityInfo(pAd, 
							 pInfo->BssIdx, 
							 pInfo->KeyIdx, 
							 pInfo->CipherAlg, 
							 pInfo->Wcid, 
							 pInfo->KeyTabFlag);
	
	return NDIS_STATUS_SUCCESS;	
}


static NTSTATUS SetAsicWcidIVEIVHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_IVEIV_ENTRY pInfo;

	pInfo = (PRT_ASIC_WCID_IVEIV_ENTRY)CMDQelmt->buffer;
	AsicUpdateWCIDIVEIV(pAd, 
						  pInfo->Wcid, 
						  pInfo->Iv, 
						  pInfo->Eiv);
						
	return NDIS_STATUS_SUCCESS;	
}


static NTSTATUS SetAsicWcidAttrHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_ATTR_ENTRY pInfo;

	pInfo = (PRT_ASIC_WCID_ATTR_ENTRY)CMDQelmt->buffer;
	AsicUpdateWcidAttributeEntry(pAd, 
								  pInfo->BssIdx, 
								  pInfo->KeyIdx, 
								  pInfo->CipherAlg, 
								  pInfo->Wcid, 
								  pInfo->KeyTabFlag);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS SETAsicSharedKeyHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_SHARED_KEY pInfo;

	pInfo = (PRT_ASIC_SHARED_KEY)CMDQelmt->buffer;
	AsicAddSharedKeyEntry(pAd, 
						       pInfo->BssIndex, 
							pInfo->KeyIdx, 
							&pInfo->CipherKey);
	
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS SetAsicPairwiseKeyHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_PAIRWISE_KEY pInfo;

	pInfo = (PRT_ASIC_PAIRWISE_KEY)CMDQelmt->buffer;
	AsicAddPairwiseKeyEntry(pAd, 
							 pInfo->WCID, 
							 &pInfo->CipherKey);

	return NDIS_STATUS_SUCCESS;
}



static NTSTATUS RemovePairwiseKeyHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR Wcid = *((PUCHAR)(CMDQelmt->buffer));	

	AsicRemovePairwiseKeyEntry(pAd, Wcid);		
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS SetClientMACEntryHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_SET_ASIC_WCID pInfo;

	pInfo = (PRT_SET_ASIC_WCID)CMDQelmt->buffer;						
	AsicUpdateRxWCIDTable(pAd, pInfo->WCID, pInfo->Addr);
	return NDIS_STATUS_SUCCESS;
}


// add by johnli, fix "in_interrupt" error when call "MacTableDeleteEntry" in Rx tasklet
static NTSTATUS UpdateProtectHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{	
	AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT), TRUE, 0);
	return NDIS_STATUS_SUCCESS;
}


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS APUpdateCapabilityAndErpieHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	APUpdateCapabilityAndErpIe(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif // CONFIG_AP_SUPPORT //
// end johnli


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS _802_11_CounterMeasureHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MAC_TABLE_ENTRY *pEntry;
						
		pEntry = (MAC_TABLE_ENTRY *)CMDQelmt->buffer;
		HandleCounterMeasure(pAd, pEntry);
	}
	
	return NDIS_STATUS_SUCCESS;
}
#endif // CONFIG_AP_SUPPORT //




#ifdef CONFIG_AP_SUPPORT
static NTSTATUS APEnableTXBurstHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		EDCA_AC_CFG_STRUC Ac0Cfg;
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_ENABLE_TX_BURST  \n"));
						
		RTUSBReadMACRegister(pAd, EDCA_AC0_CFG, &Ac0Cfg.word);
		Ac0Cfg.field.AcTxop = 0x20;
		RTUSBWriteMACRegister(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
	}
	
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS APDisableTXBurstHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		EDCA_AC_CFG_STRUC Ac0Cfg;
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_DISABLE_TX_BURST  \n"));
						
		RTUSBReadMACRegister(pAd, EDCA_AC0_CFG, &Ac0Cfg.word);
		Ac0Cfg.field.AcTxop = 0x0;
		RTUSBWriteMACRegister(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
	}

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS APAdjustEXPAckTimeHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_ADJUST_EXP_ACK_TIME  \n"));
		RTUSBWriteMACRegister(pAd, EXP_ACK_TIME, 0x005400ca);
	}

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS APRecoverEXPAckTimeHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CmdThread::CMDTHREAD_AP_RECOVER_EXP_ACK_TIME  \n"));
		RTUSBWriteMACRegister(pAd, EXP_ACK_TIME, 0x002400ca);
	}

	return NDIS_STATUS_SUCCESS;
}
#endif // CONFIG_AP_SUPPORT //


#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
static NTSTATUS SetWPSLEDStatusHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR WPSLEDStatus = *((PUCHAR)(CMDQelmt->buffer));

	RTMPSetLED(pAd, WPSLEDStatus);
						
	DBGPRINT(RT_DEBUG_TRACE, ("%s: CMDTHREAD_SET_WPS_LED_STATUS (WPSLEDStatus = %d)\n", 
								__FUNCTION__, WPSLEDStatus));	 
	
	return NDIS_STATUS_SUCCESS;
}


//WPS LED MODE 10
static NTSTATUS LEDWPSMode10Hdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UINT WPSLedMode10 = *((PUINT)(CMDQelmt->buffer));

	DBGPRINT(RT_DEBUG_INFO, ("WPS LED mode 10::ON or Flash or OFF : %x\n", WPSLedMode10));

	switch(WPSLedMode10)
	{
		case LINK_STATUS_WPS_MODE10_TURN_ON:
			RTMPSetLED(pAd, LED_WPS_MODE10_TURN_ON);
			break;				
		case LINK_STATUS_WPS_MODE10_FLASH:
			RTMPSetLED(pAd,LED_WPS_MODE10_FLASH);
			break;
		case LINK_STATUS_WPS_MODE10_TURN_OFF:
			RTMPSetLED(pAd, LED_WPS_MODE10_TURN_OFF);
			break;
		default:
			DBGPRINT(RT_DEBUG_INFO, ("WPS LED mode 10:: No this status!!!\n", WPSLedMode10));
			break;
	}
	
	return NDIS_STATUS_SUCCESS;
}
#endif // WSC_LED_SUPPORT //
#endif // WSC_INCLUDED //


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS ChannelRescanHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	DBGPRINT(RT_DEBUG_TRACE, ("cmd> Re-scan channel! \n"));

	pAd->CommonCfg.Channel = AP_AUTO_CH_SEL(pAd, TRUE);
	DBGPRINT(RT_DEBUG_TRACE, ("cmd> Switch to %d! \n", pAd->CommonCfg.Channel));
	APStop(pAd);
	APStartUp(pAd);

#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadAlarmResume(pAd);
#endif // AP_QLOAD_SUPPORT //

	return NDIS_STATUS_SUCCESS;
}
#endif // CONFIG_AP_SUPPORT


#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
static NTSTATUS RegHintHdlr (IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	CFG80211_RegHint(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RegHint11DHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	CFG80211_RegHint11D(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RT_Mac80211_ScanEnd(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	CFG80211_ScanEnd(pAd, FALSE);
	return NDIS_STATUS_SUCCESS;
}
#endif // RT_CFG80211_SUPPORT //
#endif // LINUX //

#ifdef BT_COEXISTENCE_SUPPORT
static NTSTATUS BtCoexistDetectExecAtCmdThread(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	BOOLEAN  					bIssue4020 = FALSE;
	UCHAR	 					j = 0;
#ifdef DOT11N_DRAFT3
	UCHAR 						btWifiThr = 0;
#endif // DOT11N_DRAFT3 //
	ULONG 						data = 0;
	ULONG 						data_switch=0;	
	static ULONG 				TimeCount = 0; // per 0.01 second, count it.
	static ULONG 				NoBusyTimeCount = 0; // per second, count it.
	static ULONG 				BtHistory[BT_HISTORY_RECORD_NUM] = {0};
	static ULONG 				BtHardKeyOffTimeCount = 0;
	BLUETOOTH_BUSY_DEGREE  		BusyDegree = BUSY_0;

	TimeCount++;
		
	if (BtHardKeyOffTimeCount == 0)
	{
		/* 2-wire */
		AsicSendCommandToMcu(pAd, MCU_SET_BT_ACTIVE_POLLING, 0xff , 0x01, 0x00); 
	}
	else
	{
		/*
			Hardkey push down => BT Power Off!!
			Count BT Active,but no disable Wifi Tx
		*/
		AsicSendCommandToMcu(pAd, MCU_SET_BT_ACTIVE_POLLING, 0xff , 0x01, 0x02); 	
	}

	RTMP_IO_READ32(pAd, BT_ACT_CNT, &data);
	data_switch &= 0;
	data_switch |= ((data & 0x000000FF) << 8);
	data_switch |= ((data & 0x0000FF00) >> 8);
	pAd->ulBTActiveCountPastPeriod += data_switch;

	DBGPRINT(RT_DEBUG_INFO, (">>>>> data = 0x%08lx, data_switch=0x%08lx", (data & 0x0000FFFF), data_switch));

	/* About 1 second to check busy from Buletooth history active counter */
	if (TimeCount >= BT_CHECK_TIME_INTERVAL)
	{
		DBGPRINT(RT_DEBUG_INFO,("<--- WATCH TIME"));	
		DBGPRINT(RT_DEBUG_INFO,("BW=%d, bt active per sec=%ld, No Busy Time Count =%ld", pAd->CommonCfg.BBPCurrentBW,
				pAd->ulBTActiveCountPastPeriod, NoBusyTimeCount));

		/* Hardkey push down => BT Power Off!! */
		if (pAd->ulBTActiveCountPastPeriod >= 250000)
		{	
			if (BtHardKeyOffTimeCount == 0)
			{
				/* Count BT priority,but no disable Wifi Tx */
				DBGPRINT(RT_DEBUG_INFO,("Hardware key OFF!!!!"));

				AsicSendCommandToMcu(pAd, MCU_SET_BT_ACTIVE_POLLING, 0xff , 0x01, 0x02);	
			}
			BtHardKeyOffTimeCount++;
			pAd->ulBTActiveCountPastPeriod = 0;
			DBGPRINT(RT_DEBUG_INFO,("ulBTActiveCountPastPeriod >= 250000, force ulBTActiveCountPastPeriod = 0"));
		}
		else
		{
			if (BtHardKeyOffTimeCount > 0)
			{
				BtHardKeyOffTimeCount = 0;
				DBGPRINT(RT_DEBUG_INFO,("Hardware key ON!!!!"));

				AsicSendCommandToMcu(pAd, MCU_SET_BT_ACTIVE_POLLING, 0xff , 0x01, 0x00);
			}	
		}

		for(j=BT_HISTORY_RECORD_NUM-1 ; j>=1 ; j--)
		{
			BtHistory[j]=BtHistory[j-1];
		}

		BtHistory[0]=pAd->ulBTActiveCountPastPeriod;
		BusyDegree = BtCheckBusy(&BtHistory[0],BT_HISTORY_RECORD_NUM);
		pAd->BTBusyDegree = BusyDegree;

		if (pAd->BTBusyDegree == BUSY_0)
		{	
			NoBusyTimeCount++;
		}
		else
		{
			NoBusyTimeCount = 0;
		}
		
#ifdef DOT11N_DRAFT3
		btWifiThr = (UCHAR)(GET_BT_PARAMETER_OF_TXRX_THR_THRESHOLD(pAd));
		/* 0,1,2,3 => 0, 6, 12, 18 Mbps */
		if ((btWifiThr <= 3)&& IS_ENABLE_BT_40TO20_BY_TIMER(pAd))
		{
			bIssue4020 = BtCheckWifiThroughputOverLimit(pAd,(btWifiThr*6));
		}
		else 
#endif // DOT11N_DRAFT3 //
		bIssue4020 = FALSE;
		
		pAd->ulBTActiveCountPastPeriod=0;
		TimeCount = 0;
	}

	if (TimeCount == 0)
	{
		BtCoexistAdjust(pAd, bIssue4020, NoBusyTimeCount);
	}	
	
	
	return NDIS_STATUS_SUCCESS;
}
#endif // BT_COEXISTENCE_SUPPORT //

typedef NTSTATUS (*CMDHdlr)(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt);

static CMDHdlr CMDHdlrTable[] = {
	ResetBulkOutHdlr,				// CMDTHREAD_RESET_BULK_OUT
	ResetBulkInHdlr,					// CMDTHREAD_RESET_BULK_IN
	CheckGPIOHdlr,					// CMDTHREAD_CHECK_GPIO	
	SetAsicWcidHdlr,					// CMDTHREAD_SET_ASIC_WCID
	SetClientMACEntryHdlr,			// CMDTHREAD_SET_CLIENT_MAC_ENTRY
	
	NULL, 
	NULL, 
	NULL,
	NULL,

#ifdef CONFIG_AP_SUPPORT
	APUpdateCapabilityAndErpieHdlr,	// CMDTHREAD_AP_UPDATE_CAPABILITY_AND_ERPIE
	APEnableTXBurstHdlr,			// CMDTHREAD_AP_ENABLE_TX_BURST
	APDisableTXBurstHdlr,			// CMDTHREAD_AP_DISABLE_TX_BURST
	APAdjustEXPAckTimeHdlr,		// CMDTHREAD_AP_ADJUST_EXP_ACK_TIME
	APRecoverEXPAckTimeHdlr,		// CMDTHREAD_AP_RECOVER_EXP_ACK_TIME
	ChannelRescanHdlr,				// CMDTHREAD_CHAN_RESCAN
#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
#endif // CONFIG_AP_SUPPORT //

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	SetWPSLEDStatusHdlr,			// CMDTHREAD_SET_WPS_LED_STATUS
	LEDWPSMode10Hdlr,				// CMDTHREAD_LED_WPS_MODE10
#else
	NULL,
	NULL,
#endif // WSC_LED_SUPPORT //

#else
	NULL,
	NULL,
#endif // WSC_INCLUDED //

	/* Security related */
	SetWcidSecInfoHdlr,				// CMDTHREAD_SET_WCID_SEC_INFO
	SetAsicWcidIVEIVHdlr,			// CMDTHREAD_SET_ASIC_WCID_IVEIV
	SetAsicWcidAttrHdlr,				// CMDTHREAD_SET_ASIC_WCID_ATTR
	SETAsicSharedKeyHdlr,			// CMDTHREAD_SET_ASIC_SHARED_KEY
	SetAsicPairwiseKeyHdlr,			// CMDTHREAD_SET_ASIC_PAIRWISE_KEY
	RemovePairwiseKeyHdlr,			// CMDTHREAD_REMOVE_PAIRWISE_KEY

	NULL,

#ifdef CONFIG_AP_SUPPORT
	_802_11_CounterMeasureHdlr,	// CMDTHREAD_802_11_COUNTER_MEASURE
#else
	NULL,
#endif // CONFIG_AP_SUPPORT //

	UpdateProtectHdlr,				// CMDTHREAD_UPDATE_PROTECT
	

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	RegHintHdlr,
	RegHint11DHdlr,
	RT_Mac80211_ScanEnd,
#else
	NULL, 
	NULL,
	NULL,
#endif // RT_CFG80211_SUPPORT //

#else
	NULL, 
	NULL,
	NULL,
#endif // LINUX //

#ifdef BT_COEXISTENCE_SUPPORT
	BtCoexistDetectExecAtCmdThread,	// RT_CMD_BT_COEXISTENCE_DETECTION
#else
	NULL, 
#endif // BT_COEXISTENCE_SUPPORT //
};


static inline BOOLEAN ValidCMD(IN PCmdQElmt CMDQelmt)
{
	USHORT CMDIndex = CMDQelmt->command - CMDTHREAD_FIRST_CMD_ID;
	USHORT CMDHdlrTableLength= sizeof(CMDHdlrTable) / sizeof(CMDHdlr);
	
//	if ( (CMDIndex >= 0) && (CMDIndex < CMDHdlrTableLength))
	if (CMDIndex < CMDHdlrTableLength) /* CMDIndex is unsigned, always >= 0 */
	{
		if (CMDHdlrTable[CMDIndex] > 0)
			return TRUE;
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("No corresponding CMDHdlr for this CMD(%x)\n",  CMDQelmt->command));
			return FALSE;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("CMD(%x) is out of boundary\n", CMDQelmt->command));
		return FALSE;
	}
}


VOID CMDHandler(                                                                                                                                                
    IN PRTMP_ADAPTER pAd)                                                                                                                                       
{                                                                                                                                                               
	PCmdQElmt		cmdqelmt;                                                                                                                                                                                                                                                                              
	NDIS_STATUS		NdisStatus = NDIS_STATUS_SUCCESS;                                                                                                               
	NTSTATUS		ntStatus;
//	unsigned long	IrqFlags;
	
	while (pAd && pAd->CmdQ.size > 0)	
	{                                                                                                                                                           
		NdisStatus = NDIS_STATUS_SUCCESS;
		                                                                                                                      
		NdisAcquireSpinLock(&pAd->CmdQLock);
		RTThreadDequeueCmd(&pAd->CmdQ, &cmdqelmt);
		NdisReleaseSpinLock(&pAd->CmdQLock);
		                                                                                                        
		if (cmdqelmt == NULL)                                                                                                                                   
			break; 
		
		                                                                                         
		if(!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
		{
			if(ValidCMD(cmdqelmt))
				ntStatus = (*CMDHdlrTable[cmdqelmt->command - CMDTHREAD_FIRST_CMD_ID])(pAd, cmdqelmt);
		}                 
		
		if (cmdqelmt->CmdFromNdis == TRUE)
		{
			if (cmdqelmt->buffer != NULL)
				os_free_mem(pAd, cmdqelmt->buffer);
			os_free_mem(pAd, cmdqelmt);
		}
		else
		{
			if ((cmdqelmt->buffer != NULL) && (cmdqelmt->bufferlength != 0))
				os_free_mem(pAd, cmdqelmt->buffer);
			os_free_mem(pAd, cmdqelmt);
		}
	}	/* end of while */
}


VOID RTUSBWatchDog(IN RTMP_ADAPTER *pAd)
{
	PHT_TX_CONTEXT		pHTTXContext;
	int 					idx;
	ULONG				irqFlags;
	PURB		   		pUrb;
	BOOLEAN				needDumpSeq = FALSE;
	UINT32          	MACValue;


	idx = 0;
	RTMP_IO_READ32(pAd, TXRXQ_PCNT, &MACValue);
	if ((MACValue & 0xff) !=0 )
	{
		DBGPRINT(RT_DEBUG_TRACE, ("TX QUEUE 0 Not EMPTY(Value=0x%0x). !!!!!!!!!!!!!!!\n", MACValue));
		RTMP_IO_WRITE32(pAd, PBF_CFG, 0xf40012);
		while((MACValue &0xff) != 0 && (idx++ < 10))
		{
		        RTMP_IO_READ32(pAd, TXRXQ_PCNT, &MACValue);
		        RTMPusecDelay(1);
		}
		RTMP_IO_WRITE32(pAd, PBF_CFG, 0xf40006);
	}

	idx = 0;
	if ((MACValue & 0xff00) !=0 )
	{
		DBGPRINT(RT_DEBUG_TRACE, ("TX QUEUE 1 Not EMPTY(Value=0x%0x). !!!!!!!!!!!!!!!\n", MACValue));
		RTMP_IO_WRITE32(pAd, PBF_CFG, 0xf4000a);
		while((MACValue &0xff00) != 0 && (idx++ < 10))
		{
			RTMP_IO_READ32(pAd, TXRXQ_PCNT, &MACValue);
			RTMPusecDelay(1);
		}
		RTMP_IO_WRITE32(pAd, PBF_CFG, 0xf40006);
	}

	
	if (pAd->watchDogRxOverFlowCnt >= 2)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Maybe the Rx Bulk-In hanged! Cancel the pending Rx bulks request!\n"));
		if ((!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
									fRTMP_ADAPTER_BULKIN_RESET |
									fRTMP_ADAPTER_HALT_IN_PROGRESS |
									fRTMP_ADAPTER_NIC_NOT_EXIST))))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Call CMDTHREAD_RESET_BULK_IN to cancel the pending Rx Bulk!\n"));
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
			RTEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_IN, NULL, 0);
			needDumpSeq = TRUE;
		}
		pAd->watchDogRxOverFlowCnt = 0;
	}


	for (idx = 0; idx < NUM_OF_TX_RING; idx++)
	{
		pUrb = NULL;
		
		RTMP_IRQ_LOCK(&pAd->BulkOutLock[idx], irqFlags);
		if ((pAd->BulkOutPending[idx] == TRUE) && pAd->watchDogTxPendingCnt)
		{
			pAd->watchDogTxPendingCnt[idx]++;

			if ((pAd->watchDogTxPendingCnt[idx] > 2) && 
				 (!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_BULKOUT_RESET)))
				)
			{
				// FIXME: Following code just support single bulk out. If you wanna support multiple bulk out. Modify it!
				pHTTXContext = (PHT_TX_CONTEXT)(&pAd->TxContext[idx]);
				if (pHTTXContext->IRPPending)
				{	// Check TxContext.
					pUrb = pHTTXContext->pUrb;
				}
				else if (idx == MGMTPIPEIDX)
				{
					PTX_CONTEXT pMLMEContext, pNULLContext, pPsPollContext;
					
					//Check MgmtContext.
					pMLMEContext = (PTX_CONTEXT)(pAd->MgmtRing.Cell[pAd->MgmtRing.TxDmaIdx].AllocVa);
					pPsPollContext = (PTX_CONTEXT)(&pAd->PsPollContext);
					pNULLContext = (PTX_CONTEXT)(&pAd->NullContext);
					
					if (pMLMEContext->IRPPending)
					{
						ASSERT(pMLMEContext->IRPPending);
						pUrb = pMLMEContext->pUrb;
					}
					else if (pNULLContext->IRPPending)
					{	
						ASSERT(pNULLContext->IRPPending);
						pUrb = pNULLContext->pUrb;
					}
					else if (pPsPollContext->IRPPending)
					{	
						ASSERT(pPsPollContext->IRPPending);
						pUrb = pPsPollContext->pUrb;
					}
				}
				
				RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[idx], irqFlags);
				
				DBGPRINT(RT_DEBUG_TRACE, ("Maybe the Tx Bulk-Out hanged! Cancel the pending Tx bulks request of idx(%d)!\n", idx));
				if (pUrb)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("Unlink the pending URB!\n"));
					// unlink it now
					RTUSB_UNLINK_URB(pUrb);
					// Sleep 200 microseconds to give cancellation time to work
					RTMPusecDelay(200);
					needDumpSeq = TRUE;
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Unkonw bulkOut URB maybe hanged!!!!!!!!!!!!\n"));
				}
			}
			else
			{
				RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[idx], irqFlags);
			}
		}
		else
		{
			RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[idx], irqFlags);
		}
	}

#ifdef DOT11_N_SUPPORT
	// For Sigma debug, dump the ba_reordering sequence.
	if((needDumpSeq == TRUE) && (pAd->CommonCfg.bDisableReordering == 0))
	{
		USHORT				Idx;
		PBA_REC_ENTRY		pBAEntry = NULL;
		UCHAR				count = 0;
		struct reordering_mpdu *mpdu_blk;
					
		Idx = pAd->MacTab.Content[BSSID_WCID].BARecWcidArray[0];

		pBAEntry = &pAd->BATable.BARecEntry[Idx];
		if((pBAEntry->list.qlen > 0) && (pBAEntry->list.next != NULL))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("NICUpdateRawCounters():The Queueing pkt in reordering buffer:\n"));
			NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
			mpdu_blk = pBAEntry->list.next;
			while (mpdu_blk)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("\t%d:Seq-%d, bAMSDU-%d!\n", count, mpdu_blk->Sequence, mpdu_blk->bAMSDU));
				mpdu_blk = mpdu_blk->next;
				count++;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("\npBAEntry->LastIndSeq=%d!\n", pBAEntry->LastIndSeq));
			NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
		}
	}
#endif // DOT11_N_SUPPORT //
}

#endif // RTMP_MAC_USB //
