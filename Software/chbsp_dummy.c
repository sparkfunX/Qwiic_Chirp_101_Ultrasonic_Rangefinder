/*!
 * \file chbsp_dummy.c
 *
 * \brief Dummy implementations of optional board support package IO functions allowing 
 * platforms to selectively support only needed functionality.  These are placeholder
 * routines that will satisfy references from other code to avoid link errors, but they 
 * do not peform any actual operations.
 *
 * See chirp_bsp.h for descriptions of all board support package interfaces, including 
 * details on these optional functions.
 */

/*
 * Copyright © 2017-2019 Chirp Microsystems.  All rights reserved.
 */

#include "chirp_bsp.h"



/* Functions supporting debugging */

__attribute__((weak)) void chbsp_debug_toggle(uint8_t dbg_pin_num) {}

__attribute__((weak)) void chbsp_debug_on(uint8_t dbg_pin_num) {}

__attribute__((weak)) void chbsp_debug_off(uint8_t dbg_pin_num) {}

__attribute__((weak)) void chbsp_print_str(char *str) {
	(void)(str);
}

__attribute__((weak)) uint32_t chbsp_timestamp_ms() {
	return 0;
}

__attribute__((weak)) int chbsp_i2c_deinit(void){
	return 0;
}


/* Functions supporting interrupt-based operation */

__attribute__((weak)) void chbsp_group_io_interrupt_enable(ch_group_t *grp_ptr) {
	(void)(grp_ptr);
}

__attribute__((weak)) void chbsp_io_interrupt_enable(ch_dev_t *dev_ptr) {
	(void)(dev_ptr);
}

__attribute__((weak)) void chbsp_group_io_interrupt_disable(ch_group_t *grp_ptr) {
	(void)(grp_ptr);
}

__attribute__((weak)) void chbsp_io_interrupt_disable(ch_dev_t *dev_ptr) {
	(void)(dev_ptr);
}


/* Functions supporting non-blocking operation */

__attribute__((weak)) int chbsp_i2c_write_nb(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes) {
	(void)(dev_ptr);
	(void)(data);
	(void)(num_bytes);
	return 1;
}

__attribute__((weak)) 
	int chbsp_i2c_mem_write_nb(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes) {
	(void)(dev_ptr);
	(void)(mem_addr);
	(void)(data);
	(void)(num_bytes);
	return 1;
}

__attribute__((weak)) int chbsp_i2c_read_nb(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes) {
	(void)(dev_ptr);
	(void)(data);
	(void)(num_bytes);
	return 1;
}

__attribute__((weak)) int chbsp_i2c_mem_read_nb(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes) {
	(void)(dev_ptr);
	(void)(mem_addr);
	(void)(data);
	(void)(num_bytes);
	return 1;
}


/* Functions supporting controlling int pins of individual sensors (originally only controllable in a group) */

__attribute__((weak)) void chbsp_set_io_dir_out(ch_dev_t *dev_ptr) {
	(void)(dev_ptr);
}

__attribute__((weak)) void chbsp_set_io_dir_in(ch_dev_t *dev_ptr) {
	(void)(dev_ptr);
}

__attribute__((weak)) void chbsp_io_clear(ch_dev_t *dev_ptr) {
	(void)(dev_ptr);
}

__attribute__((weak)) void chbsp_io_set(ch_dev_t *dev_ptr) {
	(void)(dev_ptr);
}

__attribute__((weak)) void chbsp_external_i2c_irq_handler(chdrv_i2c_transaction_t *trans){
	(void)(trans);
}


