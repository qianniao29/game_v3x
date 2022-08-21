#include <string.h>
#include <malloc.h>
#include <alsa/asoundlib.h>
#include <linux/input.h>
#include <sys/stat.h>

#include "nes_common.h"
#include "nes_main.h" 
#include "nes_ppu.h"
#include "nes_mapper.h"
#include "nes_apu.h"
#include "font_data.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序移植自网友ye781205的NES模拟器工程
//ALIENTEK STM32开发板
//NES主函数 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/7/1
//版本：V1.0  			  
////////////////////////////////////////////////////////////////////////////////// 	 

u32 RenderType = POST_RENDER;//此处为后面加的，对于有的游戏可能要选择不同的处理方式  VeiLiang
u32 framecnt = 0;	//nes帧计数器 
int MapperNo;			//map编号
int NES_scanline;		//nes扫描线
int VROM_1K_SIZE;
int VROM_8K_SIZE;
u32 NESrom_crc32;

u8 PADdata0 __attribute__ ((aligned (8)));  			//手柄1键值 [7:0]右7 左6 下5 上4 Start3 Select2 B1 A0  
u8 PADdata1 __attribute__ ((aligned (8)));   			//手柄2键值 [7:0]右7 左6 下5 上4 Start3 Select2 B1 A0  
u8 *NES_RAM __attribute__ ((aligned (8)));			//保持1024字节对齐
u8 *NES_RAM_NOALIGNED __attribute__ ((aligned (8)));
u8 *NES_SRAM __attribute__ ((aligned (8)));  
NES_header *RomHeader; 	//rom文件头
MAPPER *NES_Mapper;		 
MapperCommRes *MAPx;  
static u8 show_menu_mode = 0;
static u8 exit_select = 0;
static u8 exit_flag = 0;

u8* spr_ram __attribute__ ((aligned (8)));			//精灵RAM,256字节
ppu_data* ppu;			//ppu指针
u8* VROM_banks;
u8* VROM_tiles;

apu_t *apu; 			//apu指针
u16 *wave_buffers; 		
static snd_pcm_t *playback_handle;

u8* romfile __attribute__ ((aligned (8)));			//nes文件指针,指向整个nes文件的起始地址.
//////////////////////////////////////////////////////////////////////////////////////

 static u8 is_need_free = 0;
