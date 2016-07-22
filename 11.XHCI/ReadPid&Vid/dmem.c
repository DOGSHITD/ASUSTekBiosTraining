/*	TAG		Date			Author		Description
 * 			20120229		Lynn	Modify the allocated memory size from 16K to 6K: EHCI use high memory now, 6K
 * 									is enough for OHCI and Misc use.
 *
			20091111		Yelt		support high memory request.
*/



#include	<stdio.h>
//-#include	<stdlib.h>
#include	<malloc.h>
#include	<stdarg.h>
#include	<mem.h>
#include	"mem_acc.h"
#include	"dmem.h"

#define		LIST_DEL(entry)				{if((entry)->prev) (entry)->prev->next = (entry)->next; if((entry)->next) (entry)->next->prev = (entry)->prev;}

#define		LIST_INSERT(cur, entry)		{ (entry)->next = (cur)->next;\
										  (entry)->prev = (cur); \
										  if((cur)->next) (cur)->next->prev = (entry);\
										  (cur)->next=(entry);}


uint32 memUsed=0;
uint32 memUsedMax=0;

static uint32 segbase=0;



#define	DMA_MEM_SIZE	(6*1024)			/* 6K Byte is enough for OHCI&Misc use */
//#define	DMA_MEM_SIZE	0x5000			/* 20K byte */
#define	DMEM_USED	0x0001

struct dmem_block {
	uint32	lbase;
	uint32	length;
	uint32	flag;
	
	void*	ptr;
	struct dmem_block *prev;
	struct dmem_block *next;
};

/*	head[0],	queue for 16bit memory
	head[1],	queue for 32bit memory
*/
struct dmem_block header[2];




void*		le32_to_p(uint32 addr) {
	if(segbase==0){
		asm{
			mov ax,ds
			mov segbase,ax
			}
		segbase=segbase<<4;
		}
	return (void*)(addr-segbase);
}

/*			covert a pointer to 32bit linear addr. */
uint32		p_to_le32(void *p) {
#if 0
	if(segbase==0){
		asm{
			mov ax,ds
			mov segbase,ax
			}
		segbase=segbase<<4;
		}
	return (segbase+(uint16)p);
#endif
	if(p) {
		return (((((uint32)p)>>16)<<4)+(uint16)p);
		}
	else {
		return NULL;
		}
}


void memset32(uint32 addr, unsigned char pattern, unsigned int len){
	unsigned int i;
	for(i=0;i<len;i++){
		Write_Mem_Byte(addr+i,pattern);
		}
}

int memcpy32(uint32 dest, uint32 src, unsigned int size){
	unsigned int i;
	for(i=0; i<size;i++){
		Write_Mem_Byte(dest+i,Read_Mem_Byte(src+i));
		}
}


void dumpdb(struct dmem_block* cur){
	printf("%04X:%04X/%08lX, %08lX, %08lX, %ld,<%04X,%04X>\n", cur, cur->ptr, p_to_le32(cur->ptr), cur->lbase, cur->length, cur->flag, cur->prev, cur->next);	
}

void dumphead(int index){	
	struct dmem_block* cur;
/*	for(cur=&header[0];cur!=NULL;cur=cur->next){
		dumpdb(cur);
		}
	for(cur=&header[1];cur!=NULL;cur=cur->next){
		dumpdb(cur);
		}	*/

	for(cur=&header[index];cur!=NULL;cur=cur->next){
		dumpdb(cur);
		}

}

void	dminstall(uint32 force){
	struct dmem_block* head;
	
	memset(&header, 0, 2*sizeof(struct dmem_block));

	head = &header[0];
		head->ptr = malloc(DMA_MEM_SIZE);
		if(head->ptr == NULL){
			printf("dmem alloc memory fail\n");
			return;
			}
		head->lbase = p_to_le32(head->ptr);
		head->length = DMA_MEM_SIZE;

	head = &header[1];
	
	if(force){
		head->lbase = force;
//		head->length = DMA_MEM_SIZE;
		head->length =  0x00100000;	/* 1M */
		}

}

void	dmuninstall(){
	struct dmem_block* head;
	head = &header[0];

//2011.06.08. for record memory using.///// 		
printf("Max memory consume: 0x%X / 0x4000 (not in clude fragment).\n",memUsedMax/*,DMA_MEM_SIZE*/);	

	if(head)
	{
		if(head->ptr)
		{
printf("head[0] ptr: %p next: %p\n", head->ptr, head->next);
			free(head->ptr);
		}
		memset(head, 0, sizeof(struct dmem_block));
//		free((void*)head);
//		head=NULL;
	}
	else
	{
		printf("head null\n");
	}
printf("head[1] ptr: %p next: %p\n", header[1].ptr, header[1].next);
}

/* 	alloc a memory block, 
 * @flag:		0 - request 16bit memory
 			1 - request 32bit memory
 */

struct dmem_block* dmalloc_core(unsigned long size, unsigned long align, int flag) {
	struct dmem_block* cur, * dummy, *head;
	int i;
	uint32 mask;
	uint32 Boundary_Size=0x1000;				/* 4K */

	head = &header[flag];
	
	if(head == NULL){
		dminstall();		
		head = &header[flag];
		}

	if(head == NULL){
		return NULL;
		}

	// format align, only 1,2,4,8,16,32,64,128,256.... 8*1024 is valid
	for(i=0; i<32; i++){
		if(align<=(1L<<i)){
			mask=(1L<<i)-1;
			break;
			}
		}

//	fprintf(dbg, "malloc size=%ld,%ld, %lx\n", size, align, mask);
if((align==0x1000)/* && (_debug != 0)*/)
	dumphead(0);

