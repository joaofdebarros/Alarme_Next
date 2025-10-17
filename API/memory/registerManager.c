/*
 * registerManager.c
 *
 *  Created on: 10 de mar. de 2025
 *      Author: diego.marinho
 */

#include "registerManager.h"

seguranca_t seg;

uint64_t __seg_conv_buff_u64(uint8_t start)
{
	uint64_t val = ~(0x0ULL);
	uint_fast8_t i, j, k;
	uint_fast8_t up;

	// seg.i stores the amount of digits
	if (seg.i < REGISTER_CODE_MINIMUM_DIGITS){
	    return val;
	}
	up = SEG_LEN_SENHA+start;
	j = 1+start;
	for (i = start; i < seg.i && i < up ; i++) {
	    k = (uint64_t)(seg.i - j);
	    val &= (uint64_t)(~(0xFULL << (k*4)));
	    val |= (uint64_t)((seg.buff_senha[i]&0xFULL) << k*4);
	    j++;
	}

	return (uint64_t)val;
}

void __seg_fgroup_virtual_seek(uint32_t id, int16_t *index, seg_data_t *data){
    int16_t i, j;
    seg_data_t load[1] = {0};

    *index = -1;

    for (i = 0; i < MAX_CODE; i++) {
    	memset(&load[0], 0, sizeof(seg_data_t));
        // Calcula o endereço correto na EEPROM emulada
        uint32_t endereco = (i * MEM_DATA_SIZE)+22;
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_SIZE);
        if(load[0].Access_code == id){
        	*index = i;
        	if(data != NULL){
        		*data = load[0];
        	}
        	return;
        }
    }

}

void __seg_fgroup_senha_tecl_seek(uint64_t id, int16_t *index, seg_data_t *data){
    int16_t i, j;
    seg_data_t load[1] = {0};

    *index = -1;

    for (i = 0; i < MAX_CODE; i++) {
    	memset(&load[0], 0, sizeof(seg_data_t));
        // Calcula o endereço correto na EEPROM emulada
        uint32_t endereco = (i * MEM_DATA_SIZE)+22;
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_SIZE);
        if(load[0].Tecl_Code == id){
        	*index = i;
        	if(data != NULL){
        		*data = load[0];
        	}
        	return;
        }
    }

}

void __seg_fgroup_control_seek(uint64_t id, int16_t *index, seg_data_t *data){
    int16_t i, j;
    seg_data_t load[1] = {0};

    *index = -1;

    for (i = 0; i < MAX_CODE; i++) {
    	memset(&load[0], 0, sizeof(seg_data_t));
        // Calcula o endereço correto na EEPROM emulada
        uint32_t endereco = (i * MEM_DATA_SIZE)+22;
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_SIZE);
        if(load[0].Control == id){
        	*index = i;
        	if(data != NULL){
        		*data = load[0];
        	}
        	return;
        }
    }

}

void __seg_fgroup_setor_seek(uint32_t id, int16_t *index, seg_data_setor_t *data){
    int16_t i, j;
    uint16_t start_end;
    seg_data_setor_t load[1] = {0};

    *index = -1;

    start_end = (MAX_CODE*MEM_DATA_SIZE+22);

    for (i = 0; i < MAX_SETOR; i++) {
    	memset(&load[0], 0, sizeof(seg_data_setor_t));
        // Calcula o endereço correto na EEPROM emulada
        uint32_t endereco = (i * MEM_DATA_SETOR_SIZE)+ start_end;
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_SETOR_SIZE);
        if (load[0].ID_setor == id){
        	*index = i;
        	if(data != NULL){
        		*data = load[0];
        	}
        	return;
        }
    }

}

void __seg_fgroup_partition_seek(uint32_t id, int16_t *index, seg_data_partition_t *data){
	int16_t i;
    uint16_t start_end;
    seg_data_partition_t load[1] = {0};

    *index = -1;

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+22;
    for (i = 0; i < MAX_PARTITION; i++) {
    	memset(&load[0], 0, sizeof(seg_data_partition_t));
        // Calcula o endereço correto na EEPROM emulada
        uint32_t endereco = (i * MEM_DATA_PARTITION_SIZE)+ start_end;
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_PARTITION_SIZE);
        if (load[0].ID_partition == id){
        	*index = i;
        	if(data != NULL){
        		*data = load[0];
        	}
        	return;
        }
    }
}

void __seg_fgroup_sensor_seek(uint32_t id, int16_t *index, seg_data_sensor_t *data){
	int16_t i;
    uint16_t start_end;
    seg_data_sensor_t load[1] = {0};

    *index = -1;

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+22;
    for (i = 0; i < MAX_SENSOR; i++) {
    	memset(&load[0], 0, sizeof(seg_data_sensor_t));
    	uint32_t endereco = (i * MEM_DATA_SENSOR_SIZE)+ start_end;
    	mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_SENSOR_SIZE);
    	if (load[0].ID_Sensor == id){
        	*index = i;
        	if(data != NULL){
        		*data = load[0];
        	}
        	return;
    	}
    }

}

void __seg_fgroup_pgm_seek(uint8_t id, int16_t *index, seg_data_pgm_t *data){
	int16_t i;
    uint16_t start_end;
    seg_data_pgm_t load[1] = {0};

    *index = -1;

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+(MAX_SENSOR * MEM_DATA_SENSOR_SIZE)+22;
    for (i = 0; i < MAX_PGM; i++) {
    	memset(&load[0], 0, sizeof(seg_data_pgm_t));
    	uint32_t endereco = (i * MEM_DATA_PGM_SIZE)+ start_end;
    	mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_PGM_SIZE);
    	if (load[0].ID_PGM == id){
        	*index = i;
        	if(data != NULL){
        		*data = load[0];
        	}
        	return;
    	}
    }

}

