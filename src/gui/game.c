#include "game.h"
#include "src/gui/component/components.h"
#include "src/gui/theming.h"
#include "src/logging.h"
#include "src/settings/definitions.h"
#include <gtk/gtk.h>

extern AppConfig cfg;

static void ls_app_window_default_components(LSAppWindow* win)
{
    LOG_DEBUG("Creating default components...");

	// TODO: better defaults, just proof of concept right now
	// ^^ Maybe better to make this static so we don't keep searching for it?
	LSComponentAvailable const * default_components[] = {
		get_component("title"),
		get_component("splits"),
		get_component("timer"),
		/* --- */
		NULL
	};

	LSComponentAvailable const ** component_init;
	LSComponent* component;
	GtkWidget* widget;

	component_init = &default_components[0];

	while (*component_init) {
        if ((component = (*component_init)->new())) {
            widget = component->ops->widget(component);
            if (widget) {
                gtk_widget_set_margin_start(widget, WINDOW_PAD);
                gtk_widget_set_margin_end(widget, WINDOW_PAD);
                gtk_container_add(GTK_CONTAINER(win->box),
                    component->ops->widget(component));
            }
            win->components = g_list_append(win->components, component);
        }
		++component_init;
	}
}

static void ls_app_window_add_components(LSAppWindow* win)
{
	json_t ** component_config;
	json_t * component_ref;
	char const * component_name;
	LSComponentAvailable const * component_init;
	LSComponent* component;
	GtkWidget* widget;

    win->components = NULL; // TODO: gotta free old stuff before this probably

	if (win->game->component_config) {
		component_config = &win->game->component_config[0];
	} else {
		/* No component config was given, use defaults! */
		ls_app_window_default_components(win);
		return;
	}

    LOG_DEBUG("Creating components from split file...");

	while (*component_config) {
		component_ref = json_object_get(*component_config, "component");
		component_name = json_string_value(component_ref);
		if (!component_name) {
			// TODO: This might be more helpful as a popup to the user
			// ^^ Extra credit: maybe give options for "skip" or "use default"
			// ^^ Though, this has implications on what to save
			LOG_DEBUG("Invalid component config");
			break;
		} else if (!(component_init = get_component(component_name))) {
			// TODO: see above comment ^^
			LOG_DEBUGF("Unrecognized component `%s`", component_name);
			break;
		}

		// TODO: Pass config here!!
        component = component_init->new();
		json_decref(*component_config);
		*component_config = NULL;

        if (component) {
            widget = component->ops->widget(component);
            if (widget) {
                gtk_widget_set_margin_start(widget, WINDOW_PAD);
                gtk_widget_set_margin_end(widget, WINDOW_PAD);
                gtk_container_add(GTK_CONTAINER(win->box),
                    component->ops->widget(component));
            }
            win->components = g_list_append(win->components, component);
			LOG_DEBUGF("Registered component `%s`", component_name);
        }

		++component_config; // Points to next component configuration (json object)
    }

	// Ensure that memory is released even if init failed
	// (TODO/NOTE: Important because we started but didn't finish. Cleanup is easy if we
	// never start. Alternatively I could wait to do all the freeing here, but
	// the routine will be here-ish either way.)
	while (*component_config) {
		json_decref(*component_config);
		*component_config = NULL;
		++component_config;
	}
	free(win->game->component_config);
}

/**
 * Prepares the LibreSplit window to be shown, using the data
 * from the loaded split file.
 *
 * @param win The LibreSplit window.
 */
void ls_app_window_show_game(LSAppWindow* win)
{
    LOG_DEBUG("Showing Game...");
    GList* l;

	ls_app_window_add_components(win);

    // set dimensions
    if (win->game->width > 0 && win->game->height > 0) {
        // First set the "minimum size" allowed
        gtk_widget_set_size_request(GTK_WIDGET(win),
            win->game->width,
            win->game->height);
        // Then automatically resize the window to the preferences
        gtk_window_resize(GTK_WINDOW(win),
            win->game->width,
            win->game->height);
        // User will still be able to resize the window up, but not down
    }

    // set game theme (if it is set)
    if (win->game->theme) {
        ls_app_load_theme_with_fallback(win, win->game->theme, win->game->theme_variant);
    }

    for (l = win->components; l != NULL; l = l->next) {
        LSComponent* component = l->data;
        if (component->ops->show_game) {
            component->ops->show_game(component, win->game, win->timer);
        }
    }

    gtk_widget_show(win->box);
    gtk_widget_hide(win->welcome_box->box);
}

/**
 * Clears the current game and reset all the components.
 *
 * @param win The LibreSplit app window
 */
void ls_app_window_clear_game(LSAppWindow* win)
{
    LOG_DEBUG("Clearing Game...");
    GList* l;

    gtk_widget_hide(win->box);
    gtk_widget_show_all(win->welcome_box->box);

    for (l = win->components; l != NULL; l = l->next) {
        LSComponent* component = l->data;
        if (component->ops->clear_game) {
            component->ops->clear_game(component);
        }
    }

    ls_app_load_theme_with_fallback(win, cfg.libresplit.theme.value.s, cfg.libresplit.theme_variant.value.s);
}

gpointer save_game_thread(gpointer data)
{
    ls_game* game = data;
    ls_game_save(game);
    return NULL;
}

void save_game(ls_game* game)
{
    GThread* thread = g_thread_new("save_game", save_game_thread, game);
    g_thread_unref(thread);
}
