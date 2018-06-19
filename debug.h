//
// Created by vadim on 31.05.18.
//

#ifndef SERVER_DEBUG_H
#define SERVER_DEBUG_H

#ifndef NDEBUG
#define DEBUG(A, ...) printf("[DEBUG] (%s:%d) " A "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(A, ...) ((void)0)
#endif

#endif //SERVER_DEBUG_H
