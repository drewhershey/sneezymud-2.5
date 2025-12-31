#pragma once

#define LOWER(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + ('a' - 'A')) : (c))

#define UPPER(c) (((c) >= 'a' && (c) <= 'z') ? ((c) + ('A' - 'a')) : (c))

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')

#define IF_STR(st) ((st) ? (st) : "\0")

#define CAP(st) (*(st) = UPPER(*(st)), st)
