/*
 * Copyright (C) Madlambda Authors.
 */

#ifndef _OAK_BIN_H_
#define _OAK_BIN_H_


/*
 * variant encoders and decoders
 */
#define uvencode(v, begin, end)      uleb128encode(v, begin, end)
#define svencode(v, begin, end)      sleb128encode(v, begin, end)
#define uvdecode(begin, end, res)    uleb128decode(begin, end, res)
#define svdecode(begin, end, res)    sleb128decode(begin, end, res)

u8 u8vdecode(u8 **begin, const u8 *end, u8 *val);
u8 u32vdecode(u8 **begin, const u8 *end, u32 *val);
u8 s8vdecode(u8 **begin, const u8 *end, i8 *val);
u8 s32vdecode(u8 **begin, const u8 *end, i32 *val);
u8 s64vdecode(u8 **begin, const u8 *end, i64 *val);

/*
 * fixed little-endian encoders and decoders
 */
u8 u16encode(u16 val, u8 **begin, const u8 *end);
u8 u32encode(u32 val, u8 **begin, const u8 *end);
u8 u32decode(u8 **begin, const u8 *end, u32 *val);
u8 u64decode(u8 **begin, const u8 *end, u64 *val);
u8 u64encode(u64, u8 **begin, const u8 *end);
u8 uencode(u64 val, u8 size, u8 **begin, const u8 *end);


ssize_t uleb128encode(u64 v, u8 *begin, u8 *end);
ssize_t sleb128encode(i64 v, u8 *begin, u8 *end);
ssize_t uleb128decode(const u8 *begin, const u8 *end, u64 *res);
ssize_t sleb128decode(const u8 *begin, const u8 *end, i64 *res);

#endif /* _OAK_BIN_H_ */
