#include "std_include.hpp"

#pragma comment(linker, "/merge:.data=.cld")
#pragma comment(linker, "/merge:.rdata=.clr")
#pragma comment(linker, "/merge:.cl=.main")
#pragma comment(linker, "/merge:.text=.main")
#pragma comment(linker, "/base:0x400000")

#pragma bss_seg(".payload")
char payload_data[BINARY_PAYLOAD_SIZE];

extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 1;
__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;
}

extern "C" {
char* get_payload_data() { return payload_data; }
}