void __seg_fgroup_teclado_seek(uint8_t id, int16_t *index, seg_data_teclado_t *data){
	int16_t i;
    uint16_t start_end;
    seg_data_teclado_t load[1] = {0};

    *index = -1;

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+(MAX_SENSOR * MEM_DATA_SENSOR_SIZE)+(MAX_PGM * MEM_DATA_PGM_SIZE)+22;
    for (i = 0; i < MAX_TECLADO; i++) {
    	memset(&load[0], 0, sizeof(seg_data_teclado_t));
    	uint32_t endereco = (i * MEM_DATA_TECLADO_SIZE)+ start_end;
    	mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_TECLADO_SIZE);
    	if (load[0].ID_TECLADO == id){
        	*index = i;
        	if(data != NULL){
        		*data = load[0];
        	}
        	return;
    	}
    }

}

void __seg_fgroup_gate_seek(uint8_t id, int16_t *index, seg_data_gate_t *data){
	int16_t i;
    uint16_t start_end;
    seg_data_gate_t load[1] = {0};

    *index = -1;

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+(MAX_SENSOR * MEM_DATA_SENSOR_SIZE)+(MAX_PGM * MEM_DATA_PGM_SIZE)+(MAX_TECLADO * MEM_DATA_TECLADO_SIZE)+22;
    for (i = 0; i < MAX_GATE; i++) {
    	memset(&load[0], 0, sizeof(seg_data_gate_t));
    	uint32_t endereco = (i * MEM_DATA_GATE_SIZE)+ start_end;
    	mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_GATE_SIZE);
    	if (load[0].ID_gate == id){
        	*index = i;
        	if(data != NULL){
        		*data = load[0];
        	}
        	return;
    	}
    }

}

void __seg_fgroup_cenario_seek(uint8_t id, int16_t *index, seg_data_cenario_t *data){
	int16_t i;
    uint16_t start_end;
    seg_data_cenario_t load[1] = {0};

    *index = -1;

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+(MAX_SENSOR * MEM_DATA_SENSOR_SIZE)+(MAX_PGM * MEM_DATA_PGM_SIZE)+(MAX_TECLADO * MEM_DATA_TECLADO_SIZE)+(MAX_GATE * MEM_DATA_GATE_SIZE)+22;
    for (i = 0; i < MAX_CENARIO; i++) {
    	memset(&load[0], 0, sizeof(seg_data_cenario_t));
    	uint32_t endereco = (i * MEM_DATA_CENARIO_SIZE)+ start_end;
    	mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_CENARIO_SIZE);
    	if (load[0].ID_cenario == id){
        	*index = i;
        	if(data != NULL){
        		*data = load[0];
        	}
        	return;
    	}
    }

}

void _seg_fgroup_seek(mem_access_class_ident_e class, uint64_t id, int16_t *index, seg_data_t *data){
    switch (class) {
        case MEM_USER_CONTROL:
            __seg_fgroup_control_seek(id, index, data);
            break;
        case MEM_USER_TECL:
        	__seg_fgroup_senha_tecl_seek(id, index, data);
            break;
        case MEM_USER_VIRTUAL:
        	__seg_fgroup_virtual_seek(id, index, data);
            break;
        //case MEM_CLASS_BLE:
            //__seg_fgroup_ble_seek_permission((uint32_t)id, index, data);
            //break;
        default:
            *index = -1;
            break;
    }
}

void __seg_fgroup_code_write(int16_t index, seg_data_t *data){
    seg_data_t load[1];
    uint32_t idxFile, idxPos;

    if (index < 0 && index >= SEG_QTDE_SENHAS){
        return;
    }

    uint32_t endereco = (index * MEM_DATA_SIZE)+22;
   // idxFile = index/10;
   // idxPos = index%10;

   // if (idxFile < SEG_FILES_SENHA){
    taskENTER_CRITICAL();
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_SIZE);
        load[0] = *data;
        mem_esc_access_file(endereco, (uint32_t*)load,MEM_DATA_SIZE);
        taskEXIT_CRITICAL();
    //}
}

void __seg_fgroup_setor_write(int16_t index, seg_data_setor_t *data){
	seg_data_setor_t load[1];
	uint16_t start_end;

    if (index < 0 && index >= SEG_QTDE_SENHAS){
        return;
    }

    start_end = (MAX_CODE*MEM_DATA_SIZE+22);

    uint32_t endereco = (index * MEM_DATA_SETOR_SIZE)+ start_end;

    taskENTER_CRITICAL();
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_SETOR_SIZE);
        load[0] = *data;
        mem_esc_access_file(endereco, (uint32_t*)load,MEM_DATA_SETOR_SIZE);
        taskEXIT_CRITICAL();
}

void __seg_fgroup_partition_write(int16_t index, seg_data_partition_t *data){
	seg_data_partition_t load[1];
	uint16_t start_end;

    if (index < 0 && index >= SEG_QTDE_SENHAS){
        return;
    }

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+22;

    uint32_t endereco = (index * MEM_DATA_PARTITION_SIZE)+ start_end;

    taskENTER_CRITICAL();
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_PARTITION_SIZE);
        load[0] = *data;
        mem_esc_access_file(endereco, (uint32_t*)load,MEM_DATA_PARTITION_SIZE);
        taskEXIT_CRITICAL();
}

void __seg_fgroup_sensor_write(int16_t index, seg_data_sensor_t *data){
	seg_data_sensor_t load[1];
	uint16_t start_end;

    if (index < 0 && index >= SEG_QTDE_SENHAS){
        return;
    }

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+22;

    uint32_t endereco = (index * MEM_DATA_SENSOR_SIZE)+ start_end;

    taskENTER_CRITICAL();
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_SENSOR_SIZE);
        load[0] = *data;
        mem_esc_access_file(endereco, (uint32_t*)load,MEM_DATA_SENSOR_SIZE);
        taskEXIT_CRITICAL();
}

