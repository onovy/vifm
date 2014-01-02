/* vifm
 * Copyright (C) 2013 xaizek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "trash_menu.h"

#include <stdlib.h> /* free() */
#include <string.h> /* strdup() */

#include "../utils/string_array.h"
#include "../status.h"
#include "../trash.h"
#include "../ui.h"
#include "../undo.h"

static int trash_khandler(menu_info *m, wchar_t keys[]);

int
show_trash_menu(FileView *view)
{
	int i;

	static menu_info m;
	init_menu_info(&m, TRASH_MENU, strdup("No files in trash"));
	m.key_handler = trash_khandler;

	m.title = strdup(" Original paths of files in trash ");

	for(i = 0; i < nentries; i++)
	{
		const trash_entry_t *const entry = &trash_list[i];
		m.len = add_to_string_array(&m.items, m.len, 1, entry->path);
	}

	return display_menu(&m, view);
}

/* Processes key presses on menu items.  Returns value > 0 to request menu
 * window refresh and < 0 on unsupported key. */
static int
trash_khandler(menu_info *m, wchar_t keys[])
{
	if(wcscmp(keys, L"r") == 0)
	{
		char *const trash_path = strdup(trash_list[m->pos].trash_name);

		cmd_group_begin("restore: ");
		cmd_group_end();

		if(restore_from_trash(trash_path) != 0)
		{
			const char *const orig_path = m->items[m->pos];
			status_bar_errorf("Failed to restore %s", orig_path);
			curr_stats.save_msg = 1;
			free(trash_path);
			return -1;
		}
		free(trash_path);

		clean_menu_position(m);

		remove_from_string_array(m->items, m->len, m->pos);
		if(m->matches != NULL)
		{
			if(m->matches[m->pos])
				m->matching_entries--;
			memmove(m->matches + m->pos, m->matches + m->pos + 1,
					sizeof(int)*((m->len - 1) - m->pos));
		}
		m->len--;
		draw_menu(m);

		move_to_menu_pos(m->pos, m);
		return 1;
	}
	return -1;
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
