#include <string>
#include "types.h"


struct event{
    event_types event_t;
    int from;
    int intended_to;
    std::string transaction;
    struct block *mined_block;
};