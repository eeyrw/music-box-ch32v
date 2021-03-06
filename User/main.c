/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2020/04/30
* Description        : Main program body.
*******************************************************************************/

/*
 *@Note
 Ä£Äâ×Ô¶¨ÒåUSBÉè±¸£¨CH372Éè±¸£©Àý³Ì£º
 USBHDM(PA11)¡¢USBHDP(PA12)¡£
 ±¾Àý³ÌÑÝÊ¾Ê¹ÓÃ USBD Ä£Äâ×Ô¶¨ÒåÉè±¸ CH372£¬ºÍÉÏÎ»»úÍ¨ÐÅ¡£
 
 ×¢£º±¾Àý³ÌÐèÓëÉÏÎ»»úÈí¼þÅäºÏÑÝÊ¾¡£

*/

#include "debug.h"
#include "string.h"
#include "Player.h"

/* Global define */


/* Global Variable */
#define DevEP0SIZE	0x40
#define USB_MIDI_ENDPOINT_IN 0x82
#define USB_MIDI_ENDPOINT_IN_SIZE 0x40
#define USB_MIDI_ENDPOINT_OUT 0x02
#define USB_MIDI_ENDPOINT_OUT_SIZE 0x40


Player mPlayer;
/* Device Descriptor */
const UINT8  MyDevDescr[] = {
	0x12, /* Size of the Descriptor in Bytes (18 bytes) */
	0x01, /* Device Descriptor (0x01) */
	0x10, 0x01, /* USB Specification Number which device complies too. */
    0x00, /* Class Code (Assigned by USB Org)
            If equal to Zero, each interface specifies it¡¯s own class code
             */
	0x00, /* Subclass Code (Assigned by USB Org) */
	0x00, /* Protocol Code (Assigned by USB Org) */
	DevEP0SIZE, /* Maximum Packet Size for Zero Endpoint. Valid Sizes are 8, 16, 32, 64 */
	0x86,0x1a, /* Vendor ID (Assigned by USB Org) */
	0x22, 0x57, /* Product ID (Assigned by Manufacturer) */
	0x00, 0x01, /* Device Release Number */
	0x01, /* Index of Manufacturer String Descriptor */
	0x02,/* Index of Product String Descriptor */
	0x00,/* Index of Serial Number String Descriptor */
	0x01,/* Number of Possible Configurations */
};

/* Language Descriptor */
const UINT8  MyLangDescr[] = { 0x04, 0x03, 0x09, 0x04 };

/* Manufactor Descriptor */
const UINT8  MyManuInfo[] = { 0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0 };

/* Product Information */

const UINT8  MyProdInfo[] = { 0x0C, 0x03, 'C', 0, 'H', 0, '3', 0, '2', 0, 'V', 0 };

#define MidiJackInStrId 5
const UINT8  MidiJackInStr[] = { 16, 0x03, 'M', 0, 'I', 0, 'D', 0, 'I', 0, ' ', 0 , 'I', 0 , 'N', 0 };
#define MidiJackOutStrId 6
const UINT8  MidiJackOutStr[] = { 18, 0x03, 'M', 0, 'I', 0, 'D', 0, 'I', 0, ' ', 0 , 'O', 0 , 'U', 0 , 'T', 0 };

