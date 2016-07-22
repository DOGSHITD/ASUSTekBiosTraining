# include<stdio.h>
# include<dos.h>
# define EC_SC 0x66
# define EC_DATA 0x62

typedef unsigned char UINT8;
typedef unsigned int  UINT16;
typedef unsigned long UINT32;

void waitOutputFull()
{
	UINT8 reg=0;
	reg=inp(EC_SC);
//	printf("waiting for command output buffer empty, reg is %x\n",reg);
	while((reg&0x01)!=0x01)
	{
		reg=inp(EC_SC);
	}
//	printf("empty ready, reg is %x\n",reg);
}
void waitInputEmpty()
{
	UINT8 reg=0;
	reg=inp(EC_SC);
//	printf("waiting for input buffer full, reg is %x\n",reg);
	while(reg&0x02)
	{
		reg=inp(EC_SC);
	}
//	printf("full ready, reg is %x\n",reg);
}
void writeSMB(UINT8 ec_space,UINT8 reg)
{
	waitInputEmpty();
	outp(EC_SC,0x81);		//EC SMBus Command register
	waitInputEmpty();
	outp(EC_DATA,ec_space);
	waitInputEmpty();
	outp(EC_DATA,reg);
}
UINT8 readSMB(UINT8 ec_space)
{
	UINT8 reg=0;
	waitInputEmpty();
	outp(EC_SC,0x80);
	waitInputEmpty();
	outp(EC_DATA,ec_space);
	waitOutputFull();
	reg=inp(0x62);
	return reg;
}
void showManufacturerDate()
{
	UINT8 reg,i,SMB_BCNT,DataBase=0x1C;
	UINT8 STS,COM;
	UINT16 year,month,day;
	UINT16 reg16=0;
	//Data Sent
	writeSMB(0x1A,0x16);		//SMB_ADDR
	writeSMB(0x1B,0x1B);		//SMB_CMD
	writeSMB(0x18,0x09);		//SMB_PRTCL

	//Data Return
	STS=readSMB(0x19);
	COM=readSMB(0x18);
	printf("STS is %x\n",STS);
	printf("COM is %x\n",COM);
	reg=readSMB(DataBase+1);
	printf("high %x\n",reg);
	reg16=reg;

	reg16=reg16<<8;
	printf("reg16 %x\n",reg16);
	reg=readSMB(DataBase+0);
	printf("low %x\n",reg);
	reg16=reg16|reg;
	year=reg16>>9;
	month=(reg16&0x01E0)>>5;
	day=(reg16&0x001F);
	year=year+1980;
	printf("reg16 is %x\n",reg16);
	printf("Date:%d:%d:%d\n",year,month,day);
}
void showManufacturerName()
{
	UINT8 reg,i,SMB_BCNT,DataBase=0x1C;
	UINT8 STS,COM;
	//Data Sent
	writeSMB(0x1A,0x16);		//SMB_ADDR
	writeSMB(0x1B,0x20);		//SMB_CMD
	writeSMB(0x18,0x0B);		//SMB_PRTCL
	
	//Data Return
	STS=readSMB(0x19);
	COM=readSMB(0x18);
	printf("STS is %x\n",STS);
	printf("COM is %x\n",COM);
	SMB_BCNT=readSMB(0x3C);
	printf("SMB_BCNT is %d\n",SMB_BCNT);
	for(i=0;i<SMB_BCNT;i++)
	{
		reg=readSMB(DataBase+i);
		printf("%c",reg,reg);
	}
	printf("\n");
}
void main()
{

	showManufacturerName();
	showManufacturerDate();
	printf("\n");
}