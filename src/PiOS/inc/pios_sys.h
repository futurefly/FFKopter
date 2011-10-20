/*
 * pios_sys.h
 *
 *  Created on: 20.10.2011
 *      Author: gandhi
 */

#ifndef PIOS_SYS_H_
#define PIOS_SYS_H_

/* Public Functions */
extern void PIOS_SYS_Init(void);
extern int32_t PIOS_SYS_Reset(void);
extern uint32_t PIOS_SYS_getCPUFlashSize(void);
extern int32_t PIOS_SYS_SerialNumberGetBinary(uint8_t *array);
extern int32_t PIOS_SYS_SerialNumberGet(char *str);

#endif /* PISO_SYS_H */


#endif /* PIOS_SYS_H_ */
