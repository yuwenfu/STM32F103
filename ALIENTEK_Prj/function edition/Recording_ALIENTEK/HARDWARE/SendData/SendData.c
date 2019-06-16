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
#include "usart.h"	
#include "SendData.h"

#include "tpad.h"	
#include "mrrlcd.h"

extern u16 get_txtnum(u8 *path);


u8 DataSendInit(void)
{
	u8 res;u8 rval;
	
	
 	DIR mp3dir;	 		//目录
	FILINFO mp3fileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totmp3num; 		//音乐文件总数
	u16 curindex;		//图片当前索引
//	u8 key;				//键值		  
 	u16 temp;
	u16 *mp3indextbl;	//音乐索引表 
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
 		LCD_Fill(30,190,240,190+16,WHITE);				//清除之前的显示
		Show_Str(30,190,240-30,16,fn,16,0);				//显示歌曲名字 
		mp3_index_show(curindex+1,totmp3num);
		
		//Show_Str(30,450,310,24,pname,24,0);
		//Show_Str(30,480,310,24,fn,24,0);
		
		if(DataSend(0,pname,1))
		{	LCD_ShowString(30,300,310,16,24,"music send fail!");
			rval =1;
			return rval;
		}
		else
		{
			LCD_ShowString(30,300,310,16,24,"music send succeed!");
			break;
		}
		
			


	}
		f_closedir(&mp3dir);
		myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,mp3indextbl);			//释放内存	 
	rval=0;
	return rval;
}





//rval=0XFF ;//失败.
//rval = 0;  //成功
//type=0 传输音乐
//type=1 传输录音文件
//type=2 传输文本文件
u8 DataSend(u8 type,u8 *pname,u16 rate)//us为单位
{		
	u8 *rpname=0;
	FIL* fmp3;
	FIL* fmw;
    u16 br;
	u16 bw;
	u8 res,res1,rval;	  
	u8 *databuf,*p,tmp;	   		   
	u16 j=0;
	//Show_Str(30,510,310,24,pname,24,0);	

	rval=0;	    
	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请内存
	fmw=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请内存
	
	rpname=mymalloc(SRAMIN,30);
	
	databuf=(u8*)mymalloc(SRAMIN,4096);		//开辟4096字节的内存区域
	if(databuf==NULL||fmp3==NULL)rval=0XFF ;//内存申请失败.
	//Show_Str(30,5,310,24,pname,24,0);	 //调试
	//LCD_ShowNum(30,5,rval,2,16);  						//显示一个数字
	if(rval==0)
	{	
		//create_new_pathname(rpname);
		if(type==2)
		{
			create_nsendtxt_pathname(rpname);
		}
		else if(type==1)
		{
			create_nsendrec_pathname(rpname);
		}else
		{
			create_nsendmsc_pathname(rpname);
		}
		
		
		//LCD_ShowNum(30,5,rval,2,16); 
		//Show_Str(30,50,310,24,pname,24,0);	 //调试
		res1 = f_open(fmw,(const TCHAR*)rpname,FA_CREATE_ALWAYS|FA_WRITE);
		
		res=f_open(fmp3,(const TCHAR*)pname,FA_READ);//打开文件	
		
		
		//Show_Str(30,110,310,24,pname,24,0);	 //调试

 		if(res==0)//打开成功.
		{ 
		
			
				while(res1 == FR_OK && fmp3->fptr < fmp3->fsize)//文件是否发送完成
					{

						res=f_read(fmp3,databuf,4096,(UINT*)&br);//读出4096个字节  
						p = databuf;
				
						f_write(fmw,databuf,br,(UINT*)&bw);//4096个字节
						tmp=*p;
						
						//LCD_ShowString(30,360,310,16,24,&tmp);
						//LCD_ShowxNum(30,360,tmp,sizeof(tmp),16,0);
						for(j=0;j<8;j++)//OOK调制 高位MSB先发
						{
							if(tmp&0x80)		//高电平
							{
								LED1=0;
								LED0=0;
								/***************************/
								//添加引脚操作函数
								
								
								
								/***************************/
								//LCD_ShowString(30,380,310,16,24,"1");
							}
							else				//低电平
							{
								LED1=1;
								LED0=1;
								/***************************/
								//添加引脚操作函数
								
								
								
								/***************************/
								
								//LCD_ShowString(30,380,310,16,24,"0");
							}
							tmp<<=1;
							delay_us(rate); //控制调制速率 
						}
							p++;
						if(TPAD_Scan(0))//头像 返回上一级
						{
							rval=0xFF;
							f_close(fmp3);	
							f_close(fmw);
							
							myfree(SRAMIN,databuf);	  	 		  	    
							myfree(SRAMIN,fmp3);
							myfree(SRAMIN,fmw);
							return rval;
						}
 					 }		
						rval = 0;
		}	
		else 	
		rval=0XFF;//出现错误	   	  
	}	
	//mp3_msg_show(fmp3->fsize);//显示信息	  	
	f_close(fmp3);	
	f_close(fmw);
	
	myfree(SRAMIN,databuf);	  	 		  	    
	myfree(SRAMIN,fmp3);
	myfree(SRAMIN,fmw);
	return rval;

}




