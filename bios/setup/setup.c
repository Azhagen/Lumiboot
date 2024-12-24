#include "setup/setup.h"
#include "system/data.h"
#include "system/boot.h"

#include "drivers.h"
#include "tui.h"
#include "print.h"
#include "string.h"

#include "bios.h"
#include "debug.h"

#define KEY_LEFT    0x4B
#define KEY_RIGHT   0x4D
#define KEY_UP      0x48
#define KEY_DOWN    0x50
#define KEY_ENTER   0x1C
#define KEY_ESC     0x01


enum menu_id
{
    MENU_SYSTEM,
    MENU_CONFIG,
    MENU_ADVANCED,
    MENU_BOOT,
    MENU_EXIT,

    SUBMENU_TIME,
    SUBMENU_DATE,
    SUBMENU_LANG
};

static const char *const setup_menu[] =
{
    "System",
    "Configuration",
    "Advanced",
    "Boot",
    "Exit"
};

static const char *const boot_menu[] =
{
    "Floppy",
    "Hard Drive",
    "CD-ROM",
    "Network",
    "Exit"
};

static const char *const setup_banner = "Lumiboot Setup Utility - Copyright (C) 2024 Azhagen";

static bool update_time(time_t __far* time, uint8_t key, uint8_t sub_selected)
{
    switch (sub_selected)
    {
        case 0:
            if (key == KEY_RIGHT) { time->hour = (uint8_t)(time->hour + 1) % 24; return true; }
            if (key == KEY_LEFT)  { time->hour = (time->hour == 0) ? 23 : (uint8_t)(time->hour - 1); return true; }
            break;
        case 1:
            if (key == KEY_RIGHT) { time->minute = (uint8_t)(time->minute + 1) % 60; return true; }
            if (key == KEY_LEFT)  { time->minute = (time->minute == 0) ? 59 : (uint8_t)(time->minute - 1); return true; }
            break;
        case 2:
            if (key == KEY_RIGHT) { time->second = (uint8_t)(time->second + 1) % 60; return true; }
            if (key == KEY_LEFT)  { time->second = (time->second == 0) ? 59 : (uint8_t)(time->second - 1); return true; }
            break;
        default: break;
    }

    return false;
}

static bool update_date(date_t __far* date, uint8_t key, uint8_t sub_selected)
{
    uint8_t max_day = 31;

    switch (date->month)
    {
        case 4: case 6: case 9: case 11:
            max_day = 30;
            break;
        case 2:
            if ((date->year % 4 == 0 && date->year % 100 != 0) || (date->year % 400 == 0))
                max_day = 29;
            else
                max_day = 28;
            break;
        default:
            max_day = 31;
            break;
    }

    switch (sub_selected)
    {
        case 0:
            if (key == KEY_RIGHT) { date->day = (uint8_t)(date->day % max_day + 1); return true; }
            if (key == KEY_LEFT)  { date->day = (date->day == 1) ? max_day : (uint8_t)(date->day - 1); return true; }
            break;
        case 1:
            if (key == KEY_RIGHT) { date->month = (uint8_t)(date->month % 12 + 1); return true; }
            if (key == KEY_LEFT)  { date->month = (date->month == 1) ? 12 : (uint8_t)(date->month - 1); return true; }
            break;
        case 2:
            if (key == KEY_RIGHT) { date->year = (uint8_t)(date->year + 1) % 100; return true; }
            if (key == KEY_LEFT)  { date->year = (date->year == 0) ? 99 : (uint8_t)(date->year - 1); return true; }
            break;
        default: break;
    }

    return false;
}

