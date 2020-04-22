/*
 * Copyright (C) Madlambda Authors
 */

#include <acorn.h>
#include <string.h>
#include <stdlib.h>
#include "string.h"


int
main()
{
    String  *buf;

    static const String  ken = str("Ken Thompson");
    static const String  dmr = str("Dennis Ritchie");
    static const String  rp  = str("Rob Pike");
    static const String  ack = str("Alan Kay");

    buf = allocstring(0);
    if (slow(buf == NULL)) {
        return 1;
    }

    buf = append(buf, &ken);
    buf = append(buf, &dmr);
    buf = append(buf, &rp);
    buf = append(buf, &ack);

    if (slow(!cstringcmp(buf, "Ken ThompsonDennis RitchieRob PikeAlan Kay"))) {
        return 1;
    }

    free(buf);

    return 0;
}
