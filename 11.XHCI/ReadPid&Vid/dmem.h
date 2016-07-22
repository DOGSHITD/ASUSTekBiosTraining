

uint32		p_to_le32(void *p);
//-uint32		p_to_l32(void *p);

void	memset32(uint32 addr,unsigned char pattern,unsigned int len);
int		memcpy32(uint32 dest,uint32 src,unsigned int size);



void*	dmalloc(unsigned long size, unsigned long align);
void	dmfree(void* ptr);

uint32	dmalloc32(unsigned long size,unsigned long align);
void	dmfree32(uint32 addr32);

void	dminstall(uint32 force=0);
void	dmuninstall();

void*		le32_to_p(uint32 addr);


