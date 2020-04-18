/*
 * Copyright (C) Madlambda Authors.
 */


#ifndef _OAK_BIN_H_


ssize_t uleb128_encode(u64 v, u8 *begin, u8 *end);
ssize_t sleb128_encode(i64 v, u8 *begin, u8 *end);
ssize_t uleb128_decode(const u8 *begin, const u8 *end, u64 *res);
ssize_t sleb128_decode(const u8 *begin, const u8 *end, i64 *res);


#endif /* _OAK_BIN_H_ */