//加载ROM
//返回值:0,成功
//    1,内存错误
//    3,map错误
u8 nes_load_rom(void)
{  
    u8* p;  
	u8 i;
	u8 res=0;
	p=(u8*)romfile;	
	if(strncmp((char*)p,"NES",3)==0)
	{  
		RomHeader->ctrl_z=p[3];
		RomHeader->num_16k_rom_banks=p[4];
		RomHeader->num_8k_vrom_banks=p[5];
		RomHeader->flags_1=p[6];
		RomHeader->flags_2=p[7]; 
		if(RomHeader->flags_1&0x04)p+=512;		//有512字节的trainer:
		if(RomHeader->num_8k_vrom_banks>0)		//存在VROM,进行预解码
		{		
			VROM_banks=p+16+(RomHeader->num_16k_rom_banks*0x4000);
#if	NES_RAM_SPEED==1	//1:内存占用小 0:速度快	 
			VROM_tiles=VROM_banks;	 
#else  
			VROM_tiles=mymalloc(SRAMEX,RomHeader->num_8k_vrom_banks*8*1024);//这里可能申请多达1MB内存!!!
			if(VROM_tiles==0)
			{
				VROM_tiles=VROM_banks;//内存不够用的情况下,尝试VROM_titles与VROM_banks共用内存	
				is_need_free = 0;
			}
			else
			{
				is_need_free = 1;
			}
					
			compile(RomHeader->num_8k_vrom_banks*8*1024/16,VROM_banks,VROM_tiles);  
#endif	
		}
		else 
		{
			VROM_banks=mymalloc(SRAMIN,8*1024);
			VROM_tiles=mymalloc(SRAMEX,8*1024);
			is_need_free = 2;
			if(!VROM_banks||!VROM_tiles)res=1;
		}  	
		VROM_1K_SIZE = RomHeader->num_8k_vrom_banks * 8;
		VROM_8K_SIZE = RomHeader->num_8k_vrom_banks;  
		//VROM_8K_SIZE = RomHeader->num_16k_rom_banks * 2;//看论坛里面说改这个能解决一些rom 问题 --By VeiLiang
		MapperNo=(RomHeader->flags_1>>4)|(RomHeader->flags_2&0xf0);
		if(RomHeader->flags_2 & 0x0E)MapperNo=RomHeader->flags_1>>4;//忽略高四位，如果头看起来很糟糕 
		printf("use map:%d\r\n",MapperNo);
		if(MapperNo == 74) MapperNo = 4;//mapper74 其实和mapper4差不多
		for(i=0;i<255;i++)  // 查找支持的Mapper号
		{		
			if (MapTab[i]==MapperNo)break;		
			if (MapTab[i]==-1)res=3; 
		} 
		if(res==0)
		{
			switch(MapperNo)
			{
				case 1:  
					MAP1=mymalloc(SRAMIN,sizeof(Mapper1Res)); 
					if(!MAP1)res=1;
					break;
				case 4:  
				case 6: 
				case 16:
				case 17:
				case 18:
				case 19:
				case 21: 
				case 23:
				case 24:
				case 25:
				case 64:
				case 65:
				case 67:
				case 69:
				case 85:
				case 189:
				default:
					MAPx=mymalloc(SRAMIN,sizeof(MapperCommRes)); 
					if(!MAPx)res=1;
					break;  
			}
		}
		else
		{
			printf("Mapper%d not suppost!\r\n",MapperNo);
		}
		
	} 
	return res;	//返回执行结果
} 
//释放内存 
void nes_sram_free(void)
{ 
	myfree(SRAMIN,NES_RAM_NOALIGNED);		
	myfree(SRAMIN,NES_SRAM);	
	myfree(SRAMIN,RomHeader);	
	myfree(SRAMIN,NES_Mapper);
	myfree(SRAMIN,spr_ram);		
	myfree(SRAMIN,ppu);	
	myfree(SRAMIN,apu);	
	myfree(SRAMIN,wave_buffers);	
	myfree(SRAMEX,romfile);	  
	//if((VROM_tiles!=VROM_banks)&&VROM_banks&&VROM_tiles)//如果分别为VROM_banks和VROM_tiles申请了内存,则释放
	if(is_need_free>1)//处理一下，避免释放不存在的内存
	{
		myfree(SRAMIN,VROM_banks);
		myfree(SRAMEX,VROM_tiles);		 
	}
	else if(is_need_free)
	{
		myfree(SRAMEX,VROM_tiles);	
	}
	
	switch (MapperNo)//释放map内存
	{
		case 1: 			//释放内存
			myfree(SRAMIN,MAP1);
			break;	 	
		case 4: 
		case 6: 
		case 16:
		case 17:
		case 18:
		case 19:
		case 21:
		case 23:
		case 24:
		case 25:
		case 64:
		case 65:
		case 67:
		case 69:
		case 74:
		case 85:
		case 189:
		default:
			myfree(SRAMIN,MAPx);break;	 		//释放内存 
	}
	NES_RAM=0;
	NES_RAM_NOALIGNED = 0;
	NES_SRAM=0;
	RomHeader=0;
	NES_Mapper=0;
	spr_ram=0;
	ppu=0;
	apu=0;
	wave_buffers=0;
	romfile=0; 
	VROM_banks=0;
	VROM_tiles=0; 
	MAP1=0;
	MAPx=0;
} 
//为NES运行申请内存
//romsize:nes文件大小
//返回值:0,申请成功
//       1,申请失败
u8 nes_sram_malloc(u32 romsize)
{
	u16 i=0;
	 
	NES_RAM_NOALIGNED = mymalloc(SRAMIN,0Xc00);
	NES_RAM = (u8*)(((u32)NES_RAM_NOALIGNED & ~0x3ff) + 1024);  //1024字节对齐
 	NES_SRAM=mymalloc(SRAMIN,0X2000);
	RomHeader=mymalloc(SRAMIN,sizeof(NES_header));
	NES_Mapper=mymalloc(SRAMIN,sizeof(MAPPER));
	spr_ram=mymalloc(SRAMIN,0X100);		
	ppu=mymalloc(SRAMIN,sizeof(ppu_data));  
	apu=mymalloc(SRAMIN,sizeof(apu_t));		//sizeof(apu_t)=  12588
	wave_buffers=mymalloc(SRAMIN,APU_PCMBUF_SIZE*2);
 	romfile=mymalloc(SRAMEX,romsize);			//申请游戏rom空间,等于nes文件大小 
	if(romfile==NULL)//内存不够?释放主界面占用内存,再重新申请
	{
		//spb_delete();//释放SPB占用的内存
			printf("romfile malloc failed\r\n");
		romfile=mymalloc(SRAMEX,romsize);//重新申请
	}
	if(i==512||!NES_RAM_NOALIGNED||!NES_SRAM||!RomHeader||!NES_Mapper||!spr_ram||!ppu||!apu||!wave_buffers||!romfile)
	{
		printf("nes mem malloc failed,NES_RAM=%x,NES_SRAM=%x,RomHeader=%x,NES_Mapper=%x,spr_ram=%x,ppu=%x,apu=%x,wave_buffers=%x,romfile=%x\r\n",
			NES_RAM_NOALIGNED,NES_SRAM,RomHeader,NES_Mapper,spr_ram,ppu,apu,wave_buffers,romfile);
		nes_sram_free();
		return 1;
	}
	memset(NES_SRAM,0,0X2000);				//清零
	memset(RomHeader,0,sizeof(NES_header));	//清零
	memset(NES_Mapper,0,sizeof(MAPPER));	//清零
	memset(spr_ram,0,0X100);				//清零
	memset(ppu,0,sizeof(ppu_data));			//清零
	memset(apu,0,sizeof(apu_t));			//清零
	memset(wave_buffers,0,APU_PCMBUF_SIZE*2);//清零
	memset(romfile,0,romsize);				//清零 
	return 0;
} 
//开始nes游戏
//pname:nes游戏路径
//返回值:
//0,正常退出
//1,内存错误
//2,文件错误
//3,不支持的map
u8 nes_load(char *rom_file_name)
{
	FILE *rom_file = NULL;
	unsigned char *rom;
	int len = 0;
	u8 res=0;
	struct stat statbuff;

	if(stat(rom_file_name, &statbuff) < 0)
	{
		return -1;
	}

	rom_file = fopen(rom_file_name, "r");
	if(rom_file == NULL)
		return 2;

	len = statbuff.st_size;

	res=nes_sram_malloc(len);			//申请内存 
	if(res==0)
	{
		if(fread(romfile, 1, len, rom_file) < 0)
			goto __FUNC_RET__;
		NESrom_crc32=get_crc32(romfile+16, len-16);//获取CRC32的值	
		res=nes_load_rom();						//加载ROM
		if(res==0) 					
		{   
			
			cpu6502_init();						//初始化6502,并复位	 
			Mapper_Init();						//map初始化 	
			PPU_reset();						//ppu复位
			apu_init(); 						//apu初始化
			nes_sound_open(APU_PCMBUF_SIZE,APU_SAMPLE_RATE) ;
			nes_emulate_frame();				//进入NES模拟器主循环 
			nes_sound_close();					//关闭声音输出
		}
	}

__FUNC_RET__:
	fclose(rom_file);
	nes_sram_free();	//释放内存
	return res;
}  
u16 nes_xoff=LCD_NES_XOFFSET;	//显示在x轴方向的偏移量(实际显示宽度=256-2*nes_xoff)
u16 nes_yoff=LCD_NES_YOFFSET;	//显示在y轴方向的偏移量

