#ifndef BEATS_H
#define BEATS_H

#include <stdint.h>
#include <stdbool.h>

// Beats for rhythm game
// #define OOOO 0b00000000
// #define OOOX 0b00000001
// #define OOXO 0b00000010
// #define OOXX 0b00000011

// #define OXOO 0b00000100
// #define OXOX 0b00000101
// #define OXXO 0b00000110
// #define OXXX 0b00000111

// #define XOOO 0b00001000
// #define XOOX 0b00001001
// #define XOXO 0b00001010
// #define XOXX 0b00001011

// #define XXOO 0b00001100
// #define XXOX 0b00001101
// #define XXXO 0b00001110
// #define XXXX 0b00001111

// Beat definitions
#define OOOO 0b00000000
#define OOOX 0b00000001
#define OOXO 0b00000010
#define OXOO 0b00000100
#define XOOO 0b00001000

typedef struct {
    bool     playing;
    uint32_t index;
    uint32_t count;
    int8_t   code;
    int8_t*  pattern;
    void (*finally)();
} Beats;

#endif // BEATS_H