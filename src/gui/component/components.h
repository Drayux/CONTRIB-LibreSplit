#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include <ctype.h>
#include <gtk/gtk.h>
#include <jansson.h>
#include <stdlib.h>
#include <string.h>

#include "../../timer.h"
#include "../utils.h"

typedef struct LSComponentOps LSComponentOps; // forward declaration

typedef struct LSComponent {
    LSComponentOps* ops;
} LSComponent;

typedef struct LSComponentOps {
    void (*delete)(LSComponent* self);
    GtkWidget* (*widget)(LSComponent* self);

    void (*resize)(LSComponent* self, int win_width, int win_height);
    void (*show_game)(LSComponent* self, const ls_game* game, const ls_timer* timer);
    void (*clear_game)(LSComponent* self);
    void (*draw)(LSComponent* self, const ls_game* game, const ls_timer* timer);

    void (*start_split)(LSComponent* self, const ls_timer* timer);
    void (*skip)(LSComponent* self, const ls_timer* timer);
    void (*unsplit)(LSComponent* self, const ls_timer* timer);
    void (*stop_reset)(LSComponent* self, ls_timer* timer);
    void (*pause)(LSComponent* self, ls_timer* timer);
    void (*unpause)(LSComponent* self, ls_timer* timer);
    void (*cancel_run)(LSComponent* self, ls_timer* timer);
} LSComponentOps;

/* TODO: I added this json_t* user_config, but the timer-related
 * changes might be better suited for the show_game / clear_game
 * callbacks. The "config" might be the same way, as this would
 * be derived from a splits file (for now.)
 *
 * Alas, I need to check if there's a scenario that demands that
 * we regenerate the component entirely. If so, then here is the
 * correct location. */
typedef struct LSComponentAvailable {
    char* name;
    LSComponent* (*new)(json_t* user_config);
} LSComponentAvailable;

// A NULL-terminated array of all available components
extern LSComponentAvailable ls_components[];

#endif /* __COMPONENTS_H__ */