static void display_date_menu(date_t __far* date, setup_status_t __far* status)
{
    char __far* str = (char __far*)get_ebda()->buffer;

    tui_text(2, 3, "Set System Date", COLOR_LIGHTGRAY << 4 | COLOR_BLACK);
    tui_horizontal_line(2, 4, 49, 4, COLOR_LIGHTGRAY << 4 | COLOR_BLUE);

    snprintf(str, 16, "[%02u]", date->day);
    tui_text_item(2, 6, 30, "Day:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 0);

    snprintf(str, 16, "[%02u]", date->month);
    tui_text_item(2, 7, 30, "Month:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 1);

    snprintf(str, 16, "[%02u]", date->year);
    tui_text_item(2, 8, 30, "Year:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 2);
}

static void display_time_menu(time_t __far* time, setup_status_t __far* status)
{
    char __far* str = (char __far*)get_ebda()->buffer;

    tui_text(2, 3, "Set System Time", COLOR_LIGHTGRAY << 4 | COLOR_BLACK);
    tui_horizontal_line(2, 4, 49, 4, COLOR_LIGHTGRAY << 4 | COLOR_BLUE);

    snprintf(str, 16, "[%02u]", time->hour);
    tui_text_item(2, 6, 30, "Hours:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 0);

    snprintf(str, 16, "[%02u]", time->minute);
    tui_text_item(2, 7, 30, "Minutes:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 1);

    snprintf(str, 16, "[%02u]", time->second);
    tui_text_item(2, 8, 30, "Seconds:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 2);
}

static void setup_read_key(setup_status_t __seg_ss* status)
{
    status->code = bios_keyboard_peek_key();
    if (status->code != 0)
        status->code = bios_keyboard_read_key();
    status->key = hi(status->code);
}

static void setup_draw_background(uint8_t selected)
{
    tui_fill(0, 2, 80, 25, ' ', COLOR_BLACK, COLOR_LIGHTGRAY);
    tui_border(0, 2, 80, 23, COLOR_BLUE, COLOR_LIGHTGRAY, BORDER_LARGE, ALIGN_LEFT);
    tui_border(50, 2, 30, 23, COLOR_BLUE, COLOR_LIGHTGRAY, BORDER_LARGE, ALIGN_LEFT);

    tui_menubar(0, 0, 80, 1, 0, COLOR_CYAN << 4 | COLOR_BLACK,
        COLOR_CYAN << 4 | COLOR_BLACK, &setup_banner);
    tui_menubar(0, 1, 80, array_size(setup_menu), selected,
        COLOR_BLUE << 4 | COLOR_LIGHTGRAY, COLOR_LIGHTGRAY << 4 | COLOR_BLACK, setup_menu);
}

static void setup_set_time(setup_status_t __seg_ss* status)
{
    bool modified = false;

    if (status->refresh)
        setup_draw_background(MENU_SYSTEM);

    switch (status->key)
    {
        case KEY_UP:
            if (status->selected > 0) status->selected--;
            break;
        case KEY_DOWN:
            if (status->selected < 2) status->selected++;

            break;
        case KEY_ESC:
            status->menu_id  = MENU_SYSTEM;
            status->key      = 0;
            status->refresh  = 1;
            return;
        default: break;
    }

    cmos_read_time(&status->time);
    modified = update_time(&status->time, status->key, status->selected);
    display_time_menu(&status->time, status);

    if (modified)
        cmos_write_time(&status->time);
    
    modified = false;
    cmos_read_time(&status->time);
}

static void setup_set_date(setup_status_t __seg_ss* status)
{
    bool modified = false;
    
    if (status->refresh)
        setup_draw_background(MENU_SYSTEM);

    switch (status->key)
    {
        case KEY_UP:
            if (status->selected > 0) status->selected--;
            break;
        case KEY_DOWN:
            if (status->selected < 2) status->selected++;

            break;
        case KEY_ESC:
            status->menu_id  = MENU_SYSTEM;
            status->key      = 0;
            return;
        default: break;
    }

    cmos_read_date(&status->date);
    modified = update_date(&status->date, status->key, status->selected);
    display_date_menu(&status->date, status);

    if (modified)
        cmos_write_date(&status->date);

    modified = false;
    cmos_read_date(&status->date);
}

static void setup_set_lang(setup_status_t __seg_ss* status)
{
    if (status->refresh)
        setup_draw_background(MENU_SYSTEM);

    switch (status->key)
    {
        case KEY_ESC:
            status->menu_id  = MENU_SYSTEM;
            status->key      = 0;
            return;
        default: break;
    }

    tui_text(2, 5, "No other language available", COLOR_LIGHTGRAY << 4 | COLOR_BLACK);
}

static void setup_show_system(setup_status_t __seg_ss* status)
{
    char __far* str = (char __far*)get_ebda()->buffer;

    time_t time = {};
    date_t date = {};

    bool refresh_time = false;
    bool refresh_selectable = false;

    switch (status->key) {
        case KEY_UP:
            if (status->selected > 0) status->selected--;
            refresh_selectable = true;
            break;
        case KEY_DOWN:
            if (status->selected < 2) status->selected++;
            refresh_selectable = true;
            break;
        case KEY_ENTER:
            status->refresh = 1;
            switch (status->selected)
            {
                case 0: status->menu_id = SUBMENU_TIME; break;
                case 1: status->menu_id = SUBMENU_DATE; break;
                case 2: status->menu_id = SUBMENU_LANG; break;
                default: break;
            }
            break;
        default: break;
    }

    cmos_read_time(&time);
    cmos_read_date(&date);

    if (time.hour != status->time.hour || time.minute != status->time.minute || time.second != status->time.second ||
        date.day != status->date.day || date.month != status->date.month || date.year != status->date.year)
        refresh_time = true;

    if (refresh_time)
    {
        snprintf(str, 16, "[%02u:%02u:%02u]", time.hour, time.minute, time.second);
        tui_text_item(2, 9, 30, "Time:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
            COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 0);

        snprintf(str, 16, "[%02u/%02u/%02u]", date.day, date.month, date.year);
        tui_text_item(2, 10, 30, "Date:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 
            COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 1);

        // BUG: gcc complain with: unable to find a register to spill
        status->time.hour = time.hour;
        status->time.minute = time.minute;
        status->time.second = time.second;
        status->date.day = date.day;
        status->date.month = date.month;
        status->date.year = date.year;
    }

    if (refresh_selectable)
    {
        snprintf(str, 16, "[%02u:%02u:%02u]", time.hour, time.minute, time.second);
        tui_text_item(2, 9, 30, "Time:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
            COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 0);

        snprintf(str, 16, "[%02u/%02u/%02u]", date.day, date.month, date.year);
        tui_text_item(2, 10, 30, "Date:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 
            COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 1);

        tui_text_item(2, 20, 30, "Language:", "[English]", COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 
            COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 2);
    }

    if (status->refresh != 1)
        return;

    snprintf(str, 16, "[%02u:%02u:%02u]", time.hour, time.minute, time.second);
    tui_text_item(2, 9, 30, "Time:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 0);

    snprintf(str, 16, "[%02u/%02u/%02u]", date.day, date.month, date.year);
    tui_text_item(2, 10, 30, "Date:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 
        COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 1);

    tui_text_item(2, 20, 30, "Language:", "[English]", COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 
        COLOR_BLACK << 4 | COLOR_LIGHTGRAY, status->selected == 2);

    tui_text(2, 3, "System Information", COLOR_LIGHTGRAY << 4 | COLOR_BLACK);
    tui_horizontal_line(2, 4, 49, 4, COLOR_LIGHTGRAY << 4 | COLOR_BLUE);

    tui_text_item(2, 6, 30, "Firmware Version:", "0.1.0", COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 0);
    tui_text_item(2, 7, 30, "Build Date:", __DATE__, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 0);
    
    tui_text_item(2, 12, 30, "CPU:", "80286", COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 0);
    tui_text_item(2, 13, 30, "FPU:", "Present", COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 0);

    size_t memsize = as_uint16(cmos_read(0x15), cmos_read(0x14));
    snprintf(str, 16, "%u KB", memsize);
    tui_text_item(2, 14, 30, "Base Memory:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 0);

    size_t extsize = as_uint16(cmos_read(0x18), cmos_read(0x17));
    snprintf(str, 16, "%u KB", extsize);
    tui_text_item(2, 15, 30, "Ext. Memory:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 0);

    size_t totalsize = memsize + extsize;
    snprintf(str, 16, "%u KB", totalsize);
    tui_text_item(2, 16, 30, "Tot. Memory:", str, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
        COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 0);

    tui_text_item(2, 18, 30, "Video:", "Unknown", COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 
        COLOR_LIGHTGRAY << 4 | COLOR_BLACK, 0);
}

static void setup_show_config(setup_status_t __seg_ss* status)
{
    if (status->refresh)
        setup_draw_background(MENU_CONFIG);
}

void setup_main(void)
{
    setup_status_t status = {};
    status.refresh = 1;

    bios_video_set_cursor_type(0x20, 0x20);

    while (true)
    {
        setup_read_key(&status);

        if (status.menu_id != status.prev_id)
        {
            status.refresh = 1;
            status.prev_id = status.menu_id;
            status.selected = 0;
        }

        switch (status.key)
        {
            case KEY_LEFT:
                if (status.menu_id > 0)
                    status.menu_id--;
                break;
            case KEY_RIGHT:
                if (status.menu_id < 4)
                    status.menu_id++;
                break;
            case KEY_ESC:
                if (status.menu_id <= MENU_EXIT)
                    return;
                break;
            default: break;
        }

        if (status.refresh)
            setup_draw_background(MENU_SYSTEM);

        switch (status.menu_id)
        {
            case MENU_SYSTEM: setup_show_system(&status); break;
            case MENU_CONFIG: setup_show_config(&status); break;
            case MENU_ADVANCED: break;
            case MENU_BOOT: break;
            case MENU_EXIT: break;
            case SUBMENU_TIME: setup_set_time(&status); break;
            case SUBMENU_DATE: setup_set_date(&status); break;
            case SUBMENU_LANG: setup_set_lang(&status); break;
            default: break;
        }

        if (status.escape) return;
        status.refresh = 0;
    }
}

void setup_boot(void)
{
    setup_status_t update = {};
    update.refresh = true;
    uint8_t entries = 0;
    uint8_t id = 0;

    ebda_t __far* ebda = get_ebda();

    bios_video_set_cursor_type(0x20, 0x20);
    bios_video_scroll_wnd_down(25, COLOR_BLACK << 4 | COLOR_LIGHTGRAY, 0, 80, 0, 25);

    while (true)
    {
        setup_read_key(&update);

        switch (update.key)
        {
            case KEY_UP:
                if (update.selected > 0)
                    update.selected--;
                update.refresh = true;
                break;
            case KEY_DOWN:
                if (update.selected < entries - 1)
                    update.selected++;
                update.refresh = true;
                break;
            case KEY_ENTER:
                if (update.selected == entries - 1)
                    return setup_main();
                else
                    boot_device(id);
                break;
            case KEY_ESC:
                return;
            default: break;
        }

        if (!update.refresh)
            continue;

        entries = 0;
        uint8_t off = 0;

        for (uint8_t i = 0; i < 16; ++i)
        {
            if (ebda->block_table[i].type == BLOCK_TYPE_RESERVED)
                continue;

            snprintf(ebda->buffer, 30, "Device %02u:", i);
            off = (uint8_t)((80 / 2) - (fstrlen(ebda->buffer) + 32) / 2);

            tui_text_item(off, (uint8_t)(4 + i), 11, ebda->buffer,
                ebda->block_table[i].desc, COLOR_BLACK << 4 | COLOR_LIGHTGRAY,
                COLOR_LIGHTGRAY << 4 | COLOR_BLACK, update.selected == i);
                
            if (update.selected == i)
                id = ebda->block_table[i].id;

            entries++;
        }

        tui_text_item(off, (uint8_t)(4 + entries), 11, "", "Enter Setup",
            COLOR_BLACK << 4 | COLOR_LIGHTGRAY, COLOR_LIGHTGRAY << 4 | COLOR_BLACK,
            update.selected == entries);

        entries++;
        update.refresh = false;
    }
}

bool setup_wait_key(void)
{
    uint16_t key = bios_keyboard_peek_key();
    if (key != 0 && key != 0x3B00 && key != 0x3C00)
        key = bios_keyboard_read_key();

    return key == 0x3B00 || key == 0x3C00;
}
