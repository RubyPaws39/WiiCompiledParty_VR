#include "../../runtime/src/hle/input/wpad.cpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <tuple>
#include <vector>
static std::array<uint8_t,32> output;
static bool connected=true;
static std::vector<std::tuple<uint32_t,int32_t,uint32_t>> completions;
uint8_t* Memory::GetPointer(uint32_t address,size_t length) {
 if(address!=0x80010000 || length!=24)throw std::out_of_range("output bounds");
 return output.data();
}
void WiiRemoteInput::Poll() {}
bool WiiRemoteInput::IsRemoteChannel(uint32_t channel){return connected && channel==0;}
void NandQueueIosCallback(uint32_t cb,int32_t chan,uint32_t result){completions.emplace_back(cb,chan,result);}
int main(){
 output.fill(0xa5);
 assert(WPADGetInfoAsync_HLE(0,0x80010000,0x80001234)==0);
 assert(completions.size()==1 && completions[0]==std::make_tuple(0x80001234u,0,0u));
 assert(BigEndian::Read32(output.data())==1 && output[20]==4 && output[21]==1);
 for(size_t i=24;i<output.size();++i)assert(output[i]==0xa5);
 const auto before=output;connected=false;
 assert(WPADGetInfoAsync_HLE(0,0x80010000,0x80001234)==-1);
 assert(std::get<2>(completions.back())==uint32_t(-1) && output==before);
 assert(WPADGetInfoAsync_HLE(4,0x80010000,0x80001234)==-6);
 assert(std::get<1>(completions.back())==4 && std::get<2>(completions.back())==uint32_t(-6));
 connected=true;bool rejected=false;
 try{WPADGetInfoAsync_HLE(0,0,0);}catch(const std::invalid_argument&){rejected=true;}
 assert(rejected);const auto queued=completions.size();
 assert(WPADGetInfoAsync_HLE(0,0x80010000,0)==0 && completions.size()==queued);
 std::puts("WPAD info: deferred args, output extent, errors, null output PASS");
}