	for(cur=head;cur!=NULL;cur=cur->next){
		if(!(cur->flag&DMEM_USED)) {
			/* now, cur is a unused block  */
			uint32 start=cur->lbase; 		//p_to_le32(cur->ptr);
			uint32 end = start+cur->length;
			uint32 next = (start+mask)&(~mask);			/* next valid address */

			if(((next+size)<=end) && ((next/Boundary_Size)==((next+size-1)/Boundary_Size)))	/* is long enough ?? */
			{			
				if((next+size)<end){
					/* there is a fragment at the block tail, isolate it */
					dummy=(struct dmem_block*) malloc(sizeof(struct dmem_block));
					if(!dummy){
						printf("alloc new block fail\n");
						return NULL;
						}
					memset(dummy, 0, sizeof(struct dmem_block));
					
					if(cur->ptr){
						dummy->ptr = (void*)((uint32)(cur->ptr)+next+size-start);
						}
					dummy->lbase = next+size;
					dummy->length = end-(next+size);

					cur->length -= dummy->length;
					LIST_INSERT(cur,dummy);
					}

				if(next>start){
					dummy=(struct dmem_block*) malloc(sizeof(struct dmem_block));
					if(!dummy){
						printf("alloc new block fail\n");
						return 0;
						}
					memset(dummy, 0, sizeof(struct dmem_block));

					dummy->length = size;
					dummy->lbase = cur->lbase + cur->length - size;
					if(cur->ptr) {
						dummy->ptr = (void*)((uint32)(cur->ptr)+ cur->length-size);
						}
					cur->length -= dummy->length;
					
					LIST_INSERT(cur,dummy);
					cur = dummy;
					}
				cur->flag = DMEM_USED;
				
//2011.06.08. for record memory using.///// 			
if(flag==0)//only record low mem.
{
	memUsed=memUsed+cur->length;
	if(memUsed>memUsedMax)
	{
		memUsedMax=memUsed;
//dumphead(0);		
	}
}

				return cur;
			}
			else if(((next/Boundary_Size) != (next+size-1)/Boundary_Size)
				&& (((next & (~(Boundary_Size -1))) + Boundary_Size + size) <= end))
			{
				/* remaining at the end of memory block */
				if(((next & (~(Boundary_Size -1))) + Boundary_Size + size) <= end)
				{
					dummy=(struct dmem_block*) malloc(sizeof(struct dmem_block));
					if(!dummy)
					{
						printf("alloc new block fail\n");
						return NULL;
					}
					memset(dummy, 0, sizeof(struct dmem_block));
					
					next = next - (next & (Boundary_Size - 1)) + Boundary_Size;
					dummy->lbase = next + size;
					dummy->length = end - (next + size);
					if(cur->ptr != NULL)
					{
						dummy->ptr = (void *)(((uint32)cur->ptr) + (next + size - start));
					}
					LIST_INSERT(cur, dummy);

					cur->length -= dummy->length;					
				}

				if(1/*next>start*/)	//must have a boundary fragment
				{
					dummy=(struct dmem_block*) malloc(sizeof(struct dmem_block));
					if(!dummy)
					{
						printf("alloc new block fail\n");
						return NULL;
					}
					memset(dummy, 0, sizeof(struct dmem_block));

					dummy->flag = DMEM_USED;
					dummy->length = size;
					dummy->lbase = next;
					if(cur->ptr != NULL)
					{
						dummy->ptr = (void *)(((uint32)cur->ptr) + (next - start));
					}
					LIST_INSERT(cur, dummy);

					cur->length -= dummy->length;
				}				

//2011.06.08. for record memory using.///// 			
if(flag==0)//only record low mem.
{
	memUsed=memUsed+cur->length;
	if(memUsed>memUsedMax)
	{
		memUsedMax=memUsed;
//dumphead(0);		
	}
}

				return dummy;
			}
		}
	}

	return NULL;

	
}

void	dmfree_core(uint32 addr32, int flag) {
		struct dmem_block *cur, *prev, *next, *head;
		head = &header[flag];
	
	//fprintf(dbg, "free %lx, %x\n", p_to_le32(ptr), ptr);
		for(cur=head; cur!=NULL; cur=cur->next) {
			if(addr32==(cur->lbase)){
				cur->flag &= ~(DMEM_USED);

//2011.06.08. for record memory using./////				
if(flag==0)//only record low mem.
{
	memUsed=memUsed-cur->length;
}

				if(cur->prev){
					prev=cur->prev;
					if(!(prev->flag & DMEM_USED)) {
						if((prev->lbase+prev->length)== (cur->lbase)){
							prev->length += cur->length;
							LIST_DEL(cur);
							free(cur);
							
							cur=prev;
							}
						}
					}
	
				if(cur->next){
					next=cur->next;
					if(!(next->flag & DMEM_USED)) {
						if((cur->lbase+cur->length)==(next->lbase)){
							cur->length += next->length;
							LIST_DEL(next);
	/*
							if(next->prev)
								next->prev->next = next->next;
							if(next->next)
								next->next->prev = next->prev;
	*/
							free(next);
							}
						}
					}
				break;			
				}
			}
}

uint32	dmalloc32(unsigned long size, unsigned long align) {
	struct dmem_block* dummy = dmalloc_core(size, align, 1);
	if(!dummy) {
		return 0;
		}
	return dummy->lbase;
}

void	dmfree32(uint32 addr32) {
	dmfree_core(addr32, 1);
}

void*	dmalloc(unsigned long size, unsigned long align) {
	struct dmem_block* dummy = dmalloc_core(size, align, 0);
	if(!dummy) {
		return NULL;
		}
	return dummy->ptr;
}

void	dmfree(void* ptr){
	dmfree_core(p_to_le32(ptr), 0);
}

