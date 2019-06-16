#include "delay.h"		 	 
#include "lcd.h"  
#include "key.h"      
#include "sdio_sdcard.h"  
#include "w25qxx.h"       
#include "text.h" 
#include "tpad.h"	   
#include "recorder.h"	 
#include "24cxx.h"
#include "touch.h"
#include "led.h" 

#include "vs10xx.h"
#include "mp3player.h"	
#include "SendData.h"
#include "sram.h"	 

#include "malloc.h"
#include "string.h"
#include "exfuns.h"  
#include "ff.h"   

#include "mrrlcd.h"

#include "SendData.h"

extern void mp3playREC(u8 playindex);
extern void mp3play(u8 playindex);
extern void mp3playRECEIVE(u8 playindex);

/********************************************************/

 //5�����ص����ɫ												 
//const u16 POINT_COLOR_TBL[CT_MAX_TOUCH]={RED,GREEN,BLUE,BROWN,GRED};  
const u16 POINT_COLOR_TBL[CT_MAX_TOUCH]={RED}; 

/**************************************************************/

////////////////////////////////////////////////////////////////////////////////
//���ݴ�����ר�в���
//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//��ʵ��Բ
//x0,y0:����
//r:�뾶
//color:��ɫ
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  
//������֮��ľ���ֵ 
//x1,x2����ȡ��ֵ��������
//����ֵ��|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//��һ������
//(x1,y1),(x2,y2):��������ʼ����
//size�������Ĵ�ϸ�̶�
//color����������ɫ
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		gui_fill_circle(uRow,uCol,size,color);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}   
////////////////////////////////////////////////////////////////////////////////




//���ݴ��������Ժ���
void ctp_test(void)
{
	u8 t=0;
	u8 i=0;	  	    
 	u16 lastpos[5][2];		//��¼���һ�ε����� 
	while(1)
	{
		tp_dev.scan(0);
		for(t=0;t<CT_MAX_TOUCH;t++)
		{
			if((tp_dev.sta)&(1<<t))
			{
				if(tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)
				{
					if(lastpos[t][0]==0XFFFF)
					{
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//����
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<16)
					{
						Load_Drow_Dialog();//���
					}
				}
			}else lastpos[t][0]=0XFFFF;
		}
		
		delay_ms(5);i++;
		if(i%20==0)LED0=!LED0;
	}	
}



// ������Ϣ��ʾ
void mrrlcd_msg_show(void)
{
	POINT_COLOR=RED;//��������Ϊ��ɫ
	Show_Str(30,50,480,24,"���裺����MEMS�����ͨ��ϵͳ���",24,0);	
	Show_Str(30,80,480,24,"����������ѧ",24,0);	
	Show_Str(30,110,480,24,"���繤��ѧԺ  ��е����רҵ",24,0);	
	Show_Str(30,140,480,24,"ѧ����������  ָ����ʦ��������",24,0);	
	//LCD_DrawLine(30, 170, 450, 170);
	lcd_draw_bline(25,45, 450, 45,1.8,BRRED);
	lcd_draw_bline(450,45, 450, 170,1.8,BRRED);
	lcd_draw_bline(25,170, 450, 170,1.8,BRRED);
	lcd_draw_bline(25,45, 25, 170,1.8,BRRED);
	Show_Str(30,180,480,16,"���ܣ�¼�������ֲ��š���Ƶ���봫�����Ƶ���ս���",16,0);	
	Show_Str(30,200,480,16,"���Ʒ�ʽ��OOK  ¼���洢�ļ���ʽΪ��WAV",16,0);	
	POINT_COLOR=BLUE;//��������Ϊ��ɫ
	Show_Str(30,220,480,16,"��ɫ���ǿ��ư�ť���ɽ��д��������������",16,0);	
	Show_Str(30,240,480,16,"�㰴���������½ǵ�ͷ�񣬿�ʵ�ַ�����һ��Ŀ¼���ܣ�",16,0);	
	//Show_Str(30,210,480,16,"¼���ļ���ʽΪ��WAV",16,0);
	POINT_COLOR=RED;//��������Ϊ��ɫ

	
	

}
//�����Ļ�������Ͻ���ʾ"RST"
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);	//����   
 	POINT_COLOR=BLUE;	//��������Ϊ��ɫ 
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//��ʾ��������
  	POINT_COLOR=RED;//��������Ϊ��ɫ
}

//����MEMS��������ϵͳ�������
void Recording_Dialog(void)
{
	
	//LCD_Clear(WHITE);	//����   
 	POINT_COLOR=BLUE;	//��������Ϊ��ɫ 
	//LCD_ShowString(lcddev.width-24,400,480,24,24,"RST");//��ʾ��������
	Show_Str(60,300,510,24,"���ֲ�����",24,0);
	LCD_DrawRectangle(30,270,(30+180),(270+84));
	
	Show_Str((450-180+30),300,480,24," ¼������ ",24,0);
	LCD_DrawRectangle((450-180),270,450,(270+84));
	
	Show_Str((450-180+30),400,480,24," ���⴫�� ",24,0);
	LCD_DrawRectangle((450-180),370,450,(370+84));
	
  	POINT_COLOR=RED;	//���û�����ɫ 
	

}

//	���ƿ��ư���
void draw_button(u16 y,u16 width,u8*str,u8 size)
{
	POINT_COLOR=BLUE;	//��������Ϊ��ɫ 
	LCD_Fill((450-16*5-100),(y-10),450-100+20,(y+size+10),WHITE);				//���֮ǰ�İ�����ʾ
	Show_Str(450-16*5-100+10,y,width,size,str,size,0);
	LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
	POINT_COLOR=RED;//��������Ϊ��ɫ
}

void draw_button_dir(u16 x,u16 y,u16 width,u8*str,u8 size)
{
	POINT_COLOR=BLUE;	//��������Ϊ��ɫ 
	Show_Str(x,y,width,size,str,size,0);
	LCD_DrawRectangle((x-20),(y-20),(x+7*16+20),(y+size+20));
	POINT_COLOR=RED;//��������Ϊ��ɫ
}

