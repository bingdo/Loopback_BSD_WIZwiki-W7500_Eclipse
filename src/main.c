/**
  ******************************************************************************
  * @file    main.c
  * @author  IOP Team
  * @version V1.0.0
  * @date    01-May-2015
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, WIZnet SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2015 WIZnet Co.,Ltd.</center></h2>
  ******************************************************************************
  */ 
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "W7500x_uart.h"
#include "W7500x_crg.h"
#include "W7500x_wztoe.h"
#include "W7500x_miim.h"
#include "wizchip_conf.h"
#include "loopback.h"
/* Private typedef -----------------------------------------------------------*/
UART_InitTypeDef UART_InitStructure;

/* Private define ------------------------------------------------------------*/
#define __DEF_USED_MDIO__ 
#define __DEF_USED_IC101AG__ //for W7500 Test main Board V001

///////////////////////////////////////
// Debugging Message Printout enable //
///////////////////////////////////////
#define _MAIN_DEBUG_

///////////////////////////
// Demo Firmware Version //
///////////////////////////
#define VER_H		1
#define VER_L		01

//////////////////////////////////////////////////
// Socket & Port number definition for Examples //
//////////////////////////////////////////////////
#define SOCK_TCPS       0
#define SOCK_UDPS       1
#define PORT_TCPS		5000
#define PORT_UDPS       3000

/* Private function prototypes -----------------------------------------------*/
void delay(__IO uint32_t milliseconds); //Notice: used ioLibray
void TimingDelay_Decrement(void);

/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;
uint8_t test_buf[2048];

// For TCP client loopback examples; destination network info
uint8_t destip[4] = {192, 168, 0, 2};
uint16_t destport = 5001;