//通过时间获取文件名
//仅限在SD卡保存,不支持FLASH DISK保存
//组合成:形如"0:RECORDER/REC20120321210633.wav"的文件名
void create_new_pathname(u8 *pname)
{	 
	u8 res;					 
	u16 index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:RECEIVE/REV%05d.wav",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//尝试打开这个文件
		if(res==FR_NO_FILE)break;		//该文件名不存在=正是我们需要的.
		index++;
	}
}
void create_nsendmsc_pathname(u8 *pname)
{	 
	u8 res;					 
	u16 index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:RECEIVE/SENDMSC%03d.wav",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//尝试打开这个文件
		if(res==FR_NO_FILE)break;		//该文件名不存在=正是我们需要的.
		index++;
	}
}
void create_nsendrec_pathname(u8 *pname)
{	 
	u8 res;					 
	u16 index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:RECEIVE/SENDREC%03d.wav",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//尝试打开这个文件
		if(res==FR_NO_FILE)break;		//该文件名不存在=正是我们需要的.
		index++;
	}
}

void create_nsendtxt_pathname(u8 *pname)
{	 
	u8 res;					 
	u16 index=0;
	FIL* fp;
	
		while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:RECEIVE/SENDtxt%03d.txt",index);
		//Show_Str(30,0,310,24,pname,24,0);	 //调试
		res=f_open(fp,(const TCHAR*)pname,FA_READ);//尝试打开这个文件
		
		//Show_Str(30,0,310,24,pname,24,0);	 //调试
		if(res==FR_NO_FILE)break;		//该文件名不存在=正是我们需要的.
		index++;
	}
}







//音乐发送程序
//rval= 0发送成功
u8 musictransfer(u8 number,u16 rate)
{
			u8 rval;
			u8 res;
			u8 *fn;   			//长文件名
			u8 *pname;			//带路径的文件名
			u16 curindex;		//图片当前索引
			u16 temp;
			u16 *mp3indextbl;	//音乐索引表 
			DIR mp3dir;	 		//目录
			FILINFO mp3fileinfo;//文件信息
		u16 totmp3num; 		//音乐文件总数
		totmp3num=0; 
		totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
		mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
		mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
		pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
			
		mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
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
					if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	//表格定义在exfuns.c文件
					{
						mp3indextbl[curindex]=temp;//记录索引
						curindex++;
					}	    
				} 
			}   
			f_closedir(&mp3dir);
			
			curindex=number;											//从0开始显示
			res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); 	//打开目录													
			dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
			res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
			//if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
			fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);																
			strcpy((char*)pname,"0:/MUSIC/");				//复制路径(目录)
			strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
			
			Show_Str(30,730,240,16,pname,16,0); //调试
			
			
			
			// 发送开始传输提示信息
			LCD_Fill(30,750,240,800,WHITE);				//清除之前的显示
			Show_Str(30,750,240,16,"数据发送......",16,0);
			
			//u8 DataSend(u8 *pname,rate);//us为单位的调制速率
			//1KH调制速率
			if(DataSend(0,pname,rate)==0) //发送成功
			{
				// 发送传输完成提示信息
				LCD_Fill(30,750,240,800,WHITE);				//清除之前的显示
				Show_Str(30,750,240,16,"数据发送完成",16,0);
				rval = 0;
			
			}else
			{
			
				// 发送传输终止/失败提示信息
				LCD_Fill(30,750,240,800,WHITE);				//清除之前的显示
				Show_Str(30,750,240,16,"数据发送失败/终止",16,0);
				f_closedir(&mp3dir);
				myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
				myfree(SRAMIN,pname);				//释放内存		
				myfree(SRAMIN,mp3indextbl);			//释放内存
				delay_ms(3000);
				rval = 0xff;
			}
			
			
			f_closedir(&mp3dir);
			myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
			myfree(SRAMIN,pname);				//释放内存		
			myfree(SRAMIN,mp3indextbl);			//释放内存
			return rval;

}

