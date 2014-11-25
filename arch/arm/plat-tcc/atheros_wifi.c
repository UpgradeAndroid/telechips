#include <linux/module.h>
#include <asm/setup.h>

char* atheros_wifi_mac;
EXPORT_SYMBOL(atheros_wifi_mac);

static int __init board_wifimac_setup(char *wifimac)
{
        atheros_wifi_mac = wifimac;
        return 1;
}
__setup("androidboot.wifimac=", board_wifimac_setup);
