/*
 * Copyright (C) Madlambda Authors.
 */


#ifndef _OAK_TEST_H_


#define OAK_MAX_ERR_MSG 2048


void *oak_test_alloc(size_t size);
void oak_test_error(const char *fmt, ...);


#endif /* _OAK_TEST_H_ */