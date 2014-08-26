/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_cobt.c

	Abstract:

	Handling Bluetooth Coexistence Problem

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Sean Wang	2009-08-12		Create
	John Li		2009-11-30		Modified
*/

#include "rt_config.h"

#ifdef BT_COEXISTENCE_SUPPORT

#ifdef DOT11N_DRAFT3
BOOLEAN BtCheckWifiThroughputOverLimit(
	IN	PRTMP_ADAPTER	pAd,
	IN  UCHAR WifiThroughputLimit)
{
	BOOLEAN  bIssue4020 = FALSE;
	ULONG tmpReceivedByteCount = 0;
	ULONG tmpTransmittedByteCount = 0;
	static ULONG TxByteCount = 0;
	static ULONG RxByteCount = 0;
	static ULONG TxRxThroughputPerSeconds = 0; //Unit: bytes
	LONG diffTX = 0;
	LONG diffRX = 0;

	bIssue4020 = FALSE;
	
	if (pAd == NULL)
	{
		return FALSE;
	}
	
	if (IS_ENABLE_BT_40TO20_BY_TIMER(pAd))
	{	
		tmpReceivedByteCount = pAd->RalinkCounters.ReceivedByteCount; 
		tmpTransmittedByteCount = pAd->RalinkCounters.TransmittedByteCount;		
					
		if ( (TxByteCount != 0) || (RxByteCount != 0 ))
		{				
			diffTX = (LONG)(((tmpTransmittedByteCount - TxByteCount)*5) >> 3);
			diffRX = (LONG)(((tmpReceivedByteCount - RxByteCount)* 5) >> 3);		
						
			if ((diffTX > 0) && (diffRX > 0 ))
			{
				TxRxThroughputPerSeconds = diffTX + diffRX ;//Unit: bytes 
			}
			else if ((diffTX < 0) && (diffRX > 0))
			{
				TxRxThroughputPerSeconds = diffRX;
			}
			else if ((diffTX > 0) && (diffRX < 0))
			{
				TxRxThroughputPerSeconds = diffTX;
			}
			else 
			{
				TxRxThroughputPerSeconds = 0;
			}

			DBGPRINT(RT_DEBUG_INFO,("TxRxThroughputPerSeconds = %ld Bps, %ld KBps, %ldKbps, %ldMbps", 
				TxRxThroughputPerSeconds,
				(TxRxThroughputPerSeconds >> 10),
				(TxRxThroughputPerSeconds >> 7),
				(TxRxThroughputPerSeconds >> 17)));
		}
	
		TxByteCount = tmpTransmittedByteCount;
		RxByteCount = tmpReceivedByteCount;

		DBGPRINT(RT_DEBUG_INFO,("btWifiThr = %d, TxByteCount = %ld, RxByteCount = %ld",
			WifiThroughputLimit,
			TxByteCount,
			RxByteCount));

		if ((TxRxThroughputPerSeconds >> 17) > WifiThroughputLimit)
		{
			bIssue4020 = TRUE;
		}
		else
		{
			bIssue4020 = FALSE;
		}
	}
	
	return bIssue4020;
}
#endif // DOT11N_DRAFT3 //

