/*
 * callback.h
 *
 *  Created on: 24 de fev. de 2025
 *      Author: diego.marinho
 */

#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <stdint.h>   // Tipos padrão
#include <stdbool.h>  // Tipo booleano
#include "app.h"


void callback_connectivity_receive(uint8_t *data, uint8_t len, uint8_t mode);
void callback_longRange_receive(uint8_t *data, uint8_t len);
void callback_teclado_receive(uint8_t *data, uint8_t len);


#endif /* CALLBACK_H_ */