void __seg_fgroup_pgm_write(int16_t index, seg_data_pgm_t *data){
	seg_data_pgm_t load[1];
	uint16_t start_end;

    if (index < 0 && index >= SEG_QTDE_SENHAS){
        return;
    }

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+(MAX_SENSOR * MEM_DATA_SENSOR_SIZE)+22;

    uint32_t endereco = (index * MEM_DATA_PGM_SIZE)+ start_end;

    taskENTER_CRITICAL();
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_PGM_SIZE);
        load[0] = *data;
        mem_esc_access_file(endereco, (uint32_t*)load,MEM_DATA_PGM_SIZE);
        taskEXIT_CRITICAL();
}

void __seg_fgroup_teclado_write(int16_t index, seg_data_teclado_t *data){
	seg_data_teclado_t load[1];
	uint16_t start_end;

    if (index < 0 && index >= SEG_QTDE_SENHAS){
        return;
    }

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+(MAX_SENSOR * MEM_DATA_SENSOR_SIZE)+(MAX_PGM * MEM_DATA_PGM_SIZE)+22;

    uint32_t endereco = (index * MEM_DATA_TECLADO_SIZE)+ start_end;

    taskENTER_CRITICAL();
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_TECLADO_SIZE);
        load[0] = *data;
        mem_esc_access_file(endereco, (uint32_t*)load,MEM_DATA_TECLADO_SIZE);
        taskEXIT_CRITICAL();
}

void __seg_fgroup_gate_write(int16_t index, seg_data_gate_t *data){
	seg_data_gate_t load[1];
	uint16_t start_end;

    if (index < 0 && index >= SEG_QTDE_SENHAS){
        return;
    }

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+(MAX_SENSOR * MEM_DATA_SENSOR_SIZE)+(MAX_PGM * MEM_DATA_PGM_SIZE)+(MAX_TECLADO * MEM_DATA_TECLADO_SIZE)+22;

    uint32_t endereco = (index * MEM_DATA_GATE_SIZE)+ start_end;

    taskENTER_CRITICAL();
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_GATE_SIZE);
        load[0] = *data;
        mem_esc_access_file(endereco, (uint32_t*)load,MEM_DATA_GATE_SIZE);
        taskEXIT_CRITICAL();
}

void __seg_fgroup_cenario_write(int16_t index, seg_data_cenario_t *data){
	seg_data_cenario_t load[1];
	uint16_t start_end;

    if (index < 0 && index >= SEG_QTDE_SENHAS){
        return;
    }

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+(MAX_SENSOR * MEM_DATA_SENSOR_SIZE)+(MAX_PGM * MEM_DATA_PGM_SIZE)+(MAX_TECLADO * MEM_DATA_TECLADO_SIZE)+(MAX_GATE * MEM_DATA_GATE_SIZE)+22;

    uint32_t endereco = (index * MEM_DATA_CENARIO_SIZE)+ start_end;

    taskENTER_CRITICAL();
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_CENARIO_SIZE);
        load[0] = *data;
        mem_esc_access_file(endereco, (uint32_t*)load,MEM_DATA_CENARIO_SIZE);
        taskEXIT_CRITICAL();
}

void _seg_fgroup_write(mem_access_class_e class, uint16_t index, seg_data_t *data){
    switch (class) {
        case MEM_CLASS_CODE:
            __seg_fgroup_code_write(index, data);
            break;

        default:

            break;
    }
}


seg_status_e seg_cad_user_from_cmd(mem_access_class_e class, volatile void* data, tipo_ekey_e role, volatile void* new_user_id, uint16_t *id, uint16_t *puid){
    int16_t newIndex, existIndex;
    uint32_t *new_perm = (uint32_t*)new_user_id;
    uint64_t ident = *((uint64_t*)(new_user_id));

    __seg_fgroup_virtual_seek(SEG_IDENT_INVALID, &newIndex, NULL);
    if (newIndex == -1){
        return SEG_POS_OCUPADA;
    }

	switch (class){
	case MEM_CLASS_CODE:
		_seg_fgroup_seek(class, SEG_IDENT_INVALID, &newIndex, NULL);
	    if (newIndex == -1){
	        return SEG_POS_OCUPADA;
	    }
	    //seg_fill_data(&seg.data_aux, (data_add_t*)data);
        if (seg_fill_data(&seg.data_aux, (data_add_t*)data) == 0){
            sprintf(seg.data_aux.nome, "SENHA %d", (newIndex+1));
        }
		break;
	case MEM_CLASS_REMOTE:

        if (seg_fill_data_remote(&seg.data_aux, (data_add_remote_t*)data) == 0){
            sprintf(seg.data_aux.nome, "SENHA %d", (newIndex+1));
        }
		seg.data_aux.Access_code = perm_nova_get(seg.ble_root.perm);
		//seg.data_aux.Access_code = 0xE6201B06;
        if (new_perm != NULL){
            *new_perm = seg.data_aux.Access_code;
        }
		break;
	}


	// check if the desired access exists
	__seg_fgroup_virtual_seek(seg.data_aux.Access_code, &existIndex, NULL);
	//_seg_fgroup_seek(class, ident, &existIndex, NULL);
	if (existIndex != -1){
	    return SEG_ERR_CAD;
	}
	//seg.data_aux.Access_code = ident;
	//_seg_fgroup_write(class, newIndex, &seg.data_aux);
	__seg_fgroup_code_write(newIndex, &seg.data_aux);

	//__seg_fgroup_virtual_seek(seg.data_aux.Access_code, &newIndex, NULL);
	return SEG_OK;
}

// valida se eh dia unico
uint8_t __seg_dia_unico(mem_access_class_e class, uint8_t index, seg_data_t* user)
{
	// se o acesso eh unico
	if (user->d_acesso.d_unico) {
        seg.com_single.e = index;
        seg.com_single.pending = 1;
        seg.com_single.cls = class;
		return 1;
	}
	return 0;
}