BLUETOOTH_BUSY_DEGREE BtCheckBusy(
	IN PLONG BtHistory, 
	IN UCHAR BtHistorySize)
{
	if (BtHistory == NULL)
	{
		return BUSY_0;
	}

	DBGPRINT(RT_DEBUG_INFO,(" ---> Check Busy %ld %ld %ld %ld %ld",
		*BtHistory,
		*(BtHistory+1),
		*(BtHistory+2),
		*(BtHistory+3),
		*(BtHistory+4)));

#ifdef RTMP_USB_SUPPORT
	if ((*BtHistory > 200000) || 
		(*(BtHistory+1) > 200000) || 
		(*(BtHistory+2) > 200000) ||
		(*(BtHistory+3) > 200000) ||
		(*(BtHistory+4) > 200000))
	{
		DBGPRINT(RT_DEBUG_INFO,("200000, 200000, 200000, 200000, 200000 ==> Inquiry + other Profiles ... BT lagging .. BUSY_5\n"));
		return BUSY_5; 
	}
	
	if (((*BtHistory >= 20000) || 
		(*(BtHistory+1) >= 20000) || 
		(*(BtHistory+2) >= 20000) || 
		(*(BtHistory+3) >= 20000) || 
		(*(BtHistory+4) >= 20000))&&
		(*BtHistory >= 10000) && 
		(*(BtHistory+1) >= 10000) && 
		(*(BtHistory+2) >= 10000) && 
		(*(BtHistory+3) >= 10000) && 
		(*(BtHistory+4) >= 10000))	
	{
		DBGPRINT(RT_DEBUG_INFO,("10000, 10000, 20000, 10000, 10000 ==> Multiple Profile... BUSY_4\n"));
		return BUSY_4;	
	}
	
	if (((*BtHistory >= 12000) || 
		(*(BtHistory+1) >= 12000) || 
		(*(BtHistory+2) >= 12000) || 
		(*(BtHistory+3) >= 12000) || 
		(*(BtHistory+4) >= 12000))&&
		(*BtHistory >= 5000) && 
		(*(BtHistory+1) >= 5000) && 
		(*(BtHistory+2) >= 5000) && 
		(*(BtHistory+3) >= 5000) && 
		(*(BtHistory+4) >= 5000))	
	{
		DBGPRINT(RT_DEBUG_INFO,("5000, 5000, 12000, 5000, 5000 ==> BT work ... Jabra BT3030 A2DP Playing Music  ... BUSY_3\n"));
		return BUSY_3;	
	}
	
	if (((*BtHistory >= 12000) || 
		(*(BtHistory+1) >= 12000) || 
		(*(BtHistory+2) >= 12000) || 
		(*(BtHistory+3) >= 12000) || 
		(*(BtHistory+4) >= 12000))&&
		(*BtHistory >= 2000) && 
		(*(BtHistory+1) >= 2000) && 
		(*(BtHistory+2) >= 2000) && 
		(*(BtHistory+3) >= 2000) && 
		(*(BtHistory+4) >= 2000))	
	{
		DBGPRINT(RT_DEBUG_INFO,("2000, 2000, 12000, 2000, 2000 ==> BT works ... CSR mouse suddenly moves ... BUSY_2\n"));
		return BUSY_2;
	}

	if (((*BtHistory >= 10000) || 
		(*(BtHistory+1) >= 10000) || 
		(*(BtHistory+2) >= 10000) || 
		(*(BtHistory+3) >= 10000) || 
		(*(BtHistory+4) >= 10000)) &&
		((*BtHistory > 0) && 
		(*(BtHistory+1) > 0) && 
		(*(BtHistory+2) > 0) &&
		(*(BtHistory+3) > 0) &&
		(*(BtHistory+4) > 0)))
	{
		DBGPRINT(RT_DEBUG_INFO,("1,1,10000,1,1 ==> General Profile .. BUSY_1\n"));
		return BUSY_1;
	}	
#endif // RTMP_USB_SUPPORT //

	DBGPRINT(RT_DEBUG_INFO,("ALL BT Profile Disconnected .. BUSY_0"));
	return BUSY_0;
}

