#pragma once
#define	MAX_BLEND_ITEM_VALUE		5

bool	Blend_Item_init();
bool	Blend_Item_load(char *file);
bool	Blend_Item_set_value(LPITEM item);
bool	Blend_Item_find(uint32_t item_vnum);