// valida o acesso para usuarios ekey e senha
seg_status_e __seg_acesso_valida(mem_access_class_e class, int16_t index, seg_data_t *tp)
{
	uint64_t uts_limit;
	uint32_t uts_atual;

	if (tp->f_inval) {
		return SEG_INVALIDA;
	}

	uts_limit = tp->uts_inic + tp->uts_delta;
	uts_atual = rtc_get_timestamp();

	if (tp->uts_inic == 0) { // se nao tem condicao de data
//	    if (class == MEM_CLASS_BLE){
//            if (tp->role >= ROLE_MASTER) {
//                return SEG_OK_ADMIN;
//            }
//	    }
		// se ha dia de acesso, apesar de nao haver condicao de tempo
		if (rtc_dia_valida(tp->d_acesso, tp->min_inic, tp->min_delta)) {
			// se o acesso eh unico
			__seg_dia_unico(class, index, tp);
			return SEG_OK;
		}
        if(uts_atual < 1669379454){ //20/06/2022 17:50
          return SEG_OK;
        }
		return SEG_INVALIDA;
	}


	// se o inicio ainda nao eh valido
	if (tp->uts_inic > uts_atual) {
		return SEG_INVALIDA;
	}

	// se o limite for maior que a atual
	if (uts_limit > uts_atual) { // se ainda eh valido o acesso
        if (class == MEM_CLASS_REMOTE){
            if (tp->role >= ROLE_MASTER) { // se for admin, eh o unico campo a ser validado
                return SEG_OK_OWNER;
            }
        }
		// se nao for admin, tem que validar a hora e o dia que esta sendo feito o acesso
		if (rtc_dia_valida(tp->d_acesso, tp->min_inic, tp->min_delta)) {
			// if access is limited to one
		    // we can't invalidate the access, we need a "commit"
		    // becase the locker can fail and the user will need a retry
		    __seg_dia_unico(class, index, tp);
			return SEG_OK;
		}
        if(uts_atual < 1655758169){ //20/06/2022 17:50
            return SEG_OK;
        }
		return SEG_INVALIDA;
	}
	// se nao
	tp->f_inval = 1;
//	mem_esc_data(e, tp->raw_data);
	_seg_fgroup_write(class, index, tp);
	return SEG_INVALIDA;
}

seg_status_e seg_ident_valida(mem_access_class_e class, mem_access_class_ident_e class_iden, uint64_t ident, uint32_t uts, uint8_t *nome, uint16_t *id)
{
    seg_data_t segData;
    int16_t idx;

	if (ident == 0) {
		return SEG_ERR_CAD;
	}
	//seg.ble_root.perm = 0;

	if (class == MEM_CLASS_REMOTE && (uint32_t)ident == seg.ble_root.perm) {
	    // if permi is the root permission, the position user id MUST be 00
	    if (id == NULL || *id == 0 || *id == 1){
            rtc_set_timestamp(uts);
            if (nome != NULL) {
                memcpy(nome, seg.ble_root.uuid, 16);
            }
            return SEG_OK_ROOT;
	    }
	    else{
	        return SEG_ERR_CAD;
	    }
	}

	if (class == MEM_CLASS_TECL && (uint32_t)ident == seg.ble_root.senha) {
	    // if permi is the root permission, the position user id MUST be 00
	    if (id == NULL || *id == 0 || *id == 1){
            if (nome != NULL) {
                memcpy(nome, seg.ble_root.uuid, 16);
            }
            return SEG_OK_ROOT;
	    }
	    else{
	        return SEG_ERR_CAD;
	    }
	}

	if (class == MEM_CLASS_REMOTE && (uint32_t)ident == seg.ble_owner.perm) {
	    // if permi is the root permission, the position user id MUST be 00
	    if (id == NULL || *id == 0){
            rtc_set_timestamp(uts);
            if (nome != NULL) {
                memcpy(nome, seg.ble_owner.name, 16);
            }
            return SEG_OK_OWNER;
	    }
	    else{
	        return SEG_ERR_CAD;
	    }
	}

	_seg_fgroup_seek(class_iden, ident, &idx, &segData);
	if (idx != -1){
        // check if id to compare is NULL or differente on 0 (root)
//	    if (class == MEM_CLASS_BLE){
//            if (id != NULL && *id != 0){
//                // check if ID is the same of the permission
//                // the permission has to match the aes key used to decrypt packet
//                if (*id != (uint16_t)(idx + MEM_END_BLE_S)){
//                    return SEG_ERR_CAD;
//                }
//            }
//            if (segData.role == ROLE_MASTER) {
//                rtc_set_timestamp(uts);
//            }
//	    }
        if (nome != NULL) {
            memcpy(nome, segData.nome, 16);
        }
        return __seg_acesso_valida(class, idx, &segData);
	}
	return SEG_ERR_CAD; // essa permissao nao esta cadastrada
}

void seg_inic_buff_senha(void)
{
	seg.i = 0;
	memset(seg.buff_senha, 0xFF, SEG_LEN_BUFF_SENHA);
}

seg_status_e seg_code_val_acesso(uint8_t *nome)
{
	uint64_t antSpyCode;
	int16_t idx;
	uint_fast8_t i;
	seg_status_e ret;

    // check code length only if isn't change root code
    // because default admin code are 1234
    if (__seg_conv_buff_u64(0) == SEG_CODE_INVALID){
        seg_inic_buff_senha();
        return SEG_CODE_SHORT;
    }

	ret = SEG_ERR_CAD;
	i = 0;
    do{
        antSpyCode = __seg_conv_buff_u64(i);
        if (antSpyCode == seg.ble_root.senha) {
            if (nome != NULL){
                memset(nome, 0, 16);
                strcpy((char*)nome, "INSTALADOR CODE");
            }
            ret = SEG_OK_ROOT;
            break;
        }
        if (antSpyCode == seg.ble_owner.senha) {
            if (nome != NULL){
                memset(nome, 0, 16);
                strcpy((char*)nome, "OWNER CODE");
            }
            ret = SEG_OK_OWNER;
            break;
        }
        if (antSpyCode == seg.senha_hard_reset){
            ret = SEG_HARD_RESET;
            break;
        }
        _seg_fgroup_seek(MEM_USER_TECL, antSpyCode, &idx, &seg.data_aux);
        if (idx != -1){
            ret = __seg_acesso_valida(MEM_CLASS_CODE, idx, &seg.data_aux);
            if (nome != NULL){
                memset(nome, 0, 16);
                strcpy((char*)nome, seg.data_aux.nome);
            }
            break;
        }
        i++;
    } while ((seg.i-i) >= REGISTER_CODE_MINIMUM_DIGITS);
	seg_inic_buff_senha();

	return ret;
}

