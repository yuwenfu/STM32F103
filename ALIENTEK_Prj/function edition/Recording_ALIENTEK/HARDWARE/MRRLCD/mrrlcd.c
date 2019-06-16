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

 //5个触控点的颜色												 
//const u16 POINT_COLOR_TBL[CT_MAX_TOUCH]={RED,GREEN,BLUE,BROWN,GRED};  
const u16 POINT_COLOR_TBL[CT_MAX_TOUCH]={RED}; 

/**************************************************************/

////////////////////////////////////////////////////////////////////////////////
//电容触摸屏专有部分
//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
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
//两个数之差的绝对值 
//x1,x2：需取差值的两个数
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//画一条粗线
//(x1,y1),(x2,y2):线条的起始坐标
//size：线条的粗细程度
//color：线条的颜色
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//画点 
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




//电容触摸屏测试函数
void ctp_test(void)
{
	u8 t=0;
	u8 i=0;	  	    
 	u16 lastpos[5][2];		//记录最后一次的数据 
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
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//画线
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<16)
					{
						Load_Drow_Dialog();//清除
					}
				}
			}else lastpos[t][0]=0XFFFF;
		}
		
		delay_ms(5);i++;
		if(i%20==0)LED0=!LED0;
	}	
}



// 封面信息显示
void mrrlcd_msg_show(void)
{
	POINT_COLOR=RED;//设置字体为红色
	Show_Str(30,50,480,24,"毕设：基于MEMS逆向光通信系统设计",24,0);	
	Show_Str(30,80,480,24,"长春理工大学",24,0);	
	Show_Str(30,110,480,24,"机电工程学院  机械电子专业",24,0);	
	Show_Str(30,140,480,24,"学生：付煜文  指导老师：孟立新",24,0);	
	//LCD_DrawLine(30, 170, 450, 170);
	lcd_draw_bline(25,45, 450, 45,1.8,BRRED);
	lcd_draw_bline(450,45, 450, 170,1.8,BRRED);
	lcd_draw_bline(25,170, 450, 170,1.8,BRRED);
	lcd_draw_bline(25,45, 25, 170,1.8,BRRED);
	Show_Str(30,180,480,16,"功能：录音、音乐播放、音频编码传输和音频接收解码",16,0);	
	Show_Str(30,200,480,16,"调制方式：OOK  录音存储文件格式为：WAV",16,0);	
	POINT_COLOR=BLUE;//设置字体为蓝色
	Show_Str(30,220,480,16,"蓝色框是控制按钮，可进行触屏点击操作！！",16,0);	
	Show_Str(30,240,480,16,"点按开发板右下角的头像，可实现返回上一级目录功能！",16,0);	
	//Show_Str(30,210,480,16,"录音文件格式为：WAV",16,0);
	POINT_COLOR=RED;//设置字体为红色

	
	

}
//清空屏幕并在右上角显示"RST"
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);	//清屏   
 	POINT_COLOR=BLUE;	//设置字体为蓝色 
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//显示清屏区域
  	POINT_COLOR=RED;//设置字体为红色
}

//基于MEMS逆向光调制系统界面设计
void Recording_Dialog(void)
{
	
	//LCD_Clear(WHITE);	//清屏   
 	POINT_COLOR=BLUE;	//设置字体为蓝色 
	//LCD_ShowString(lcddev.width-24,400,480,24,24,"RST");//显示清屏区域
	Show_Str(60,300,510,24,"音乐播放器",24,0);
	LCD_DrawRectangle(30,270,(30+180),(270+84));
	
	Show_Str((450-180+30),300,480,24," 录制语音 ",24,0);
	LCD_DrawRectangle((450-180),270,450,(270+84));
	
	Show_Str((450-180+30),400,480,24," 激光传输 ",24,0);
	LCD_DrawRectangle((450-180),370,450,(370+84));
	
  	POINT_COLOR=RED;	//设置画笔蓝色 
	

}

//	绘制控制按键
void draw_button(u16 y,u16 width,u8*str,u8 size)
{
	POINT_COLOR=BLUE;	//设置字体为蓝色 
	LCD_Fill((450-16*5-100),(y-10),450-100+20,(y+size+10),WHITE);				//清除之前的按键显示
	Show_Str(450-16*5-100+10,y,width,size,str,size,0);
	LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
	POINT_COLOR=RED;//设置字体为红色
}

void draw_button_dir(u16 x,u16 y,u16 width,u8*str,u8 size)
{
	POINT_COLOR=BLUE;	//设置字体为蓝色 
	Show_Str(x,y,width,size,str,size,0);
	LCD_DrawRectangle((x-20),(y-20),(x+7*16+20),(y+size+20));
	POINT_COLOR=RED;//设置字体为红色
}

//绘制目录
void draw_dir(u16 y,u8 checkkey)
{
	
	u8 res;u8 keyindex;
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 curindex;		//图片当前索引
	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
	u16 totmp3num; 		//音乐文件总数
	 DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息

	totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数

	keyindex=checkkey;
	
	//Show_Str(450-16*5-80,y-40,240,16,str,16,0);				//调试
	//LCD_ShowNum(450-16*5-80,y-40,checkkey,2,16);
	
	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
	
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
	
	while(f_opendir(&mp3dir,"0:/MUSIC"))//打开录音文件夹
 	{	 
		Show_Str(30,630+120,240,16,"音乐文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,630+120,240,246,WHITE);		//清除显示	     
		delay_ms(200);				  
		f_mkdir("0:/MUSIC");				//创建该目录 
		f_closedir(&mp3dir);		
	} 
	
	
	//记录索引
    res=f_opendir(&mp3dir,"0:/MUSIC"); //打开目录
	
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=mp3dir.index;								//记录当前index
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
			{
				mp3indextbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
	f_closedir(&mp3dir);
	
		
	Show_Str(450-16*5-80,y-20,240,16,"音乐目录列表",16,0);				//显示歌曲名字 
	//Show_Str(450-16*5-80-16*5,y+0,240,16,"上一首：",16,0);				//显示歌曲名字 
	Show_Str(450-16*5-80-16*5,y,240,16,"当前选择->",16,0);				//显示歌曲名字 
	//Show_Str(450-16*5-80-16*5,y+40,240,16,"下一首：",16,0);				//显示歌曲名字 
	Show_Str(450-16*5-80-16*5,y+60,240,16,"点击右下角头像返回上一级",16,0);
	//LCD_Fill(450-16*5-80-16*5,y+60,480,y+60+16,WHITE);				//清除之前的显示
	
	
	if(keyindex>totmp3num)keyindex=0;
	if((keyindex-1)>totmp3num)keyindex=totmp3num-1;


 	curindex=keyindex;											//从0开始显示
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); 	//打开目录
	//if(res!=FR_OK)  LCD_Fill(30,50,800,480,WHITE);//清除显示
	//{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
        //if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		
		strcpy((char*)pname,"0:/MUSIC/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y,500,y+16,WHITE);				//清除之前的显示
		Show_Str(450-16*5-80,y,240,16,pname,16,0);				//显示歌曲名字 
	
	
		f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/MUSIC/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y+20,500,y+20+16,WHITE);				//清除之前的显示
		Show_Str(450-16*5-80,y+20,240,16,pname,16,0);				//显示歌曲名字 
			
		f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/MUSIC/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y+40,500,y+40+16,WHITE);				//清除之前的显示
		Show_Str(450-16*5-80,y+40,240,16,pname,16,0);				//显示歌曲名字 
		
		
	//}
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存		
	myfree(SRAMIN,mp3indextbl);			//释放内存
}

