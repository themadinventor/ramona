/*
 * Ericsson Baseband Controller
 * Entry points into the original firmware
 *
 * 2012-07-25 <fredrik@etf.nu>
 */

.thumb

.global __irma_uidivmod
.thumb_set __irma_uidivmod, 0x01040104

.global __irma_idivmod
.thumb_set __irma_idivmod, 0x0104021c

.global __aeabi_uidivmod
.global __aeabi_uidiv
__aeabi_uidivmod:
__aeabi_uidiv:
    push    {r0}
    mov     r0, r1
    pop     {r1}
    ldr     r2, =(__irma_uidivmod+1)
    bx      r2

.global __aeabi_idivmod
.global __aeabi_idiv
__aeabi_idivmod:
__aeabi_idiv:
    push    {r0}
    mov     r0, r1
    pop     {r1}
    ldr     r2, =(__irma_idivmod+1)
    bx      r2

.global I2C_Init
.thumb_set I2C_Init, 0x01010170

.global I2C_Read
.thumb_set I2C_Read, 0x010101ac

.global I2C_Write
.thumb_set I2C_Write, 0x010102ac

.global NVDS_ReadFile
.thumb_set NVDS_ReadFile, 0x010119f4

.global NVDS_WriteFile
.thumb_set NVDS_WriteFile, 0x010119a4

.global NVDS_DeleteFile
.thumb_set NVDS_DeleteFile, 0x01011a74

.global RegisterTransport
.thumb_set RegisterTransport, 0x0103b658

.global HCI_Trans_Event_Sent
.thumb_set HCI_Trans_Event_Sent, 0x0103baa8

.global HCI_Input_ACL
.thumb_set HCI_Input_ACL, 0x0103bba4

.global HCI_Trans_ACL_Sent
.thumb_set HCI_Trans_ACL_Sent, 0x0103bae8

.global timer_add
.thumb_set timer_add, 0x01010ad4

.global proc_hci
.set proc_hci, 0x0000de4c

.global proc_flash_eraser
.set proc_flash_eraser, 0x0000df38

.global proc_flash_handler
.set proc_flash_handler, 0x0000c46c

.global os_build_time
.set os_build_time, 0x01010030

.global os_build_comment
.set os_build_comment, 0x01010050

.global bp_build_time
.set bp_build_time, 0x01000030

.global bp_build_comment
.set bp_build_comment, 0x01000050

.global NVDS_Context
.set NVDS_Context, 0x000006A4

.global ose_crash_info
.set ose_crash_info, 0x7218

#.global ROM_CRC16
#.thumb_set ROM_CRC16, 0x00c00624
