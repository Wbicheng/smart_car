#include "common.h"
#include "include.h"
#include "allinit.h"


struct WSNSTATE  
{
unsigned diskok:1; 
unsigned fileok:1; 
unsigned record:1; 
unsigned position:1; 
};
volatile struct WSNSTATE state; 
FATFS fs;            // Work area (file system object) for logical drive
FRESULT res;         // FatFs function common result code
UINT br,bw;         // File Write count
FIL testdata;  
FILINFO* sfil;
FRESULT rc;

/************ͼ��****************/
uint8 imgbuff[CAMERA_SIZE];                             //����洢����ͼ�������
uint8 img[CAMERA_H][CAMERA_W];
int DMA_Over_Flag=0;
uint8 *imgdata;

/*************LCD****************/
Site_t site     = {0, 0};                           //��ʾͼ�����Ͻ�λ��
Size_t imgsize  = {CAMERA_W, CAMERA_H};             //ͼ���С
Size_t size;
   
//��������
void PORTA_IRQHandler();
void DMA0_IRQHandler();

/*************���뿪��*************/
uint8 BM1=0; //D4
uint8 BM2=0; //D3
uint8 BM3=0; //D2
uint8 BM4=0; //D1
uint8 deepblue=0; //ģʽ
uint8 fznum=0;

//SD����ʼ��
void init_disk_fatfs(void)   //state.diskok==1;
{
	if(state.diskok==0)  
	{
		if(disk_initialize(0))
		{
			state.diskok=0;
		}
		else
		{	
			res = f_mount(0,&fs);      //for logical drive 0
//			SCI0_putchar(res);
			state.diskok=1;
		}
	}
}