uint8_t seg_eh_nova_senha(void)
{
	return seg.f_nova_senha;
}

seg_status_e seg_control_val_acesso(mem_access_class_ident_e mem_access_class_ident, uint8_t *nome, uint8_t *ID, uint8_t button, uint8_t *resource, uint8_t *resource_type)
{
	int32_t antSpyCode;
	int16_t idx;
	seg_status_e ret;


	ret = SEG_ERR_CAD;

	if(mem_access_class_ident == MEM_USER_CONTROL){

		antSpyCode = *((uint8_t*)(ID));

		_seg_fgroup_seek(mem_access_class_ident, antSpyCode, &idx, &seg.data_aux);
		if (idx != -1){
			ret = __seg_acesso_valida(MEM_CLASS_CODE, idx, &seg.data_aux);
			ret = SEG_OK;
			if (ret == SEG_OK && nome != NULL){
				if(button == 1){
					*resource = seg.data_aux.resource_Button1;
					*resource_type = seg.data_aux.type_resource_Button1;
				}
				else if(button == 2){
					*resource = seg.data_aux.resource_Button2;
					*resource_type = seg.data_aux.type_resource_Button2;
				}
				else if(button == 3){
					*resource = seg.data_aux.resource_Button3;
					*resource_type = seg.data_aux.type_resource_Button3;
				}
				else{
					*resource = seg.data_aux.resource_Button4;
					*resource_type = seg.data_aux.type_resource_Button4;
				}

				memset(nome, 0, 16);
				seg.data_aux.nome[15] = 0;//fazer isso em outras rotinas similares ou receber 0 nas posições não usadas
				strcpy((char*)nome, seg.data_aux.nome);
			}
		}
	}
	else if(mem_access_class_ident == MEM_USER_VIRTUAL){
		antSpyCode = *((uint32_t*)(ID));

		_seg_fgroup_seek(mem_access_class_ident, antSpyCode, &idx, &seg.data_aux);
		if (idx != -1){
			ret = __seg_acesso_valida(MEM_CLASS_CODE, idx, &seg.data_aux);
			ret = SEG_OK;
			if (ret == SEG_OK && nome != NULL){
				//*resource = seg.data_aux.resource;
				//*resource_type = seg.data_aux.type_resource;
				memset(nome, 0, 16);
				//seg.data_aux.nome[15] = 0;//fazer isso em outras rotinas similares ou receber 0 nas posições não usadas
				strcpy((char*)nome, seg.data_aux.nome);
			}
		}
	}

	return ret;
}

/**
 * uint32_t seg_ble_cad_user(ekey_add_t* data);
 * @brief: cadastro de um app conectado a fechadora por ble
 * @param: ponteiro para estrutura de dados da ekey
 * @param: ponteiro para variavel da nova permissao
 * @retval:status da operacao
 */
seg_status_e seg_ble_root_cad(uint8_t name[16], uint8_t *key, uint32_t* new_perm)
{
	*new_perm = 0;
	if (seg.ble_root.perm != 0) {
		return SEG_INVALIDA;
	}
	seg.ble_root.perm = perm_nova_get(0);

	memcpy(seg.ble_root.uuid, name, 16);

	*new_perm = seg.ble_root.perm;

	return SEG_OK;
}

seg_status_e seg_owner_cad(uint8_t name[16], uint8_t id[16], uint32_t* new_perm)
{
	*new_perm = 0;
	if (seg.ble_owner.perm != 0) {
		return SEG_INVALIDA;
	}
	seg.ble_owner.perm = perm_nova_get(0);

	memcpy(seg.ble_owner.uuid, id, 16);
	memcpy(seg.ble_owner.name, name, 16);

	*new_perm = seg.ble_owner.perm;

	return SEG_OK;
}

seg_status_e seg_ble_commit(remote_commit_op op, uint8_t *outData)
{
    seg_data_t segData = {0};
    ble_root_t test_root;

	switch (op) {
	case BLE_COMMIT_INST:
		taskENTER_CRITICAL();
		mem_esc_root_ble(seg.ble_root.raw);
		taskEXIT_CRITICAL();

		// se o commit do root for uma promocao de algum usuario
//		_seg_fgroup_load(MEM_CLASS_BLE, seg.ekey.end, &segData);
//		if (seg.ble_root.perm == segData.perm) {
//			// remove ele da lista de usuarios cadastrados
//			seg.ekey.count--;
//			_seg_fgroup_del(MEM_CLASS_BLE, seg.ekey.end);
//			// procura a melhor posicao
//			seg_inic_ble(&seg.ekey);
//		}
//		if (seg.ble_root.perm == 0) { // se foi remocao
//			seg_secret_apaga(0);
//		}
		return SEG_OK;
	case BLE_COMMIT_OWNER:
		taskENTER_CRITICAL();
		mem_esc_owner_ble(seg.ble_owner.raw);
		taskEXIT_CRITICAL();

		return SEG_OK;
	case BLE_COMMIT_EDIT:
        _seg_fgroup_load(MEM_CLASS_REMOTE, seg.ekey.end, &segData);
		if (segData.Access_code != 0) {
            //_seg_fgroup_write(MEM_CLASS_BLE, seg.ekey.end, &seg.data_aux);
            __seg_fgroup_code_write(seg.ekey.end, &seg.data_aux);
            seg_data_t testettt;
            _seg_fgroup_load(MEM_CLASS_REMOTE, seg.ekey.end, &testettt);
			//seg.ekey.end++;
			return SEG_OK;
		}
		return SEG_ERR_CAD;
	case BLE_COMMIT_SETOR:
		__seg_fgroup_setor_write(seg.ekey.end, &seg.data_aux_setor);
		return SEG_OK;

	default:
		return SEG_INVALIDA;
	}
}