//����Ŀ¼
void draw_dir(u16 y,u8 checkkey)
{
	
	u8 res;u8 keyindex;
	u8 *fn;   			//���ļ���
	u8 *pname;			//��·�����ļ���
	u16 curindex;		//ͼƬ��ǰ����
	u16 temp;
	u16 *mp3indextbl;	//���������� 
	u16 totmp3num; 		//�����ļ�����
	 DIR mp3dir;	 		//Ŀ¼
	FILINFO mp3fileinfo;//�ļ���Ϣ

	totmp3num=mp3_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���

	keyindex=checkkey;
	
	//Show_Str(450-16*5-80,y-40,240,16,str,16,0);				//����
	//LCD_ShowNum(450-16*5-80,y-40,checkkey,2,16);
	
	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//Ϊ���ļ������������ڴ�
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
	
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
	
	while(f_opendir(&mp3dir,"0:/MUSIC"))//��¼���ļ���
 	{	 
		Show_Str(30,630+120,240,16,"�����ļ��д���!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,630+120,240,246,WHITE);		//�����ʾ	     
		delay_ms(200);				  
		f_mkdir("0:/MUSIC");				//������Ŀ¼ 
		f_closedir(&mp3dir);		
	} 
	
	
	//��¼����
    res=f_opendir(&mp3dir,"0:/MUSIC"); //��Ŀ¼
	
	if(res==FR_OK)
	{
		curindex=0;//��ǰ����Ϊ0
		while(1)//ȫ����ѯһ��
		{
			temp=mp3dir.index;								//��¼��ǰindex
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				mp3indextbl[curindex]=temp;//��¼����
				curindex++;
			}	    
		} 
	}   
	f_closedir(&mp3dir);
	
		
	Show_Str(450-16*5-80,y-20,240,16,"����Ŀ¼�б�",16,0);				//��ʾ�������� 
	//Show_Str(450-16*5-80-16*5,y+0,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
	Show_Str(450-16*5-80-16*5,y,240,16,"��ǰѡ��->",16,0);				//��ʾ�������� 
	//Show_Str(450-16*5-80-16*5,y+40,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
	Show_Str(450-16*5-80-16*5,y+60,240,16,"������½�ͷ�񷵻���һ��",16,0);
	//LCD_Fill(450-16*5-80-16*5,y+60,480,y+60+16,WHITE);				//���֮ǰ����ʾ
	
	
	if(keyindex>totmp3num)keyindex=0;
	if((keyindex-1)>totmp3num)keyindex=totmp3num-1;


 	curindex=keyindex;											//��0��ʼ��ʾ
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); 	//��Ŀ¼
	//if(res!=FR_OK)  LCD_Fill(30,50,800,480,WHITE);//�����ʾ
	//{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//�ı䵱ǰĿ¼����	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
        //if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		
		strcpy((char*)pname,"0:/MUSIC/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y,500,y+16,WHITE);				//���֮ǰ����ʾ
		Show_Str(450-16*5-80,y,240,16,pname,16,0);				//��ʾ�������� 
	
	
		f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/MUSIC/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y+20,500,y+20+16,WHITE);				//���֮ǰ����ʾ
		Show_Str(450-16*5-80,y+20,240,16,pname,16,0);				//��ʾ�������� 
			
		f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/MUSIC/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y+40,500,y+40+16,WHITE);				//���֮ǰ����ʾ
		Show_Str(450-16*5-80,y+40,240,16,pname,16,0);				//��ʾ�������� 
		
		
	//}
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�		
	myfree(SRAMIN,mp3indextbl);			//�ͷ��ڴ�
}

void draw_dir_record(u16 y,u8 checkkey)
{
	
	u8 res;u8 keyindex;
	u8 *fn;   			//���ļ���
	u8 *pname;			//��·�����ļ���
	u16 curindex;		//ͼƬ��ǰ����
	u16 temp;
	u16 *mp3indextbl;	//���������� 
	u16 totmp3num; 		//�����ļ�����
	 DIR mp3dir;	 		//Ŀ¼
	FILINFO mp3fileinfo;//�ļ���Ϣ

	totmp3num=mp3_get_tnum("0:/RECORDER"); //�õ�����Ч�ļ���

	keyindex=checkkey;
	
	//Show_Str(450-16*5-80,y-40,240,16,str,16,0);				//����
	//LCD_ShowNum(450-16*5-80,y-40,checkkey,2,16);
	//LCD_ShowNum(450-16*5-80,y-40,totmp3num,2,16);
	
	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//Ϊ���ļ������������ڴ�
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
	
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
	
		while(f_opendir(&mp3dir,"0:/RECORDER"))//��¼���ļ���
 	{	 
		Show_Str(30,630+120,240,16,"RECORDER�ļ��д���!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,630+120,240,246,WHITE);		//�����ʾ	     
		delay_ms(200);				  
		f_mkdir("0:/RECORDER");				//������Ŀ¼ 
		f_closedir(&mp3dir);		
	} 
	
	//��¼����
    res=f_opendir(&mp3dir,"0:/RECORDER"); //��Ŀ¼
	
	if(res==FR_OK)
	{
		curindex=0;//��ǰ����Ϊ0
		while(1)//ȫ����ѯһ��
		{
			temp=mp3dir.index;								//��¼��ǰindex
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				mp3indextbl[curindex]=temp;//��¼����
				curindex++;
			}	    
		} 
	}   
	f_closedir(&mp3dir);
	
		
	Show_Str(450-16*5-80,y-20,240,16,"¼��Ŀ¼�б�",16,0);				//��ʾ�������� 
	//Show_Str(450-16*5-80-16*5,y+0,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
	Show_Str(450-16*5-80-16*5,y,240,16,"��ǰѡ��->",16,0);				//��ʾ�������� 
	//Show_Str(450-16*5-80-16*5,y+40,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
	Show_Str(450-16*5-80-16*5,y+60,240,16,"������½�ͷ�񷵻���һ��",16,0);
	//LCD_Fill(450-16*5-80-16*5,y+60,500,y+60+16,WHITE);				//���֮ǰ����ʾ
	
	
	if(keyindex>totmp3num)keyindex=0;
	if((keyindex-1)>totmp3num)keyindex=totmp3num-1;


 	curindex=keyindex;											//��0��ʼ��ʾ
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/RECORDER"); 	//��Ŀ¼
	//if(res!=FR_OK)  LCD_Fill(30,50,800,480,WHITE);//�����ʾ
	//{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//�ı䵱ǰĿ¼����	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
        //if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		
		strcpy((char*)pname,"0:/RECORDER/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y,500,y+16,WHITE);				//���֮ǰ����ʾ
		Show_Str(450-16*5-80,y,240,16,pname,16,0);				//��ʾ�������� 
	
	
		f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/RECORDER/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y+20,500,y+20+16,WHITE);				//���֮ǰ����ʾ
		Show_Str(450-16*5-80,y+20,240,16,pname,16,0);				//��ʾ�������� 
			
		f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/RECORDER/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y+40,500,y+40+16,WHITE);				//���֮ǰ����ʾ
		Show_Str(450-16*5-80,y+40,240,16,pname,16,0);				//��ʾ�������� 
		
		
	//}
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�		
	myfree(SRAMIN,mp3indextbl);			//�ͷ��ڴ�
}

