/** \file components.c
 *
 * Available Components and related utilities
 */
#include "components.h"

#define OPTIONAL 0
#define DEFAULT 1

LSComponent* ls_component_best_sum_new(json_t* config);
LSComponent* ls_component_timer_new(json_t* config);
LSComponent* ls_component_detailed_timer_new(json_t* config);
LSComponent* ls_component_pb_new(json_t* config);
LSComponent* ls_component_prev_segment_new(json_t* config);
LSComponent* ls_component_splits_new(json_t* config);
LSComponent* ls_component_title_new(json_t* config);
LSComponent* ls_component_wr_new(json_t* config);

const LSComponentAvailable ls_components[] = {
    { "title", ls_component_title_new, DEFAULT },
    { "splits", ls_component_splits_new, DEFAULT },
    { "detailed-timer", ls_component_detailed_timer_new, DEFAULT },
    { "prev-segment", ls_component_prev_segment_new, DEFAULT },
    { "best-sum", ls_component_best_sum_new, DEFAULT },
    { "pb", ls_component_pb_new, DEFAULT },
    { "wr", ls_component_wr_new, DEFAULT },

    { "timer", ls_component_timer_new, OPTIONAL },

    { NULL, NULL }
};

/**
 * Look up a component by name.
 *
 * @return Returns a reference to the component via the LSComponentAvailable
 * struct if found, NULL otherwise
 */
const LSComponentAvailable* get_component(const char* const name)
{
    const LSComponentAvailable* ref = &ls_components[0];

    if (!name) {
        return NULL;
    }

    while (ref->name) {
        if (!strcmp(ref->name, name)) {
            return ref;
        }
        ++ref;
    }
    return NULL;
}