seg_status_e seg_code_root(uint32_t perm, uint64_t code)
{
	if (perm == seg.ble_root.perm && code != seg.senha_hard_reset) {
		//seg.senha_root.valor = code;
		seg.ble_root.senha = code;
		seg_code_root_salva_mem();
		return SEG_OK;
	}
	return SEG_INVALIDA;
}

// armazena na flash interna
void seg_code_root_salva_mem(void)
{
	taskENTER_CRITICAL();
	mem_esc_root_ble(seg.ble_root.raw);
	taskEXIT_CRITICAL();
	//mem_esc_root_senha(seg.senha_root.valor);
	seg.f_troca_senha_root = 0; // desliga a flag pra troca de senha root
}

void __seg_fgroup_ble_load(int16_t index, seg_data_t *data){
    seg_data_t load[1];
    uint32_t idxFile, idxPos;

    //if (index < 0 && index >= SEG_QTDE_BLE){
        //return;
    //}
    //idxFile = index/10;
    //idxPos = index%10;

    uint32_t endereco = (index * MEM_DATA_SIZE)+22; //10->tamanho root e 12-> tamanho owner

    //if (idxFile < SEG_FILES_BLE){
        mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_SIZE);
        memcpy(data, &load[0], sizeof(seg_data_t));
    //}
}

void _seg_fgroup_load(mem_access_class_e class, uint16_t index, seg_data_t *data){
    switch (class) {
//        case MEM_CLASS_CODE:
//            __seg_fgroup_code_load(index, data);
//            break;
//        case MEM_CLASS_RFID:
//            __seg_fgroup_rfid_load(index, data);
//            break;
//        case MEM_CLASS_BIOM:
//            __seg_fgroup_fingerprint_load(index, data);
//            break;
        case MEM_CLASS_REMOTE:
            __seg_fgroup_ble_load(index, data);
            break;
        default:

            break;
    }
}

void __seg_fgroup_setor_load(int16_t index, seg_data_setor_t *data){
	seg_data_setor_t load[1];
    uint32_t idxFile, idxPos;


    uint32_t endereco = (index * MEM_DATA_SETOR_SIZE)+(MAX_CODE*MEM_DATA_SIZE+22);
    mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_SETOR_SIZE);
    memcpy(data, &load[0], sizeof(seg_data_setor_t));
}

// recebe a permissao de quem esta removendo e a estrutura corrigida
seg_status_e seg_edit_user(mem_access_class_ident_e class, uint32_t perm, uint8_t *ident, void* data, uint8_t *code, uint16_t *puid){
    int16_t aux,existIndex;
    uint32_t id = *((uint32_t*)(ident));

	__seg_fgroup_virtual_seek(id, &aux, NULL);
    if (aux == -1) { // se nao encontrou o uuid
        return SEG_ERR_CAD;
    }
    seg.ekey.end = aux;

    if(data != NULL)
    	seg_fill_data_user(&seg.data_aux, (data_add_user_t*)data);

    switch (class){
    case MEM_USER_TECL:
    	uint64_t senha = *((uint64_t*)(code));
    	_seg_fgroup_load(MEM_CLASS_REMOTE, seg.ekey.end, &seg.data_aux);
    	seg.data_aux.Tecl_Code = senha;

    	// check if the desired access exists
    	_seg_fgroup_seek(class,seg.data_aux.Tecl_Code,&existIndex,NULL);
    	//__seg_fgroup_code_seek(seg.data_aux.Tecl_Code, &existIndex, NULL);

    	if (existIndex != -1){
    	    return SEG_ERR_CAD;
    	}

    	return SEG_OK;
    	break;

    case MEM_USER_ROLE:
    	return SEG_OK;
    	break;

    case MEM_USER_CONTROL:
    	uint8_t Code_control = *((uint8_t*)(code));
    	_seg_fgroup_load(MEM_CLASS_REMOTE, seg.ekey.end, &seg.data_aux);
    	seg.data_aux.Control = Code_control;

    	// check if the desired access exists
    	_seg_fgroup_seek(class,seg.data_aux.Control,&existIndex,NULL);

    	if (existIndex != -1){
    	    return SEG_ERR_CAD;
    	}
    	return SEG_OK;
    	break;
    }
}

//////////////////////////////cadastro de setores////////////////////////////////
seg_status_e seg_cad_setor_from_cmd(void* data){
    int16_t newIndex;

	__seg_fgroup_setor_seek((uint32_t)SEG_IDENT_INVALID, &newIndex, NULL);
    if (newIndex == -1){
        return SEG_POS_OCUPADA;
    }


	if (seg_fill_data_setor(&seg.data_aux_setor, (data_add_setor_t*)data) == 0){
		sprintf(seg.data_aux_setor.nome, "SENHA %d", (newIndex+1));
	}

	seg.ekey.end = newIndex;

	//__seg_fgroup_setor_write(newIndex, &seg.data_aux_setor);


    return SEG_OK;
}

seg_status_e seg_ident_valida_setor(uint32_t id, seg_data_setor_t *data_setor){
	int16_t idx;
	__seg_fgroup_setor_seek(id,&idx,data_setor);
	if (idx == -1){
		return SEG_ERR_CAD;
	}
	return SEG_OK;
}

