/** \file components.c
 *
 * Available Components and related utilities
 */
#include "components.h"

LSComponent* ls_component_basic_timer_new(json_t* user_config);
LSComponent* ls_component_best_sum_new(json_t* user_config);
LSComponent* ls_component_detailed_timer_new(json_t* user_config);
LSComponent* ls_component_pb_new(json_t* user_config);
LSComponent* ls_component_prev_segment_new(json_t* user_config);
LSComponent* ls_component_splits_new(json_t* user_config);
LSComponent* ls_component_title_new(json_t* user_config);
LSComponent* ls_component_wr_new(json_t* user_config);

LSComponentAvailable ls_components[] = {
    { "title", ls_component_title_new },
    { "splits", ls_component_splits_new },
    { "timer", ls_component_basic_timer_new },
    { "detailed-timer", ls_component_detailed_timer_new },
    { "prev-segment", ls_component_prev_segment_new },
    { "best-sum", ls_component_best_sum_new },
    { "pb", ls_component_pb_new },
    { "wr", ls_component_wr_new },
    { NULL, NULL }
};
