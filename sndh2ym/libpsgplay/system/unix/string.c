// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "internal/assert.h"
#include "internal/print.h"
#include "internal/string.h"

#include "system/unix/memory.h"
#include "system/unix/string.h"

char *xstrdup(const char *s)
{
	void *t = strdup(s);

	if (!t)
		pr_fatal_errno("strdup");

	return t;
}

char *xstrcat(const char *a, const char *b)
{
	const size_t alen = strlen(a);
	const size_t blen = strlen(b);
	char *s = xmalloc(alen + blen + 1);

	memcpy(&s[0], a, alen);
	memcpy(&s[alen], b, blen);
	s[alen + blen] = '\0';

	return s;
}

char *xstrndup(const char *s, size_t n)
{
	void *t = strndup(s, n);

	if (!t)
		pr_fatal_errno("strndup");

	return t;
}

char *strrep(const char *s, const char *from, const char *to)
{
	const size_t s_len = strlen(s);
	const size_t from_len = strlen(from);
	const size_t to_len = strlen(to);
	struct string_split split;

	size_t n = 0;
	for_each_string_split (split, s, from)
		if (split.sep)
			n++;

	const size_t t_len = to_len < from_len ?
		s_len - (from_len - to_len) * n :
		s_len + (to_len - from_len) * n;

	char *t = xmalloc(t_len + 1);

	size_t i = 0;
	for_each_string_split (split, s, from)
		if (split.sep) {
			memcpy(&t[i], to, to_len);
			i += to_len;
		} else {
			memcpy(&t[i], split.s, split.length);
			i += split.length;
		}

	BUG_ON(i != t_len);

	t[i] = '\0';

	return t;
}

void sbfree(struct strbuf *sb)
{
	free(sb->s);

	*sb = (struct strbuf) { };
}

bool sbprintf(struct strbuf *sb, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	const int length = vsbprintf(sb, fmt, ap);
	va_end(ap);

	return length;
}

bool sbmprintf(struct strbuf *sb, size_t margin, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	const int length = vsbmprintf(sb, margin, fmt, ap);
	va_end(ap);

	return length;
}

bool vsbprintf(struct strbuf *sb, const char *fmt, va_list ap)
{
	return vsbmprintf(sb, 4096, fmt, ap);
}

bool vsbmprintf(struct strbuf *sb, size_t margin, const char *fmt, va_list ap)
{
	if (!margin)
		return false;

	if (sb->capacity < sb->length + margin) {
		const size_t capacity = sb->length + 2 * margin;
		char *s = realloc(sb->s, capacity);

		if (!s)
			return false;

		sb->capacity = capacity;
		sb->s = s;
	}

	const int length = vsnprintf(&sb->s[sb->length],
		sb->capacity - sb->length, fmt, ap);

	if (length < 0 || sb->capacity <= sb->length + length + 1) {
		sb->s[sb->length] = '\0';
		return false;
	}

	sb->length += length;

	return true;
}
