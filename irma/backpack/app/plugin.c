#include "host/app.h"
#include "host/irma.h"
#include "plugin.h"

#include <stdint.h>
#include <stdio.h>

static struct plugin *plugin = (struct plugin *) PLUGIN_BASE;
static enum {PLUGIN_DISABLED, PLUGIN_ENABLED, PLUGIN_FAILED, PLUGIN_INVALID} plugin_state;
static void (*plugin_teardown_proc)(void);

extern void _ebss;

int plugin_present(void)
{
    return (plugin->magic != 0xffffffff) && (plugin->magic != 0);
}

int plugin_valid(void)
{
    if (plugin->magic != PLUGIN_MAGIC) {
        //printf("plugin_valid: wrong magic\n");
        return 0;
    }

    size_t plugin_size = plugin->etext - plugin->text + plugin->bss - plugin->data;
    //printf("plugin_valid: plugin_size = %d\n", plugin_size);
    if (plugin_size > PLUGIN_LIMIT) {
        //printf("plugin_valid: too large\n");
        return 0;
    }

    uint16_t crc = ROM_CRC16(plugin, PLUGIN_CRC_1, 0);
    crc = ROM_CRC16(&plugin->entry, plugin_size-PLUGIN_CRC_2, crc);

    //printf("plugin_valid: checksum = %04x, expected %04x\n", crc, plugin->checksum);

    return crc == plugin->checksum;
}

void *plugin_entry(void)
{
    return plugin->entry + 1;
}

void plugin_enable(void)
{
    if (plugin_state != PLUGIN_DISABLED || !plugin_valid()) {
        return;
    }

    if (plugin->text != PLUGIN_BASE) {
        printf("Plugin text is not at linked address\n");
        return;
    }
    if (plugin->data < (uint32_t) &_ebss) {
        printf("Plugin data is below OS ebss\n");
        return;
    }
    if (plugin->ebss > 0x12000) {
        printf("Plugin ebss is beyond end of ram\n");
        return;
    }
    if (plugin->etext < plugin->text || plugin->bss < plugin->data || plugin->ebss < plugin->bss) {
        printf("Plugin has funky memory map\n");
        return;
    }

    /* Okay, the plugin is valid. Let's initialize some memory */
    uint8_t *src = (uint8_t *) plugin->etext;
    uint8_t *dst = (uint8_t *) plugin->data;
    while (dst < (uint8_t *) plugin->bss)
        *dst++ = *src++;
    while (dst < (uint8_t *) plugin->ebss)
        *dst++ = 0;

    /* Time to run this shizzle */
    int (*proc)(int) = (void *) plugin_entry();
    int result = proc(0);

    /* Did it work out? */
    if (result) {
        plugin_state = PLUGIN_FAILED;
    } else {
        plugin_state = PLUGIN_ENABLED;
    }
}

void plugin_teardown(void (*proc)(void))
{
    plugin_teardown_proc = proc;
}

void plugin_disable(void)
{
    if (plugin_state != PLUGIN_DISABLED && plugin_teardown_proc) {
        plugin_teardown_proc();

        set_uart2_rx_handler(NULL);
        set_tick_handler(NULL);
        set_signal_handler(NULL);
    }

    plugin_state = PLUGIN_DISABLED;
}

int plugin_enabled(void)
{
    return plugin_state != PLUGIN_DISABLED;
}

int plugin_is_autostart(void)
{
    return (plugin->magic == PLUGIN_MAGIC) && (plugin->flags & PLUGIN_FLAG_AUTOSTART);
}