//RGB屏需要的3个参数
//扩大4倍,参数计算方法(800*480为例):
//offset=lcdltdc.pixsize*(lcdltdc.pwidth*(lcdltdc.pheight-(i-sx)*2-1)+nes_yoff+LineNo) 
//offset=2*(800*(480+(sx-i)*2-1)+nes_yoff+LineNo)
//      =1600*(480+(sx-i)*2-1)+2*nes_yoff+LineNo*2
//      =766400+3200*(sx-i)+2*nes_yoff+LineNo*2 
//nes_rgb_parm1=766400
//nes_rgb_parm2=3200
//nes_rgb_parm3=nes_rgb_parm2/2

//不扩大,参数计算方法(480*272为例):
//offset=lcdltdc.pixsize*(lcdltdc.pwidth*(lcdltdc.pheight-(i-sx)-1)+nes_yoff+LineNo*2) 
//offset=2*(480*(272+sx-i-1)+nes_yoff+LineNo*2)
//      =960*(272+sx-i-1)+2*nes_yoff+LineNo*4
//      =260160+960*(sx-i)+2*nes_yoff+LineNo*4 
//nes_rgb_parm1=260160
//nes_rgb_parm2=960
//nes_rgb_parm3=nes_rgb_parm2/2

u32 nes_rgb_parm1;
u16 nes_rgb_parm2;
u16 nes_rgb_parm3;

extern int evdev_fd;

