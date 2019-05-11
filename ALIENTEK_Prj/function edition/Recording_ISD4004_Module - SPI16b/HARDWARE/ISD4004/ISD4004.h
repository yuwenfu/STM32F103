#ifndef __ISD4004_H
#define __ISD4004_H	 
#include "sys.h"
#include "spi.h"
#include "delay.h"

//////////////////////////////////////////////////////////////////////////////////	 
//ISD4004��������	   
//�޸�����:2019/5/2
//�汾��V1.0								  
////////////////////////////////////////////////////////////////////////////////// 

/********************************************************************************/
//ָ���
//���ֻ���ڷ���������ʼʱѡ��
#define ISD4004_PwrUpEnable 0x2000  //�ϵ�:�ȴ�TPUD ���������Թ���
#define ISD4004_SetPlayAddr 0xE000 //��ָ����ַ��ʼ���������PLAY ָ���ʹ��������������ȥ
#define ISD4004_PlayAtCrtAddr 0xF000 //�ӵ�ǰ��ַ��ʼ����(ֱ��EOM ��OVF)
#define ISD4004_SetRecAddr 0xA000 //��ָ����ַ��ʼ¼�������REC ָ���ʹ¼������������ȥ
#define ISD4004_RecAtCrtAddr 0xB000  //�ӵ�ǰ��ַ��ʼ¼��(ֱ��OVF ��ֹͣ)
#define ISD4004_SetMc 0xE800 //��ָ����ַ��ʼ��������MC ָ���ʹ�������������ȥ
#define ISD4004_Mc 0xF8 //ִ�п��,ֱ��EOM.��������Ϣ,�����OVF ״̬
#define ISD4004_StopCrtWr 0x3000 //ֹͣ��ǰ����
#define ISD4004_StopWrPwrDn 0x1000 //ֹͣ��ǰ����������
//#define 4004_ReadStaus 0x30 //��״̬:OVF ��EOM



/********************************************************************************/

// PC0 CS_4004
// PC1 SCLK_4004
// PC2 MOSI_4004
// PC3 MISO_4004
// PC4 INT_4004
// PC5 RAC_4004

//#define CS_4004_H PCout(0)=1 // PC0=1
//#define CS_4004_L PCout(0)=0 // PC0=0

//#define SCLK_4004_H PCout(1)=1 // PC1=1
//#define SCLK_4004_L PCout(1)=0 // PC1=0

//#define MOSI_4004_H PCout(2)=1 // PC2=1
//#define MOSI_4004_L PCout(2)=0 // PC2=0

//#define MISO_4004_H PCout(3)=1 // PC3=1
//#define MISO_4004_L PCout(3)=0 // PC3=0

#define ISD4004_CS PCout(0) // PC0 SS�����
#define ISD4004_INT PCout(4) // PC4 INT �����
#define ISD4004_RAC_ PCout(5) // PC5 RAC �����

#define	W25QXX_CS 	PBout(12)  		//W25QXX��Ƭѡ�ź�


void ISD4004_Init(void);//��ʼ��
void ISD4004_PwrUp(void);//PWRUP
void ISD4004_StopMode(void);//ֹͣ¼��
void ISD4004_Rec(u16 addr); //��ʼ¼��
void ISD4004_PlayRec(u16 addr);//����¼��

void ISD4004(u16 ISD4004_order);
void ISD4004_StopPWRDN(void);

		 				    
#endif