//////////////////////////////cadastro de partições////////////////////////////////
seg_status_e seg_cad_partition_from_cmd(void* data){
	int16_t newIndex;

	__seg_fgroup_partition_seek((uint32_t)SEG_IDENT_INVALID, &newIndex, NULL);
    if (newIndex == -1){
        return SEG_POS_OCUPADA;
    }
	if (seg_fill_data_partition(&seg.data_aux_partition, (data_add_partition_t*)data) == 0){
		sprintf(seg.data_aux_partition.nome, "SENHA %d", (newIndex+1));
	}
	__seg_fgroup_partition_write(newIndex, &seg.data_aux_partition);

	return SEG_OK;
}

seg_status_e seg_ident_valida_partition(uint32_t id, seg_data_partition_t *data_setor){
	int16_t idx;
	__seg_fgroup_partition_seek(id,&idx,data_setor);
	if (idx == -1){
		return SEG_ERR_CAD;
	}
	return SEG_OK;
}

seg_status_e get_configuration_environment(uint32_t id_setor, seg_data_envionment_t *data_environment){
	seg_status_e seg_status;

	seg_status = seg_ident_valida_setor(id_setor,&data_environment->seg_data_setor);

	if(seg_status == SEG_OK){
		if(data_environment->seg_data_setor.ID_partition != 0){
			seg_status = seg_ident_valida_partition(data_environment->seg_data_setor.ID_partition,&data_environment->seg_data_partition);
		}
		else{
			//setor não esta associado a nenhuma partição
			seg_status = SEG_INVALIDA;
		}
	}
	return seg_status;
}

//////////////////////////////cadastro de sensores////////////////////////////////
seg_status_e seg_cad_sensor_from_cmd(data_add_sensor_t* data){
	int16_t newIndex;
	data_add_sensor_t data_aux;

	__seg_fgroup_sensor_seek((uint32_t)SEG_IDENT_INVALID, &newIndex, NULL);
    if (newIndex == -1){
        return SEG_POS_OCUPADA;
    }
    data_aux = *((data_add_sensor_t*)data);

    seg.data_aux_sensor.ID_Sensor = data->ID_sensor;
    seg.data_aux_sensor.ID_Setor = data->ID_Setor;
    seg.data_aux_sensor.binario_flags = data->config_sensor;
    seg.data_aux_sensor.Range_type = data->Range_type;

	__seg_fgroup_sensor_write(newIndex, &seg.data_aux_sensor);

	return SEG_OK;
}

seg_status_e seg_ident_valida_sensor(uint8_t id, seg_data_sensor_t *data_setor){
	int16_t idx;
	__seg_fgroup_sensor_seek(id,&idx,data_setor);
	if (idx == -1){
		return SEG_ERR_CAD;
	}
	return SEG_OK;
}


//////////////////////////////cadastro de PGM///////////////////////////////////
seg_status_e seg_cad_pgm_from_cmd(data_add_pgm_t* data){
	int16_t newIndex;

	__seg_fgroup_pgm_seek((uint32_t)SEG_IDENT_INVALID, &newIndex, NULL);

    if (newIndex == -1){
        return SEG_POS_OCUPADA;
    }


    seg.data_aux_pgm.ID_PGM = data->ID_PGM;
    seg.data_aux_pgm.index_rele = data->index_rele;
    seg.data_aux_pgm.time = 0;
    seg.data_aux_pgm.type_association = 0;
    seg.data_aux_pgm.type_trigger = 0;

    __seg_fgroup_pgm_write(newIndex, &seg.data_aux_pgm);

	return SEG_OK;
}

//////////////////////////////cadastro de TECLADO///////////////////////////////////
seg_status_e seg_cad_teclado_from_cmd(data_add_teclado_t* data){
	int16_t newIndex;

	__seg_fgroup_teclado_seek((uint32_t)SEG_IDENT_INVALID, &newIndex, NULL);

    if (newIndex == -1){
        return SEG_POS_OCUPADA;
    }


    seg.data_aux_teclado.ID_TECLADO = data->ID_TECLADO;

    __seg_fgroup_teclado_write(newIndex, &seg.data_aux_teclado);

	return SEG_OK;
}

// recebe a permissao de quem esta removendo e a estrutura corrigida
seg_status_e seg_edit_pgm(uint8_t *ident, data_add_pgm_t* data){
    int16_t aux;
    uint8_t id = *ident;

	__seg_fgroup_pgm_seek(id, &aux, NULL);
    if (aux == -1) { // se nao encontrou o uuid
        return SEG_ERR_CAD;
    }
    seg.ekey.end = aux;

    if(data != NULL){
        seg.data_aux_pgm.ID_PGM = id;
        seg.data_aux_pgm.index_rele = data->index_rele;
        seg.data_aux_pgm.time = data->time;
        seg.data_aux_pgm.type_association = data->type_association;
        seg.data_aux_pgm.type_trigger = data->type_trigger;
        seg.data_aux_pgm.mode = data->mode;
    }

    __seg_fgroup_pgm_write(seg.ekey.end, &seg.data_aux_pgm);

    return SEG_OK;

}

seg_status_e seg_ident_valida_pgm(uint8_t id, seg_data_pgm_t *data_pgm){
	int16_t idx;
	__seg_fgroup_pgm_seek(id,&idx,data_pgm);
	if (idx == -1){
		return SEG_ERR_CAD;
	}
	return SEG_OK;
}

seg_status_e seg_ident_valida_teclado(uint8_t id, seg_data_teclado_t *data_teclado){
	int16_t idx;
	__seg_fgroup_teclado_seek(id,&idx,data_teclado);
	if (idx == -1){
		return SEG_ERR_CAD;
	}
	return SEG_OK;
}

seg_status_e seg_ident_valida_gate(uint8_t id, seg_data_gate_t *data_gate){
	int16_t idx;
	__seg_fgroup_gate_seek(id,&idx,data_gate);
	if (idx == -1){
		return SEG_ERR_CAD;
	}
	return SEG_OK;
}

