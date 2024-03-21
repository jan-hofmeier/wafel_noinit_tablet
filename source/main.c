#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <wafel/dynamic.h>
#include <wafel/ios_dynamic.h>
#include <wafel/utils.h>
#include <wafel/patch.h>
#include <wafel/ios/svc.h>
#include <wafel/ios/memory.h>
#include <wafel/ios/prsh.h>

#define PON_SMC_TIMER               (0x00020000)

bool is_ecomode(void){
    boot_info_t *boot_info;
    size_t boot_info_size;
    int res = prsh_get_entry("boot_info", (void**)&boot_info, &boot_info_size );
    if(res){
        debug_printf("Error getting boot_info from PRSH\n");
        return false;
    }
    if(sizeof(boot_info_t) != boot_info_size){
        debug_printf("boot_info has wrong size\n");
    }
    return !!(boot_info->boot_state & PON_SMC_TIMER);
}

// This fn runs before everything else in kernel mode.
// It should be used to do extremely early patches
// (ie to BSP and kernel, which launches before MCP)
// It jumps to the real IOS kernel entry on exit.
__attribute__((target("arm")))
void kern_main()
{
    // Make sure relocs worked fine and mappings are good
    debug_printf("we in here noinit_tablet patch kern %p\n", kern_main);

    debug_printf("init_linking symbol at: %08x\n", wafel_find_symbol("init_linking"));

    bool eco_mode = is_ecomode();

    if(eco_mode){
        debug_printf("Eco mode detected, not applying no tablet patch\n");
    } else {
        // stub the WOWL command in the IOSU
        ASM_PATCH_K(0x11f55b1c, "bx lr")
    }

    debug_printf("noinit_tablet patch applied\n");
}

// This fn runs before MCP's main thread, and can be used
// to perform late patches and spawn threads under MCP.
// It must return.
void mcp_main()
{


}