//读取游戏手柄数据
void nes_get_gamepadval(void)
{
    struct input_event in;
	uint8_t shift = 0;
    static uint64_t start_ms = 0;
	struct timeval tv_now;

	//4016  手柄1键值 [7:0]右7 左6 下5 上4 Start3 Select2 B1 A0
    while(read(evdev_fd, &in, sizeof(struct input_event)) > 0)
	{
		if(in.type == EV_KEY)
		{
			switch(in.code)
			{
				case KEY_UP:
					shift = 4;
					break;
				case KEY_DOWN:
				    shift = 5;
					break;
				case KEY_RIGHT:
				    shift = 7;
					break;
				case KEY_LEFT:
					shift = 6;
					break;
				case KEY_A:
				   shift = 0;
				   break;
				case KEY_B:
				   shift = 1;
				   break;
				case KEY_MENU:
				    shift = 2;
				   break;
				case KEY_ENTER:
				   shift = 3;
				   break;
				default:
				   goto __FUNC_RET__;
			}
			
			if(in.value)
			{
				PADdata0 |= 1 << shift;
				if(shift == 2)
				{
			        gettimeofday(&tv_now, NULL);
			        start_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;
				}
			}
			else
			{
				uint64_t now_ms;
				PADdata0 &= ~(1 << shift);
				if(shift == 2)
				{
					gettimeofday(&tv_now, NULL);
				    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;
					if(now_ms - start_ms > 1500)
						show_menu_mode = 1;
				}
			}
		}

    }
__FUNC_RET__:
	return;
}   


extern u32 *nes_fbp;

int show_exit_hz24(const u32 *hz_data, u16 x, u16 y)
{
	u32 *plcd= nes_fbp + x * LCD_HEIGHT + y;
	u32 i  = 0, j = 0;

	for(i = 0; i < 24; i++)
	{
		memcpy(&plcd[i * LCD_HEIGHT], &hz_data[i * 24], 24*4);
	}
}

int show_exit_menu(void)
{
	u32 *plcd= nes_fbp + EXIT_MENU_START_W * LCD_HEIGHT + EXIT_MENU_START_H;
	u32 i  = 0;

	for(i = 0; i < EXIT_MENU_WIDTH; i++)
	{
		memcpy(&plcd[i * LCD_HEIGHT], &exit_menu_frame[i * EXIT_MENU_HIGH], EXIT_MENU_HIGH*4);
	}
	if(exit_select)
	{	
		show_exit_hz24(exit_menu_hz_reverse[0], EXIT_MENU_START_W+60, EXIT_MENU_START_H+50);
		show_exit_hz24(exit_menu_hz[1], EXIT_MENU_START_W+156, EXIT_MENU_START_H+50);
	}
	else
	{	
		show_exit_hz24(exit_menu_hz[0], EXIT_MENU_START_W+60, EXIT_MENU_START_H+50);
		show_exit_hz24(exit_menu_hz_reverse[1], EXIT_MENU_START_W+156, EXIT_MENU_START_H+50);
	}

	return 0;
}