#if LOOPBACK_MODE == LOOPBACK_MAIN_NOBLOCK
#if defined(TEST_TCPS)
extern int32_t loopback_tcps_bsd(uint8_t sn, uint8_t* buf, uint16_t port);
#elif defined(TEST_UDPS)
extern int32_t loopback_udps_bsd(uint8_t sn, uint8_t* buf, uint16_t port);
#elif defined(TEST_TCPC)
extern int32_t loopback_tcpc_bsd(uint8_t sn, uint8_t* buf, struct sockaddr *sa);
#endif
#elif LOOPBACK_MODE == LOOPBACK_MAIN_SAMPLE
#if defined(TEST_TCPS)
extern int32_t loopback_tcps_bsd(uint8_t* buf);
#elif defined(TEST_TCPC)
extern int32_t loopback_tcpc_bsd(uint8_t* buf);
#endif
#endif

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int main()
{
    //uint8_t tx_size[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };
    //uint8_t rx_size[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };
    uint8_t mac_addr[6] = {0x00, 0x08, 0xDC, 0x11, 0x22, 0x34};
    uint8_t src_addr[4] = {192, 168,  0,  80};
    uint8_t gw_addr[4]  = {192, 168,  0,  1};
    uint8_t sub_addr[4] = {255, 255, 255,  0};		
    //uint8_t dns_server[4] = {8, 8, 8, 8};           // for Example domain name server
    uint8_t tmp[8];
#if LOOPBACK_MODE == LOOPBACK_MAIN_NOBLOCK
    int ret;
    uint8_t sn;
#if defined(TEST_TCPC)
    int32_t loopback_ret;
    struct sockaddr_in sa;
    uint32_t s_addr;
#endif
#endif

    // For TCP client loopback examples; destination network info
    //uint8_t destip[4] = {192, 168, 0, 2};
    //uint16_t destport = 5000;
    //int32_t loopback_ret;

    /* External Clock */
    CRG_PLL_InputFrequencySelect(CRG_OCLK);

    /* Set Systme init */
    SystemInit();

    /* UART Init */
    UART_StructInit(&UART_InitStructure);
    UART_Init(UART1,&UART_InitStructure);

    /* SysTick_Config */
    SysTick_Config((GetSystemClock()/1000));

    /* Set WZ_100US Register */
    setTIC100US((GetSystemClock()/10000));
    //getTIC100US();	
    //printf(" GetSystemClock: %X, getTIC100US: %X, (%X) \r\n", 
    //      GetSystemClock, getTIC100US(), *(uint32_t *)TIC100US);        

#ifdef _MAIN_DEBUG_
	uint8_t tmpstr[6] = {0,};

	ctlwizchip(CW_GET_ID,(void*)tmpstr);
    printf("\r\n============================================\r\n");
	printf(" WIZnet %s EVB Demo v%d.%.2d\r\n", tmpstr, VER_H, VER_L);
	printf("============================================\r\n");
	printf(" WIZwiki Platform based Loopback Example\r\n");
	printf("============================================\r\n");
#endif

#ifdef __DEF_USED_IC101AG__ //For using IC+101AG
    *(volatile uint32_t *)(0x41003068) = 0x64; //TXD0 - set PAD strengh and pull-up
    *(volatile uint32_t *)(0x4100306C) = 0x64; //TXD1 - set PAD strengh and pull-up
    *(volatile uint32_t *)(0x41003070) = 0x64; //TXD2 - set PAD strengh and pull-up
    *(volatile uint32_t *)(0x41003074) = 0x64; //TXD3 - set PAD strengh and pull-up
    *(volatile uint32_t *)(0x41003050) = 0x64; //TXE  - set PAD strengh and pull-up
#endif

#ifdef __DEF_USED_MDIO__ 
    /* mdio Init */
    mdio_init(GPIOB, MDC, MDIO );
    /* PHY Link Check via gpio mdio */
    while( link() == 0x0 )
    {
        printf(".");  
        delay(500);
    }
    printf("PHY is linked. \r\n");  
#else
    delay(1000);
    delay(1000);
#endif

    /* Network Configuration */
    setSHAR(mac_addr);
    setSIPR(src_addr);
    setGAR(gw_addr);
    setSUBR(sub_addr);

    getSHAR(tmp);
    printf("MAC ADDRESS : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\r\n",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
    getSIPR(tmp);
    printf("IP ADDRESS : %.3d.%.3d.%.3d.%.3d\r\n",tmp[0],tmp[1],tmp[2],tmp[3]); 
    getGAR(tmp);
    printf("GW ADDRESS : %.3d.%.3d.%.3d.%.3d\r\n",tmp[0],tmp[1],tmp[2],tmp[3]); 
    getSUBR(tmp);
    printf("SN MASK: %.3d.%.3d.%.3d.%.3d\r\n",tmp[0],tmp[1],tmp[2],tmp[3]); 

    /* Set Network Configuration */
    //wizchip_init(tx_size, rx_size);

#if LOOPBACK_MODE == LOOPBACK_MAIN_NOBLOCK
#if defined(TEST_TCPS) || defined(TEST_TCPC)
	ret = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#elif defined(TEST_UDPS)
    ret = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
#endif
#if defined(TEST_TCPC)
    memset(&sa, 0, sizeof(sa));
    sa.sin_len = (uint16_t)sizeof(sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(destport);
    s_addr = (((((destip[0] << 8) | destip[1]) << 8) | destip[2]) << 8) | destip[3];
    sa.sin_addr.s_addr = htonl(s_addr);
#endif

    if(ret == -1)
    {
#ifdef _LOOPBACK_DEBUG_
        printf("%d:Socket Error\r\n", ret);
#endif
        return ret;
    }
    sn = ret;
#endif

    while(1)
    {
		/* Loopback Test: TCP Server and UDP */
#if LOOPBACK_MODE == LOOPBACK_MAIN_NOBLOCK
#if defined(TEST_TCPS)
			loopback_tcps_bsd(sn, gDATABUF, TEST_PORT);
#elif defined(TEST_UDPS)
			loopback_udps_bsd(sn, gDATABUF, TEST_PORT);
#elif defined(TEST_TCPC)
			loopback_ret = loopback_tcpc_bsd(sn, gDATABUF, (struct sockaddr*)&sa);
			if(loopback_ret < 0) printf("loopback ret: %ld\r\n", loopback_ret); // TCP Socket Error code
#endif
#elif LOOPBACK_MODE == LOOPBACK_MAIN_SAMPLE
#if defined(TEST_TCPS)
			loopback_tcps_bsd(test_buf);
#elif defined(TEST_TCPC)
			loopback_tcpc_bsd(test_buf);
#endif
#endif
    }

}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay(__IO uint32_t milliseconds)
{
  TimingDelay = milliseconds;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

