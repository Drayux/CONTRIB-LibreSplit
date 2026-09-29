/** \file title.c
 *
 * Implementation of the title component
 */
#include "components.h"

#include "../../lasr/export.h"
#include "../../lasr/utils.h"
#include "../../logging.h"

/**
 * @brief The component representing the title.
 *
 * Represents the title of the run, as well as the count of attempts, both finished and total.
 */
typedef struct LSTitle {
    LSComponent base; /*!< The base struct that is extended */
    GtkWidget* header; /*!< The container for the title */
    GtkWidget* title; /*!< The label containing the title itself */
    lasr_global* title_content; /*<! Lua export container for dynamic title */
    GtkWidget* attempts; /*!< The label containing the number of attempts. */
} LSTitle;
extern LSComponentOps ls_title_operations; // defined at the end of the file

/**
 * Constructor
 */
LSComponent* ls_component_title_new(json_t* config)
{
    LSTitle* self;
    GtkWidget* counts;

    char const* config_source = NULL;
    bool config_simple = false;

    self = calloc(1, sizeof(LSTitle));
    if (!self) {
        return NULL;
    }
    self->base.ops = &ls_title_operations;

    /* Configuration option: `simple`
	 * default: false
	 * If true, only show the title, not the finished/attempts count. */
	config_simple = json_is_true(json_object_get(config, "simple"));
    
	/* Configuration option: `source`
	 * default: none
	 * If provided, the corresponding :luavar will be tracked. Whenever this is
	 * is a string of non-zero length, it will be displayed in place of the
	 * title. */
	config_source = json_string_value(json_object_get(config, "source"));
	if (config_source != NULL) {
        self->title_content = lasr_global_create(config_source);
        register_shared_global(self->title_content);
	}

    self->header = gtk_center_box_new();
    gtk_center_box_set_shrink_center_last(GTK_CENTER_BOX(self->header), FALSE);
    add_class(self->header, "header");

    self->title = gtk_label_new(NULL);
    add_class(self->title, "title");
    gtk_label_set_justify(GTK_LABEL(self->title), GTK_JUSTIFY_CENTER);
    gtk_label_set_wrap(GTK_LABEL(self->title), TRUE);
    gtk_widget_set_hexpand(self->title, TRUE);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(self->header), self->title);

    if (!config_simple) {
        counts = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

        self->attempts = gtk_label_new(NULL);
        add_class(self->attempts, "attempt-count");
        gtk_widget_set_margin_start(self->attempts, 8);
        gtk_widget_set_valign(self->attempts, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(counts), self->attempts);

        gtk_center_box_set_end_widget(GTK_CENTER_BOX(self->header), counts);
    }

    return (LSComponent*)self;
}

/**
 * Destructor
 *
 * @param self The component to destroy
 */
static void title_delete(LSComponent* self_)
{
    LSTitle* self = (LSTitle*)self_;
    lasr_global_release(self->title_content);
    free(self);
}

/**
 * Returns the Title GTK widget.
 *
 * @param self The Title component itself.
 * @return The container as a GTK Widget.
 */
static GtkWidget* title_widget(LSComponent* self)
{
    return ((LSTitle*)self)->header;
}

/**
 * Function to execute when ls_app_window_show_game is executed.
 *
 * @param self_ The Title component itself.
 * @param game The game struct instance.
 * @param timer The timer instance.
 */
static void title_show_game(LSComponent* self_, const ls_game* game,
    const ls_timer* timer)
{
    LSTitle* self = (LSTitle*)self_;

    /* Always start with game title -- User might only update this with a lua
     * var mid-run */
    gtk_label_set_text(GTK_LABEL(self->title), game->title);
}

/**
 * Function to execute when ls_app_window_draw is executed.
 *
 * @param self_ The Title component itself.
 * @param game The game struct instance.
 * @param timer The timer instance.
 */
static void title_draw(LSComponent* self_, const ls_game* game, const ls_timer* timer)
{
    char buf[64];
    lasr_export title_export;
    int title_type;
    LSTitle* self = (LSTitle*)self_;

    if (self->attempts) {
        snprintf(buf, sizeof(buf), "#%d/#%d",
            game->finished_count,
            game->attempt_count);
        gtk_label_set_text(GTK_LABEL(self->attempts), buf);
    }
    
    title_type = import_shared_global(self->title_content, &title_export);
    if (title_type == LASR_TYPE_DYNAMIC) {
        gtk_label_set_text(GTK_LABEL(self->title), title_export.dynamic->bytes);
    } else if (title_type == LASR_TYPE_ATOMIC) {
        /* Numeric type */
        snprintf(buf, sizeof(buf), "%.2lf", title_export.fixed);
        gtk_label_set_text(GTK_LABEL(self->title), buf);
    }

    /* Cleanup -- no memory was allocated if string not changed */
    lasr_export_resize(&title_export, 0);
}

LSComponentOps ls_title_operations = {
    .delete = title_delete,
    .widget = title_widget,
    .show_game = title_show_game,
    .draw = title_draw
};
