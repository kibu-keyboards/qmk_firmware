/*
 * Modified by KIBU (kibu-keyboards) <M@kibu.jp> on 2026-09-24.
 * Changes: Translated and clarified existing comments in English.
 * No executable code was changed by this comment revision.
 * Original authorship, copyright, and license notices are retained below.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "matrix.h"
    

extern unsigned int Debounce_Delay;   //Key debounce interval; maximum 127

/**
 * @brief Debounce raw matrix events according to the choosen debounce algorithm.
 *
 * @param raw The current key state
 * @param cooked The debounced key state
 * @param num_rows Number of rows to debounce
 * @param changed True if raw has changed since the last call
 * @return true Cooked has new keychanges after debouncing
 * @return false Cooked is the same as before
 */
bool debounce(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, bool changed);

void debounce_init(uint8_t num_rows);

void debounce_free(void);
