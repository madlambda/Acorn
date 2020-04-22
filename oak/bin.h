/*
 * Copyright (C) Madlambda Authors.
 */


#ifndef _OAK_BIN_H_


#define uencode(v, begin, end)      uleb128encode(v, begin, end)
#define sencode(v, begin, end)      sleb128encode(v, begin, end)
#define udecode(begin, end, res)    uleb128decode(begin, end, res)
#define sdecode(begin, end, res)    sleb128decode(begin, end, res)


ssize_t uleb128encode(u64 v, u8 *begin, u8 *end);
ssize_t sleb128encode(i64 v, u8 *begin, u8 *end);
ssize_t uleb128decode(const u8 *begin, const u8 *end, u64 *res);
ssize_t sleb128decode(const u8 *begin, const u8 *end, i64 *res);

void u32decode(const u8 *data, u32 *val);

#endif /* _OAK_BIN_H_ */
