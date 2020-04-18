/*
 * Copyright (C) Madlambda Authors.
 */


#ifndef _OAK_BIN_H_


ssize_t oak_uleb128_encode(uint64_t v, uint8_t *begin, uint8_t *end);
ssize_t oak_sleb128_encode(int64_t v, uint8_t *begin, uint8_t *end);
ssize_t oak_bin_uleb128_decode(const uint8_t *begin, const uint8_t *end,
    uint64_t *res);
ssize_t oak_bin_sleb128_decode(const uint8_t *begin, const uint8_t *end,
    int64_t *res);


#endif /* _OAK_BIN_H_ */
