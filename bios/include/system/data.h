#pragma once

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "attrib.h"
#include "utility.h"

#include "system/block.h"
#include "system/boot.h"

typedef struct __packed
{
    uint8_t x;
    uint8_t y;
} position;

#define MACHINE_ISA_PC 0
#define MACHINE_ISA_XT 1
#define MACHINE_ISA_AT 2

struct __packed bios_data_area
{
    /*** COM/LPT iobase ***/
    uint16_t com_iobase[4];
    uint16_t lpt_iobase[3];

    uint16_t ebda_segment;

    /*** Equipement data area ***/
    uint16_t equipment_list;
    uint8_t  interrupt_flag;
    uint16_t memory_size;
    uint8_t  __reserved_3;
    uint8_t  __reserved_4;

    /*** Keyboard data area ***/
    union {
    uint16_t keyboard_flags;
    struct {
    uint16_t rshift_pressed     : 1;
    uint16_t lshift_pressed     : 1;
    uint16_t ctrl_pressed       : 1;
    uint16_t alt_pressed        : 1;
    uint16_t scroll_active      : 1;
    uint16_t numlock_active     : 1;
    uint16_t capslock_active    : 1;
    uint16_t insert_active      : 1;
    uint16_t lctrl_pressed      : 1;
    uint16_t lalt_pressed       : 1;
    uint16_t syskey_pressed     : 1;
    uint16_t suspend_active     : 1;
    uint16_t scroll_pressed     : 1;
    uint16_t numlock_pressed    : 1;
    uint16_t capslock_pressed   : 1;
    uint16_t insert_pressed     : 1;
    };};
    uint8_t  alt_keypad_entry;
    uint16_t keyboard_buffer_head;
    uint16_t keyboard_buffer_tail;
    uint16_t keyboard_buffer[16];

    /*** Floppy data area ***/
    uint8_t  floppy_recalibration;
    uint8_t  floppy_motor;
    uint8_t  floppy_motor_shutoff;
    uint8_t  floppy_status;
    uint8_t  floppy_fdc_status[7];

    /*** Video data area ***/
    uint8_t  video_mode;
    uint16_t screen_width;
    uint16_t video_buffer_size;
    uint16_t video_buffer_offset;
    position cursor_pos[8];
    uint8_t  cursor_end;
    uint8_t  cursor_begin;
    uint8_t  active_video_page;
    uint16_t crtc_iobase;
    uint8_t  mode_control;
    uint8_t  palette_mask;

    uint32_t return_address;
    uint8_t  last_unexpected_int;

    /*** Timer data area ***/
    uint16_t timer_lo;
    uint16_t timer_hi;
    uint8_t  timer_of;

    /*** System data area ***/
    uint8_t  ctrl_break;
    uint16_t reset_flag;

    /*** Fixed Disk Area ***/
    uint8_t  disk_status;
    uint8_t  disk_count;
    uint8_t  disk_control;
    uint8_t  disk_offset;

    /*** COM/LPT Timeouts ***/
    uint8_t  com_timeout[4];
    uint8_t  lpt_timeout[4];

    /*** Keyboard Extra Area 1 ***/
    uint16_t keyboard_head;
    uint16_t keyboard_tail;

    /*** VGA Data Area ***/
    uint8_t  video_row_count;
    uint16_t video_char_height;
    uint8_t  video_control_bits_0;
    uint8_t  video_switch_data;
    uint8_t  video_control_bits_1;
    uint8_t  video_dcc_table_index;

    /*** Floppy Extra Data 1 ***/
    uint8_t  floppy_data_rate;

    /*** AT Fixed Disk Area ***/
    uint8_t  disk_at_status;
    uint8_t  disk_at_error;
    uint8_t  disk_at_interrupt;

    /*** Floppy Extra Data 2 ***/
    uint8_t  floppy_controller_info;
    uint8_t  floppy_media_type[4];
    uint8_t  floppy_current_track[2];

    /*** Keyboard Extra Area 2 ***/
    uint8_t  keyboard_status;
    uint8_t  keyboard_led_status;

    /*** User Wait Data ***/
    uint32_t user_wait_address;
    uint32_t user_wait_count;
    uint8_t  user_wait_flag;

    uint8_t  __reserved_5[7];
    uint32_t video_parameter_ptr;
    uint32_t return_stack;
    // uint16_t return_ds;
    // uint16_t return_es;
    uint8_t  __reserved_6[29];
    uint16_t day_counter;

    // uint8_t __reserved_4[66];

    /*** Internal variables ***/
    union
    {
        struct {
            uint32_t machine_type;
            uint32_t chipset_type;
            uint16_t cpuid;
            uint8_t  debug_device;
            uint8_t  debug_x;
            uint8_t  debug_y;
            uint8_t  keyboard_init;
            // gdb_regs registers;
        };
        uint8_t  __reserved_7[48];
    };

    uint8_t  prtscrn_status;
};

typedef struct bios_data_area bda_t;

struct __packed extended_bios_data_area
{
    uint32_t    size;
    block_t     block_table[32];
    uint8_t     buffer[2048];
    uint8_t     gdb_buffer[2048];
    // uint8_t     gdb_buf1[1024];
    // boot_entry_t boot_table[16];
    // uint16_t     boot_device;
    // uint8_t      is_emulating;  // TODO: should be in a flag field
    // uint8_t      emulated_drives[2]; 
    // char         block_names[32][32];
    // fdpt_t       fdpt[32];
};

typedef struct extended_bios_data_area ebda_t;

ebda_t __far* get_ebda(void);
extern volatile bda_t  __far* const bda;