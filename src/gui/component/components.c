/** \file components.c
 *
 * Available Components and related utilities
 */
#include "components.h"

LSComponent* ls_component_best_sum_new(json_t* config);
LSComponent* ls_component_timer_new(json_t* config);
LSComponent* ls_component_detailed_timer_new(json_t* config);
LSComponent* ls_component_lua_title_new(json_t* config);
LSComponent* ls_component_pb_new(json_t* config);
LSComponent* ls_component_prev_segment_new(json_t* config);
LSComponent* ls_component_splits_new(json_t* config);
LSComponent* ls_component_title_new(json_t* config);
LSComponent* ls_component_wr_new(json_t* config);

LSComponentAvailable const ls_components[] = {
    { "title", ls_component_title_new },
    { "lua_title", ls_component_lua_title_new },
    { "splits", ls_component_splits_new },
    { "timer", ls_component_timer_new },
    { "detailed-timer", ls_component_detailed_timer_new },
    { "prev-segment", ls_component_prev_segment_new },
    { "best-sum", ls_component_best_sum_new },
    { "pb", ls_component_pb_new },
    { "wr", ls_component_wr_new },
    { NULL, NULL }
};

/**
 * Look up a component by name.
 *
 * @return Returns a reference to the component via the LSComponentAvailable
 * struct if found, NULL otherwise
 */
LSComponentAvailable const * get_component(char const * const name) {
	LSComponentAvailable const * ref = &ls_components[0];

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