void draw_dir_txt(u16 y,u8 checkkey)
{
	
	u8 res;u8 keyindex;
	u8 *fn;   			//���ļ���
	u8 *pname;			//��·�����ļ���
	u16 curindex;		//ͼƬ��ǰ����
	u16 temp;
	u16 *mp3indextbl;	//���������� 
	u16 totmp3num; 		//�����ļ�����
	 DIR mp3dir;	 		//Ŀ¼
	FILINFO mp3fileinfo;//�ļ���Ϣ

	totmp3num=get_txtnum("0:/TEXT"); //�õ�����Ч�ļ���

	keyindex=checkkey;
	
	//Show_Str(450-16*5-80,y-40,240,16,str,16,0);				//����
	//LCD_ShowNum(450-16*5-80,y-40,checkkey,2,16);
	//LCD_ShowNum(450-16*5-80,y-40,totmp3num,2,16);
	
	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//Ϊ���ļ������������ڴ�
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
	
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
	
	//��¼����
	
	  while(f_opendir(&mp3dir,"0:/TEXT"))//��¼���ļ���
 	{	 
		Show_Str(30,630+120,240,16,"TEXT�ļ��д���!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,630+120,240,246,WHITE);		//�����ʾ	     
		delay_ms(200);				  
		f_mkdir("0:/TEXT");				//������Ŀ¼ 
		f_closedir(&mp3dir);		
	} 
	
    res=f_opendir(&mp3dir,"0:/TEXT"); //��Ŀ¼
	
	if(res==FR_OK)
	{
		curindex=0;//��ǰ����Ϊ0
		while(1)//ȫ����ѯһ��
		{
			temp=mp3dir.index;								//��¼��ǰindex
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X30)//ȡ����λ,�����ǲ����ı��ļ�	//��������exfuns.c�ļ�
			{
				mp3indextbl[curindex]=temp;//��¼����
				curindex++;
			}	    
		} 
	}   
	f_closedir(&mp3dir);
	
		
	Show_Str(450-16*5-80,y-20,240,16,"�ı�Ŀ¼�б�",16,0);				//��ʾ�������� 
	//Show_Str(450-16*5-80-16*5,y+0,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
	Show_Str(450-16*5-80-16*5,y,240,16,"��ǰѡ��->",16,0);				//��ʾ�������� 
	//Show_Str(450-16*5-80-16*5,y+40,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
	Show_Str(450-16*5-80-16*5,y+60,240,16,"������½�ͷ�񷵻���һ��",16,0);
	//LCD_Fill(450-16*5-80-16*5,y+60,500,y+60+16,WHITE);				//���֮ǰ����ʾ
	
	
	if(keyindex>totmp3num)keyindex=0;
	if((keyindex-1)>totmp3num)keyindex=totmp3num-1;


 	curindex=keyindex;											//��0��ʼ��ʾ
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/TEXT"); 	//��Ŀ¼
	//if(res!=FR_OK)  LCD_Fill(30,50,800,480,WHITE);//�����ʾ
	//{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//�ı䵱ǰĿ¼����	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
        //if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		
		strcpy((char*)pname,"0:/TEXT/");				//����·��(Ŀ¼)
		//strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y,500,y+16,WHITE);				//���֮ǰ����ʾ
		//Show_Str(450-16*5-80,y,240,16,pname,16,0);				//��ʾ�������� 
		Show_Str(450-16*5-80,y,240,16,fn,16,0);
	
		f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/TEXT/");				//����·��(Ŀ¼)
		//strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y+20,500,y+20+16,WHITE);				//���֮ǰ����ʾ
		//Show_Str(450-16*5-80,y+20,240,16,pname,16,0);				//��ʾ�������� 
		Show_Str(450-16*5-80,y+20,240,16,fn,16,0);
			
		f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/TEXT/");				//����·��(Ŀ¼)
		//strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y+40,500,y+40+16,WHITE);				//���֮ǰ����ʾ
		//Show_Str(450-16*5-80,y+40,240,16,pname,16,0);				//��ʾ�������� 
		Show_Str(450-16*5-80,y+40,240,16,fn,16,0);
		
	//}
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�		
	myfree(SRAMIN,mp3indextbl);			//�ͷ��ڴ�
}




void draw_dir_receive(u16 y,u8 checkkey)
{
	
	u8 res;u8 keyindex;
	u8 *fn;   			//���ļ���
	u8 *pname;			//��·�����ļ���
	u16 curindex;		//ͼƬ��ǰ����
	u16 temp;
	u16 *mp3indextbl;	//���������� 
	u16 totmp3num; 		//�����ļ�����
	 DIR mp3dir;	 		//Ŀ¼
	FILINFO mp3fileinfo;//�ļ���Ϣ

	totmp3num=mp3_get_tnum("0:/RECEIVE"); //�õ�����Ч�ļ���

	keyindex=checkkey;
	
	//Show_Str(450-16*5-80,y-40,240,16,str,16,0);				//����
	//LCD_ShowNum(450-16*5-80,y-40,checkkey,2,16);
	
	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//Ϊ���ļ������������ڴ�
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//Ϊ��·�����ļ��������ڴ�
	
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
	
	while(f_opendir(&mp3dir,"0:/RECEIVE"))//��¼���ļ���
 	{	 
		Show_Str(30,630+120,240,16,"�����ļ��д���!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,630+120,240,246,WHITE);		//�����ʾ	     
		delay_ms(200);				  
		f_mkdir("0:/MUSIC");				//������Ŀ¼ 
		f_closedir(&mp3dir);		
	} 
	
	
	//��¼����
    res=f_opendir(&mp3dir,"0:/RECEIVE"); //��Ŀ¼
	
	if(res==FR_OK)
	{
		curindex=0;//��ǰ����Ϊ0
		while(1)//ȫ����ѯһ��
		{
			temp=mp3dir.index;								//��¼��ǰindex
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				mp3indextbl[curindex]=temp;//��¼����
				curindex++;
			}	    
		} 
	}   
	f_closedir(&mp3dir);
	
		
	Show_Str(450-16*5-80,y-20,240,16,"����Ŀ¼�б�",16,0);				//��ʾ�������� 
	//Show_Str(450-16*5-80-16*5,y+0,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
	Show_Str(450-16*5-80-16*5,y,240,16,"��ǰѡ��->",16,0);				//��ʾ�������� 
	//Show_Str(450-16*5-80-16*5,y+40,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
	Show_Str(450-16*5-80-16*5,y+60,240,16,"������½�ͷ�񷵻���һ��",16,0);
	//LCD_Fill(450-16*5-80-16*5,y+60,480,y+60+16,WHITE);				//���֮ǰ����ʾ
	
	
	if(keyindex>totmp3num)keyindex=0;
	if((keyindex-1)>totmp3num)keyindex=totmp3num-1;


 	curindex=keyindex;											//��0��ʼ��ʾ
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/RECEIVE"); 	//��Ŀ¼
	//if(res!=FR_OK)  LCD_Fill(30,50,800,480,WHITE);//�����ʾ
	//{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//�ı䵱ǰĿ¼����	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
        //if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		
		strcpy((char*)pname,"0:/RECEIVE/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y,500,y+16,WHITE);				//���֮ǰ����ʾ
		Show_Str(450-16*5-80,y,240,16,pname,16,0);				//��ʾ�������� 
	
	
		f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/RECEIVE/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y+20,500,y+20+16,WHITE);				//���֮ǰ����ʾ
		Show_Str(450-16*5-80,y+20,240,16,pname,16,0);				//��ʾ�������� 
			
		f_readdir(&mp3dir,&mp3fileinfo);       		//��ȡĿ¼�µ�һ���ļ�
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/RECEIVE/");				//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)fn);  			//���ļ������ں���
 		LCD_Fill(450-16*5-80,y+40,500,y+40+16,WHITE);				//���֮ǰ����ʾ
		Show_Str(450-16*5-80,y+40,240,16,pname,16,0);				//��ʾ�������� 
		
		
	//}
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�		
	myfree(SRAMIN,mp3indextbl);			//�ͷ��ڴ�
}


//ͳ��Ŀ¼�ı�����
u16 get_txtnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ		
	u8 *fn; 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //��Ŀ¼
  	tfileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);	//Ϊ���ļ������������ڴ�
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X30)//ȡ����λ,�����ǲ��������ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	myfree(SRAMIN,tfileinfo.lfname);
	return rval;
}

