#include "nes_mapper.h"



// Mapper 87
void MAP87_Reset()
{
  // set CPU bank pointers
//  set_CPU_banks(0,1,2,3);

  // set PPU bank pointers
  if(num_1k_VROM_banks)
  {
    set_PPU_banks(0,1,2,3,4,5,6,7);
  }
}

void MAP87_MemoryWrite(uint16 addr, uint8 data)
{
  if(addr == 0x6000)
  {
    uint8 chr_bank = (data & 0x02) >> 1;
    
    set_PPU_bank0(chr_bank*8+0);
    set_PPU_bank1(chr_bank*8+1);
    set_PPU_bank2(chr_bank*8+2);
    set_PPU_bank3(chr_bank*8+3);
    set_PPU_bank4(chr_bank*8+4);
    set_PPU_bank5(chr_bank*8+5);
    set_PPU_bank6(chr_bank*8+6);
    set_PPU_bank7(chr_bank*8+7);
  }
}
void MAP87_Init()
{
	NES_Mapper->Reset = MAP87_Reset;
	NES_Mapper->Write = MAP87_MemoryWrite;
}
