/* Configration Descriptor */
UINT8  MyCfgDescr[] = {
        0x09, /* Size of Descriptor in Bytes */
        0x02, /* Configuration Descriptor (0x02) */
        0x00,0x00, /* Total length in bytes of data returned */
        0x02,0x01,0x00,0x80,0x32,            //ÅäÖÃÃèÊö·û£¨Á½¸ö½Ó¿Ú£©
        //ÒÔÏÂÎª½Ó¿Ú0£¨ÒôÆµ½Ó¿Ú£©ÃèÊö·û
        0x09,0x04,0x00,0x00,0x00,0x01,0x01,0x00,0x00,   // USB Audio Class ÃèÊö£¬ÎÞ¶Ëµã
        //ÒÔÏÂÎª¹¦ÄÜÃèÊö·û
        0x09,0x24,0x01,0x00,0x01,0x09,0x00,0x01,0x01,   //¹¦ÄÜÃèÊö·û£¬³¤¶È´ó¶Ë
        //ÒÔÏÂÎª½Ó¿Ú1£¨MIDI½Ó¿ÚÃèÊö·û£©
        0x09,0x04,0x01,0x00,0x02,0x01,0x03,0x00,0x00,
        //¹¦ÄÜÃèÊö·û
        0x07,0x24,0x01,0x00,0x01,0x25,0x00,
        //IN-JACK
        0x06,0x24,0x02,0x01,0x01,MidiJackInStrId,
        0x06,0x24,0x02,0x02,0x02,MidiJackInStrId,
        //OUT-JACK
        0x09,0x24,0x03,0x01,0x03,0x01,0x02,0x01,MidiJackOutStrId,
        0x09,0x24,0x03,0x02,0x04,0x01,0x01,0x01,MidiJackOutStrId,
        //ÒÔÏÂÎªÁ½¸ö¶ËµãµÄÃèÊö·û
        0x07,0x05,USB_MIDI_ENDPOINT_OUT,0x02,USB_MIDI_ENDPOINT_OUT_SIZE,0x00,0x00,
        0x05,0x25,0x01,0x01,0x01, /* EMB MIDI JACK = 1, AssocJACKID=1, OUT */
        0x07,0x05,USB_MIDI_ENDPOINT_IN,0x02,USB_MIDI_ENDPOINT_IN_SIZE,0x00,0x00,
        0x05,0x25,0x01,0x01,0x03 /* EMB MIDI JACK = 1, AssocJACKID=3, IN */
};



/**********************************************************/
UINT8   DevConfig;
UINT8   SetupReqCode;
UINT16  SetupReqLen;
const UINT8 *pDescr;

/* Endpoint Buffer */
__attribute__ ((aligned(4))) UINT8 EP0_Databuf[64];	//ep0(64)
__attribute__ ((aligned(4))) UINT8 EP1_Databuf[64+64];	//ep1_out(64)+ep1_in(64)
__attribute__ ((aligned(4))) UINT8 EP2_Databuf[64+64];	//ep2_out(64)+ep2_in(64)
__attribute__ ((aligned(4))) UINT8 EP3_Databuf[64+64];	//ep3_out(64)+ep3_in(64)
__attribute__ ((aligned(4))) UINT8 EP4_Databuf[64+64];	//ep4_out(64)+ep4_in(64)
__attribute__ ((aligned(4))) UINT8 EP5_Databuf[64+64];	//ep5_out(64)+ep5_in(64)
__attribute__ ((aligned(4))) UINT8 EP6_Databuf[64+64];	//ep6_out(64)+ep6_in(64)
__attribute__ ((aligned(4))) UINT8 EP7_Databuf[64+64];	//ep7_out(64)+ep7_in(64)

void USBHD_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/*******************************************************************************
* Function Name  : Set_USBConfig
* Description    : Set USB clock.
* Input          : None
* Return         : None
*******************************************************************************/
void USBHD_ClockCmd(UINT32 RCC_USBCLKSource,FunctionalState NewState)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, NewState);
	EXTEN->EXTEN_CTR |= EXTEN_USBHD_IO_EN;
	RCC_USBCLKConfig(RCC_USBCLKSource);             //USBclk=PLLclk/1.5=48Mhz
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHD,NewState);
}

