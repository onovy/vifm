#include <stic.h>

#include <unistd.h> /* chdir() rmdir() symlink() */

#include <stdio.h> /* remove() */
#include <stdlib.h> /* free() */
#include <string.h> /* strdup() */

#include "../../src/cfg/config.h"
#include "../../src/compat/os.h"
#include "../../src/compat/fs_limits.h"
#include "../../src/filelist.h"
#include "../../src/filtering.h"
#include "../../src/utils/dynarray.h"
#include "../../src/utils/fs.h"
#include "../../src/utils/path.h"
#include "../../src/utils/str.h"
#include "../../src/cmd_core.h"

static void init_view(FileView *view);
static void free_view(FileView *view);
static int not_windows(void);

SETUP()
{
	assert_success(chdir(SANDBOX_PATH));

	curr_view = &lwin;
	other_view = &rwin;

	init_commands();

	cfg.slow_fs_list = strdup("");
	cfg.chase_links = 1;

	init_view(&lwin);
	init_view(&rwin);
}

TEARDOWN()
{
	reset_cmds();

	update_string(&cfg.slow_fs_list, NULL);

	cfg.chase_links = 0;

	free_view(&lwin);
	free_view(&rwin);
}

static void
init_view(FileView *view)
{
	filter_init(&view->local_filter.filter, 1);
	filter_init(&view->manual_filter, 1);
	filter_init(&view->auto_filter, 1);

	view->dir_entry = NULL;
	view->list_rows = 0;

	view->window_rows = 1;
	view->sort[0] = SK_NONE;
	ui_view_sort_list_ensure_well_formed(view, view->sort);
}

static void
free_view(FileView *view)
{
	int i;

	for(i = 0; i < view->list_rows; ++i)
	{
		free(view->dir_entry[i].name);
	}
	dynarray_free(view->dir_entry);

	filter_dispose(&view->local_filter.filter);
	filter_dispose(&view->manual_filter);
	filter_dispose(&view->auto_filter);
}

TEST(link_is_not_resolved_by_default, IF(not_windows))
{
	assert_success(os_mkdir("dir", 0700));

	cfg.chase_links = 0;

	/* symlink() is not available on Windows, but other code is fine. */
#ifndef _WIN32
	assert_success(symlink("dir", "dir-link"));
#endif

	assert_non_null(get_cwd(curr_view->curr_dir, sizeof(curr_view->curr_dir)));
	assert_true(change_directory(curr_view, "dir-link") >= 0);
	assert_string_equal("dir-link", get_last_path_component(curr_view->curr_dir));

	/* Go out of the directory so that we can remove it. */
	assert_true(change_directory(curr_view, "..") >= 0);

	assert_success(rmdir("dir"));
	assert_success(remove("dir-link"));
}

TEST(chase_links_causes_link_to_be_resolved, IF(not_windows))
{
	assert_success(os_mkdir("dir", 0700));

	/* symlink() is not available on Windows, but other code is fine. */
#ifndef _WIN32
	assert_success(symlink("dir", "dir-link"));
#endif

	assert_non_null(get_cwd(curr_view->curr_dir, sizeof(curr_view->curr_dir)));
	assert_true(change_directory(curr_view, "dir-link") >= 0);
	assert_string_equal("dir", get_last_path_component(curr_view->curr_dir));

	/* Go out of the directory so that we can remove it. */
	assert_true(change_directory(curr_view, "..") >= 0);

	assert_success(rmdir("dir"));
	assert_success(remove("dir-link"));
}

TEST(chase_links_is_not_affected_by_chdir, IF(not_windows))
{
	char pwd[PATH_MAX];

	assert_success(os_mkdir("dir", 0700));

	/* symlink() is not available on Windows, but other code is fine. */
#ifndef _WIN32
	assert_success(symlink("dir", "dir-link"));
#endif

	assert_non_null(get_cwd(pwd, sizeof(pwd)));
	strcpy(curr_view->curr_dir, pwd);

	assert_true(change_directory(curr_view, "dir-link") >= 0);
	assert_success(chdir(".."));
	assert_true(change_directory(curr_view, "..") >= 0);
	assert_true(paths_are_equal(curr_view->curr_dir, pwd));

	assert_success(rmdir("dir"));
	assert_success(remove("dir-link"));
}

static int
not_windows(void)
{
#ifdef _WIN32
	return 0;
#else
	return 1;
#endif
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 filetype=c : */
