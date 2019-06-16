#include "mp3player.h"
#include "vs10xx.h"	 
#include "delay.h"
#include "led.h"
#include "key.h"
#include "lcd.h"		 
#include "malloc.h"
#include "text.h"
#include "string.h"
#include "exfuns.h"  
#include "ff.h"   
#include "flac.h"	
#include "usart.h"	
#include "tpad.h"	

#include "mrrlcd.h"
#include "touch.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板V3
//MP3播放驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/1/20
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved								  						    								  
//////////////////////////////////////////////////////////////////////////////////


//static u16 looknumb;

//显示曲目索引
//index:当前索引
//total:总文件数
void mp3_index_show(u16 index,u16 total)
{
	//显示当前曲目的索引,及总曲目数
	LCD_ShowxNum(30+0,530+220,index,3,16,0X80);		//索引
	LCD_ShowChar(30+24,530+220,'/',16,0);
	LCD_ShowxNum(30+32,530+220,total,3,16,0X80); 	//总曲目				  	  
}
//显示当前音量
void mp3_vol_show(u8 vol)
{			    
	LCD_ShowString(30+110,530+220,200,16,16,"VOL:");	  	  
	LCD_ShowxNum(30+142,530+220,vol,2,16,0X80); 	//显示音量	 
}
u16 f_kbps=0;//歌曲文件位率	
//显示播放时间,比特率 信息 
//lenth:歌曲总长度
void mp3_msg_show(u32 lenth)
{	
	static u16 playtime=0;//播放时间标记	     
 	u16 time=0;// 时间变量
	u16 temp=0;	  
	if(f_kbps==0xffff)//未更新过
	{
		playtime=0;
		f_kbps=VS_Get_HeadInfo();	   //获得比特率
	}	 	 
	time=VS_Get_DecodeTime(); //得到解码时间
	if(playtime==0)playtime=time;
	else if((time!=playtime)&&(time!=0))//1s时间到,更新显示数据
	{
		playtime=time;//更新时间 	 				    
		temp=VS_Get_HeadInfo(); //获得比特率	   				 
		if(temp!=f_kbps)
		{
			f_kbps=temp;//更新KBPS	  				     
		}			 
		//显示播放时间			 
		LCD_ShowxNum(30,510+220,time/60,2,16,0X80);		//分钟
		LCD_ShowChar(30+16,510+220,':',16,0);
		LCD_ShowxNum(30+24,510+220,time%60,2,16,0X80);	//秒钟		
 		LCD_ShowChar(30+40,510+220,'/',16,0); 	    	 
		//显示总时间
		if(f_kbps)time=(lenth/f_kbps)/125;//得到秒钟数   (文件长度(字节)/(1000/8)/比特率=持续秒钟数    	  
		else time=0;//非法位率	  
 		LCD_ShowxNum(30+48,510+220,time/60,2,16,0X80);	//分钟
		LCD_ShowChar(30+64,510+220,':',16,0);
		LCD_ShowxNum(30+72,510+220,time%60,2,16,0X80);	//秒钟	  		    
		//显示位率			   
   		LCD_ShowxNum(30+110,510+220,f_kbps,3,16,0X80); 	//显示位率	 
		LCD_ShowString(30+134,510+220,200,16,16,"Kbps");	  	  
		LED0=!LED0;		//DS0翻转
	}   		 
}			  		 
//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 mp3_get_tnum(u8 *path)
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
			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	} 
	myfree(SRAMIN,tfileinfo.lfname);
	return rval;
}
//播放音乐
void mp3_play(void)
{
	u8 res;
 	DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totmp3num; 		//音乐文件总数
	u16 curindex;		//图片当前索引
	u8 key;				//键值		  
 	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
	//u8 looknumb=0;
	
 	while(f_opendir(&mp3dir,"0:/MUSIC"))//打开音乐文件夹
 	{	    
		Show_Str(30,190,240,16,"MUSIC文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,206,WHITE);//清除显示	     
		delay_ms(200);				  
	} 									  
	totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
  	while(totmp3num==NULL)//音乐文件总数为0		
 	{	    
		Show_Str(30,190,240,16,"没有音乐文件!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}										   
  	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);				//申请2*totmp3num个字节的内存,用于存放音乐文件索引
 	while(mp3fileinfo.lfname==NULL||pname==NULL||mp3indextbl==NULL)//内存分配出错
 	{	    
		Show_Str(30,190,240,16,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}  	
	VS_HD_Reset();
	VS_Soft_Reset();
	vsset.mvol=220;						//默认设置音量为220.
	mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
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
   	curindex=0;											//从0开始显示
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); 	//打开目录
	while(res==FR_OK)//打开成功
	{	
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
	
		strcpy((char*)pname,"0:/MUSIC/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(30,490+220,480,190+16,WHITE);				//清除之前的显示
		Show_Str(30,490+220,480-30,16,fn,16,0);				//显示歌曲名字 
		//Show_Str(30,760,480,16,fn,16,0);
		mp3_index_show(curindex+1,totmp3num);
		key=mp3_play_song(pname); 				 		//播放这个MP3  
	
		
		if(key==2)		//上一曲
		{
			if(curindex)
			{
				curindex--;
		
			}
			else curindex=totmp3num-1;
 		}else if(key<=1)//下一曲
		{
			curindex++;		   	
			if(curindex>=totmp3num)curindex=0;//到末尾的时候,自动从头开始
 		}else if(key == 3)
		{
		  goto endplay;
		}
		else
			 break;	//产生了错误 	 
	}
endplay:	
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,mp3indextbl);			//释放内存	
	
}


void mp3play(u8 playindex)
{
	
	u8 firsttime=0;
	u8 lastupdate=0;
	
	u8 res;
 	DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totmp3num; 		//音乐文件总数
	u16 curindex;		//图片当前索引
	u8 key;				//键值		  
 	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
	//u8 looknumb=0;
	
 	while(f_opendir(&mp3dir,"0:/MUSIC"))//打开音乐文件夹
 	{	    
		Show_Str(30,190,240,16,"MUSIC文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,206,WHITE);//清除显示	     
		delay_ms(200);				  
	} 									  
	totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
  	while(totmp3num==NULL)//音乐文件总数为0		
 	{	    
		Show_Str(30,190,240,16,"没有音乐文件!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}										   
  	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);				//申请2*totmp3num个字节的内存,用于存放音乐文件索引
 	while(mp3fileinfo.lfname==NULL||pname==NULL||mp3indextbl==NULL)//内存分配出错
 	{	    
		Show_Str(30,190,240,16,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}  	
	VS_HD_Reset();
	VS_Soft_Reset();
	vsset.mvol=220;						//默认设置音量为220.
	mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
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
   	curindex=playindex;											//从0开始显示
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); 	//打开目录
	while(res==FR_OK)//打开成功
	{	
				if(!firsttime)
		{
			firsttime=1;
			draw_dir(540,curindex);//显示歌曲目录，y,目录索引
		}
		if(lastupdate!=curindex)
		{
			draw_dir(540,curindex);//显示歌曲目录，y,目录索引
		}
		lastupdate=curindex-1;	
		
		
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
	
		strcpy((char*)pname,"0:/MUSIC/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(30,490+220,480,190+16,WHITE);				//清除之前的显示
		Show_Str(30,490+220,480-30,16,fn,16,0);				//显示歌曲名字 
		//Show_Str(30,760,480,16,fn,16,0);
		
		key=mp3_play_song(pname); 				 		//播放这个MP3  
		
	
		mp3_index_show(curindex+1,totmp3num);
		if(key==2)		//上一曲
		{
			if(curindex)
			{
				curindex--;
		
			}
			else curindex=totmp3num-1;
 		}else if(key<=1)//下一曲
		{
			curindex++;		   	
			if(curindex>=totmp3num)curindex=0;//到末尾的时候,自动从头开始
 		}else if(key == 3)
		{	
			
			//Show_Str(450-16*5-80-16*5,540+80,240,16,"点击右下角头像返回上一级/停止播放",16,0);
			LCD_Fill(450-16*5-80-16*5,540+80,500,540+80+16,WHITE);				//清除之前的显示
			LCD_Fill(30,400+220,250,800,WHITE);				//清除之前的显示
			LCD_Fill(450-16*5-80-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
			
			LCD_Fill(450-16*5-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
			Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
			delay_ms(3000);
			LCD_Fill(450-16*5-80-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
			Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
		  goto endplay;
		}
		else
			 break;	//产生了错误 	 
	}
endplay:	
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,mp3indextbl);			//释放内存	
	
}

void mp3playREC(u8 playindex)
{
	
	u8 firsttime=0;
	u8 lastupdate=0;
	
	u8 res;
 	DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totmp3num; 		//音乐文件总数
	u16 curindex;		//图片当前索引
	u8 key;				//键值		  
 	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
	//u8 looknumb=0;
	
 	while(f_opendir(&mp3dir,"0:/RECORDER"))//打开音乐文件夹
 	{	    
		Show_Str(30,190,240,16,"RECORDER文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,206,WHITE);//清除显示	     
		delay_ms(200);				  
	} 									  
	totmp3num=mp3_get_tnum("0:/RECORDER"); //得到总有效文件数
  	while(totmp3num==NULL)//音乐文件总数为0		
 	{	    
		Show_Str(30,190,240,16,"没有录音文件!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}										   
  	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);				//申请2*totmp3num个字节的内存,用于存放音乐文件索引
 	while(mp3fileinfo.lfname==NULL||pname==NULL||mp3indextbl==NULL)//内存分配出错
 	{	    
		Show_Str(30,190,240,16,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}  	
	VS_HD_Reset();
	VS_Soft_Reset();
	vsset.mvol=220;						//默认设置音量为220.
	mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
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
   	curindex=playindex;											//从0开始显示
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/RECORDER"); 	//打开目录
	while(res==FR_OK)//打开成功
	{	
				if(!firsttime)
		{
			firsttime=1;
			draw_dir_record(540,curindex);//显示歌曲目录，y,目录索引
		}
		if(lastupdate!=curindex)
		{
			draw_dir_record(540,curindex);//显示歌曲目录，y,目录索引
		}
		lastupdate=curindex-1;	
		
		
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
	
		strcpy((char*)pname,"0:/RECORDER/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(30,490+220,480,190+16,WHITE);				//清除之前的显示
		Show_Str(30,490+220,480-30,16,fn,16,0);				//显示歌曲名字 
		//Show_Str(30,760,480,16,fn,16,0);
		
		key=mp3_play_song(pname); 				 		//播放这个MP3  
		
	
		mp3_index_show(curindex+1,totmp3num);
		if(key==2)		//上一曲
		{
			if(curindex)
			{
				curindex--;
		
			}
			else curindex=totmp3num-1;
 		}else if(key<=1)//下一曲
		{
			curindex++;		   	
			if(curindex>=totmp3num)curindex=0;//到末尾的时候,自动从头开始
 		}else if(key == 3)
		{	
			
			//Show_Str(450-16*5-80-16*5,540+80,240,16,"点击右下角头像返回上一级/停止播放",16,0);
			LCD_Fill(450-16*5-80-16*5,540+80,500,540+80+16,WHITE);				//清除之前的显示
			LCD_Fill(30,400+220,250,800,WHITE);				//清除之前的显示
			LCD_Fill(450-16*5-80-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
			
			LCD_Fill(450-16*5-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
			Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
			delay_ms(3000);
			LCD_Fill(450-16*5-80-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
			Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
		  goto endplay;
		}
		else
			 break;	//产生了错误 	 
	}
endplay:	
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,mp3indextbl);			//释放内存	
	
}


void mp3playRECEIVE(u8 playindex)
{
	
	u8 firsttime=0;
	u8 lastupdate=0;
	
	u8 res;
 	DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totmp3num; 		//音乐文件总数
	u16 curindex;		//图片当前索引
	u8 key;				//键值		  
 	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
	//u8 looknumb=0;
	
 	while(f_opendir(&mp3dir,"0:/RECEIVE"))//打开音乐文件夹
 	{	    
		Show_Str(30,190,240,16,"RECEIVE文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,206,WHITE);//清除显示	     
		delay_ms(200);				  
	} 									  
	totmp3num=mp3_get_tnum("0:/RECEIVE"); //得到总有效文件数
  	while(totmp3num==NULL)//音乐文件总数为0		
 	{	    
		Show_Str(30,190+650,240,16,"没有接收文件!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190+650,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}										   
  	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);				//申请2*totmp3num个字节的内存,用于存放音乐文件索引
 	while(mp3fileinfo.lfname==NULL||pname==NULL||mp3indextbl==NULL)//内存分配出错
 	{	    
		Show_Str(30,190,240,16,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
		delay_ms(200);				  
	}  	
	VS_HD_Reset();
	VS_Soft_Reset();
	vsset.mvol=220;						//默认设置音量为220.
	mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
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
   	curindex=playindex;											//从0开始显示
   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/RECEIVE"); 	//打开目录
	while(res==FR_OK)//打开成功
	{	
				if(!firsttime)
		{
			firsttime=1;
			draw_dir_record(540,curindex);//显示歌曲目录，y,目录索引
		}
		if(lastupdate!=curindex)
		{
			draw_dir_record(540,curindex);//显示歌曲目录，y,目录索引
		}
		lastupdate=curindex-1;	
		
		
		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);		
	
		strcpy((char*)pname,"0:/RECEIVE/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Fill(30,490+220,480,190+16,WHITE);				//清除之前的显示
		Show_Str(30,490+220,480-30,16,fn,16,0);				//显示歌曲名字 
		//Show_Str(30,760,480,16,fn,16,0);
		
		key=mp3_play_song(pname); 				 		//播放这个MP3  
		
	
		mp3_index_show(curindex+1,totmp3num);
		if(key==2)		//上一曲
		{
			if(curindex)
			{
				curindex--;
		
			}
			else curindex=totmp3num-1;
 		}else if(key<=1)//下一曲
		{
			curindex++;		   	
			if(curindex>=totmp3num)curindex=0;//到末尾的时候,自动从头开始
 		}else if(key == 3)
		{	
			
			//Show_Str(450-16*5-80-16*5,540+80,240,16,"点击右下角头像返回上一级/停止播放",16,0);
			LCD_Fill(450-16*5-80-16*5,540+80,500,540+80+16,WHITE);				//清除之前的显示
			LCD_Fill(30,400+220,250,800,WHITE);				//清除之前的显示
			LCD_Fill(450-16*5-80-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
			
			LCD_Fill(450-16*5-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
			Show_Str(450-16*5-80-16*5,540+60,240,16,"返回成功",16,0);
			delay_ms(3000);
			LCD_Fill(450-16*5-80-16*5,540+60,500,540+60+16,WHITE);				//清除之前的显示
			Show_Str(450-16*5-80-16*5,540+60,240,16,"请选择功能",16,0);
		  goto endplay;
		}
		else
			 break;	//产生了错误 	 
	}
endplay:	
	f_closedir(&mp3dir);
	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,mp3indextbl);			//释放内存	
	
}
//播放一曲指定的歌曲				     	   									    	 
//返回值:0,正常播放完成
//		 1,下一曲
//       2,上一曲
//       0XFF,出现错误了
u8 mp3_play_song(u8 *pname)
{	 
 	FIL* fmp3;
    u16 br;
	u8 res,rval;	  
	u8 *databuf;	   		   
	u16 i=0; 
	u8 key;  	
	
//	u8 looknumb=0;
//	u16 totmp3num; 		//音乐文件总数

			   
	rval=0;	    
	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请内存
	databuf=(u8*)mymalloc(SRAMIN,4096);		//开辟4096字节的内存区域
	if(databuf==NULL||fmp3==NULL)rval=0XFF ;//内存申请失败.
	if(rval==0)
	{	  
	  	VS_Restart_Play();  					//重启播放 
		VS_Set_All();        					//设置音量等信息 			 
		VS_Reset_DecodeTime();					//复位解码时间 	  
		res=f_typetell(pname);	 	 			//得到文件后缀	 			  	 						 
		if(res==0x4c)//如果是flac,加载patch
		{	
			VS_Load_Patch((u16*)vs1053b_patch,VS1053B_PATCHLEN);
		}  				 		   		 						  
		res=f_open(fmp3,(const TCHAR*)pname,FA_READ);//打开文件	 
 		if(res==0)//打开成功.
		{ 
			VS_SPI_SpeedHigh();	//高速						   
			while(rval==0)
			{
				res=f_read(fmp3,databuf,4096,(UINT*)&br);//读出4096个字节  
				i=0;
				do//主播放循环
			    {  	
					if(VS_Send_MusicData(databuf+i)==0)//给VS10XX发送音频数据
					{
						i+=32;
					}else   
					{
						key=KEY_Scan(0);
						if(TPAD_Scan(0))
						{
							 rval = 3;
							return rval;
							
						}
						
						
//						tp_dev.scan(0);
//						if((tp_dev.sta)&(1<<0)) //
//						{
//								//查看目录上一个信息
//							//draw_button(430,16*5,"查看上一曲",16);
//							//LCD_DrawRectangle((450-16*5-100),(y-10),450-100+20,(y+size+10));
//							//LCD_DrawRectangle((450-16*5-100),(430-10),(450-100+20),(430+16+10));
//							if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(430-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(430+16+10))
//							{	
//									
//								if(looknumb)
//								{
//									looknumb--;
//							
//								}
//								else 
//								{
//									totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
//									looknumb=totmp3num-1;
//								}

//							}
//								//draw_button(500,16*5,"查看下一曲",16);
//								//LCD_DrawRectangle((450-16*5-100),(500-10),(450-100+20),(500+16+10));
//								if(tp_dev.x[0]>(450-16*5-100)&&tp_dev.y[0]>(500-10)&&tp_dev.x[0]<(450-100+20)&&tp_dev.y[0]<(500+16+10))
//							{
//								looknumb++;
//							}
//						}
//						draw_dir(540,looknumb);//显示歌曲目录，y,目录索引
//			
						
						
						switch(key)
						{
							case KEY0_PRES:
								rval=1;		//下一曲
							//return 0;
								break;
							case KEY2_PRES:
								rval=2;		//上一曲
								break;
							case WKUP_PRES:	//音量增加
								if(vsset.mvol<250)
								{
									vsset.mvol+=5;
						 			VS_Set_Vol(vsset.mvol);	
								}else vsset.mvol=250;
								mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
								break;
							case KEY1_PRES:	//音量减
								if(vsset.mvol>100)
								{
									vsset.mvol-=5;
						 			VS_Set_Vol(vsset.mvol);	
								}else vsset.mvol=100;
								mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
								break;
						}
						mp3_msg_show(fmp3->fsize);//显示信息	    
					}	    	    
				}while(i<4096);//循环发送4096个字节 
				if(br!=4096||res!=0)
				{
					rval=0;
					break;//读完了.		  
				} 							 
			}
			f_close(fmp3);
		}else rval=0XFF;//出现错误	   	  
	}						     	 
	myfree(SRAMIN,databuf);	  	 		  	    
	myfree(SRAMIN,fmp3);
	return rval;	  	 		  	    
}




