seg_status_e seg_ident_valida_cenario(uint8_t id, seg_data_cenario_t *data_cenario){
	int16_t idx;
	__seg_fgroup_cenario_seek(id,&idx,data_cenario);
	if (idx == -1){
		return SEG_ERR_CAD;
	}
	return SEG_OK;
}


//////////////////////////////cadastro de GATE////////////////////////////////
seg_status_e seg_cad_gate_from_cmd(data_add_gate_t* data){
	int16_t newIndex;

	__seg_fgroup_gate_seek((uint32_t)SEG_IDENT_INVALID, &newIndex, NULL);
    if (newIndex == -1){
        return SEG_POS_OCUPADA;
    }

    seg.data_aux_gate.ID_gate = data->ID_gate;
    seg.data_aux_gate.ID_automation = data->ID_automation;
    seg.data_aux_gate.Range_type = data->Range_type;

    __seg_fgroup_gate_write(newIndex, &seg.data_aux_gate);

	return SEG_OK;
}


//////////////////////////////cadastro de CENARIO////////////////////////////////
seg_status_e seg_cad_cenario_from_cmd(data_add_cenario_t* data){
	int16_t newIndex;

	__seg_fgroup_cenario_seek((uint32_t)SEG_IDENT_INVALID, &newIndex, NULL);
    if (newIndex == -1){
        return SEG_POS_OCUPADA;
    }

    seg.data_aux_cenario.ID_cenario = data->ID_cenario;
    seg.data_aux_cenario.qtd_acoes = data->qtd_acoes;

    seg.data_aux_cenario.trigger_type = data->trigger_type;
	seg.data_aux_cenario.trigger_ID_trigger_action = data->trigger_ID_trigger_action;
	seg.data_aux_cenario.trigger_mode = data->trigger_mode;
	seg.data_aux_cenario.trigger_event_command = data->trigger_event_command;
	seg.data_aux_cenario.trigger_timeout = data->trigger_timeout;

	memcpy(seg.data_aux_cenario.acoes, data->acoes, sizeof(seg.data_aux_cenario.acoes)*seg.data_aux_cenario.qtd_acoes);


	__seg_fgroup_cenario_write(newIndex, &seg.data_aux_cenario);

	return SEG_OK;
}



///////////////////////////////////////sincronismo controle////////////////////////
type_user_t get_sync_user_teclado(uint16_t index, seg_data_t *data){
	type_user_t type_user;
	__seg_fgroup_ble_load(index, data);
	if(data->Access_code == 0){
		type_user = USER_END;
	}
	else{
		if(data->resource == 1){
			type_user = USER_TECLADO;
		}
		else{
			type_user = USER_APP;
		}
	}
	return type_user;
}



seg_status_e clean_sync_user_teclado(uint16_t index){
	_seg_fgroup_load(MEM_CLASS_REMOTE, index, &seg.data_aux);
	seg.data_aux.resource = 0;

    __seg_fgroup_code_write(index, &seg.data_aux);
    seg_data_t testettt;
    _seg_fgroup_load(MEM_CLASS_REMOTE, seg.ekey.end, &testettt);

    return SEG_OK;
}

seg_status_e get_name_user(uint16_t index, uint8_t *nome){
	_seg_fgroup_load(MEM_CLASS_REMOTE, index, &seg.data_aux);
	if(seg.data_aux.Access_code != 0){
		memcpy(nome, seg.data_aux.nome, 16);
		return SEG_OK;
	}
	else{
		return SEG_POS_OCUPADA;
	}
}

seg_status_e set_name_user(uint16_t index, uint8_t *nome){
	_seg_fgroup_load(MEM_CLASS_REMOTE, index, &seg.data_aux);

	memcpy(&seg.data_aux.nome, nome, 16);

	if(seg.data_aux.Access_code != 0){
		__seg_fgroup_code_write(index, &seg.data_aux);

	    seg_data_t testettt;
	    _seg_fgroup_load(MEM_CLASS_REMOTE, index, &testettt);
		return SEG_OK;
	}
	else{
		return SEG_POS_OCUPADA;
	}
}

uint32_t get_perm_user(uint32_t senha_teclado){
	int16_t idx;
	seg_data_t segData;

	_seg_fgroup_seek(MEM_USER_TECL,senha_teclado,&idx,&segData);

	if(idx != -1){
		return segData.Access_code;
	}
	else{
		return 0;
	}
}

seg_status_e get_partition(uint16_t index, seg_data_partition_t *data){

    uint16_t start_end;
    seg_data_partition_t load[1] = {0};

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+22;

    uint32_t endereco = (index * MEM_DATA_PARTITION_SIZE)+ start_end;

    mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_PARTITION_SIZE);

    memcpy(data, &load[0], sizeof(seg_data_partition_t));

    return SEG_OK;

}


seg_status_e get_cenario(uint16_t index, seg_data_cenario_t *data){

    uint16_t start_end;
    seg_data_cenario_t load[1] = {0};

    start_end = (MAX_CODE*MEM_DATA_SIZE)+(MAX_SETOR * MEM_DATA_SETOR_SIZE)+(MAX_PARTITION * MEM_DATA_PARTITION_SIZE)+(MAX_SENSOR * MEM_DATA_SENSOR_SIZE)+(MAX_PGM * MEM_DATA_PGM_SIZE)+(MAX_TECLADO * MEM_DATA_TECLADO_SIZE)+(MAX_GATE * MEM_DATA_GATE_SIZE)+22;

    uint32_t endereco = (index * MEM_DATA_CENARIO_SIZE)+ start_end;

    mem_le_access_file(endereco, (uint32_t*)load,MEM_DATA_CENARIO_SIZE);

    memcpy(data, &load[0], sizeof(seg_data_cenario_t));

    return SEG_OK;

}