//nes模拟器主循环
void nes_emulate_frame(void)
{
#ifdef NES_SKIP_FRAM
	u8 nes_frame=0;
#endif
	u8 last_key = 0;

	while(1)
	{
		if(show_menu_mode)
		{
			nes_get_gamepadval();//每3帧查询一次USB
			
			if(last_key != PADdata0)
			{
				if(((PADdata0 ^ last_key) & 0x04) && !(PADdata0 & 0x04))
				{
					exit_select = !exit_select;
					if(exit_select)
					{	
						show_exit_hz24(exit_menu_hz_reverse[0], EXIT_MENU_START_W+60, EXIT_MENU_START_H+50);
						show_exit_hz24(exit_menu_hz[1], EXIT_MENU_START_W+156, EXIT_MENU_START_H+50);
					}
					else
					{	
						show_exit_hz24(exit_menu_hz[0], EXIT_MENU_START_W+60, EXIT_MENU_START_H+50);
						show_exit_hz24(exit_menu_hz_reverse[1], EXIT_MENU_START_W+156, EXIT_MENU_START_H+50);
					}	
				}
				else if(((PADdata0 ^ last_key) & 0x08) && !(PADdata0 & 0x08))
				{
					show_menu_mode = 0;
					if(exit_select)
					{
						exit_select = 0;
						return;
					}

					exit_select = 0;
				}
				
				last_key = PADdata0;
			}
			
			continue;
		}
		//printf("%d\r\n",nes_frame);
		// LINES 0-239
		PPU_start_frame();
		for(NES_scanline = 0; NES_scanline< 240; NES_scanline++)
		{
			run6502(113*256);
			NES_Mapper->HSync(NES_scanline);
	
#ifdef NES_SKIP_FRAM
			if(nes_frame==0)
				scanline_draw(NES_scanline);
			else
				do_scanline_and_dont_draw(NES_scanline); 
#else
			scanline_draw(NES_scanline);
#endif
		}  
#ifdef FRAME_IRQ	  
	  if(!(frame_irq_enabled & 0xC0))
	  {
		 CPU_IRQ;//cpu->DoPendingIRQ();
	  }
#endif	 

		NES_Mapper->VSync();
		NES_scanline=240;
		run6502(113*256);//运行1线
		NES_Mapper->HSync(NES_scanline); 

		if(RenderType < POST_RENDER)//这个也是根据实际rom区别的 
		{ 
			NES_scanline=240;  //243??    //冒险岛2，马戏团不运行这线
			run6502(113*256);//运行1线
			NES_Mapper->HSync(NES_scanline);
			//
		}

		start_vblank(); 
		if(NMI_enabled()) 
		{ 
			cpunmi=1;
			if(RenderType >= POST_RENDER)//有的要运行,有的rom不要 
			{
				run6502(7*256);//运行中断
			}
			
		}

		
		// LINES 242-261    
		for(NES_scanline=241;NES_scanline<262;NES_scanline++)
		{
			run6502(113*256);	  
			NES_Mapper->HSync(NES_scanline);		  
		}	   
		end_vblank(); 

		apu_soundoutput();//输出游戏声音
		nes_get_gamepadval();//每3帧查询一次USB
		if(show_menu_mode)
		{
			show_exit_menu();
			last_key = PADdata0;
		}
		framecnt++;
#ifdef NES_SKIP_FRAM
		nes_frame++;
		if(nes_frame>NES_SKIP_FRAME)nes_frame=0;//跳帧 
#endif
	}
}
//在6502.s里面被调用
void debug_6502(u16 reg0,u8 reg1)
{
	printf("6502 error:%x,%d\r\n",reg0,reg1);
}
////////////////////////////////////////////////////////////////////////////////// 	 
//nes,音频输出支持部分

//NES打开音频输出
int nes_sound_open(int samples_per_sync,int sample_rate) 
{
	//sample_rate 采样率 44100
	//samples_per_sync  735
	unsigned int rate      = sample_rate;
	snd_pcm_hw_params_t *hw_params;
	
	if(0 > snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) 
	{
		printf("snd_pcm_open err\n");
		return -1;
	}
	
	if(0 > snd_pcm_hw_params_malloc(&hw_params))
	{
		printf("snd_pcm_hw_params_malloc err\n");
		return -1;
	}
	
	if(0 > snd_pcm_hw_params_any(playback_handle, hw_params))
	{
		printf("snd_pcm_hw_params_any err\n");
		return -1;
	}
	if(0 > snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) 
	{
		printf("snd_pcm_hw_params_any err\n");
		return -1;
	}

	//8bit PCM 数据
	if(0 > snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_U16))
	{
		printf("snd_pcm_hw_params_set_format err\n");
		return -1;
	}

	if(0 > snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0)) 
	{
		printf("snd_pcm_hw_params_set_rate_near err\n");
		return -1;
	}

	//单声道 非立体声
	if(0 > snd_pcm_hw_params_set_channels(playback_handle, hw_params, 1))
	{
		printf("snd_pcm_hw_params_set_channels err\n");
		return -1;
	}

	if(0 > snd_pcm_hw_params(playback_handle, hw_params)) 
	{
		printf("snd_pcm_hw_params err\n");
		return -1;
	}
	
	snd_pcm_hw_params_free(hw_params);
	
	if(0 > snd_pcm_prepare(playback_handle)) 
	{
		printf("snd_pcm_prepare err\n");
		return -1;
	}
	return 1;
}
//NES关闭音频输出
void nes_sound_close(void) 
{ 
	snd_pcm_close(playback_handle);
} 
//NES音频输出到SAI缓存
void nes_apu_fill_buffer(int samples,u16* wavebuf)
{	
	int ret;

	ret = snd_pcm_writei(playback_handle, wavebuf, samples);
	if(-EPIPE == ret)
    {
        snd_pcm_prepare(playback_handle);
    }
} 