void draw_dir_record(u16 y,u8 checkkey)
{
	
	u8 res;u8 keyindex;
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 curindex;		//图片当前索引
	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
	u16 totmp3num; 		//音乐文件总数
	 DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息

	totmp3num=mp3_get_tnum("0:/RECORDER"); //得到总有效文件数

	keyindex=checkkey;
	
	//Show_Str(450-16*5-80,y-40,240,16,str,16,0);				//调试
	//LCD_ShowNum(450-16*5-80,y-40,checkkey,2,16);
	//LCD_ShowNum(450-16*5-80,y-40,totmp3num,2,16);
	
	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
	
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
	
		while(f_opendir(&mp3dir,"0:/RECORDER"))//打开录音文件夹
 	{	 
		Show_Str(30,630+120,240,16,"RECORDER文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,630+120,240,246,WHITE);		//清除显示	     
		delay_ms(200);				  
		f_mkdir("0:/RECORDER");				//创建该目录 
		f_closedir(&mp3dir);		
	} 
	
	//记录索引
    res=f_opendir(&mp3dir,"0:/RECORDER"); //打开目录
	
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=mp3dir.index;								//记录当前index
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
			{
				mp3indextbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
	f_closedir(&mp3dir);
	
		
	Show_Str(450-16*5-80,y-20,240,16,"录音目录列表",16,0);				//显示歌曲名字 
	//Show_Str(450-16*5-80-16*5,y+0,240,16,"上一首：",16,0);				//显示歌曲名字 
	Show_Str(450-16*5-80-16*5,y,240,16,"当前选择->",16,0);				//显示歌曲名字 
	//Show_Str(450-16*5-80-16*5,y+40,240,16,"下一首：",16,0);				//显示歌曲名字 
	Show_Str(450-16*5-80-16*5,y+60,240,16,"点击右下角头像返回上一级",16,0);
	//LCD_Fill(450-16*5-80-16*5,y+60,500,y+60+16,WHITE);				//清除之前的显示
	
	
	if(keyindex>totmp3num)keyindex=0;
	if((keyindex-1)>totmp3num)keyindex=totmp3num-1;


 	curindex=keyindex;											//从0开始显示
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/RECORDER"); 	//打开目录
	//if(res!=FR_OK)  LCD_Fill(30,50,800,480,WHITE);//清除显示
	//{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
        //if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		
		strcpy((char*)pname,"0:/RECORDER/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y,500,y+16,WHITE);				//清除之前的显示
		Show_Str(450-16*5-80,y,240,16,pname,16,0);				//显示歌曲名字 
	
	
		f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/RECORDER/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y+20,500,y+20+16,WHITE);				//清除之前的显示
		Show_Str(450-16*5-80,y+20,240,16,pname,16,0);				//显示歌曲名字 
			
		f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/RECORDER/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y+40,500,y+40+16,WHITE);				//清除之前的显示
		Show_Str(450-16*5-80,y+40,240,16,pname,16,0);				//显示歌曲名字 
		
		
	//}
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存		
	myfree(SRAMIN,mp3indextbl);			//释放内存
}

void draw_dir_txt(u16 y,u8 checkkey)
{
	
	u8 res;u8 keyindex;
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 curindex;		//图片当前索引
	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
	u16 totmp3num; 		//音乐文件总数
	 DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息

	totmp3num=get_txtnum("0:/TEXT"); //得到总有效文件数

	keyindex=checkkey;
	
	//Show_Str(450-16*5-80,y-40,240,16,str,16,0);				//调试
	//LCD_ShowNum(450-16*5-80,y-40,checkkey,2,16);
	//LCD_ShowNum(450-16*5-80,y-40,totmp3num,2,16);
	
	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
	
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
	
	//记录索引
	
	  while(f_opendir(&mp3dir,"0:/TEXT"))//打开录音文件夹
 	{	 
		Show_Str(30,630+120,240,16,"TEXT文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,630+120,240,246,WHITE);		//清除显示	     
		delay_ms(200);				  
		f_mkdir("0:/TEXT");				//创建该目录 
		f_closedir(&mp3dir);		
	} 
	
    res=f_opendir(&mp3dir,"0:/TEXT"); //打开目录
	
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=mp3dir.index;								//记录当前index
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X30)//取高四位,看看是不是文本文件	//表格定义在exfuns.c文件
			{
				mp3indextbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
	f_closedir(&mp3dir);
	
		
	Show_Str(450-16*5-80,y-20,240,16,"文本目录列表",16,0);				//显示歌曲名字 
	//Show_Str(450-16*5-80-16*5,y+0,240,16,"上一首：",16,0);				//显示歌曲名字 
	Show_Str(450-16*5-80-16*5,y,240,16,"当前选择->",16,0);				//显示歌曲名字 
	//Show_Str(450-16*5-80-16*5,y+40,240,16,"下一首：",16,0);				//显示歌曲名字 
	Show_Str(450-16*5-80-16*5,y+60,240,16,"点击右下角头像返回上一级",16,0);
	//LCD_Fill(450-16*5-80-16*5,y+60,500,y+60+16,WHITE);				//清除之前的显示
	
	
	if(keyindex>totmp3num)keyindex=0;
	if((keyindex-1)>totmp3num)keyindex=totmp3num-1;


 	curindex=keyindex;											//从0开始显示
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/TEXT"); 	//打开目录
	//if(res!=FR_OK)  LCD_Fill(30,50,800,480,WHITE);//清除显示
	//{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
        //if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		
		strcpy((char*)pname,"0:/TEXT/");				//复制路径(目录)
		//strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y,500,y+16,WHITE);				//清除之前的显示
		//Show_Str(450-16*5-80,y,240,16,pname,16,0);				//显示歌曲名字 
		Show_Str(450-16*5-80,y,240,16,fn,16,0);
	
		f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/TEXT/");				//复制路径(目录)
		//strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y+20,500,y+20+16,WHITE);				//清除之前的显示
		//Show_Str(450-16*5-80,y+20,240,16,pname,16,0);				//显示歌曲名字 
		Show_Str(450-16*5-80,y+20,240,16,fn,16,0);
			
		f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/TEXT/");				//复制路径(目录)
		//strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y+40,500,y+40+16,WHITE);				//清除之前的显示
		//Show_Str(450-16*5-80,y+40,240,16,pname,16,0);				//显示歌曲名字 
		Show_Str(450-16*5-80,y+40,240,16,fn,16,0);
		
	//}
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存		
	myfree(SRAMIN,mp3indextbl);			//释放内存
}




void draw_dir_receive(u16 y,u8 checkkey)
{
	
	u8 res;u8 keyindex;
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 curindex;		//图片当前索引
	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
	u16 totmp3num; 		//音乐文件总数
	 DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息

	totmp3num=mp3_get_tnum("0:/RECEIVE"); //得到总有效文件数

	keyindex=checkkey;
	
	//Show_Str(450-16*5-80,y-40,240,16,str,16,0);				//调试
	//LCD_ShowNum(450-16*5-80,y-40,checkkey,2,16);
	
	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
	
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
	
	while(f_opendir(&mp3dir,"0:/RECEIVE"))//打开录音文件夹
 	{	 
		Show_Str(30,630+120,240,16,"接收文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,630+120,240,246,WHITE);		//清除显示	     
		delay_ms(200);				  
		f_mkdir("0:/MUSIC");				//创建该目录 
		f_closedir(&mp3dir);		
	} 
	
	
	//记录索引
    res=f_opendir(&mp3dir,"0:/RECEIVE"); //打开目录
	
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=mp3dir.index;								//记录当前index
	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
			{
				mp3indextbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
	f_closedir(&mp3dir);
	
		
	Show_Str(450-16*5-80,y-20,240,16,"接收目录列表",16,0);				//显示歌曲名字 
	//Show_Str(450-16*5-80-16*5,y+0,240,16,"上一首：",16,0);				//显示歌曲名字 
	Show_Str(450-16*5-80-16*5,y,240,16,"当前选择->",16,0);				//显示歌曲名字 
	//Show_Str(450-16*5-80-16*5,y+40,240,16,"下一首：",16,0);				//显示歌曲名字 
	Show_Str(450-16*5-80-16*5,y+60,240,16,"点击右下角头像返回上一级",16,0);
	//LCD_Fill(450-16*5-80-16*5,y+60,480,y+60+16,WHITE);				//清除之前的显示
	
	
	if(keyindex>totmp3num)keyindex=0;
	if((keyindex-1)>totmp3num)keyindex=totmp3num-1;


 	curindex=keyindex;											//从0开始显示
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/RECEIVE"); 	//打开目录
	//if(res!=FR_OK)  LCD_Fill(30,50,800,480,WHITE);//清除显示
	//{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
        //if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		
		strcpy((char*)pname,"0:/RECEIVE/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y,500,y+16,WHITE);				//清除之前的显示
		Show_Str(450-16*5-80,y,240,16,pname,16,0);				//显示歌曲名字 
	
	
		f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/RECEIVE/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y+20,500,y+20+16,WHITE);				//清除之前的显示
		Show_Str(450-16*5-80,y+20,240,16,pname,16,0);				//显示歌曲名字 
			
		f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
		strcpy((char*)pname,"0:/RECEIVE/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(450-16*5-80,y+40,500,y+40+16,WHITE);				//清除之前的显示
		Show_Str(450-16*5-80,y+40,240,16,pname,16,0);				//显示歌曲名字 
		
		
	//}
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存		
	myfree(SRAMIN,mp3indextbl);			//释放内存
}


//统计目录文本数量
u16 get_txtnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息		
	u8 *fn; 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //打开目录
  	tfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);	//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X30)//取高四位,看看是不是音乐文件	
			{
				rval++;//有效文件数增加1
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
	
			u16 totmp3num; 		//音乐文件总数
//			u8 res;
//			u8 *fn;   			//长文件名
//			u8 *pname;			//带路径的文件名
//			u16 curindex;		//图片当前索引
//			u16 temp;
//			u16 *mp3indextbl;	//音乐索引表 
//			DIR mp3dir;	 		//目录
//			FILINFO mp3fileinfo;//文件信息
	
		
		LED0=0;LED1=0;
//		delay_ms(1000);
//		LED0=1;LED1=0;
	
		tp_dev.scan(0);
		if((tp_dev.sta)&(1<<0))
	{
			//音乐播放器
			if(tp_dev.x[0]>30&&tp_dev.y[0]>270&&tp_dev.x[0]<(30+180)&&tp_dev.y[0]<(270+84))
		{

			LED0=0;LED0=0;
			delay_ms(2000);	
			LED1=1; LED1=1; 
			
			draw_button(650,16*5,"查看上一曲",16);
			draw_button(720,16*5,"查看下一曲",16);
			
			draw_button_dir(50,400,16*7,"查看音乐文件夹",16);
			draw_button_dir(50,460,16*7,"查看录音文件夹",16);
			draw_button_dir(50,520,16*7,"播放选择/音乐",16);
			draw_button_dir(50,580,16*7,"查看接收文件夹",16);
			
			while(1)
			{	
					totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
					tp_dev.scan(0);
					firsttime=0;
				 //查询音乐目录触摸屏按键
					if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
					{			
						while(1)
						{
						
							tp_dev.scan(0);
								
							if(!firsttime)
							{
								firsttime=1;
								draw_dir(540,looknumb);//显示歌曲目录，y,目录索引
							}
							if(lastupdate!=looknumb)
							{
								draw_dir(540,looknumb);//显示歌曲目录，y,目录索引
							}
							
							lastupdate=looknumb;
							delay_ms(500);
	
								
								if(TPAD_Scan(0))
								{
									
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
									delay_ms(1000);
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
									break;
									
								}
								
								//播放当前选择
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
							{
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
									{
										playindex=looknumb;
										goto L1;	
									}
							}
								if((tp_dev.sta)&(1<<0)) //检查“查看上/下一曲”触摸屏按键是否被按下
								{
									
								
									//查看目录上一个信息
									//draw_button(650,16*5,"查看上一曲",16);
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
										//draw_button(720,16*5,"查看下一曲",16);
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
					//查看录音文件夹
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //查看录音目录触摸屏按键
					{
					
						//LCD_Fill(30,50,800,480,WHITE);//清除显示	
						
						looknumb1=0;
						while(1)
						{
							totmp3num=mp3_get_tnum("0:/RECORDER"); //得到总有效文件数
							tp_dev.scan(0);
								
							if(!firsttime1)
							{
								firsttime1=1;
								draw_dir_record(540,looknumb1);//显示录音目录，y,目录索引
							}
							if(lastupdate!=looknumb1)
							{
								draw_dir_record(540,looknumb1);//显示录音目录，y,目录索引
							}
							
							lastupdate=looknumb1;
							delay_ms(500);
	
								
								if(TPAD_Scan(0))//头像 返回上一级
								{
									
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
									//delay_ms(5000);
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
									break;
									
								}
								
								//播放当前选择
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
							{
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
									{
										playindex1=looknumb1;
										Show_Str(30,400+220,200,16,"音乐播放器实验",16,0);				    	 
										Show_Str(30,420+220,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
										Show_Str(30,440+220,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
										Show_Str(30,460+220,200,16,"存储器测试...",16,0);
										//printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
										Show_Str(30,460+220,200,16,"正弦波测试...",16,0); 	 	 
										VS_Sine_Test();	   
										Show_Str(30,460+220,200,16,"<<音乐播放器>>",16,0); 		 

										LCD_Fill(450-16*5-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
										Show_Str(450-16*5-80-16*5,540+60,240,16,"歌曲播放中......",16,0);
										Show_Str(450-16*5-80-16*5,540+80,240,16,"点击右下角头像返回上一级/停止播放",16,0);
										delay_ms(1000);

										mp3playREC(playindex1);
										break;
										
									}
							}
								if((tp_dev.sta)&(1<<0)) //检查“查看上/下一曲”触摸屏按键是否被按下
								{
									
								
									//查看目录上一个信息
									//draw_button(650,16*5,"查看上一曲",16);
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
										//draw_button(720,16*5,"查看下一曲",16);
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
					
					//播放当前选择
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
					{
							delay_ms(2500);tp_dev.scan(0);
							if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
								{
									
		L1:						Show_Str(30,400+220,200,16,"音乐播放器实验",16,0);				    	 
								Show_Str(30,420+220,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
								Show_Str(30,440+220,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
								Show_Str(30,460+220,200,16,"存储器测试...",16,0);
								//	printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
								Show_Str(30,460+220,200,16,"正弦波测试...",16,0); 	 	 
								VS_Sine_Test();	   
								Show_Str(30,460+220,200,16,"<<音乐播放器>>",16,0); 		 

								LCD_Fill(450-16*5-16*5,540+60,480,540+60+16,WHITE);				//清除之前的显示
								Show_Str(450-16*5-80-16*5,540+60,240,16,"歌曲播放中......",16,0);
								Show_Str(450-16*5-80-16*5,540+80,240,16,"点击右下角头像返回上一级/停止播放",16,0);
								delay_ms(1000);

								mp3play(playindex);	
								//LCD_Fill(30,50,800,480,WHITE);//清除显示	 
								}
						
					}
					

					
				
						//查看传输完成的文件
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //查看录音目录触摸屏按键
					{
									
						delay_ms(2500);tp_dev.scan(0);
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //查看录音目录触摸屏按键
						{
							playindex2=0;
							while(1)
						{
						
							tp_dev.scan(0);
								
							if(!firsttime)
							{
								firsttime=1;
								draw_dir_receive(540,looknumb);//显示歌曲目录，y,目录索引
							}
							if(lastupdate!=looknumb)
							{
								draw_dir_receive(540,looknumb);//显示歌曲目录，y,目录索引
							}
							
							lastupdate=looknumb;
							delay_ms(500);
	
								
								if(TPAD_Scan(0))
								{
									
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
									delay_ms(1000);
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
									break;
									
								}
								
								//播放当前选择
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
							{
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
									{
										playindex2=looknumb;
										//goto L1;
										
										Show_Str(30,400+220,200,16,"音乐播放器实验",16,0);				    	 
										Show_Str(30,420+220,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
										Show_Str(30,440+220,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
										Show_Str(30,460+220,200,16,"存储器测试...",16,0);
										//printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
										Show_Str(30,460+220,200,16,"正弦波测试...",16,0); 	 	 
										VS_Sine_Test();	   
										Show_Str(30,460+220,200,16,"<<音乐播放器>>",16,0); 		 

										LCD_Fill(450-16*5-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
										Show_Str(450-16*5-80-16*5,540+60,240,16,"歌曲播放中......",16,0);
										Show_Str(450-16*5-80-16*5,540+80,240,16,"点击右下角头像返回上一级/停止播放",16,0);
										delay_ms(1000);

										mp3playRECEIVE(playindex2);
										break;


										
									}
							}
								if((tp_dev.sta)&(1<<0)) //检查“查看上/下一曲”触摸屏按键是否被按下
								{
									
								
									//查看目录上一个信息
									//draw_button(650,16*5,"查看上一曲",16);
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
										//draw_button(720,16*5,"查看下一曲",16);
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
					if(TPAD_Scan(0))//头像 返回上一级
				{
					
//					Show_Str(60,300,510,24,"音乐播放器",24,0);
//					LCD_DrawRectangle(30,270,(30+180),(270+84));
//					
//					Show_Str((450-180+30),300,480,24," 录制语音 ",24,0);
//					LCD_DrawRectangle((450-180),270,450,(270+84));
//					
//					Show_Str((450-180+30),400,480,24," 激光传输 ",24,0);
//					LCD_DrawRectangle((450-180),370,450,(370+84));
					
					LCD_Fill(0,370,250,800,WHITE);				//清除之前的显示
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
					//delay_ms(5000);
					LCD_Fill(0,(370+84+5),480,800,WHITE);				//清除之前的显示
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
					break;
					
				}					
					
				
		}
		
	}
		//录制语音
			if(tp_dev.x[0]>(450-180)&&tp_dev.y[0]>270&&tp_dev.x[0]<450&&tp_dev.y[0]<(270+84))
		{
			tp_dev.scan(0);
			looknumb=0;firsttime=0;playindex=0;looknumb1=0;firsttime1=0;playindex1=0;
			LED0=0;LED0=0;
			delay_ms(1000);	
			LED1=1; LED1=1;

				draw_button(650,16*5,"查看上一曲",16);
				draw_button(720,16*5,"查看下一曲",16);
				
				draw_button_dir(50,400,16*7,"查看录音文件夹",16);
				draw_button_dir(50,460,16*7,"录制音频",16);
				draw_button_dir(50,520,16*7,"播放选择",16);
			
			while(1)
			{
				
					totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
					tp_dev.scan(0);
					firsttime=0;
					//查看录音文件夹
					if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
					{			
						
						while(1)
						{
							totmp3num=mp3_get_tnum("0:/RECORDER"); //得到总有效文件数
							tp_dev.scan(0);
								
							if(!firsttime1)
							{
								firsttime1=1;
								draw_dir_record(540,looknumb1);//显示录音目录，y,目录索引
							}
							if(lastupdate!=looknumb1)
							{
								draw_dir_record(540,looknumb1);//显示录音目录，y,目录索引
							}
							
							lastupdate=looknumb1;
							delay_ms(500);
	
								
								if(TPAD_Scan(0))//头像 返回上一级
								{
									
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
									delay_ms(1000);
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
									break;
									
								}
								
								//播放当前选择
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
							{
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
									{
										playindex1=looknumb1;

										Show_Str(30,400+220,200,16,"录制音频播放实验",16,0);				    	 
										Show_Str(30,420+220,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
										Show_Str(30,440+220,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
										Show_Str(30,460+220,200,16,"存储器测试...",16,0);
										//printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
										Show_Str(30,460+220,200,16,"正弦波测试...",16,0); 	 	 
										VS_Sine_Test();	   
										Show_Str(30,460+220,200,16,"<<音频播放器>>",16,0); 		 

										LCD_Fill(450-16*5-16*5,540+60,480,540+60+16,WHITE);				//清除之前的显示
										Show_Str(450-16*5-80-16*5,540+60,240,16,"音频播放中......",16,0);
										Show_Str(450-16*5-80-16*5,540+80,240,16,"点击右下角头像返回上一级/停止播放",16,0);
										delay_ms(1000);

										mp3playREC(playindex1);
										break;
										
									}
							}
								if((tp_dev.sta)&(1<<0)) //检查“查看上/下一曲”触摸屏按键是否被按下
								{
									
								
									//查看目录上一个信息
									//draw_button(650,16*5,"查看上一曲",16);
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
										//draw_button(720,16*5,"查看下一曲",16);
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

					//录制音频子文件夹
						
					if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //查看录音目录触摸屏按键
					{
						
						delay_ms(2500);tp_dev.scan(0);
					if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //查看录音目录触摸屏按键
					{		

									

								LED1=0; 
								LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//清除之前的显示	
								Show_Str(450-16*5-80-16*5,540+60,240,16,"录制音频",16,0);
								Show_Str(30,510+120,200,16,"WAV录音机实验",16,0);				    	 
								Show_Str(30,530+120,200,16,"KEY0:REC/PAUSE",16,0);
								Show_Str(30,550+120,200,16,"KEY2:STOP&SAVE",16,0);
								Show_Str(30,570+120,200,16,"KEY_UP:AGC+ KEY1:AGC-",16,0);
								Show_Str(30,590+120,200,16,"TPAD:Play The File",16,0);								
								Show_Str(30,610+120,200,16,"存储器测试...",16,0);
								//printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
								Show_Str(30,610+120,200,16,"正弦波测试...",16,0); 	 	 
								VS_Sine_Test();	   
								Show_Str(30,610+120,200,16,"<<WAV录音机>>",16,0); 		 
								LED1=1;
								recoder_play();
								LCD_Fill(0,510+120,250,800,WHITE);				//清除之前的显示
						tp_dev.scan(0);
						//LCD_DrawRectangle(30,700,300,750);
								
					}
						
					}
					//播放选择
						
					if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
					{
							delay_ms(2500);tp_dev.scan(0);
							if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
								{
									
								Show_Str(30,400+220,200,16,"录制音频播放实验",16,0);				    	 
								Show_Str(30,420+220,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
								Show_Str(30,440+220,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
								Show_Str(30,460+220,200,16,"存储器测试...",16,0);
								//	printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
								Show_Str(30,460+220,200,16,"正弦波测试...",16,0); 	 	 
								VS_Sine_Test();	   
								Show_Str(30,460+220,200,16,"<<音乐播放器>>",16,0); 		 

								LCD_Fill(450-16*5-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
								Show_Str(450-16*5-80-16*5,540+60,240,16,"录音播放中......",16,0);
								Show_Str(450-16*5-80-16*5,540+80,240,16,"点击右下角头像返回上一级/停止播放",16,0);
								delay_ms(1000);

								 mp3playREC(playindex1);	
								//LCD_Fill(30,50,800,480,WHITE);//清除显示	 
								}
						
					}	
						

					if(TPAD_Scan(0))//头像 返回上一级
			{
					
//					Show_Str(60,300,510,24,"音乐播放器",24,0);
//					LCD_DrawRectangle(30,270,(30+180),(270+84));
//					
//					Show_Str((450-180+30),300,480,24," 录制语音 ",24,0);
//					LCD_DrawRectangle((450-180),270,450,(270+84));
//					
//					Show_Str((450-180+30),400,480,24," 激光传输 ",24,0);
//					LCD_DrawRectangle((450-180),370,450,(370+84));
					
					LCD_Fill(0,370,250,1000,WHITE);				//清除之前的显示
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
					//delay_ms(5000);
					LCD_Fill(0,(370+84+5),480,1000,WHITE);				//清除之前的显示
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
					break;
					
				}
						
		
		}
			
			

		}
	

	
		//激光传输
		if(tp_dev.x[0]>(450-180)&&tp_dev.y[0]>370&&tp_dev.x[0]<450&&tp_dev.y[0]<(370+84))
		{
				
				
			delay_ms(2000);	
			LED1=1; LED1=1; 
			rect=0,triagl=0,teeth=0;
			looknumb=0;firsttime=0;playindex=0;looknumb1=0;firsttime1=0;playindex1=0;
			lastupdate=0;
			totmp3num=0; 		//音乐文件总数
				
			draw_button(650,16*5,"查看上一个",16);
			draw_button(720,16*5,"查看下一下",16);
			
			draw_button_dir(50,400,16*7,"测试模式",16);
			draw_button_dir(50,460,16*7,"发送模式",16);
			draw_button_dir(50,520,16*7,"接收模式",16);
			
			//查询 激光传输目录 触摸屏按键
			while(1)
			{	
					totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
					tp_dev.scan(0);
					firsttime=0;
					if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
					{			//进入测试模式 选择：方波、三角波和锯齿波
						
						//LCD_Fill(450-200,540,480,800,WHITE);				//清除之前的显示
						LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//清除之前的显示
						Show_Str(450-16*5-80-16*5,540+60,240,16,"进入测试模式成功，请选择类型。",16,0);
						Show_Str(450-16*5-80-16*5,540+80,240,16,"长按右下头像，返回上一级",16,0);
						LCD_Fill(50,390,260,550,WHITE);				//清除之前的显示
						draw_button_dir(50,400,16*7,"方波",16);
						draw_button_dir(50,460,16*7,"三角波",16);
						draw_button_dir(50,520,16*7,"锯齿波",16);
						
						draw_button(650,16*5,"发送选择",16);
						draw_button(720,16*5,"重新选择",16);
						while(1)
						{
							
							tp_dev.scan(0);
							//方波 预备发送
							if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
							{
								delay_ms(2500);tp_dev.scan(0);
								if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
								{
									rect=1,triagl=0,teeth=0;
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"方波等待发送，点击 发送选择。",16,0);
									Show_Str(450-16*5-80-16*5,540+80,240,16,"长按右下头像，返回上一级",16,0);
									
									LCD_Fill(450-16*5-80-16*5,540+0,480,540+59,WHITE);
									Show_Str(450-16*5-80-16*5,540+0,240,16,"波形信息:",16,0);
									Show_Str(450-16*5-80-16*5,540+20,240,16,"幅值3V3",16,0);
									Show_Str(450-16*5-80-16*5,540+40,240,16,"频率为1KHz",16,0);
								}
							}
							
							
							//三角波 预备发送
							
							if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //查看录音目录触摸屏按键
							{	
								delay_ms(2500);tp_dev.scan(0);
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //查看录音目录触摸屏按键
								{
									rect=0,triagl=1,teeth=0;
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"三角波等待发送，点击 发送选择。",16,0);
									Show_Str(450-16*5-80-16*5,540+80,240,16,"长按右下头像，返回上一级",16,0);
									
									LCD_Fill(450-16*5-80-16*5,540+0,480,540+59,WHITE);
									Show_Str(450-16*5-80-16*5,540+0,240,16,"波形信息:",16,0);
									Show_Str(450-16*5-80-16*5,540+20,240,16,"幅值3V3",16,0);
									Show_Str(450-16*5-80-16*5,540+40,240,16,"频率为1KHz",16,0);
							
								}
							}
							
							//锯齿波 预备发送
							if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
							{
								
								delay_ms(2500);tp_dev.scan(0);
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
								{
									rect=0,triagl=0,teeth=1;
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"锯齿波等待发送，点击 发送选择。",16,0);
									Show_Str(450-16*5-80-16*5,540+80,240,16,"长按右下头像，返回上一级",16,0);
									
									LCD_Fill(450-16*5-80-16*5,540+0,480,540+59,WHITE);
									Show_Str(450-16*5-80-16*5,540+0,240,16,"波形信息:",16,0);
									Show_Str(450-16*5-80-16*5,540+20,240,16,"幅值3V3",16,0);
									Show_Str(450-16*5-80-16*5,540+40,240,16,"频率为1KHz",16,0);
								
								}
								
							}
							
							
							
							if((tp_dev.sta)&(1<<0)) //检查“发送选择/重新选择”触摸屏按键是否被按下
								{
									//实际波形发送
						
									//查看目录上一个信息
									//draw_button(650,16*5,"发送选择",16);
									//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
									//LCD_DrawRectangle((450-16*5-100),(650-10),(450-100+20),(650+16+10));
									if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
									{ 	
										delay_ms(2500);tp_dev.scan(0);
										if(tp_dev.x[0]>(450-16*5-100) &&tp_dev.y[0]> (650-10)  &&tp_dev.x[0]<(450-100+20)  &&tp_dev.y[0]<(650+16+10) ) 
										{	
											while(rect==1){
												LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//清除之前的显示
												Show_Str(450-16*5-80-16*5,540+60,240,16,"已选择方波，正在发送中。",16,0);
												Show_Str(450-16*5-80-16*5,540+80,240,16,"长按右下头像，停止/返回上一级",16,0);
												//此处添加波形发送函数
												
											}//调用方波发送函数
											while(triagl==1){
												LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//清除之前的显示
												Show_Str(450-16*5-80-16*5,540+60,240,16,"已选择三角波，正在发送中。",16,0);
												Show_Str(450-16*5-80-16*5,540+80,240,16,"长按右下头像，停止/返回上一级",16,0);
												//此处添加波形发送函数
												
												
											}//调用三角波发送函数
											while(teeth==1){
												LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//清除之前的显示
												Show_Str(450-16*5-80-16*5,540+60,240,16,"已选择锯齿波，正在发送中。",16,0);
												Show_Str(450-16*5-80-16*5,540+80,240,16,"长按右下头像，停止/返回上一级",16,0);
												//此处添加波形发送函数
												
												
											}//调用锯齿波发送函数
												
											rect=0,triagl=0,teeth=0;

										}
											
									}
										//draw_button(720,16*5,"重新选择",16);
										//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
										if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
									{
											delay_ms(2500);tp_dev.scan(0);
											if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(720-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(720+16+10))
										{
											rect=0,triagl=0,teeth=0;	
											LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//清除之前的显示
											Show_Str(450-16*5-80-16*5,540+60,240,16,"进入测试模式成功，请选择类型。",16,0);
											Show_Str(450-16*5-80-16*5,540+80,240,16,"长按右下头像，返回上一级",16,0);
											
											LCD_Fill(450-16*5-80-16*5,540+0,480,540+59,WHITE); //清除波形信息
										}
									}
									
								}
								
							if(TPAD_Scan(0))
							{
									
									LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
									LCD_Fill(450-200,540+80,480,540+60+16,WHITE);				//清除之前的显示
									Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
									delay_ms(1000);
									LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//清除之前的显示
								
									Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
									draw_button_dir(50,400,16*7,"测试模式",16);
									draw_button_dir(50,460,16*7,"发送模式",16);
									draw_button_dir(50,520,16*7,"接收模式",16);
									LCD_Fill(450-16*5-80-16*5,540+0,480,540+59,WHITE); //清除波形信息
									break;
									
							}
								
						}
										
							
					}
							
	
	
							//发送模式 选择
					if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //查看录音目录触摸屏按键
					{
						delay_ms(2500);tp_dev.scan(0);
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //查看录音目录触摸屏按键
						{
							
							LCD_Fill(450-16*5-80-16*5,540+60,480,540+85,WHITE);				//清除之前的显示
							Show_Str(450-16*5-80-16*5,540+60,240,16,"进入发送模式成功，请选择功能。",16,0);
							Show_Str(450-16*5-80-16*5,540+80,240,16,"长按右下头像，返回上一级",16,0);
							LCD_Fill(50,390,260,550,WHITE);				//清除之前的显示
							draw_button_dir(50,400,16*7,"查看文本目录",16);
							draw_button_dir(50,460,16*7,"查看音乐目录",16);
							draw_button_dir(50,520,16*7,"查看录音目录",16);
							draw_button_dir(50,580,16*7,"发送当前选择",16);
							
							Show_Str(450-16*5-80,540-20,240,16,"目录列表",16,0);				//显示歌曲名字 
							//Show_Str(450-16*5-80-16*5,y+0,240,16,"上一首：",16,0);				//显示歌曲名字 
							Show_Str(450-16*5-80-16*5,540,240,16,"当前选择->",16,0);				//显示歌曲名字 
							
							draw_button(650,16*5,"查看上一个",16);
							draw_button(720,16*5,"查看下一个",16);
							while(1)
							{		
								tp_dev.scan(0);
								//查看文本目录
								if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
								{
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20)&&tp_dev.y[0]>(400-20)&&tp_dev.x[0]<(50+7*16+20)&&tp_dev.y[0]<(400+16+20))
									{
										looknumb=0;firsttime=0;playindex=0;looknumb1=0;firsttime1=0;playindex1=0;
										lastupdate=0;
										totmp3num=0; 
										totmp3num=get_txtnum("0:/TEXT"); //得到总有效文件数
										LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //去除右侧所有文本信息
										while(1)	
										{
											
											tp_dev.scan(0);
												
											if(!firsttime1)
											{
												firsttime1=1;
												draw_dir_txt(540,looknumb1);//显示音乐目录，y,目录索引
											}
											if(lastupdate!=looknumb1)
											{
												draw_dir_txt(540,looknumb1);//显示音乐目录，y,目录索引
											}
											
												lastupdate=looknumb1;
												delay_ms(500);
					
												
												if(TPAD_Scan(0))//头像 返回上一级
												{
													
													
													LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //去除右侧所有文本信息
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的目录显示
													
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
													Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
													//delay_ms(5000);
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
													Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
													
													Show_Str(450-16*5-80,540-20,240,16,"目录列表",16,0);				//显示歌曲名字 
													//Show_Str(450-16*5-80-16*5,y+0,240,16,"上一首：",16,0);				//显示歌曲名字 
													Show_Str(450-16*5-80-16*5,540,240,16,"当前选择->",16,0);		
													break;
													
												}
												
													if((tp_dev.sta)&(1<<0)) //检查“查看上/下一个”触摸屏按键是否被按下
													{
														
													
														//查看目录上一个信息
														//draw_button(650,16*5,"查看上一个",16);
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
															//draw_button(720,16*5,"查看下一个",16);
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
																										//调用 发送当前选择  音乐
													if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //查看录音目录触摸屏按键
													{
														
														delay_ms(2500);tp_dev.scan(0);
														if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //查看录音目录触摸屏按键
														{
															smscinx=0,srecinx=0,stxtinx=looknumb1;
															smscflg=0;srecflg=0;stxtflg=1;
															goto S1;
															
														}
													}
											
										}
										
									
									}
								}
								
								
								
								
								//查看音乐目录
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //查看录音目录触摸屏按键
								{	
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (460-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (460+16+20) ) //查看录音目录触摸屏按键
									{
										
										
										looknumb=0;firsttime=0;playindex=0;looknumb1=0;firsttime1=0;playindex1=0;
										lastupdate=0;
										totmp3num=0; 
										totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
										LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //去除右侧所有文本信息
										while(1)	
										{
											
											tp_dev.scan(0);
												
											if(!firsttime1)
											{
												firsttime1=1;
												draw_dir(540,looknumb1);//显示音乐目录，y,目录索引
											}
											if(lastupdate!=looknumb1)
											{
												draw_dir(540,looknumb1);//显示音乐目录，y,目录索引
											}
											
												lastupdate=looknumb1;
												delay_ms(500);
					
												
												if(TPAD_Scan(0))//头像 返回上一级
												{
													
													
													LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //去除右侧所有文本信息
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的目录显示
													
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
													Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
													//delay_ms(5000);
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
													Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
													
													Show_Str(450-16*5-80,540-20,240,16,"目录列表",16,0);				//显示歌曲名字 
													//Show_Str(450-16*5-80-16*5,y+0,240,16,"上一首：",16,0);				//显示歌曲名字 
													Show_Str(450-16*5-80-16*5,540,240,16,"当前选择->",16,0);		
													break;
													
												}
												
													if((tp_dev.sta)&(1<<0)) //检查“查看上/下一个”触摸屏按键是否被按下
													{
														
													
														//查看目录上一个信息
														//draw_button(650,16*5,"查看上一个",16);
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
															//draw_button(720,16*5,"查看下一个",16);
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
											
											//调用 发送当前选择  音乐
											if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //查看录音目录触摸屏按键
											{
												
												delay_ms(2500);tp_dev.scan(0);
												if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //查看录音目录触摸屏按键
												{
													smscinx=looknumb1,srecinx=0,stxtinx=0;
													smscflg=1;srecflg=0;stxtflg=0;
													goto S1;
													
												}
											}
										}
										
										
										
									}
								}
								
								//查看录音目录
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
								{
									
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
									{
										looknumb=0;firsttime=0;playindex=0;looknumb1=0;firsttime1=0;playindex1=0;
										lastupdate=0;
										totmp3num=0; 
										totmp3num=mp3_get_tnum("0:/RECORDER"); //得到总有效文件数
										LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //去除右侧所有文本信息
										while(1)	
										{
											
											tp_dev.scan(0);
												
											if(!firsttime1)
											{
												firsttime1=1;
												draw_dir_record(540,looknumb1);//显示录音目录，y,目录索引
											}
											if(lastupdate!=looknumb1)
											{
												draw_dir_record(540,looknumb1);//显示录音目录，y,目录索引
											}
											
												lastupdate=looknumb1;
												delay_ms(500);
					
												
												if(TPAD_Scan(0))//头像 返回上一级
												{
													
													
													LCD_Fill(450-16*5-80-16*5,540-20,480,620+16,WHITE); //去除右侧所有文本信息
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的目录显示
													
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
													Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
													//delay_ms(5000);
													LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
													Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
													
													Show_Str(450-16*5-80,540-20,240,16,"目录列表",16,0);				//显示歌曲名字 
													//Show_Str(450-16*5-80-16*5,y+0,240,16,"上一首：",16,0);				//显示歌曲名字 
													Show_Str(450-16*5-80-16*5,540,240,16,"当前选择->",16,0);		
													break;
													
												}
												
													if((tp_dev.sta)&(1<<0)) //检查“查看上/下一个”触摸屏按键是否被按下
													{
														
													
														//查看目录上一个信息
														//draw_button(650,16*5,"查看上一个",16);
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
															//draw_button(720,16*5,"查看下一个",16);
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
													
													//调用 发送当前选择  录音
													if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //查看录音目录触摸屏按键
													{
														
														delay_ms(2500);tp_dev.scan(0);
														if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //查看录音目录触摸屏按键
														{
															smscinx=0,srecinx=looknumb1,stxtinx=0;
															smscflg=0;srecflg=1;stxtflg=0;
															goto S1;
															
														}
													}
											
										}
										
									}
								}
								
								//发送当前选择 调用数据发送程序
								if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //查看录音目录触摸屏按键
								{
									
									delay_ms(2500);tp_dev.scan(0);
									if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (580-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (580+16+20) ) //查看录音目录触摸屏按键
									{
										S1:		
												
											while(1)
											{
												//发送当前选择的音乐文件
												if((smscflg!=0)&(srecflg==0)&(stxtflg==0))
												{
													musictransfer(smscinx,10);
													break;
												
												}
												//发送当前选择的录音文件
						
													if((smscflg==0)&(srecflg!=0)&(stxtflg==0))
												{
													recordertransfer(srecinx,10);//10us
													break;
												
												}
												//发送当前选择的文本文件
												//文本类型判别函数待添加
													if((smscflg==0)&(srecflg==0)&(stxtflg!=0))
												{
													
													txttransfer(stxtinx,10);//10us
													break;
												}
											
												
												//复位
												smscflg=0;srecflg=0;stxtflg=0;
												smscinx=0;srecinx=0;stxtinx=0;
											}
										
									}
								}
								
								
								
									if(TPAD_Scan(0)) //退出 发送模式目录 回到功能选择
								{
										
										LCD_Fill(450-200,540+60,480,540+60+16,WHITE);				//清除之前的显示
										LCD_Fill(450-200,540+80,480,540+60+16,WHITE);				//清除之前的显示
										Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
										delay_ms(1000);
										LCD_Fill(450-16*5-80-16*5,540+60,480,540+60+16,WHITE);				//清除之前的显示
									
										Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择模式",16,0);
										LCD_Fill(0,390,200,800,WHITE);
									
										draw_button_dir(50,400,16*7,"测试模式",16);
										draw_button_dir(50,460,16*7,"发送模式",16);
										draw_button_dir(50,520,16*7,"接收模式",16);
										break;
										
								}
							
							}
						}							
					
					}
							//接收模式 选择
				
					if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
					{
						delay_ms(2500);tp_dev.scan(0);
						if(tp_dev.x[0]>(50-20) &&tp_dev.y[0]> (520-20)  &&tp_dev.x[0]<(50+7*16+20)  &&tp_dev.y[0]< (520+16+20) ) //查看录音目录触摸屏按键
						{
							
														
							while(1)
							{
							
							
							}
							
						}
						
					}
			
			
			
				if(TPAD_Scan(0))//头像 返回上一级
				{
					
//					Show_Str(60,300,510,24,"音乐播放器",24,0);
//					LCD_DrawRectangle(30,270,(30+180),(270+84));
//					
//					Show_Str((450-180+30),300,480,24," 录制语音 ",24,0);
//					LCD_DrawRectangle((450-180),270,450,(270+84));
//					
//					Show_Str((450-180+30),400,480,24," 激光传输 ",24,0);
//					LCD_DrawRectangle((450-180),370,450,(370+84));
					
					LCD_Fill(0,370,250,800,WHITE);				//清除之前的显示
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
					//delay_ms(5000);
					LCD_Fill(0,(370+84+5),480,800,WHITE);				//清除之前的显示
					//Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
					break;
					
				}
				
			}
		}
	

	}
}