/*
	Wifi adjust according to BT Acitivitity  
	Case 1: LNA Middle Gain 	 @HT40/HT20 and BUSY_1 equal & above
	Case 2: TxPower 				 @HT40/HT20 and BUSY_1 equal & above
	Case 3: BA Window Size		 @HT40/HT20 and BUSY_2 equal & above
	Case 4: BA Density			@HT40/HT20 and BUSY_2 equal & above
	Case 5: MCS Rate				@HT40 and BUSY_2 equal & above
	Case 6: REC BA Request			@HT40/HT20 and BUSY_2 equal & above
	Case 7: 40/20 BSS Coexistence	@HT40 and BUSY_4
*/
VOID BtCoexistAdjust(
	IN PRTMP_ADAPTER	pAd, 
	IN BOOLEAN			bIssue4020, 
	IN ULONG			NoBusyTimeCount)
{
	CHAR	Rssi;

	Rssi = RTMPMaxRssi(pAd, 
					   pAd->StaCfg.RssiSample.AvgRssi0, 
					   pAd->StaCfg.RssiSample.AvgRssi1, 
					   pAd->StaCfg.RssiSample.AvgRssi2);
	
	DBGPRINT(RT_DEBUG_INFO,("RSSI = %d\n", Rssi));

	if (IS_ENABLE_BT_LNA_MID_GAIN_DOWN_BY_TIMER(pAd))
	{
		UCHAR BbpR65 = 0;
		if (pAd->BTBusyDegree >= BUSY_1)
		{
			DBGPRINT(RT_DEBUG_INFO, ("Lower LNA Middle Gain at HT20 or HT40\n"));
			pAd->bBTPermitLnaGainDown = TRUE;

			DBGPRINT(RT_DEBUG_INFO,("RSSI = %d\n", Rssi));

			if (Rssi <= -35)
			{
			/* if RSSI is smaller than -80, then set R65 to High Gain to fix long distance issue */
				if (Rssi <= -80)
				{
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BbpR65);

					if (BbpR65 == 0x29)
					{
					DBGPRINT(RT_DEBUG_INFO,("Set R65 to 0x2C from 0x29 (Highest LNA)\n"));
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x2C); /* Highest LNA Gain */
					}
				}
				else
				{
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BbpR65);

					if(BbpR65 == 0x2C)
					{
					DBGPRINT(RT_DEBUG_INFO,("Set R65 to 0x29 from 0x2C (Middle LNA)\n"));
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x29); /* Middle LNA Gain */
					}
				}
			}
			else
			{
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BbpR65);

				if(BbpR65 == 0x29)
				{
				DBGPRINT(RT_DEBUG_INFO,("Set R65 to 0x2C from 0x29 (Highest LNA)\n"));
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x2C); /* Highest LNA Gain */
				}
			}
		}
		else
		{
			if ( NoBusyTimeCount > BT_IDLE_STATE_THRESHOLD)
			{	
			DBGPRINT(RT_DEBUG_INFO, ("Lower LNA Middle Gain at HT20 or HT40 (Highest LNA)\n"));

			pAd->bBTPermitLnaGainDown = FALSE;
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BbpR65);
				if (BbpR65 == 0x29)
				{
				DBGPRINT(RT_DEBUG_INFO,("Set R65 to 0x2C from 0x29 (Highest LNA)\n"));
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x2C); /* Highest LNA Gain */
				}
			}
			else
			{
			DBGPRINT(RT_DEBUG_INFO, ("Lower LNA Middle Gain at HT20 or HT40\n"));
			}
		}
	}

	if (IS_ENABLE_BT_TX_POWER_DOWN_BY_TIMER(pAd))
	{
		if (pAd->BTBusyDegree >= BUSY_1)
		{
		DBGPRINT(RT_DEBUG_INFO, ("Lower Tx Power at HT20 or HT40\n"));
			pAd->bBTPermitTxPowerDown= TRUE;
		}
		else
		{
			if ( NoBusyTimeCount > BT_IDLE_STATE_THRESHOLD)
			{
			DBGPRINT(RT_DEBUG_INFO, ("Higher Tx Power at HT20 or HT40\n"));
				pAd->bBTPermitTxPowerDown= FALSE;
			}
			else
			{
			DBGPRINT(RT_DEBUG_INFO, ("Lower Tx Power at HT20 or HT40\n"));
			}
		}
	}

