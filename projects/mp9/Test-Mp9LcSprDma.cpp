#include "ppc_runtime.h"
#include "memory.h"
#include <array>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>
static std::array<uint8_t,8192> ram;
static std::array<uint8_t,16384> cache;
static unsigned notifications=0;
uint8_t* Memory::GetPointer(uint32_t address,size_t length) {
    if(address>=0x1000 && uint64_t(address)+length<=0x3000)return ram.data()+address-0x1000;
    if(address>=0xe0000000 && uint64_t(address)+length<=0xe0004000)return cache.data()+address-0xe0000000;
    throw std::out_of_range("test checked range");
}
extern "C" void GxNotifyGuestRamDmaWrite(uint32_t,uint32_t){++notifications;}
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void transfer(uint32_t address,uint32_t lc,unsigned lines,bool load,unsigned bits=2) {
    PPC_WriteSpr(922,(address&~31u)|((lines>>2)&31));
    PPC_WriteSpr(923,(0xe0000000u|lc)|((lines&3)<<2)|(load?16:0)|bits);
}
int main(){
    for(unsigned i=0;i<cache.size();++i)cache[i]=uint8_t(i*7+3);
    transfer(0x1000,0,2,false);
    check(std::memcmp(ram.data(),cache.data(),64)==0,"SPR store did not copy locked cache into RAM");
    check((PPC_ReadSpr(923)&2)==0,"DMA trigger did not clear");
    ram.fill(0x5a);cache.fill(0);
    transfer(0x1000,64,5,true);
    check(std::all_of(cache.begin()+64,cache.begin()+224,[](uint8_t v){return v==0x5a;}),"DMA load/count split failed");
    check(cache[63]==0 && cache[224]==0,"DMA load overwrote bounds");
    cache.fill(0x33);ram.fill(0);
    transfer(0x1000,0,0,false);
    check(ram[4095]==0x33 && ram[4096]==0,"zero count must mean 128 lines");
    cache.fill(0);cache[16352]=0x11;cache[0]=0x22;
    transfer(0x1000,16352,2,false);
    check(ram[0]==0x11 && ram[32]==0x22,"locked-cache boundary did not wrap");
    ram.fill(0x7e);transfer(0x1000,0,2,false,3);
    check(ram[0]==0x7e,"flush command must not transfer");
    transfer(0x1000,0,2,false,0);
    check(ram[0]==0x7e,"untriggered command must not transfer");
    const unsigned previous=notifications;bool rejected=false;
    try{transfer(0x2fe0,0,2,false);}catch(const std::out_of_range&){rejected=true;}
    check(rejected && notifications==previous && ram[0]==0x7e,"invalid DMA must fail before memory mutation");
    std::printf("LC SPR DMA store/load/count/wrap/flush/trigger/bounds PASS; notifications=%u\n",notifications);
}