//录音文件发送程序
//rval= 0发送成功
u8 recordertransfer(u8 number,u16 rate)
{
			u8 rval;
			u8 res;
			u8 *fn;   			//长文件名
			u8 *pname;			//带路径的文件名
			u16 curindex;		//图片当前索引
			u16 temp;
			u16 *mp3indextbl;	//音乐索引表 
			DIR mp3dir;	 		//目录
			FILINFO mp3fileinfo;//文件信息
		u16 totmp3num; 		//音乐文件总数
		totmp3num=0; 
		totmp3num=mp3_get_tnum("0:/RECORDER"); //得到总有效文件数
		mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
		mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
		pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
			
		mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
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
					if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	//表格定义在exfuns.c文件
					{
						mp3indextbl[curindex]=temp;//记录索引
						curindex++;
					}	    
				} 
			}   
			f_closedir(&mp3dir);
			
			curindex=number;											//从0开始显示
			res=f_opendir(&mp3dir,(const TCHAR*)"0:/RECORDER"); 	//打开目录													
			dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
			res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
			//if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
			fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);																
			strcpy((char*)pname,"0:/RECORDER/");				//复制路径(目录)
			strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
			
			Show_Str(30,730,240,16,pname,16,0); //调试
			
			
			
			// 发送开始传输提示信息
			LCD_Fill(30,750,240,800,WHITE);				//清除之前的显示
			Show_Str(30,750,240,16,"数据发送......",16,0);
			
			//u8 DataSend(u8 *pname,rate);//us为单位的调制速率
			//1KH调制速率
			if(DataSend(1,pname,rate)==0) //发送成功
			{
				// 发送传输完成提示信息
				LCD_Fill(30,750,240,800,WHITE);				//清除之前的显示
				Show_Str(30,750,240,16,"数据发送完成",16,0);
				rval = 0;
			
			}else
			{
			
				// 发送传输终止/失败提示信息
				LCD_Fill(30,750,240,800,WHITE);				//清除之前的显示
				Show_Str(30,750,240,16,"数据发送失败/终止",16,0);
				f_closedir(&mp3dir);
				myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
				myfree(SRAMIN,pname);				//释放内存		
				myfree(SRAMIN,mp3indextbl);			//释放内存
				delay_ms(3000);
				rval = 0xff;
			}
			
			
			f_closedir(&mp3dir);
			myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
			myfree(SRAMIN,pname);				//释放内存		
			myfree(SRAMIN,mp3indextbl);			//释放内存
			return rval;

}

//文本发送程序
//rval= 0发送成功
u8 txttransfer(u8 number,u16 rate)
{
			u8 rval;
			u8 res;
			u8 *fn;   			//长文件名
			u8 *pname;			//带路径的文件名
			u16 curindex;		//图片当前索引
			u16 temp;
			u16 *mp3indextbl;	//音乐索引表 
			DIR mp3dir;	 		//目录
			FILINFO mp3fileinfo;//文件信息
		u16 totmp3num; 		//音乐文件总数
		totmp3num=0; 
		totmp3num=get_txtnum("0:/TEXT"); //得到总有效文件数
		mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
		mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
		pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
			
		mp3indextbl=mymalloc(SRAMIN,2*totmp3num);	
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
					if((res&0XF0)==0X30)//取高四位,看看是不是音乐文件	//表格定义在exfuns.c文件
					{
						mp3indextbl[curindex]=temp;//记录索引
						curindex++;
					}	    
				} 
			}   
			f_closedir(&mp3dir);
			
			curindex=number;											//从0开始显示
			res=f_opendir(&mp3dir,(const TCHAR*)"0:/TEXT"); 	//打开目录													
			dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
			res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
			//if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
			fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);																
			strcpy((char*)pname,"0:/TEXT/");				//复制路径(目录)
			strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
			
			Show_Str(30,730,240,16,pname,16,0); //调试
			
			
			
			// 发送开始传输提示信息
			LCD_Fill(30,750,240,800,WHITE);				//清除之前的显示
			Show_Str(30,750,240,16,"数据发送......",16,0);
			
			//u8 DataSend(u8 *pname,rate);//us为单位的调制速率
			//1KH调制速率
			if(DataSend(2,pname,rate)==0) //发送成功
			{
				// 发送传输完成提示信息
				LCD_Fill(30,750,240,800,WHITE);				//清除之前的显示
				Show_Str(30,750,240,16,"数据发送完成",16,0);
				rval = 0;
			
			}else
			{
			
				// 发送传输终止/失败提示信息
				LCD_Fill(30,750,240,800,WHITE);				//清除之前的显示
				Show_Str(30,750,240,16,"数据发送失败/终止",16,0);
				f_closedir(&mp3dir);
				myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
				myfree(SRAMIN,pname);				//释放内存		
				myfree(SRAMIN,mp3indextbl);			//释放内存
				delay_ms(3000);
				rval = 0xff;
			}
			
			
			f_closedir(&mp3dir);
			myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
			myfree(SRAMIN,pname);				//释放内存		
			myfree(SRAMIN,mp3indextbl);			//释放内存
			return rval;

}


