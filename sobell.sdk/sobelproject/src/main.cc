/*
 * Empty C++ Application
 */
#include <stdio.h>
#include "XAxiDma.h"
#include "zebracode.h"
#include <xnewsob.h>

//base address for the ddr
//memory used by DMA from the ddr

#define ADDR_OFFSET 0x01000000
#define TX (ADDR_OFFSET + 0x00100000)
#define RX (ADDR_OFFSET + 0x00300000)
#define TOTAL_PIXEL (1024*1024)
//memory used by dma from ddr


unsigned char *Transmission_Buffer = (unsigned char*) TX;
unsigned char *Receiver_Buffer = (unsigned char*) RX;
unsigned char Input_Image[TOTAL_PIXEL];


XAxiDma MainDma;

int initialization_dma()
{
	XAxiDma_Config *ConfigDma;
	ConfigDma = XAxiDma_LookupConfig(XPAR_AXI_DMA_0_DEVICE_ID);
	XAxiDma_CfgInitialize(&MainDma,ConfigDma);
	//disabling the maindma interrupts
	XAxiDma_IntrDisable(&MainDma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&MainDma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DMA_TO_DEVICE);
	return XST_SUCCESS;

}
//end of intializing dma
XNewsob SobVal;

int initialization_newsob()
{
	int state;

	XNewsob_Config *ConfigNewsob;
	ConfigNewsob = XNewsob_LookupConfig(XPAR_NEWSOB_0_DEVICE_ID);
	if(!ConfigNewsob){
		printf("configuration error\n");
	}
	state = XNewsob_CfgInitialize(&SobVal,ConfigNewsob);
	if(state != XST_SUCCESS){
		printf("initialization error\n");

	}
	return state;
}

int main()
{
	//initialize the maindma
	initialization_dma();
	initialization_newsob();

	//get the data from header zebracode.h
	for(int eachpixel =0;eachpixel <TOTAL_PIXEL;eachpixel++){
		Input_Image[eachpixel] = Img[eachpixel];
	}

	printf("starting hardware......\n");
	XNewsob_Start(&SobVal);



	Xil_DCacheFlushRange((u32)Input_Image,TOTAL_PIXEL*sizeof(unsigned char));
	Xil_DCacheFlushRange((u32)Receiver_Buffer,TOTAL_PIXEL*sizeof(unsigned char));


	XAxiDma_SimpleTransfer(&MainDma,(u32)Input_Image,TOTAL_PIXEL*sizeof(unsigned char),XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_SimpleTransfer(&MainDma,(u32)Receiver_Buffer,TOTAL_PIXEL*sizeof(unsigned char),XAXIDMA_DEVICE_TO_DMA);

	//check whether the dma is busy or not
	while(XAxiDma_Busy(&MainDma,XAXIDMA_DMA_TO_DEVICE));
	while(XAxiDma_Busy(&MainDma,XAXIDMA_DEVICE_TO_DMA));

	Xil_DCacheInvalidateRange((u32)Receiver_Buffer,TOTAL_PIXEL*sizeof(unsigned char));

	printf("stopping the hardware....");


	return 0;




}