void allinit()
{
  
    DisableInterrupts;
  /*********************LCD************************/
    ASSERT(enter_init());           //����Ƿ���¼FWD
   /********************���뿪��*************************/
      //���뿪�س�ʼ��
    gpio_init (PTD4,GPI,0);  //1
    gpio_init (PTD3,GPI,0);  //2
    gpio_init (PTD2,GPI,0); //3
    gpio_init (PTD1,GPI,0); //4
      
    //��ȡ���뿪��״̬
    BM1=gpio_get(PTD4);
    BM2=gpio_get(PTD3);
    BM3=gpio_get(PTD2);
    BM4=gpio_get(PTD1);
    
    //���뿪�ض�Ӧģʽ   
    if(!BM1 && !BM2 && !BM3 && BM4  )      //ģʽ1  ����160 ֱ��200
    { 
       deepblue=1; 
    }
    if(!BM1 && !BM2 && BM3 && !BM4  )   //ģʽ2  ����165 ֱ��200
    {
       deepblue=2;
    }
    if(!BM1 && !BM2 && BM3 && BM4  )    //ģʽ3 ����165 ֱ��220
    {
       deepblue=3;
    }
     if(!BM1 && BM2 && !BM3 && !BM4  )  //ģʽ4 ����165 ֱ��250
    {
       deepblue=4;
    }
     if(!BM1 && BM2 && !BM3 && BM4  )      //ģʽ5 ����170 ֱ��200
    {
       deepblue=5;
    }
   if(!BM1 && BM2 && BM3 && !BM4  )   //ģʽ6 ����170 ֱ��220
    {
       deepblue=6;
    }
    if(!BM1 && BM2 && BM3 && BM4  )    //ģʽ7 ����170 ֱ��250
    {
       deepblue=7;
    }
     if(BM1 && !BM2 && !BM3 && !BM4  )  //ģʽ8 ����175 ֱ��200
    {
       deepblue=8; 
    }
        if(BM1 && !BM2 && !BM3 && BM4  )  //ģʽ9 ����175 ֱ��220
    {
       deepblue=9;
    }
        if(BM1 && !BM2 && BM3 && !BM4  )  //ģʽ10 ����175 ֱ��250
    {
       deepblue=10;
    }
        if(BM1 && !BM2 && BM3 && BM4  )  //ģʽ11 ����180 ֱ��200
    {
       deepblue=11; 
    }
        if(BM1 && BM2 && !BM3 && !BM4  )  //ģʽ12 ����180 ֱ��220
    {
       deepblue=12;
    }
        if(BM1 && BM2 && !BM3 && BM4  )  //ģʽ13 ����180 ֱ��250
    {
       deepblue=13;
    }

    LCD_init();
   // LCD_Str_ENCH            (site,"ӥ�����ڳ�ʼ��",FCOLOUR,BCOLOUR);

    camera_init(imgbuff);
    //LCD_Str_ENCH            (site,"ӥ�۳�ʼ���ɹ�,׼���ɼ�",FCOLOUR,BCOLOUR);
    site.y = 110;
    LCD_FSTR_CH(site,vcan_str,FCOLOUR,BCOLOUR);
    site.y = 0;
    /*******************��ת����***********************/
 //   fznum=50;
   switch(deepblue)
    {
    case 1:fznum=50;
    case 2:fznum=60;
    case 3:fznum=60;
    case 4:fznum=60;
    case 5:fznum=60;
    case 6:fznum=60;
    case 7:fznum=60;
    case 8:fznum=60;
    case 9:fznum=60;
    case 10:fznum=60;
    case 11:fznum=60;
    case 12:fznum=60;
    case 13:fznum=60;
    
    
    } 
    /*****************���************************/
    ftm_pwm_init(FTM3,FTM_CH5,300,duojiMid);
    /*****************���************************/
    ftm_pwm_init(FTM0,FTM_CH2,10000,dianjispeed);
    ftm_pwm_init(FTM0,FTM_CH3,10000,0);
    /*****************����ͷ**********************/
        //�����жϷ�����
    set_vector_handler(PORTA_VECTORn , PORTA_IRQHandler);   //���� PORTA ���жϷ�����Ϊ PORTA_IRQHandler
    set_vector_handler(DMA0_VECTORn , DMA0_IRQHandler);     //���� DMA0 ���жϷ�����Ϊ PORTA_IRQHandler
    /*****************������**********************/
    Encoder(right_Encoder,10);
    /******************������***********************/
     gpio_init (PTD0,GPO,1);
    /******************sd��***********************/
    //SPI_Configuration();
    Site_t sitelcd={0,0};
    LCD_Str_ENCH(sitelcd,"UartInit is OK!",FCOLOUR,BCOLOUR);
    uart_putchar(UART0,0xff);
    uart_putstr(UART0,"UartInit is OK!");
  /* init_disk_fatfs();
    if(state.diskok)
    {
      sitelcd.y=10;
      LCD_Str_ENCH(sitelcd,"init_disk is OK!",FCOLOUR,BCOLOUR); 
      uart_putstr(UART0,"init_disk is OK!");
    } 

     rc =f_open(&testdata,"0:/data2.txt",FA_OPEN_ALWAYS|FA_WRITE | FA_READ);
      if(FR_OK == rc)
      {
        sitelcd.y=20;
        LCD_Str_ENCH(sitelcd,"f_open is ok",FCOLOUR,BCOLOUR);
        uart_putstr(UART0,"f_open is OK!");
      }
      else 
      {
        sitelcd.y=30;
        LCD_Str_ENCH(sitelcd,"no SDsave!",FCOLOUR,BCOLOUR);
        uart_putstr(UART0,"no SDsave!");
      }
       LCD_num(sitelcd,rc,FCOLOUR,BCOLOUR);
      while(1);
    */
    EnableInterrupts;
}

/*!
 *  @brief      PORTA�жϷ�����
 *  @since      v6.0
 */
void PORTA_IRQHandler()
{
    uint8  n;    //���ź�
    uint32 flag;

    while(!PORTA_ISFR);
    flag = PORTA_ISFR;
    PORTA_ISFR  = ~0;                                   //���жϱ�־λ

    n = 29;                                             //���ж�
    if(flag & (1 << n))                                 //PTA29�����ж�
    {
        camera_vsync();
    }
#if ( CAMERA_USE_HREF == 1 )                            //ʹ�����ж�
    n = 28;
    if(flag & (1 << n))                                 //PTA28�����ж�
    {
        camera_href();
    }
#endif


}

/*!
 *  @brief      DMA0�жϷ�����
 *  @since      v6.0
 */
void DMA0_IRQHandler()
{
    camera_dma();
}