//u8 DataSend(u8 *pname)
//{
//			
//	FIL* fmp3;
//	FIL* fmw;
//    u16 br;
//	u16 bw;
//	u8 res,rval;	  
//	u8 *databuf,*p,tmp;	   		   
//	u16 i=0,j=0;
//	Show_Str(30,510,310,24,pname,24,0);	

//	rval=0;	    
//	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请内存
//	fmw=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请内存
//	databuf=(u8*)mymalloc(SRAMIN,4096);		//开辟4096字节的内存区域
//	if(databuf==NULL||fmp3==NULL)rval=0XFF ;//内存申请失败.
//	if(rval==0)
//	{	

//		
//		f_open(fmw,(const TCHAR*)"0:/TEXT/test.mp3",FA_CREATE_ALWAYS|FA_WRITE);
//		res=f_open(fmp3,(const TCHAR*)pname,FA_READ);//打开文件	

// 		if(res==0)//打开成功.
//		{ 
//				res=f_read(fmp3,databuf,4096,(UINT*)&br);//读出4096个字节  
//				p = databuf;
//				
//				f_write(fmw,databuf,br,(UINT*)&bw);//4096个字节
//			
//			
//				i=0;
//				do
//				{
//					tmp=*p;
//					//LCD_ShowString(30,360,310,16,24,&tmp);
//					//LCD_ShowxNum(30,360,tmp,sizeof(tmp),16,0);
//					for(j=0;j<8;j++)
//					{
//						if(tmp&0x80)
//						{
//							LED1=0;
//							LED0=0;
//							//LCD_ShowString(30,380,310,16,24,"1");
//						}
//						else
//						{
//							LED1=1;
//							LED0=1;
//							//LCD_ShowString(30,380,310,16,24,"0");
//						}
//						tmp<<=1;
//						delay_us(1);
//					}
//					
//					i++;
//					p++;
//					
//				}while(i<4096);//循环发送4096个字节 
//				
//				if(br!=4096||res!=0)
//				{
//					rval=0;	  
//				} 							 
//		}	
//		else 	
//		rval=0XFF;//出现错误	   	  
//	}	
//	//mp3_msg_show(fmp3->fsize);//显示信息	  	
//	f_close(fmp3);	
//	f_close(fmw);
//	
//	myfree(SRAMIN,databuf);	  	 		  	    
//	myfree(SRAMIN,fmp3);
//	myfree(SRAMIN,fmw);
//	return rval;

//}
/************************************************************************************/













///*-----------------------------------------------------------------------*/
///* 示例代码：数据传输函数，将被f_forward函数调用                                 */
///*-----------------------------------------------------------------------*/
// 
//UINT out_stream (   /* 返回已发送字节数或流状态 */
//    const BYTE *p,  /* 将被发送的数据块的指针 */
//    UINT btf        /* >0: 传输调用(将被发送的字节数)。0: 检测调用 */
//)
//{
//    UINT cnt = 0;
// 
//    if (btf == 0) {     /* 检测调用 */
//        /* 返回流状态(0: 忙，1: 就绪) */
//        /* 当检测调用时，一旦它返回就绪，那么在后续的传输调用时，它必须接收至少一个字节，或者f_forward将以FR_INT_ERROR而失败。 */
//        if (FIFO_READY) cnt = 1;
//    }
//    else {              /* 传输调用 */
//        do {    /* 当有数据要发送并且流就绪时重复 */
//            FIFO_PORT = *p++;
//            cnt++;
//        } while (cnt < btf && FIFO_READY);
//    }
// 
//    return cnt;
//}


///*-----------------------------------------------------------------------*/
///* 示例代码：使用f_forward函数                                                    */
///*-----------------------------------------------------------------------*/
// 
//FRESULT play_file (
//    char *fn        /* 待播放的音频文件名的指针 */
//)
//{
//    FRESULT rc;
//    FIL fil;
//    UINT dmy;
// 
//    /* 以只读模式打开音频文件 */
//    rc = f_open(&fil, fn, FA_READ);
// 
//    /* 重复，直到文件指针到达文件结束位置 */
//    while (rc == FR_OK && fil.fptr < fil.fsize) {
// 
//        /* 任何其他处理... */
// 
//        /* 定期或请求式填充输出流 */
//        rc = f_forward(&fil, out_stream, 1000, &dmy);
//    }
// 
//    /* 该只读的音频文件对象不需要关闭就可以被丢弃 */
//    return rc;
//}
//	



