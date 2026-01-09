#ifndef _UNBRICK_H_
#define _UNBRICK_H_

#include <utils/types.h>

void unbrick(const char *sd_folder_path, bool reset);
void wip_nand();
void fix_downgrade();
void del_erpt_save();
void remove_parental_control();
void sync_joycons_between_nands();
void build_prodinfo_and_flash(bool from_scratch);

#endif