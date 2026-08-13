/** \file basic-timer.c
 *
 * Implementation of the basic clock/timer component (no summary elements).
 */
#include "components.h"
#include "../../timer.h"

/**
 * @brief The Timer component itself.
 */
typedef struct _LSBasicTimer LSBasicTimer;
struct _LSBasicTimer {
    LSComponent base; /*!< The base struct that is extended. */
	long long (*timer_source)(LSBasicTimer const * self, ls_timer const * timer); /*< What should be used to derive the displayed time value. */
	char * lua_timer_source;
    GtkWidget* time; /*!< The timer container */
    GtkWidget* time_seconds; /*!< The label representing the seconds part of the timer */
    GtkWidget* time_millis; /*!< The label representing the milliseconds part of the timer */
};
LSComponentOps ls_basic_timer_operations;
/* ^^ TODO (for final PR)
 * The above variable was extern (as is for all the components.)
 * But why?? I removed it for now, but I should understand if this was
 * intentional for some reason before committing to its removal. */

/**
 * Constructor
 */
LSComponent* ls_component_basic_timer_new(json_t* user_config)
{
	(void) user_config;

    LSBasicTimer* self;
    GtkWidget* spacer;

    self = malloc(sizeof(LSBasicTimer));
    if (!self) {
        return NULL;
    }
    self->base.ops = &ls_basic_timer_operations;

    self->time = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    add_class(self->time, "timer");
    add_class(self->time, "time");
    gtk_widget_show(self->time);

    spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_container_add(GTK_CONTAINER(self->time), spacer);
    gtk_widget_show(spacer);

    self->time_seconds = gtk_label_new(NULL);
    add_class(self->time_seconds, "timer-seconds");
    gtk_widget_set_valign(self->time_seconds, GTK_ALIGN_BASELINE);
    gtk_container_add(GTK_CONTAINER(self->time), self->time_seconds);
    gtk_widget_show(self->time_seconds);

    spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_valign(spacer, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(self->time), spacer);
    gtk_widget_show(spacer);

    self->time_millis = gtk_label_new(NULL);
    add_class(self->time_millis, "timer-millis");
    gtk_widget_set_valign(self->time_millis, GTK_ALIGN_BASELINE);
    gtk_container_add(GTK_CONTAINER(spacer), self->time_millis);
    gtk_widget_show(self->time_millis);

    return (LSComponent*)self;
}

/* NOTE: No inline specifiers because the compiler *can't* inline them when
 * used as function pointers */
static long long bt_real_timer_source(LSBasicTimer const * self, ls_timer const * timer) {
	(void) self;
	// return 9999999999;
	return ls_timer_get_real_time(timer);
}
static long long bt_adjusted_timer_source(LSBasicTimer const * self, ls_timer const * timer) {
	(void) self;
	// return 123456789;
	return ls_timer_get_load_removed_time(timer);
}
static long long bt_game_timer_source(LSBasicTimer const * self, ls_timer const * timer) {
	(void) self;
	// return 4140420;
	return ls_timer_get_game_time(timer);
}

/**
 * TODO!!
 *
 * This current implementation will not work! I can't (safely) retrieve the lua
 * context because it's handled by a separate thread. Thus, I need to create a
 * function "registration" system in the autosplitter. Likely some form of table
 * where I can read it to grab the last computed value.
 *
 * Function registration would probably be a function name, expected return
 * value type, and perhaps a return fallback value. The table would probably
 * contain that, as well as if the function was found, the last return
 * value, and maybe a boolean if it's enabled. (So for example, the autosplitter
 * is still loaded, but the timer is paused, so we probably don't need to
 * update the derived expression value for now.
 *
 * @param L The Lua State
 * @param fn_name Name of the function to be called
 * @param fn_name Name of the function to be called
 * @param out_buf Pre-allocated output buffer
 */
static long long bt_lua_timer_source(LSBasicTimer const * self, ls_timer const * timer)
{
	(void) timer;

	// lua_State* L = NULL; // Placeholder so compilation is happy
    // int lua_time;
	long long rval = -1000; // Fallback to -1 second

	if (!self->lua_timer_source) {
		return rval;
	}

    // lua_getglobal(L, self->lua_timer_source);
    // if (lua_isfunction(L, -1)) {
		// if (call_va(L, self->lua_timer_source, ">i", &lua_time)) {
			// /* TODO: Taken from the implementation for gameTime, but probably needs
			 // * improvement since we lose quite a bit of precision. */
			// rval = (long long) lua_time * 1000;
		// }
		// lua_pop(L, 1); // Remove the return value from the stack
	// }
    // lua_pop(L, 1); // Remove lua_timer_source from the stack

	return rval;
}


