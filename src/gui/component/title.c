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
    GtkWidget* header; /*!< The container for the title component */
    GtkWidget* title; /*!< The container for the composite title/subtitle */
    GtkWidget* game; /*!< The label containing the title itself */
    GtkWidget* category; /*!< The label containing the subtitle itself */
    GtkWidget* attempts; /*!< The label containing the number of attempts. */
    lasr_global* title_content; /*<! Lua export container for dynamic title */
    lasr_global* category_content; /*<! Lua export container for dynamic title */
} LSTitle;
extern LSComponentOps ls_title_operations; // defined at the end of the file

/**
 * Constructor
 */
LSComponent* ls_component_title_new(json_t* config)
{
    LSTitle* self;
    GtkWidget* counts;

    struct {
        bool show_attempts;
        bool show_category;
        const char* title_source;
        const char* category_source;
    } opt = { 0 };

    self = calloc(1, sizeof(LSTitle));
    if (!self) {
        return NULL;
    }
    self->base.ops = &ls_title_operations;

    /* Configuration option: `show-attempts`
     * default: true
     * If true, only show the title, not the finished/attempts count. */
    opt.show_attempts = !(json_is_false(json_object_get(config, "show-attempts")));

    /* Configuration option: `show-category`
     * default: false
     * If true, show the current category as a subtitle below the title. */
    opt.show_category = json_is_true(json_object_get(config, "show-category"));

    /* Configuration option: `title-source`
     * default: none
     * If provided, the corresponding :luavar will be tracked. Whenever this is
     * is a string of non-zero length, it will be displayed in place of the
     * title. */
    opt.title_source = json_string_value(json_object_get(config, "title-source"));

    /* Configuration option: `category-source`
     * default: none
     * If provided, the corresponding :luavar will be tracked. Whenever this is
     * is a string of non-zero length, it will be displayed in place of the
     * category subtitle. */
    opt.category_source = json_string_value(json_object_get(config, "category-source"));

    /* --- End of configuration options --- */

    self->header = gtk_center_box_new();
    gtk_center_box_set_shrink_center_last(GTK_CENTER_BOX(self->header), FALSE);
    add_class(self->header, "header");

    self->title = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(self->header), self->title);

    if (opt.title_source != NULL) {
        self->title_content = lasr_global_create(opt.title_source);
        register_shared_global(self->title_content);
    }

    self->game = gtk_label_new(NULL);
    add_class(self->title, "title");
    gtk_label_set_justify(GTK_LABEL(self->game), GTK_JUSTIFY_CENTER);
    gtk_label_set_wrap(GTK_LABEL(self->game), TRUE);
    gtk_widget_set_hexpand(self->game, TRUE);
    gtk_box_append(GTK_BOX(self->title), self->game);

    if (opt.show_category) {
        if (opt.category_source != NULL) {
            self->category_content = lasr_global_create(opt.category_source);
            register_shared_global(self->category_content);
        }

        self->category = gtk_label_new(NULL);
        add_class(self->category, "category");
        gtk_label_set_justify(GTK_LABEL(self->category), GTK_JUSTIFY_CENTER);
        gtk_label_set_wrap(GTK_LABEL(self->category), TRUE);
        gtk_widget_set_hexpand(self->category, TRUE);
        gtk_widget_set_visible(self->category, FALSE); // only show once we know there's content
        gtk_box_append(GTK_BOX(self->title), self->category);
    }

    if (opt.show_attempts) {
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
    lasr_global_release(self->category_content);
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
    gtk_label_set_text(GTK_LABEL(self->game), game->name);

    if (self->category) {
        if (game->category || self->category_content) {
            gtk_label_set_text(GTK_LABEL(self->category), game->category);
            gtk_widget_set_visible(self->category, TRUE);
        }
    }
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

    /* Main title export */
    title_type = import_shared_global(self->title_content, &title_export);
    if (title_type == LASR_TYPE_DYNAMIC) {
        gtk_label_set_text(GTK_LABEL(self->game), title_export.dynamic->bytes);
    } else if (title_type == LASR_TYPE_ATOMIC) {
        /* Numeric type */
        snprintf(buf, sizeof(buf), "%.2lf", title_export.fixed);
        gtk_label_set_text(GTK_LABEL(self->game), buf);
    }
    lasr_export_resize(&title_export, 0);

    /* Subtitle export */
    title_type = import_shared_global(self->category_content, &title_export);
    if (title_type == LASR_TYPE_DYNAMIC) {
        gtk_label_set_text(GTK_LABEL(self->category), title_export.dynamic->bytes);
    } else if (title_type == LASR_TYPE_ATOMIC) {
        /* Numeric type */
        snprintf(buf, sizeof(buf), "%.2lf", title_export.fixed);
        gtk_label_set_text(GTK_LABEL(self->category), buf);
    }
    lasr_export_resize(&title_export, 0);
}

LSComponentOps ls_title_operations = {
    .delete = title_delete,
    .widget = title_widget,
    .show_game = title_show_game,
    .draw = title_draw
};
