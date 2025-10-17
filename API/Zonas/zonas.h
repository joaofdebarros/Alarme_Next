/*
 * zonas.h
 *
 *  Created on: 27 de fev. de 2025
 *      Author: diego.marinho
 */

#ifndef ZONAS_ZONAS_H_
#define ZONAS_ZONAS_H_

#include "stdint.h"
#include "cyabs_rtos.h"
#include "alarm.h"
#include "cy_pdl.h"
#include "cybsp.h"

/* The number of ADC channels */
#define ADC_CHANNNELS_NUM                   (8U)


#define NUM_ZONAS 1
#define ZONAS_MAX_QUEUE 1
#define QUEUE_SIZE_ZONA sizeof(setup_zonas_t)

// Definição dos limiares para cada configuração
#define RESISTOR_1K   1000
#define RESISTOR_2K2  2200
#define RESISTOR_3K9  3900

#define RESISTOR1_PPA RESISTOR_2K2
#define RESISTOR2_PPA RESISTOR_3K9

#define RESISTOR1_JFL RESISTOR_2K2
#define RESISTOR2_JFL RESISTOR_3K9

#define RESISTOR1_INTELBRAS RESISTOR_2K2
#define RESISTOR2_INTELBRAS RESISTOR_3K9

// Estados das zonas
typedef enum {
    ZONA_INVIOLADA,
	ZONA_INFERIOR_VIOLADA,
	ZONA_SUPERIOR_VIOLADA,
    ZONA_VIOLADA = 1,
    TENTATIVA_FRAUDE = 3
} state_zona_t;

typedef enum{
	MODEL_PPA = 0,
	MODEL_INTELBRAS,
	MODEL_JFL
}Model_resistor_t;

typedef enum{
	Type_ZonaDupla_Serie = 1,
	Type_ZonaDupla_Paralelo,
	Type_ZonaSimples,
	Type_ZonaSimplesSR
}Type_Zona_t;

typedef struct{
	Model_resistor_t Model_resistor;
	Type_Zona_t Type_Zona;
}setup_t;

typedef struct{
    //uint16_t leitura_adc;
    state_zona_t state;
    uint8_t i_chanel;
    //char name[20];
    uint16_t ADC;
    bool status_channel;
}setup_zonas_t;

typedef struct{
    struct{
        hEvents_t waitInt;
        hEvents_t handle;
        uint32_t index;
    }event;

	cy_rslt_t status_rtos_connectivity;
	cy_thread_t thread_zona;
	cy_queue_t zona_queue;

	uint16_t adc_result_buf[ADC_CHANNNELS_NUM];
	uint8_t channel_z1_11;
	uint8_t channel_z2_12;
	uint8_t channel_z3_13;
	uint8_t channel_z4_14;
	uint8_t channel_z5_15;
	uint8_t channel_z6_16;
	uint8_t channel_z7_17;
	uint8_t channel_z8_18;

	setup_zonas_t config_zonas[8];
	setup_t setup;
	const uint16_t buffer_models[3][5];
}zonas_t;


extern zonas_t zonas;

void zonas_init(hEvents_t eventHandle, uint32_t eventIndex);
void zonas_task(void *pvParameters);

state_zona_t analyze_zona(setup_t setup,uint16_t read_ADC);
void getZonaNotification(setup_zonas_t *getZonaNotify);
void Get_SetorViolation(uint8_t *ID_Setor, state_zona_t *state);
void Set_Configuration_setor(setup_t *setup_zona);

#endif /* ZONAS_ZONAS_H_ */
