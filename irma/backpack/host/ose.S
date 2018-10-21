/*
 * Ericsson Baseband Controller
 * Entry points into the original firmware
 *
 * 2014-01-26 <fredrik@etf.nu>
 */

.thumb

.global alloc
.thumb_set alloc, 0x01042630

.global UserError
.thumb_set UserError, 0x01042784

.global free_buf
.thumb_set free_buf, 0x010427dc

.global receive
.thumb_set receive, 0x010428d4

.global delay
.thumb_set delay, 0x01042a98

.global start
.thumb_set start, 0x01042af4

.global stop
.thumb_set stop, 0x01042bb6

.global restore
.thumb_set restore, 0x01042c70

.global set_fsem
.thumb_set set_fsem, 0x01042eae

.global wait_fsem
.thumb_set wait_fsem, 0x01042eba

.global signal_fsem
.thumb_set signal_fsem, 0x01042f04

.global get_fsem
.thumb_set get_fsem, 0x01042f8a

.global create_sem
.thumb_set create_sem, 0x01042f98

.global wait_sem
.thumb_set wait_sem, 0x01042faa

.global signal_sem
.thumb_set signal_sem, 0x01043012

.global get_sem
.thumb_set get_sem, 0x01043096

.global kill_sem
.thumb_set kill_sem, 0x0104309c

.global addressee
.thumb_set addressee, 0x010430ac

.global current_process
.thumb_set current_process, 0x010430ce

.global get_ticks
.thumb_set get_ticks, 0x010430d4

.global tick
.thumb_set tick, 0x010430e8

.global get_pri
.thumb_set get_pri, 0x0104328c

.global set_pri
.thumb_set set_pri, 0x010432b4

.global send
.thumb_set send, 0x01043bb8

.global sender
.thumb_set sender, 0x01043cb4

.global sigsize
.thumb_set sigsize, 0x01043cce

.global readyq_1
.set readyq_1, 0x000073b8

.global readyq_2
.set readyq_2, 0x000073c0

.global readyq_3
.set readyq_3, 0x000073c8

.global readyq_4
.set readyq_4, 0x000073d0

.global readyq_31
.set readyq_31, 0x000074a8