#ifdef DOT11_N_SUPPORT
	if (IS_ENABLE_BT_TXWI_AMPDU_SIZE_BY_TIMER(pAd))
	{
	/* Fixed long distance issue */
		if ((pAd->BTBusyDegree >= BUSY_2) && 
		((pAd->CommonCfg.BBPCurrentBW == BW_40) || 
		(pAd->CommonCfg.BBPCurrentBW == BW_20)) && 
		(Rssi <= -80))
		{
			pAd->bBTPermitTxBaSizeDown = TRUE;
		}
		else
		{
			pAd->bBTPermitTxBaSizeDown = FALSE;
		}
	}

	if (IS_ENABLE_BT_TXWI_AMPDU_DENSITY_BY_TIMER(pAd))
	{
	/* Fixed long distance issue */
		if ((pAd->BTBusyDegree >= BUSY_2) && 
		((pAd->CommonCfg.BBPCurrentBW == BW_40) || 
		(pAd->CommonCfg.BBPCurrentBW == BW_20)) && 
		(Rssi <= -80))
		{
			pAd->bBTPermitTxBaDensityDown = TRUE;
		}
		else
		{
			pAd->bBTPermitTxBaDensityDown = FALSE;
		}
	}
		
	if (IS_ENABLE_BT_RATE_ADAPTIVE_BY_TIMER(pAd))
	{
		if ((pAd->BTBusyDegree >= BUSY_2) && 
				(pAd->CommonCfg.BBPCurrentBW == BW_40))
		{
			pAd->bBTPermitMcsDown = TRUE;
		}
		else
		{
			pAd->bBTPermitMcsDown = FALSE;
		}
	}

	if (IS_ENABLE_BT_REJECT_ORE_BA_BY_TIMER(pAd))
	{	
	/* Fixed long distance issue */
		if ((pAd->BTBusyDegree >= BUSY_2) && 
		((pAd->CommonCfg.BBPCurrentBW == BW_40) || 
		(pAd->CommonCfg.BBPCurrentBW == BW_20)) && 
		(Rssi <= -80))
		{
			BASessionTearDownALL(pAd, BSSID_WCID);
			pAd->bBTPermitRecBaDown = TRUE;
		}
		else
		{
			pAd->bBTPermitRecBaDown = FALSE;
		}
	}
	
#ifdef DOT11N_DRAFT3
	if (IS_ENABLE_BT_40TO20_BY_TIMER(pAd))
	{
	if (((pAd->BTBusyDegree >= WIFI_2040_SWITCH_THRESHOLD) && 
			(pAd->BTBusyDegree != BUSY_5)) && 
			(pAd->CommonCfg.BBPCurrentBW == BW_40) && 
			(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) &&
			(bIssue4020 == TRUE))
		{
			BSS_2040_COEXIST_IE OldValue;

		DBGPRINT(RT_DEBUG_INFO, ("HT40 --> HT20\n"));
		DBGPRINT(RT_DEBUG_INFO,("ACT - Update2040CoexistFrameAndNotify. BSSCoexist2040 = %x. EventANo = %d. \n", pAd->CommonCfg.BSSCoexist2040.word, pAd->CommonCfg.TriggerEventTab.EventANo));
			OldValue.word = pAd->CommonCfg.BSSCoexist2040.word;
			pAd->CommonCfg.BSSCoexist2040.word = 0;

			//if (pAd->CommonCfg.TriggerEventTab.EventBCountDown > 0)
			pAd->CommonCfg.BSSCoexist2040.field.BSS20WidthReq = 1;

		/*
			Need to check !!!!
			How STA will set Intolerant40 if implementation dependent. Now we don't set this bit first!!!!!
			So Only check BSS20WidthReq change.
		*/
			//if (OldValue.field.BSS20WidthReq != pAd->CommonCfg.BSSCoexist2040.field.BSS20WidthReq)
			{
				Send2040CoexistAction(pAd, BSSID_WCID, TRUE);
			}
		}
	}
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //
}

#ifdef RTMP_USB_SUPPORT
VOID BtCoexistDetectExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
		
	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
				DBGPRINT(RT_DEBUG_INFO,("error RESET, HALT, RADIO_OFF, NIC_NOT_EXIST"));
				return;
	}

	if ((pAd->bHWCoexistenceInit == TRUE) && 
		(!IDLE_ON(pAd))&& 
		(pAd->OpMode == OPMODE_STA))
	{
		RTEnqueueInternalCmd(pAd,RT_CMD_BT_COEXISTENCE_DETECTION, NULL, 0);
	}
	else
	{
#ifdef DOT11_N_SUPPORT
		pAd->bBTPermitRecBaDown = FALSE;
		pAd->bBTPermitMcsDown = FALSE;
		pAd->bBTPermitTxBaSizeDown = FALSE;
		pAd->bBTPermitTxBaDensityDown = FALSE;
#endif // DOT11_N_SUPPORT //
		pAd->bBTPermitTxPowerDown = FALSE;
		pAd->bBTPermitLnaGainDown = FALSE;
	}	
}
#endif // RTMP_USB_SUPPORT //


#endif // BT_COEXISTENCE_SUPPORT //