// Avoid collision with timer_delete of time.h
/**
 * Destructor.
 *
 * @param self The clock component itself
 */
static void bt_delete(LSComponent* self)
{
    free(self);
}

/**
 * Returns the clock GTK widget.
 *
 * @param self The clock component itself.
 * @return The container as a GTK Widget.
 */
static GtkWidget* bt_widget(LSComponent* self)
{
    return ((LSBasicTimer*)self)->time;
}

/**
 * Function to execute when ls_app_window_show_game is executed. Sets the game-
 * specific component configuration.
 *
 * @param self_ The splits component itself.
 * @param game The game struct instance.
 * @param timer The timer instance.
 */
static void bt_show_game(LSComponent* self_, const ls_game* game,
	const ls_timer* timer)
{
    LSBasicTimer* self = (LSBasicTimer*)self_;
	char const * const lua_pfx = "lua:";
	char const * source_str = game->bt_timer_source;
	size_t source_str_len;

	if (source_str) {
		if (0 == strcmp(source_str, "real")) {
			self->timer_source = bt_real_timer_source;
		} else if (0 == strcmp(source_str, "adjusted")) {
			self->timer_source = bt_adjusted_timer_source;
		} else if (0 == strcmp(source_str, "game")) {
			self->timer_source = bt_game_timer_source;
		} else if (0 == strncmp(source_str, lua_pfx, strlen(lua_pfx))) {
			source_str += strlen(lua_pfx);
			if (!(source_str_len = strlen(source_str))) {
				// Configured string was only the prefix and no function name
				return;
			}

			self->lua_timer_source = malloc(source_str_len + 1);
			if (!self->lua_timer_source) {
				printf("[basic-timer] Malloc fail for lua function name");
				return;
			}

			strncpy(self->lua_timer_source, source_str, source_str_len + 1);
			self->timer_source = bt_lua_timer_source;
		} else {
			// TODO: Assert what the fallback behavior should be....
			// Current implementation is only to accept known values but a
			// fallback method would be trivial to implement
			(void) 0;
		}
	} else {
		// Default: Use RTA if option not provided
		self->timer_source = bt_adjusted_timer_source;
	}

}

/**
 * Function to execute when ls_app_window_clear_game is executed.
 *
 * @param self_ The best time component itself.
 */
static void bt_clear_game(LSComponent* self_)
{
    LSBasicTimer* self = (LSBasicTimer*)self_;
    gtk_label_set_text(GTK_LABEL(self->time_seconds), "");
    gtk_label_set_text(GTK_LABEL(self->time_millis), "");
    remove_class(self->time, "behind");
    remove_class(self->time, "losing");
	self->timer_source = NULL;
}

/**
 * Function to execute when ls_app_window_draw is executed.
 *
 * @param self_ The best time component itself.
 * @param game The game struct instance.
 * @param timer The timer instance.
 */
static void bt_draw(LSComponent* self_, const ls_game* game, const ls_timer* timer)
{
    LSBasicTimer* self = (LSBasicTimer*)self_;
    char str[256], millis[256];

    unsigned int curr = timer->curr_split;
    if (curr && curr == game->split_count) {
        --curr;
    }

	long long current_time = (self->timer_source) ?
		self->timer_source(self, timer) : -1;

    remove_class(self->time, "delay");
    remove_class(self->time, "behind");
    remove_class(self->time, "losing");
    remove_class(self->time, "best-split");

    if (curr && curr == game->split_count) {
        curr = game->split_count - 1;
    }
    if (current_time <= 0) {
        add_class(self->time, "delay");
    } else {
        if (timer->curr_split == game->split_count
            && timer->split_info[curr]
                & LS_INFO_BEST_SPLIT) {
            add_class(self->time, "best-split");
        } else {
            if (timer->split_info[curr]
                & LS_INFO_BEHIND_TIME) {
                add_class(self->time, "behind");
            }
            if (timer->split_info[curr]
                & LS_INFO_LOSING_TIME) {
                add_class(self->time, "losing");
            }
        }
    }
    ls_time_millis_string(str, &millis[1], current_time);
    millis[0] = '.';
    gtk_label_set_text(GTK_LABEL(self->time_seconds), str);
    gtk_label_set_text(GTK_LABEL(self->time_millis), millis);
}

LSComponentOps ls_basic_timer_operations = {
    .delete = bt_delete,
    .widget = bt_widget,
    .show_game = bt_show_game,
    .clear_game = bt_clear_game,
    .draw = bt_draw
};
