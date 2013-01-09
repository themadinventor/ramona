#ifndef PLUGIN_H
#define PLUGIN_H

#define PLUGIN_BASE 0x01060000
#define PLUGIN_LIMIT 0x90000

#define PLUGIN_MAGIC 0x414d5249
#define PLUGIN_CRC_1 28
#define PLUGIN_CRC_2 32

struct plugin {
    unsigned int magic;
    unsigned int flags;
    unsigned int text;
    unsigned int etext;
    unsigned int data;
    unsigned int bss;
    unsigned int ebss;
    unsigned int checksum;
    char entry[0];
};

int plugin_present(void);
int plugin_valid(void);
void *plugin_entry(void);
void plugin_enable(void);
void plugin_teardown(void (*proc)(void));
void plugin_disable(void);
int plugin_enabled(void);

#endif