/*******************************************************************************
* Function Name  : USB_DevTransProcess
* Description    : USB device transfer process.
* Input          : None
* Return         : None
*******************************************************************************/
void USB_DevTransProcess( void )
{
	UINT8  len, chtype;
	UINT8  intflag, errflag = 0;

	intflag = R8_USB_INT_FG;

	if( intflag & RB_UIF_TRANSFER )
	{
		switch ( R8_USB_INT_ST & MASK_UIS_TOKEN)
		{
			case UIS_TOKEN_SETUP:
				R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
				len = R8_USB_RX_LEN;

				if ( len == sizeof( USB_SETUP_REQ ) )
				{
					SetupReqLen = pSetupReqPak->wLength;
					SetupReqCode = pSetupReqPak->bRequest;
					chtype = pSetupReqPak->bRequestType;

					len = 0;
					errflag = 0;
					if ( ( pSetupReqPak->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
					{
						errflag = 0xFF;
					}
					else
					{
						switch( SetupReqCode )
						{
							case USB_GET_DESCRIPTOR:
							{
								switch( ((pSetupReqPak->wValue)>>8) )
								{
									case USB_DESCR_TYP_DEVICE:
										pDescr = MyDevDescr;
										len = MyDevDescr[0];
										break;

									case USB_DESCR_TYP_CONFIG:
										pDescr = MyCfgDescr;
										len = MyCfgDescr[2];
										break;

									case USB_DESCR_TYP_STRING:
										switch( (pSetupReqPak->wValue)&0xff )
										{
											case 1:
												pDescr = MyManuInfo;
												len = MyManuInfo[0];
												break;

											case 2:
												pDescr = MyProdInfo;
												len = MyProdInfo[0];
												break;

											case 0:
												pDescr = MyLangDescr;
												len = MyLangDescr[0];
												break;

											case MidiJackInStrId:
											    pDescr = MidiJackInStr;
											    len = MidiJackInStr[0];
											    break;
                                            case MidiJackOutStrId:
                                                pDescr = MidiJackOutStr;
                                                len = MidiJackOutStr[0];
                                                break;

											default:
												errflag = 0xFF;
												break;
										}
										break;

									default :
										errflag = 0xff;
										break;
								}

								if( SetupReqLen>len )	SetupReqLen = len;
								len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
								memcpy( pEP0_DataBuf, pDescr, len );
								pDescr += len;
							}
								break;

							case USB_SET_ADDRESS:
								SetupReqLen = (pSetupReqPak->wValue)&0xff;
								break;

							case USB_GET_CONFIGURATION:
								pEP0_DataBuf[0] = DevConfig;
								if ( SetupReqLen > 1 ) SetupReqLen = 1;
								break;

							case USB_SET_CONFIGURATION:
								DevConfig = (pSetupReqPak->wValue)&0xff;
								break;

							case USB_CLEAR_FEATURE:
								if ( ( pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
								{
									switch( (pSetupReqPak->wIndex)&0xff )
									{
									case 0x82:
										R8_UEP2_CTRL = (R8_UEP2_CTRL & ~( RB_UEP_T_TOG|MASK_UEP_T_RES )) | UEP_T_RES_NAK;
										break;

									case 0x02:
										R8_UEP2_CTRL = (R8_UEP2_CTRL & ~( RB_UEP_R_TOG|MASK_UEP_R_RES )) | UEP_R_RES_ACK;
										break;

									case 0x81:
										R8_UEP1_CTRL = (R8_UEP1_CTRL & ~( RB_UEP_T_TOG|MASK_UEP_T_RES )) | UEP_T_RES_NAK;
										break;

									case 0x01:
										R8_UEP1_CTRL = (R8_UEP1_CTRL & ~( RB_UEP_R_TOG|MASK_UEP_R_RES )) | UEP_R_RES_ACK;
										break;

									default:
										errflag = 0xFF;
										break;

									}
								}
								else	errflag = 0xFF;
								break;

							case USB_GET_INTERFACE:
								pEP0_DataBuf[0] = 0x00;
								if ( SetupReqLen > 1 ) SetupReqLen = 1;
								break;

							case USB_GET_STATUS:
								pEP0_DataBuf[0] = 0x00;
								pEP0_DataBuf[1] = 0x00;
								if ( SetupReqLen > 2 ) SetupReqLen = 2;
								break;

							default:
								errflag = 0xff;
								break;
						}
					}
				}
				else	errflag = 0xff;

				if( errflag == 0xff)
				{
					R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;
				}
				else
				{
					if( chtype & 0x80 )
					{
						len = (SetupReqLen>DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
						SetupReqLen -= len;
					}
					else  len = 0;

					R8_UEP0_T_LEN = len;
					R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
				}
				break;

			case UIS_TOKEN_IN:
				switch ( R8_USB_INT_ST & ( MASK_UIS_TOKEN | MASK_UIS_ENDP ) )
				{
					case UIS_TOKEN_IN:
						switch( SetupReqCode )
						{
							case USB_GET_DESCRIPTOR:
									len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen;
									memcpy( pEP0_DataBuf, pDescr, len );
									SetupReqLen -= len;
									pDescr += len;
									R8_UEP0_T_LEN = len;
									R8_UEP0_CTRL ^= RB_UEP_T_TOG;
									break;

							case USB_SET_ADDRESS:
									R8_USB_DEV_AD = (R8_USB_DEV_AD&RB_UDA_GP_BIT) | SetupReqLen;
									R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
									break;

							default:
									R8_UEP0_T_LEN = 0;
									R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
									break;

						}
						break;

				case UIS_TOKEN_IN | 1:
					R8_UEP1_CTRL ^=  RB_UEP_T_TOG;
					R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
					break;

				case UIS_TOKEN_IN | 2:
					R8_UEP2_CTRL ^=  RB_UEP_T_TOG;
					R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
					break;

				case UIS_TOKEN_IN | 3:
					R8_UEP3_CTRL ^=  RB_UEP_T_TOG;
					R8_UEP3_CTRL = (R8_UEP3_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
					break;

				case UIS_TOKEN_IN | 4:
					R8_UEP4_CTRL ^=  RB_UEP_T_TOG;
					R8_UEP4_CTRL = (R8_UEP4_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
					break;

				case UIS_TOKEN_IN | 5:
					R8_UEP5_CTRL ^=  RB_UEP_T_TOG;
					R8_UEP5_CTRL = (R8_UEP5_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
					break;

				case UIS_TOKEN_IN | 6:
					R8_UEP6_CTRL ^=  RB_UEP_T_TOG;
					R8_UEP6_CTRL = (R8_UEP6_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
					break;

				case UIS_TOKEN_IN | 7:
					R8_UEP7_CTRL ^=  RB_UEP_T_TOG;
					R8_UEP7_CTRL = (R8_UEP7_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
					break;

				default :
					break;

				}
				break;

			case UIS_TOKEN_OUT:
				switch ( R8_USB_INT_ST & ( MASK_UIS_TOKEN | MASK_UIS_ENDP ) )
				{
					case UIS_TOKEN_OUT:
							len = R8_USB_RX_LEN;
							break;

					case UIS_TOKEN_OUT | 1:
						if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
						{
							R8_UEP1_CTRL ^= RB_UEP_R_TOG;
							len = R8_USB_RX_LEN;
							DevEP1_OUT_Deal( len );
						}
						break;

					case UIS_TOKEN_OUT | 2:
						if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
						{
							R8_UEP2_CTRL ^= RB_UEP_R_TOG;
							len = R8_USB_RX_LEN;
							DevEP2_OUT_Deal( len );
						}
						break;

					case UIS_TOKEN_OUT | 3:
						if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
						{
							R8_UEP3_CTRL ^= RB_UEP_R_TOG;
							len = R8_USB_RX_LEN;
							DevEP3_OUT_Deal( len );
						}
						break;

					case UIS_TOKEN_OUT | 4:
						if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
						{
							R8_UEP4_CTRL ^= RB_UEP_R_TOG;
							len = R8_USB_RX_LEN;
							DevEP4_OUT_Deal( len );
						}
						break;

					case UIS_TOKEN_OUT | 5:
						if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
						{
							R8_UEP5_CTRL ^= RB_UEP_R_TOG;
							len = R8_USB_RX_LEN;
							DevEP5_OUT_Deal( len );
						}
						break;

					case UIS_TOKEN_OUT | 6:
						if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
						{
							R8_UEP6_CTRL ^= RB_UEP_R_TOG;
							len = R8_USB_RX_LEN;
							DevEP6_OUT_Deal( len );
						}
						break;

					case UIS_TOKEN_OUT | 7:
						if ( R8_USB_INT_ST & RB_UIS_TOG_OK )
						{
							R8_UEP7_CTRL ^= RB_UEP_R_TOG;
							len = R8_USB_RX_LEN;
							DevEP7_OUT_Deal( len );
						}
						break;
				}

				break;

			case UIS_TOKEN_SOF:

				break;

			default :
				break;

		}


		R8_USB_INT_FG = RB_UIF_TRANSFER;
	}
	else if( intflag & RB_UIF_BUS_RST )
	{
		R8_USB_DEV_AD = 0;
		R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
		R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
		R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
		R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
		R8_UEP4_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
		R8_UEP5_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
		R8_UEP6_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
		R8_UEP7_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;		
		
		R8_USB_INT_FG |= RB_UIF_BUS_RST;
	}
	else if( intflag & RB_UIF_SUSPEND )
	{
		if ( R8_USB_MIS_ST & RB_UMS_SUSPEND ) {;}
		else{;}
		R8_USB_INT_FG = RB_UIF_SUSPEND;
	}
	else
	{
		R8_USB_INT_FG = intflag;
	}
}

void RCC_Configuration(void)
{
    /* GPIOA, GPIOB clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void TIM2_IRQHandler()
{


    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        Player32kProc(&mPlayer);
    }
    //GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void TIMER_Config(void)
{
    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Prescaler = 1;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = 1124;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timerInitStructure);
    TIM_ClearFlag(TIM2, TIM_IT_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);

    NVIC_InitTypeDef nvicStructure;
    nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

//TIM3 PWM部分初始化
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM3_PWM_Init(u16 arr, u16 psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);                        //使能定时器3时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE); //使能GPIO外设和AFIO复用功能模块时钟

    //GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); //Timer3部分重映射  TIM3_CH2->PB5

    //设置该引脚为复用输出功能,输出TIM3 CH2的PWM脉冲波形        GPIOB.5
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;       //TIM_CH2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化GPIO

    //        GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
    //设置该引脚为复用输出功能,输出TIM3 CH3的PWM脉冲波形        GPIOB.0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;       //TIM_CH3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化GPIO

    //初始化TIM3
    TIM_TimeBaseStructure.TIM_Period = arr;                     //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler = psc;                  //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;                //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);             //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

    //初始化TIM3 Channel2 PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;             //选择定时器模式:TIM脉冲宽度调制模式2
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     //输出极性:TIM输出比较极性高
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);                      //根据T指定的参数初始化外设TIM3 OC2
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);             //使能TIM3在CCR2上的预装载寄存器

    //初始化TIM3 Channel2 PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;             //选择定时器模式:TIM脉冲宽度调制模式2
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;      //输出极性:TIM输出比较极性高
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);                      //根据T指定的参数初始化外设TIM3 OC3
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_Cmd(TIM3, ENABLE); //使能TIM3
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);

	printf("USBHD Device Test\r\n");
	pEP0_RAM_Addr = EP0_Databuf;
	pEP1_RAM_Addr = EP1_Databuf;
	pEP2_RAM_Addr = EP2_Databuf;
	pEP3_RAM_Addr = EP3_Databuf;
	pEP4_RAM_Addr = EP4_Databuf;
	pEP5_RAM_Addr = EP5_Databuf;
	pEP6_RAM_Addr = EP6_Databuf;
	pEP7_RAM_Addr = EP7_Databuf;	
	
	uint16_t MyCfgDescrSize =sizeof(MyCfgDescr);
	MyCfgDescr[2] = MyCfgDescrSize & 0xFF;
    MyCfgDescr[3] = ( MyCfgDescrSize >> 8 ) & 0xFF;
    RCC_Configuration();
    GPIO_Configuration();
    PlayerInit(&mPlayer);
    PlayerPlay(&mPlayer);
    TIM3_PWM_Init(1023, 0);
    TIMER_Config();

	USBHD_ClockCmd(RCC_USBCLKSource_PLLCLK_1Div5,ENABLE);
	USB_DeviceInit();
	NVIC_EnableIRQ(USBHD_IRQn);

	while(1)
  {
	    PlayerProcess(&mPlayer);
  }

}

/*******************************************************************************
* Function Name  : DevEP1_OUT_Deal
* Description    : Deal device Endpoint 1 OUT.
* Input          : l: Data length.
* Return         : None
*******************************************************************************/
void DevEP1_OUT_Deal( UINT8 l )
{
	UINT8 i;

	for(i=0; i<l; i++)
	{
		pEP1_IN_DataBuf[i] = ~pEP1_OUT_DataBuf[i];
	}

	DevEP1_IN_Deal( l );
}

/*******************************************************************************
* Function Name  : DevEP2_OUT_Deal
* Description    : Deal device Endpoint 2 OUT.
* Input          : l: Data length.
* Return         : None
*******************************************************************************/
void DevEP2_OUT_Deal( UINT8 l )
{
	UINT8 i;

	for(i=0; i<l; i++)
	{
		pEP2_IN_DataBuf[i] = ~pEP2_OUT_DataBuf[i];
	}

	DevEP2_IN_Deal( l );
}

/*******************************************************************************
* Function Name  : DevEP3_OUT_Deal
* Description    : Deal device Endpoint 3 OUT.
* Input          : l: Data length.
* Return         : None
*******************************************************************************/
void DevEP3_OUT_Deal( UINT8 l )
{
	UINT8 i;

	for(i=0; i<l; i++)
	{
		pEP3_IN_DataBuf[i] = ~pEP3_OUT_DataBuf[i];
	}

	DevEP3_IN_Deal( l );
}

/*******************************************************************************
* Function Name  : DevEP4_OUT_Deal
* Description    : Deal device Endpoint 4 OUT.
* Input          : l: Data length.
* Return         : None
*******************************************************************************/
void DevEP4_OUT_Deal( UINT8 l )
{
	UINT8 i;

	for(i=0; i<l; i++)
	{
		pEP4_IN_DataBuf[i] = ~pEP4_OUT_DataBuf[i];
	}

	DevEP4_IN_Deal( l );
}

/*******************************************************************************
* Function Name  : DevEP5_OUT_Deal
* Description    : Deal device Endpoint 5 OUT.
* Input          : l: Data length.
* Return         : None
*******************************************************************************/
void DevEP5_OUT_Deal( UINT8 l )
{
	UINT8 i;

	for(i=0; i<l; i++)
	{
		pEP5_IN_DataBuf[i] = ~pEP5_OUT_DataBuf[i];
	}

	DevEP5_IN_Deal( l );
}

/*******************************************************************************
* Function Name  : DevEP6_OUT_Deal
* Description    : Deal device Endpoint 6 OUT.
* Input          : l: Data length.
* Return         : None
*******************************************************************************/
void DevEP6_OUT_Deal( UINT8 l )
{
	UINT8 i;

	for(i=0; i<l; i++)
	{
		pEP6_IN_DataBuf[i] = ~pEP6_OUT_DataBuf[i];
	}

	DevEP6_IN_Deal( l );
}

/*******************************************************************************
* Function Name  : DevEP7_OUT_Deal
* Description    : Deal device Endpoint 7 OUT.
* Input          : l: Data length.
* Return         : None
*******************************************************************************/
void DevEP7_OUT_Deal( UINT8 l )
{
	UINT8 i;

	for(i=0; i<l; i++)
	{
		pEP7_IN_DataBuf[i] = ~pEP7_OUT_DataBuf[i];
	}

	DevEP7_IN_Deal( l );
}

/*******************************************************************************
* Function Name  : USB_IRQHandler
* Description    : This function handles USB exception.
* Input          : None
* Return         : None
*******************************************************************************/
void USBHD_IRQHandler (void)
{
	USB_DevTransProcess();
}

