/*
 * Display.h
 *
 *  Created on: Oct 9, 2025
 *      Author: Nagy Ákos
 */

#ifndef ROBONAUT_DISPLAY_DISPLAY_H_
#define ROBONAUT_DISPLAY_DISPLAY_H_

#include <stdbool.h>
#include <stdint.h>
#include "NeuralNetwork/NeuralNetwork.h"

bool DS_Init(void);
void DS_ResetState(void);
void DS_NetworkOutput(od_pp_out_t* pp_out);
uint8_t DS_IsReadyToUpdate(void);

#endif /* ROBONAUT_DISPLAY_DISPLAY_H_ */