void touch_swtich(void)
{
		
		u8 looknumb=0,firsttime=0,playindex=0,looknumb1=0,firsttime1=0,playindex1=0,playindex2;
		u8 lastupdate=0;
		
	
		u8 smscflg=0,srecflg=0,stxtflg=0;
	    u8 smscinx=0,srecinx=0,stxtinx=0;
		
		u8 rect=0,triagl=0,teeth=0;
	
			u16 totmp3num; 		//�����ļ�����
//			u8 res;
//			u8 *fn;   			//���ļ���
//			u8 *pname;			//��·�����ļ���
//			u16 curindex;		//ͼƬ��ǰ����
//			u16 temp;
//			u16 *mp3indextbl;	//���������� 
//			DIR mp3dir;	 		//Ŀ¼
//			FILINFO mp3fileinfo;//�ļ���Ϣ
	
		
		LED0=0;LED1=0;
//		delay_ms(1000);
//		LED0=1;LED1=0;
	
		tp_dev.scan(0);
		if((tp_dev.sta)&(1<<0))
	{
			//���ֲ�����
			if(tp_dev.x[0]>30&&tp_dev.y[0]>270&&tp_dev.x[0]<(30+180)&&tp_dev.y[0]<(270+84))
		{

			LED0=0;LED0=0;
			delay_ms(2000);	
			LED1=1; LED1=1; 
			
			draw_button(650,16*5,"�鿴��һ��",16);
			draw_button(720,16*5,"�鿴��һ��",16);
			
			draw_button_dir(50,400,16*7,"�鿴�����ļ���",16);
			draw_button_dir(50,460,16*7,"�鿴¼���ļ���",16);
			draw_button_dir(50,520,16*7,"����ѡ��/����",16);
			draw_button_dir(50,580,16*7,"�鿴�����ļ���",16);
			
			while(1)
			{	
					totmp3num=mp3_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���
					tp_dev.scan(0);
					firsttime=0;
				 //��ѯ����Ŀ¼����������
					if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
					{			
						while(1)
						{
						
							tp_dev.scan(0);
								
							if(!firsttime)
							{
								firsttime=1;
								draw_dir(540,looknumb);//��ʾ����Ŀ¼��y,Ŀ¼����
							}
							if(lastupdate!=looknumb)
							{
								draw_dir(540,looknumb);//��ʾ����Ŀ¼��y,Ŀ¼����
							}
							
							lastupdate=looknumb;
							delay_ms(500);
	
								
								if(TPAD_Scan(0))
								{
									
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
									delay_ms(1000);
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
									break;
									
								}
								
								//���ŵ�ǰѡ��
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
							{
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
									{
										playindex=looknumb;
										goto L1;	
									}
							}
								if((tp_dev.sta)&(1<<0)) //��顰�鿴��/��һ���������������Ƿ񱻰���
								{
									
								
									//�鿴Ŀ¼��һ����Ϣ
									//draw_button(650,16*5,"�鿴��һ��",16);
									//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
									//LCD_DrawRectangle((450-16*5-100),(650-10),(450-100+20),(650+16+10));
									if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
									{ 	
										delay_ms(2500);tp_dev.scan(0);
										if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
										{	
											
										if(looknumb)
										{
											looknumb--;
							
										}
										else 
										{
											
											looknumb=totmp3num-1;
										}
									}
											
									}
										//draw_button(720,16*5,"�鿴��һ��",16);
										//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
										if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
									{
										delay_ms(2500);tp_dev.scan(0);
											if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
										{
											//delay_ms(5000);
										looknumb++;
									
										if(looknumb>=totmp3num)looknumb=0;
										
											}
									}
									
								}
								
							}
								
					
						}
					//�鿴¼���ļ���
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //�鿴¼��Ŀ¼����������
					{
					
						//LCD_Fill(30,50,800,480,WHITE);//�����ʾ	
						
						looknumb1=0;
						while(1)
						{
							totmp3num=mp3_get_tnum("0:/RECORDER"); //�õ�����Ч�ļ���
							tp_dev.scan(0);
								
							if(!firsttime1)
							{
								firsttime1=1;
								draw_dir_record(540,looknumb1);//��ʾ¼��Ŀ¼��y,Ŀ¼����
							}
							if(lastupdate!=looknumb1)
							{
								draw_dir_record(540,looknumb1);//��ʾ¼��Ŀ¼��y,Ŀ¼����
							}
							
							lastupdate=looknumb1;
							delay_ms(500);
	
								
								if(TPAD_Scan(0))//ͷ�� ������һ��
								{
									
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
									//delay_ms(5000);
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
									break;
									
								}
								
								//���ŵ�ǰѡ��
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
							{
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
									{
										playindex1=looknumb1;
										Show_Str(30,400+220,200,16,"���ֲ�����ʵ��",16,0);				    	 
										Show_Str(30,420+220,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
										Show_Str(30,440+220,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
										Show_Str(30,460+220,200,16,"�洢������...",16,0);
										//printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡRAM���Խ��	    
										Show_Str(30,460+220,200,16,"���Ҳ�����...",16,0); 	 	 
										VS_Sine_Test();	   
										Show_Str(30,460+220,200,16,"<<���ֲ�����>>",16,0); 		 

										LCD_Fill(450-16*5-16*5,540+60,500,540+60+16,WHITE);				//���֮ǰ����ʾ
										Show_Str(450-16*5-80-16*5,540+60,240,16,"����������......",16,0);
										Show_Str(450-16*5-80-16*5,540+80,240,16,"������½�ͷ�񷵻���һ��/ֹͣ����",16,0);
										delay_ms(1000);

										mp3playREC(playindex1);
										break;
										
									}
							}
								if((tp_dev.sta)&(1<<0)) //��顰�鿴��/��һ���������������Ƿ񱻰���
								{
									
								
									//�鿴Ŀ¼��һ����Ϣ
									//draw_button(650,16*5,"�鿴��һ��",16);
									//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
									//LCD_DrawRectangle((450-16*5-100),(650-10),(450-100+20),(650+16+10));
									if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
									{ 	
										delay_ms(2500);tp_dev.scan(0);
										if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
										{	
											
										if(looknumb1)
										{
											looknumb1--;
							
										}
										else 
										{
											
											looknumb1=totmp3num-1;
										}
									}
											
									}
										//draw_button(720,16*5,"�鿴��һ��",16);
										//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
										if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
									{
										delay_ms(2500);tp_dev.scan(0);
											if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
										{
											//delay_ms(5000);
											looknumb1++;
									
												if(looknumb1>=totmp3num)looknumb1=0;
										
											}
									}
									
								}
							
						
						
							}
						}
					
					//���ŵ�ǰѡ��
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
					{
							delay_ms(2500);tp_dev.scan(0);
							if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
								{
									
		L1:						Show_Str(30,400+220,200,16,"���ֲ�����ʵ��",16,0);				    	 
								Show_Str(30,420+220,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
								Show_Str(30,440+220,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
								Show_Str(30,460+220,200,16,"�洢������...",16,0);
								//	printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡRAM���Խ��	    
								Show_Str(30,460+220,200,16,"���Ҳ�����...",16,0); 	 	 
								VS_Sine_Test();	   
								Show_Str(30,460+220,200,16,"<<���ֲ�����>>",16,0); 		 

								LCD_Fill(450-16*5-16*5,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
								Show_Str(450-16*5-80-16*5,540+60,240,16,"����������......",16,0);
								Show_Str(450-16*5-80-16*5,540+80,240,16,"������½�ͷ�񷵻���һ��/ֹͣ����",16,0);
								delay_ms(1000);

								mp3play(playindex);	
								//LCD_Fill(30,50,800,480,WHITE);//�����ʾ	 
								}
						
					}
					

					
				
						//�鿴������ɵ��ļ�
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //�鿴¼��Ŀ¼����������
					{
									
						delay_ms(2500);tp_dev.scan(0);
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //�鿴¼��Ŀ¼����������
						{
							playindex2=0;
							while(1)
						{
						
							tp_dev.scan(0);
								
							if(!firsttime)
							{
								firsttime=1;
								draw_dir_receive(540,looknumb);//��ʾ����Ŀ¼��y,Ŀ¼����
							}
							if(lastupdate!=looknumb)
							{
								draw_dir_receive(540,looknumb);//��ʾ����Ŀ¼��y,Ŀ¼����
							}
							
							lastupdate=looknumb;
							delay_ms(500);
	
								
								if(TPAD_Scan(0))
								{
									
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
									delay_ms(1000);
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
									break;
									
								}
								
								//���ŵ�ǰѡ��
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
							{
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
									{
										playindex2=looknumb;
										//goto L1;
										
										Show_Str(30,400+220,200,16,"���ֲ�����ʵ��",16,0);				    	 
										Show_Str(30,420+220,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
										Show_Str(30,440+220,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
										Show_Str(30,460+220,200,16,"�洢������...",16,0);
										//printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡRAM���Խ��	    
										Show_Str(30,460+220,200,16,"���Ҳ�����...",16,0); 	 	 
										VS_Sine_Test();	   
										Show_Str(30,460+220,200,16,"<<���ֲ�����>>",16,0); 		 

										LCD_Fill(450-16*5-16*5,540+60,500,540+60+16,WHITE);				//���֮ǰ����ʾ
										Show_Str(450-16*5-80-16*5,540+60,240,16,"����������......",16,0);
										Show_Str(450-16*5-80-16*5,540+80,240,16,"������½�ͷ�񷵻���һ��/ֹͣ����",16,0);
										delay_ms(1000);

										mp3playRECEIVE(playindex2);
										break;


										
									}
							}
								if((tp_dev.sta)&(1<<0)) //��顰�鿴��/��һ���������������Ƿ񱻰���
								{
									
								
									//�鿴Ŀ¼��һ����Ϣ
									//draw_button(650,16*5,"�鿴��һ��",16);
									//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
									//LCD_DrawRectangle((450-16*5-100),(650-10),(450-100+20),(650+16+10));
									if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
									{ 	
										delay_ms(2500);tp_dev.scan(0);
										if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
										{	
											
										if(looknumb)
										{
											looknumb--;
							
										}
										else 
										{
											
											looknumb=totmp3num-1;
										}
									}
											
									}
										//draw_button(720,16*5,"�鿴��һ��",16);
										//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
										if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
									{
										delay_ms(2500);tp_dev.scan(0);
											if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
										{
											//delay_ms(5000);
										looknumb++;
									
										if(looknumb>=totmp3num)looknumb=0;
										
											}
									}
									
								}
								
							}
							
							
						}
					}
					if(TPAD_Scan(0))//ͷ�� ������һ��
				{
					
//					Show_Str(60,300,510,24,"���ֲ�����",24,0);
//					LCD_DrawRectangle(30,270,(30+180),(270+84));
//					
//					Show_Str((450-180+30),300,480,24," ¼������ ",24,0);
//					LCD_DrawRectangle((450-180),270,450,(270+84));
//					
//					Show_Str((450-180+30),400,480,24," ���⴫�� ",24,0);
//					LCD_DrawRectangle((450-180),370,450,(370+84));
					
					LCD_Fill(0,370,250,800,WHITE);				//���֮ǰ����ʾ
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
					//delay_ms(5000);
					LCD_Fill(0,(370+84+5),480,800,WHITE);				//���֮ǰ����ʾ
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
					break;
					
				}					
					
				
		}
		
	}
		//¼������
			if(tp_dev.x[0]>(450-180)&&tp_dev.y[0]>270&&tp_dev.x[0]<450&&tp_dev.y[0]<(270+84))
		{
			tp_dev.scan(0);
			looknumb=0;firsttime=0;playindex=0;looknumb1=0;firsttime1=0;playindex1=0;
			LED0=0;LED0=0;
			delay_ms(1000);	
			LED1=1; LED1=1;

				draw_button(650,16*5,"�鿴��һ��",16);
				draw_button(720,16*5,"�鿴��һ��",16);
				
				draw_button_dir(50,400,16*7,"�鿴¼���ļ���",16);
				draw_button_dir(50,460,16*7,"¼����Ƶ",16);
				draw_button_dir(50,520,16*7,"����ѡ��",16);
			
			while(1)
			{
				
					totmp3num=mp3_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���
					tp_dev.scan(0);
					firsttime=0;
					//�鿴¼���ļ���
					if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
					{			
						
						while(1)
						{
							totmp3num=mp3_get_tnum("0:/RECORDER"); //�õ�����Ч�ļ���
							tp_dev.scan(0);
								
							if(!firsttime1)
							{
								firsttime1=1;
								draw_dir_record(540,looknumb1);//��ʾ¼��Ŀ¼��y,Ŀ¼����
							}
							if(lastupdate!=looknumb1)
							{
								draw_dir_record(540,looknumb1);//��ʾ¼��Ŀ¼��y,Ŀ¼����
							}
							
							lastupdate=looknumb1;
							delay_ms(500);
	
								
								if(TPAD_Scan(0))//ͷ�� ������һ��
								{
									
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
									delay_ms(1000);
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
									break;
									
								}
								
								//���ŵ�ǰѡ��
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
							{
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
									{
										playindex1=looknumb1;

										Show_Str(30,400+220,200,16,"¼����Ƶ����ʵ��",16,0);				    	 
										Show_Str(30,420+220,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
										Show_Str(30,440+220,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
										Show_Str(30,460+220,200,16,"�洢������...",16,0);
										//printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡRAM���Խ��	    
										Show_Str(30,460+220,200,16,"���Ҳ�����...",16,0); 	 	 
										VS_Sine_Test();	   
										Show_Str(30,460+220,200,16,"<<��Ƶ������>>",16,0); 		 

										LCD_Fill(450-16*5-16*5,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
										Show_Str(450-16*5-80-16*5,540+60,240,16,"��Ƶ������......",16,0);
										Show_Str(450-16*5-80-16*5,540+80,240,16,"������½�ͷ�񷵻���һ��/ֹͣ����",16,0);
										delay_ms(1000);

										mp3playREC(playindex1);
										break;
										
									}
							}
								if((tp_dev.sta)&(1<<0)) //��顰�鿴��/��һ���������������Ƿ񱻰���
								{
									
								
									//�鿴Ŀ¼��һ����Ϣ
									//draw_button(650,16*5,"�鿴��һ��",16);
									//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
									//LCD_DrawRectangle((450-16*5-100),(650-10),(450-100+20),(650+16+10));
									if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
									{ 	
										delay_ms(2500);tp_dev.scan(0);
										if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
										{	
											
										if(looknumb1)
										{
											looknumb1--;
							
										}
										else 
										{
											
											looknumb1=totmp3num-1;
										}
									}
											
									}
										//draw_button(720,16*5,"�鿴��һ��",16);
										//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
										if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
									{
										delay_ms(2500);tp_dev.scan(0);
											if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
										{
											//delay_ms(5000);
											looknumb1++;
									
												if(looknumb1>=totmp3num)looknumb1=0;
										
											}
									}
									
								}
							
						
						
							}
						}				

					//¼����Ƶ���ļ���
						
					if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //�鿴¼��Ŀ¼����������
					{
						
						delay_ms(2500);tp_dev.scan(0);
					if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //�鿴¼��Ŀ¼����������
					{		

									

								LED1=0; 
								LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ	
								Show_Str(450-16*5-80-16*5,540+60,240,16,"¼����Ƶ",16,0);
								Show_Str(30,510+120,200,16,"WAV¼����ʵ��",16,0);				    	 
								Show_Str(30,530+120,200,16,"KEY0:REC/PAUSE",16,0);
								Show_Str(30,550+120,200,16,"KEY2:STOP&SAVE",16,0);
								Show_Str(30,570+120,200,16,"KEY_UP:AGC+ KEY1:AGC-",16,0);
								Show_Str(30,590+120,200,16,"TPAD:Play The File",16,0);								
								Show_Str(30,610+120,200,16,"�洢������...",16,0);
								//printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡRAM���Խ��	    
								Show_Str(30,610+120,200,16,"���Ҳ�����...",16,0); 	 	 
								VS_Sine_Test();	   
								Show_Str(30,610+120,200,16,"<<WAV¼����>>",16,0); 		 
								LED1=1;
								recoder_play();
								LCD_Fill(0,510+120,250,800,WHITE);				//���֮ǰ����ʾ
						tp_dev.scan(0);
						//LCD_DrawRectangle(30,700,300,750);
								
					}
						
					}
					//����ѡ��
						
					if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
					{
							delay_ms(2500);tp_dev.scan(0);
							if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
								{
									
								Show_Str(30,400+220,200,16,"¼����Ƶ����ʵ��",16,0);				    	 
								Show_Str(30,420+220,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
								Show_Str(30,440+220,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
								Show_Str(30,460+220,200,16,"�洢������...",16,0);
								//	printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡRAM���Խ��	    
								Show_Str(30,460+220,200,16,"���Ҳ�����...",16,0); 	 	 
								VS_Sine_Test();	   
								Show_Str(30,460+220,200,16,"<<���ֲ�����>>",16,0); 		 

								LCD_Fill(450-16*5-16*5,540+60,500,540+60+16,WHITE);				//���֮ǰ����ʾ
								Show_Str(450-16*5-80-16*5,540+60,240,16,"¼��������......",16,0);
								Show_Str(450-16*5-80-16*5,540+80,240,16,"������½�ͷ�񷵻���һ��/ֹͣ����",16,0);
								delay_ms(1000);

								 mp3playREC(playindex1);	
								//LCD_Fill(30,50,800,480,WHITE);//�����ʾ	 
								}
						
					}	
						

					if(TPAD_Scan(0))//ͷ�� ������һ��
			{
					
//					Show_Str(60,300,510,24,"���ֲ�����",24,0);
//					LCD_DrawRectangle(30,270,(30+180),(270+84));
//					
//					Show_Str((450-180+30),300,480,24," ¼������ ",24,0);
//					LCD_DrawRectangle((450-180),270,450,(270+84));
//					
//					Show_Str((450-180+30),400,480,24," ���⴫�� ",24,0);
//					LCD_DrawRectangle((450-180),370,450,(370+84));
					
					LCD_Fill(0,370,250,1000,WHITE);				//���֮ǰ����ʾ
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
					//delay_ms(5000);
					LCD_Fill(0,(370+84+5),480,1000,WHITE);				//���֮ǰ����ʾ
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
					break;
					
				}
						
		
		}
			
			

		}
	

	
		//���⴫��
		if(tp_dev.x[0]>(450-180)&&tp_dev.y[0]>370&&tp_dev.x[0]<450&&tp_dev.y[0]<(370+84))
		{
				
				
			delay_ms(2000);	
			LED1=1; LED1=1; 
			rect=0,triagl=0,teeth=0;
			looknumb=0;firsttime=0;playindex=0;looknumb1=0;firsttime1=0;playindex1=0;
			lastupdate=0;
			totmp3num=0; 		//�����ļ�����
				
			draw_button(650,16*5,"�鿴��һ��",16);
			draw_button(720,16*5,"�鿴��һ��",16);
			
			draw_button_dir(50,400,16*7,"����ģʽ",16);
			draw_button_dir(50,460,16*7,"����ģʽ",16);
			draw_button_dir(50,520,16*7,"����ģʽ",16);
			
			//��ѯ ���⴫��Ŀ¼ ����������
			while(1)
			{	
					totmp3num=mp3_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���
					tp_dev.scan(0);
					firsttime=0;
					if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
					{			//�������ģʽ ѡ�񣺷��������ǲ��;�ݲ�
						
						//LCD_Fill(450-200,540,480,800,WHITE);				//���֮ǰ����ʾ
						LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//���֮ǰ����ʾ
						Show_Str(450-16*5-80-16*5,540+60,240,16,"�������ģʽ�ɹ�����ѡ�����͡�",16,0);
						Show_Str(450-16*5-80-16*5,540+80,240,16,"��������ͷ�񣬷�����һ��",16,0);
						LCD_Fill(50,390,260,550,WHITE);				//���֮ǰ����ʾ
						draw_button_dir(50,400,16*7,"����",16);
						draw_button_dir(50,460,16*7,"���ǲ�",16);
						draw_button_dir(50,520,16*7,"��ݲ�",16);
						
						draw_button(650,16*5,"����ѡ��",16);
						draw_button(720,16*5,"����ѡ��",16);
						while(1)
						{
							
							tp_dev.scan(0);
							//���� Ԥ������
							if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
							{
								delay_ms(2500);tp_dev.scan(0);
								if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
								{
									rect=1,triagl=0,teeth=0;
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"�����ȴ����ͣ���� ����ѡ��",16,0);
									Show_Str(450-16*5-80-16*5,540+80,240,16,"��������ͷ�񣬷�����һ��",16,0);
									
									LCD_Fill(450-16*5-80-16*5,540+0,480,540+59,WHITE);
									Show_Str(450-16*5-80-16*5,540+0,240,16,"������Ϣ:",16,0);
									Show_Str(450-16*5-80-16*5,540+20,240,16,"��ֵ3V3",16,0);
									Show_Str(450-16*5-80-16*5,540+40,240,16,"Ƶ��Ϊ1KHz",16,0);
								}
							}
							
							
							//���ǲ� Ԥ������
							
							if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //�鿴¼��Ŀ¼����������
							{	
								delay_ms(2500);tp_dev.scan(0);
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //�鿴¼��Ŀ¼����������
								{
									rect=0,triagl=1,teeth=0;
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"���ǲ��ȴ����ͣ���� ����ѡ��",16,0);
									Show_Str(450-16*5-80-16*5,540+80,240,16,"��������ͷ�񣬷�����һ��",16,0);
									
									LCD_Fill(450-16*5-80-16*5,540+0,480,540+59,WHITE);
									Show_Str(450-16*5-80-16*5,540+0,240,16,"������Ϣ:",16,0);
									Show_Str(450-16*5-80-16*5,540+20,240,16,"��ֵ3V3",16,0);
									Show_Str(450-16*5-80-16*5,540+40,240,16,"Ƶ��Ϊ1KHz",16,0);
							
								}
							}
							
							//��ݲ� Ԥ������
							if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
							{
								
								delay_ms(2500);tp_dev.scan(0);
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
								{
									rect=0,triagl=0,teeth=1;
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"��ݲ��ȴ����ͣ���� ����ѡ��",16,0);
									Show_Str(450-16*5-80-16*5,540+80,240,16,"��������ͷ�񣬷�����һ��",16,0);
									
									LCD_Fill(450-16*5-80-16*5,540+0,480,540+59,WHITE);
									Show_Str(450-16*5-80-16*5,540+0,240,16,"������Ϣ:",16,0);
									Show_Str(450-16*5-80-16*5,540+20,240,16,"��ֵ3V3",16,0);
									Show_Str(450-16*5-80-16*5,540+40,240,16,"Ƶ��Ϊ1KHz",16,0);
								
								}
								
							}
							
							
							
							if((tp_dev.sta)&(1<<0)) //��顰����ѡ��/����ѡ�񡱴����������Ƿ񱻰���
								{
									//ʵ�ʲ��η���
						
									//�鿴Ŀ¼��һ����Ϣ
									//draw_button(650,16*5,"����ѡ��",16);
									//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
									//LCD_DrawRectangle((450-16*5-100),(650-10),(450-100+20),(650+16+10));
									if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
									{ 	
										delay_ms(2500);tp_dev.scan(0);
										if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
										{	
											while(rect==1){
												LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//���֮ǰ����ʾ
												Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ�񷽲������ڷ����С�",16,0);
												Show_Str(450-16*5-80-16*5,540+80,240,16,"��������ͷ��ֹͣ/������һ��",16,0);
												//�˴����Ӳ��η��ͺ���
												
											}//���÷������ͺ���
											while(triagl==1){
												LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//���֮ǰ����ʾ
												Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ�����ǲ������ڷ����С�",16,0);
												Show_Str(450-16*5-80-16*5,540+80,240,16,"��������ͷ��ֹͣ/������һ��",16,0);
												//�˴����Ӳ��η��ͺ���
												
												
											}//�������ǲ����ͺ���
											while(teeth==1){
												LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//���֮ǰ����ʾ
												Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ���ݲ������ڷ����С�",16,0);
												Show_Str(450-16*5-80-16*5,540+80,240,16,"��������ͷ��ֹͣ/������һ��",16,0);
												//�˴����Ӳ��η��ͺ���
												
												
											}//���þ�ݲ����ͺ���
												
											rect=0,triagl=0,teeth=0;

										}
											
									}
										//draw_button(720,16*5,"����ѡ��",16);
										//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
										if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
									{
											delay_ms(2500);tp_dev.scan(0);
											if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
										{
											rect=0,triagl=0,teeth=0;	
											LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//���֮ǰ����ʾ
											Show_Str(450-16*5-80-16*5,540+60,240,16,"�������ģʽ�ɹ�����ѡ�����͡�",16,0);
											Show_Str(450-16*5-80-16*5,540+80,240,16,"��������ͷ�񣬷�����һ��",16,0);
											
											LCD_Fill(450-16*5-80-16*5,540+0,480,540+59,WHITE); //���������Ϣ
										}
									}
									
								}
								
							if(TPAD_Scan(0))
							{
									
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									LCD_Fill(450-200,540+80,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
									delay_ms(1000);
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
								
									Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
									draw_button_dir(50,400,16*7,"����ģʽ",16);
									draw_button_dir(50,460,16*7,"����ģʽ",16);
									draw_button_dir(50,520,16*7,"����ģʽ",16);
									LCD_Fill(450-16*5-80-16*5,540+0,480,540+59,WHITE); //���������Ϣ
									break;
									
							}
								
						}
										
							
					}
							
	
	
							//����ģʽ ѡ��
					if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //�鿴¼��Ŀ¼����������
					{
						delay_ms(2500);tp_dev.scan(0);
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //�鿴¼��Ŀ¼����������
						{
							
							LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//���֮ǰ����ʾ
							Show_Str(450-16*5-80-16*5,540+60,240,16,"���뷢��ģʽ�ɹ�����ѡ���ܡ�",16,0);
							Show_Str(450-16*5-80-16*5,540+80,240,16,"��������ͷ�񣬷�����һ��",16,0);
							LCD_Fill(50,390,260,550,WHITE);				//���֮ǰ����ʾ
							draw_button_dir(50,400,16*7,"�鿴�ı�Ŀ¼",16);
							draw_button_dir(50,460,16*7,"�鿴����Ŀ¼",16);
							draw_button_dir(50,520,16*7,"�鿴¼��Ŀ¼",16);
							draw_button_dir(50,580,16*7,"���͵�ǰѡ��",16);
							
							Show_Str(450-16*5-80,540-20,240,16,"Ŀ¼�б�",16,0);				//��ʾ�������� 
							//Show_Str(450-16*5-80-16*5,y+0,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
							Show_Str(450-16*5-80-16*5,540,240,16,"��ǰѡ��->",16,0);				//��ʾ�������� 
							
							draw_button(650,16*5,"�鿴��һ��",16);
							draw_button(720,16*5,"�鿴��һ��",16);
							while(1)
							{		
								tp_dev.scan(0);
								//�鿴�ı�Ŀ¼
								if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
								{
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
									{
										looknumb=0;firsttime=0;playindex=0;looknumb1=0;firsttime1=0;playindex1=0;
										lastupdate=0;
										totmp3num=0; 
										totmp3num=get_txtnum("0:/TEXT"); //�õ�����Ч�ļ���
										LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //ȥ���Ҳ������ı���Ϣ
										while(1)	
										{
											
											tp_dev.scan(0);
												
											if(!firsttime1)
											{
												firsttime1=1;
												draw_dir_txt(540,looknumb1);//��ʾ����Ŀ¼��y,Ŀ¼����
											}
											if(lastupdate!=looknumb1)
											{
												draw_dir_txt(540,looknumb1);//��ʾ����Ŀ¼��y,Ŀ¼����
											}
											
												lastupdate=looknumb1;
												delay_ms(500);
					
												
												if(TPAD_Scan(0))//ͷ�� ������һ��
												{
													
													
													LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //ȥ���Ҳ������ı���Ϣ
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ��Ŀ¼��ʾ
													
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
													Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
													//delay_ms(5000);
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
													Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
													
													Show_Str(450-16*5-80,540-20,240,16,"Ŀ¼�б�",16,0);				//��ʾ�������� 
													//Show_Str(450-16*5-80-16*5,y+0,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
													Show_Str(450-16*5-80-16*5,540,240,16,"��ǰѡ��->",16,0);		
													break;
													
												}
												
													if((tp_dev.sta)&(1<<0)) //��顰�鿴��/��һ���������������Ƿ񱻰���
													{
														
													
														//�鿴Ŀ¼��һ����Ϣ
														//draw_button(650,16*5,"�鿴��һ��",16);
														//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
														//LCD_DrawRectangle((450-16*5-100),(650-10),(450-100+20),(650+16+10));
														if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
														{ 	
															delay_ms(2500);tp_dev.scan(0);
															if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
															{	
																
															if(looknumb1)
															{
																looknumb1--;
												
															}
															else 
															{
																
																looknumb1=totmp3num-1;
															}
														}
																
														}
															//draw_button(720,16*5,"�鿴��һ��",16);
															//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
															if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
														{
															delay_ms(2500);tp_dev.scan(0);
																if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
															{
																//delay_ms(5000);
																looknumb1++;
														
																	if(looknumb1>=totmp3num)looknumb1=0;
															
																}
														}
														
													}
																										//���� ���͵�ǰѡ��  ����
													if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //�鿴¼��Ŀ¼����������
													{
														
														delay_ms(2500);tp_dev.scan(0);
														if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //�鿴¼��Ŀ¼����������
														{
															smscinx=0,srecinx=0,stxtinx=looknumb1;
															smscflg=0;srecflg=0;stxtflg=1;
															goto S1;
															
														}
													}
											
										}
										
									
									}
								}
								
								
								
								
								//�鿴����Ŀ¼
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //�鿴¼��Ŀ¼����������
								{	
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //�鿴¼��Ŀ¼����������
									{
										
										
										looknumb=0;firsttime=0;playindex=0;looknumb1=0;firsttime1=0;playindex1=0;
										lastupdate=0;
										totmp3num=0; 
										totmp3num=mp3_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���
										LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //ȥ���Ҳ������ı���Ϣ
										while(1)	
										{
											
											tp_dev.scan(0);
												
											if(!firsttime1)
											{
												firsttime1=1;
												draw_dir(540,looknumb1);//��ʾ����Ŀ¼��y,Ŀ¼����
											}
											if(lastupdate!=looknumb1)
											{
												draw_dir(540,looknumb1);//��ʾ����Ŀ¼��y,Ŀ¼����
											}
											
												lastupdate=looknumb1;
												delay_ms(500);
					
												
												if(TPAD_Scan(0))//ͷ�� ������һ��
												{
													
													
													LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //ȥ���Ҳ������ı���Ϣ
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ��Ŀ¼��ʾ
													
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
													Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
													//delay_ms(5000);
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
													Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
													
													Show_Str(450-16*5-80,540-20,240,16,"Ŀ¼�б�",16,0);				//��ʾ�������� 
													//Show_Str(450-16*5-80-16*5,y+0,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
													Show_Str(450-16*5-80-16*5,540,240,16,"��ǰѡ��->",16,0);		
													break;
													
												}
												
													if((tp_dev.sta)&(1<<0)) //��顰�鿴��/��һ���������������Ƿ񱻰���
													{
														
													
														//�鿴Ŀ¼��һ����Ϣ
														//draw_button(650,16*5,"�鿴��һ��",16);
														//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
														//LCD_DrawRectangle((450-16*5-100),(650-10),(450-100+20),(650+16+10));
														if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
														{ 	
															delay_ms(2500);tp_dev.scan(0);
															if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
															{	
																
															if(looknumb1)
															{
																looknumb1--;
												
															}
															else 
															{
																
																looknumb1=totmp3num-1;
															}
														}
																
														}
															//draw_button(720,16*5,"�鿴��һ��",16);
															//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
															if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
														{
															delay_ms(2500);tp_dev.scan(0);
																if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
															{
																//delay_ms(5000);
																looknumb1++;
														
																	if(looknumb1>=totmp3num)looknumb1=0;
															
																}
														}
														
													}
											
											//���� ���͵�ǰѡ��  ����
											if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //�鿴¼��Ŀ¼����������
											{
												
												delay_ms(2500);tp_dev.scan(0);
												if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //�鿴¼��Ŀ¼����������
												{
													smscinx=looknumb1,srecinx=0,stxtinx=0;
													smscflg=1;srecflg=0;stxtflg=0;
													goto S1;
													
												}
											}
										}
										
										
										
									}
								}
								
								//�鿴¼��Ŀ¼
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
								{
									
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
									{
										looknumb=0;firsttime=0;playindex=0;looknumb1=0;firsttime1=0;playindex1=0;
										lastupdate=0;
										totmp3num=0; 
										totmp3num=mp3_get_tnum("0:/RECORDER"); //�õ�����Ч�ļ���
										LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //ȥ���Ҳ������ı���Ϣ
										while(1)	
										{
											
											tp_dev.scan(0);
												
											if(!firsttime1)
											{
												firsttime1=1;
												draw_dir_record(540,looknumb1);//��ʾ¼��Ŀ¼��y,Ŀ¼����
											}
											if(lastupdate!=looknumb1)
											{
												draw_dir_record(540,looknumb1);//��ʾ¼��Ŀ¼��y,Ŀ¼����
											}
											
												lastupdate=looknumb1;
												delay_ms(500);
					
												
												if(TPAD_Scan(0))//ͷ�� ������һ��
												{
													
													
													LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //ȥ���Ҳ������ı���Ϣ
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ��Ŀ¼��ʾ
													
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
													Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
													//delay_ms(5000);
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
													Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
													
													Show_Str(450-16*5-80,540-20,240,16,"Ŀ¼�б�",16,0);				//��ʾ�������� 
													//Show_Str(450-16*5-80-16*5,y+0,240,16,"��һ�ף�",16,0);				//��ʾ�������� 
													Show_Str(450-16*5-80-16*5,540,240,16,"��ǰѡ��->",16,0);		
													break;
													
												}
												
													if((tp_dev.sta)&(1<<0)) //��顰�鿴��/��һ���������������Ƿ񱻰���
													{
														
													
														//�鿴Ŀ¼��һ����Ϣ
														//draw_button(650,16*5,"�鿴��һ��",16);
														//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
														//LCD_DrawRectangle((450-16*5-100),(650-10),(450-100+20),(650+16+10));
														if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
														{ 	
															delay_ms(2500);tp_dev.scan(0);
															if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
															{	
																
															if(looknumb1)
															{
																looknumb1--;
												
															}
															else 
															{
																
																looknumb1=totmp3num-1;
															}
														}
																
														}
															//draw_button(720,16*5,"�鿴��һ��",16);
															//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
															if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
														{
															delay_ms(2500);tp_dev.scan(0);
																if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
															{
																//delay_ms(5000);
																looknumb1++;
														
																	if(looknumb1>=totmp3num)looknumb1=0;
															
																}
														}
														
													}
													
													//���� ���͵�ǰѡ��  ¼��
													if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //�鿴¼��Ŀ¼����������
													{
														
														delay_ms(2500);tp_dev.scan(0);
														if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //�鿴¼��Ŀ¼����������
														{
															smscinx=0,srecinx=looknumb1,stxtinx=0;
															smscflg=0;srecflg=1;stxtflg=0;
															goto S1;
															
														}
													}
											
										}
										
									}
								}
								
								//���͵�ǰѡ�� �������ݷ��ͳ���
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //�鿴¼��Ŀ¼����������
								{
									
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //�鿴¼��Ŀ¼����������
									{
										S1:		
												
											while(1)
											{
												//���͵�ǰѡ��������ļ�
												if((smscflg!=0)&(srecflg==0)&(stxtflg==0))
												{
													musictransfer(smscinx,10);
													break;
												
												}
												//���͵�ǰѡ���¼���ļ�
						
													if((smscflg==0)&(srecflg!=0)&(stxtflg==0))
												{
													recordertransfer(srecinx,10);//10us
													break;
												
												}
												//���͵�ǰѡ����ı��ļ�
												//�ı������б���������
													if((smscflg==0)&(srecflg==0)&(stxtflg!=0))
												{
													
													txttransfer(stxtinx,10);//10us
													break;
												}
											
												
												//��λ
												smscflg=0;srecflg=0;stxtflg=0;
												smscinx=0;srecinx=0;stxtinx=0;
											}
										
									}
								}
								
								
								
									if(TPAD_Scan(0)) //�˳� ����ģʽĿ¼ �ص�����ѡ��
								{
										
										LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
										LCD_Fill(450-200,540+80,480,540+60+16,WHITE);				//���֮ǰ����ʾ
										Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
										delay_ms(1000);
										LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//���֮ǰ����ʾ
									
										Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ��ģʽ",16,0);
										LCD_Fill(0,390,200,800,WHITE);
									
										draw_button_dir(50,400,16*7,"����ģʽ",16);
										draw_button_dir(50,460,16*7,"����ģʽ",16);
										draw_button_dir(50,520,16*7,"����ģʽ",16);
										break;
										
								}
							
							}
						}							
					
					}
							//����ģʽ ѡ��
				
					if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
					{
						delay_ms(2500);tp_dev.scan(0);
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //�鿴¼��Ŀ¼����������
						{
							
														
							while(1)
							{
							
							
							}
							
						}
						
					}
			
			
			
				if(TPAD_Scan(0))//ͷ�� ������һ��
				{
					
//					Show_Str(60,300,510,24,"���ֲ�����",24,0);
//					LCD_DrawRectangle(30,270,(30+180),(270+84));
//					
//					Show_Str((450-180+30),300,480,24," ¼������ ",24,0);
//					LCD_DrawRectangle((450-180),270,450,(270+84));
//					
//					Show_Str((450-180+30),400,480,24," ���⴫�� ",24,0);
//					LCD_DrawRectangle((450-180),370,450,(370+84));
					
					LCD_Fill(0,370,250,800,WHITE);				//���֮ǰ����ʾ
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"���سɹ�",16,0);
					//delay_ms(5000);
					LCD_Fill(0,(370+84+5),480,800,WHITE);				//���֮ǰ����ʾ
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"��ѡ����",16,0);
					break;
					
				}
				
			}
		}
	

	}
}



