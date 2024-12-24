#include "system/data.h"
#include "drivers.h"
#include "attrib.h"

#include "bios.h"

void print_screen(void)
{
    uint16_t lpt = bda->lpt_iobase[0];
    if (!lpt) return;

    bda->prtscrn_status = 0x01;

    position pos = bda->cursor_pos[bda->active_video_page];

    for (uint8_t y = 0; y < bda->video_row_count; ++y)
    {
        for (uint8_t x = 0; x < bda->screen_width; ++x)
        {
            character_t ch = {};
            bios_move_cursor(0, x, y);
            bios_read_at_cursor(0, &ch);
            lpt_print(lpt, ch.ch);
        }

        lpt_print(lpt, '\n');
        lpt_print(lpt, '\r');
    }

    
    bda->cursor_pos[bda->active_video_page] = pos;
    bda->prtscrn_status = 0x00;
}