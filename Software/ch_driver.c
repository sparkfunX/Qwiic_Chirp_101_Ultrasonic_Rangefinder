/*! \file ch_driver.c
 \brief Internal driver functions for operation with the Chirp ultrasonic sensor.

 The user should not need to edit this file. This file relies on hardware interface
 functions declared in ch_bsp.h. If switching to a different hardware platform, the functions 
 declared in ch_bsp.h must be provided by the user.
 */

/*
 Copyright © 2016-2019, Chirp Microsystems.  All rights reserved.

 Chirp Microsystems CONFIDENTIAL

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL CHIRP MICROSYSTEMS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 You can contact the authors of this program by email at support@chirpmicro.com
 or by mail at 2560 Ninth Street, Suite 220A, Berkeley, CA 94710.
 */

#include "soniclib.h"
#include "chirp_bsp.h"
#include "ch_driver.h"



/*!
 * \brief Write bytes to a sensor device in programming mode.
 *
 * \param dev_ptr 	pointer to the ch_dev_t config structure for a sensor
 * \param message 	pointer to a buffer containing the bytes to write
 * \param len 		number of bytes to write
 *
 * \return 0 if successful, non-zero otherwise
 *
 * This function writes bytes to the device using the programming I2C address.  The
 * PROG line for the device must have been asserted before this function is called.
 */
int chdrv_prog_i2c_write(ch_dev_t *dev_ptr, uint8_t *message, uint16_t len) {

	dev_ptr->i2c_address = CH_I2C_ADDR_PROG;
	int ch_err = chbsp_i2c_write(dev_ptr, message, len);
	dev_ptr->i2c_address = dev_ptr->app_i2c_address;

	return ch_err;
}

/*!
 * \brief Read bytes from a sensor device in programming mode.
 *
 * \param dev_ptr 	pointer to the ch_dev_t config structure for a sensor
 * \param message 	pointer to a buffer where read bytes will be placed
 * \param len 		number of bytes to read
 *
 * \return 0 if successful, non-zero otherwise
 *
 * This function reads bytes from the device using the programming I2C address.  The
 * PROG line for the device must have been asserted before this function is called.
 */
int chdrv_prog_i2c_read(ch_dev_t *dev_ptr, uint8_t *message, uint16_t len) {

	dev_ptr->i2c_address = CH_I2C_ADDR_PROG;
	int ch_err = chbsp_i2c_read(dev_ptr, message, len);
	dev_ptr->i2c_address = dev_ptr->app_i2c_address;

	return ch_err;
}

/*!
 * \brief Read bytes from a sensor device in programming mode, non-blocking.
 *
 * \param dev_ptr 	pointer to the ch_dev_t config structure for a sensor
 * \param message 	pointer to a buffer where read bytes will be placed
 * \param len 		number of bytes to read
 *
 * \return 0 if successful, non-zero otherwise
 *
 * This function temporarily changes the device I2C address to the low-level programming
 * interface, and issues a non-blocking read request. The PROG line for the device must have 
 * been asserted before this function is called.
 */
int chdrv_prog_i2c_read_nb(ch_dev_t *dev_ptr, uint8_t *message, uint16_t len) {

	dev_ptr->i2c_address = CH_I2C_ADDR_PROG;
	int ch_err = chbsp_i2c_read_nb(dev_ptr, message, len);
	dev_ptr->i2c_address = dev_ptr->app_i2c_address;

	return ch_err;
}


/*!
 * \brief Write byte to a sensor application register.
 *
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor
 * \param mem_addr 		sensor memory/register address
 * \param data_value	data value to transmit
 *
 * \return 0 if successful, non-zero otherwise
 */
int chdrv_write_byte(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t data_value) {
	uint8_t message[] = { sizeof(data_value), data_value };		// insert byte count (1) at start of data

	int ch_err = chbsp_i2c_mem_write(dev_ptr, mem_addr, message, sizeof(message));

	return ch_err;
}

/*!
 * \brief Write multiple bytes to a sensor application register location.
 *
 * \param dev_ptr 	pointer to the ch_dev_t config structure for a sensor
 * \param mem_addr 	sensor memory/register address
 * \param data 		pointer to transmit buffer containing data to send
 * \param len 		number of bytes to write
 *
 * \return 0 if successful, non-zero otherwise
 */
int chdrv_burst_write(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint8_t len){

	uint8_t message[CHDRV_I2C_MAX_WRITE_BYTES + 1];
	message[0] = (uint8_t) mem_addr;
	message[1] = len;
	memcpy(&(message[2]), data, len);

	int ch_err = chbsp_i2c_write(dev_ptr, message, len+2);

	return ch_err;
}

/*!
 * \brief Write 16 bits to a sensor application register.
 *
 * \param dev_ptr 	pointer to the ch_dev_t config structure for a sensor
 * \param reg_addr 	sensor register address
 * \param data 		data value to transmit
 *
 * \return 0 if successful, non-zero otherwise
 */
int chdrv_write_word(ch_dev_t *dev_ptr, uint16_t mem_addr, uint16_t data_value) {
	// First we write the register address, then the number of bytes we're writing

	// Place byte count (2) in first byte of message
	// Sensor is little-endian, so LSB goes in at the lower address
	uint8_t message[] = { sizeof(data_value), (uint8_t) data_value, (uint8_t) (data_value >> 8) }; 

	int ch_err = chbsp_i2c_mem_write(dev_ptr, mem_addr, message, sizeof(message));

	return ch_err;
}

/*!
 * \brief Read byte from a sensor application register.
 *
 * \param dev_ptr 	pointer to the ch_dev_t config structure for a sensor
 * \param mem_addr 	sensor memory/register address
 * \param data 		pointer to receive buffer
 *
 * \return 0 if successful, non-zero otherwise
 */
int chdrv_read_byte(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data) {

	return (chbsp_i2c_mem_read(dev_ptr, mem_addr, data, 1));
}

/*!
 * \brief Read multiple bytes from a sensor application register location.
 *
 * \param dev_ptr 	pointer to the ch_dev_t config structure for a sensor
 * \param mem_addr 	sensor memory/register address
 * \param data 		pointer to receive buffer
 * \param len 		number of bytes to read
 *
 * \return 0 if successful, non-zero otherwise
 */
int chdrv_burst_read(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes) {

	return (chbsp_i2c_mem_read(dev_ptr, mem_addr, data, num_bytes));
}

/*!
 * \brief Read 16 bits from a sensor application register.
 *
 * \param dev_ptr 	pointer to the ch_dev_t config structure for a sensor
 * \param mem_addr 	sensor memory/register address
 * \param data 		pointer to receive buffer
 *
 * \return 0 if successful, non-zero otherwise
 */
int chdrv_read_word(ch_dev_t *dev_ptr, uint16_t mem_addr, uint16_t * data) {
	//
	return (chbsp_i2c_mem_read(dev_ptr, mem_addr, (uint8_t *) data, 2));
}

/*!
 * \brief  Calibrate the sensor real-time clock against the host microcontroller clock.
 * 
 * \param grp_ptr pointer to the ch_group_t config structure for a group of sensors
 * 
 * This function sends a pulse (timed by the host MCU) on the INT line to each sensor device 
 * in the group, then reads back the counts of sensor RTC cycles that elapsed during that pulse 
 * on each individual device.  The result is stored in the ch_dev_t config structure for each 
 * device and is subsequently used during range calculations.
 *
 * The length of the pulse is \a dev_ptr->rtc_cal_pulse_ms milliseconds (typically 100).  This 
 * value is set during \a ch_init().
 *
 * \note The calibration pulse is sent to all devices in the group at the same time.  Therefore 
 * all connected devices will see the same reference pulse length.
 * 
 */
void chdrv_group_measure_rtc(ch_group_t *grp_ptr) {
	uint8_t i;
	const uint32_t pulselength = grp_ptr->rtc_cal_pulse_ms;

	/* Configure the host's side of the IO pin as a low output */
	chbsp_group_io_clear(grp_ptr);
	chbsp_group_set_io_dir_out(grp_ptr);

	/* Set up RTC calibration */
	for (i = 0; i < grp_ptr->num_ports; i++) {
		if (grp_ptr->device[i]->sensor_connected) {
			grp_ptr->device[i]->prepare_pulse_timer(grp_ptr->device[i]);
		}
	}

	/* Trigger a pulse on the IO pin */
	chbsp_group_io_set(grp_ptr);
	chbsp_delay_ms(pulselength);
	chbsp_group_io_clear(grp_ptr);

	chbsp_group_set_io_dir_in(grp_ptr);

	chbsp_delay_ms(1);

	for (i = 0; i < grp_ptr->num_ports; i++) {
		if (grp_ptr->device[i]->sensor_connected) {
			grp_ptr->device[i]->store_pt_result(grp_ptr->device[i]);
			grp_ptr->device[i]->store_op_freq(grp_ptr->device[i]);
			grp_ptr->device[i]->store_bandwidth(grp_ptr->device[i]);
			grp_ptr->device[i]->store_scalefactor(grp_ptr->device[i]);
		}
	}
}

/*!
 * \brief Convert register values to a range using the calibration data.
 *
 * \param dev_ptr 	pointer to the ch_dev_t config structure for a sensor
 * \param tof 		value of sensor TOF register
 * \param tof_sf 	value of sensor TOF_SF register
 *
 * \return range in millimeters, or \a CH_NO_TARGET (0xFFFFFFFF) if no object is detected.  
 *
 * The range result format is fixed point with 5 binary fractional digits (divide by 32 to 
 * convert to mm).
 *
 * This function takes the time-of-flight and scale factor values from the sensor,
 * and computes the actual one-way range based on the formulas given in the sensor 
 * datasheet.
 */
uint32_t chdrv_one_way_range(ch_dev_t *dev_ptr, uint16_t tof, uint16_t tof_sf) {
	uint32_t num = (CH_SPEEDOFSOUND_MPS * dev_ptr->group->rtc_cal_pulse_ms * (uint32_t) tof);
	uint32_t den = ((uint32_t) dev_ptr->rtc_cal_result * (uint32_t) tof_sf) >> 10;
	uint32_t range = (num / den);

	if (tof == UINT16_MAX) {
		return CH_NO_TARGET;
	}

	if (dev_ptr->part_number == 201) {
		range *= 2;				// CH-201 range (TOF) encoding is 1/2 of CH-101 value
	}

#ifdef CHDRV_DEBUG
	char cbuf[80];

	snprintf(cbuf, sizeof(cbuf), "%u:%u: timeOfFlight=%u, scaleFactor=%u, num=%lu, den=%lu, range=%lu\n", 
				dev_ptr->i2c_bus_index, dev_ptr->i2c_address, tof, tof_sf, num, den, range);
	chbsp_print_str(cbuf);
#endif

	return range;
}

/*!
 * \brief Add an I2C transaction to the non-blocking queue
 *
 * \param grp_ptr 	pointer to the ch_group_t config structure for a group of sensors
 * \param dev_ptr 	pointer to an individual ch_dev_t config structure for a sensor
 * \param rd_wrb 	read/write indicator: 0 if write operation, 1 if read operation
 * \param type		type of I2C transaction (standard, program interface, or external)
 * \param addr		I2C address
 * \param nbytes 	number of bytes to read/write
 * \param data 		pointer to buffer to receive data or containing data to send
 *
 * \return 0 if successful, non-zero otherwise
 */
int chdrv_group_i2c_queue(ch_group_t *grp_ptr, ch_dev_t *dev_ptr, uint8_t rd_wrb, uint8_t type, 
						  uint16_t addr, uint16_t nbytes, uint8_t *data) {

	uint8_t bus_num = ch_get_i2c_bus(dev_ptr);
	int	ret_val;

	chdrv_i2c_queue_t *q = &(grp_ptr->i2c_queue[bus_num]);
	chdrv_i2c_transaction_t *t = &(q->transaction[q->len]);

	if (q->len < CHDRV_MAX_I2C_QUEUE_LENGTH) {
		t->databuf = data;
		t->dev_ptr = dev_ptr;
		t->addr = addr;
		t->nbytes = nbytes;
		t->rd_wrb = rd_wrb;
		t->type = type;
		t->xfer_num = 0;
		q->len++;
		ret_val = 0;
	} else {
		ret_val = 1;
	}

	return ret_val;
}


/*!
 * \brief Start a non-blocking sensor readout
 *
 * \param grp_ptr 	pointer to the ch_group_t config structure for a group of sensors
 *
 * This function starts a non-blocking I/O operation on the specified group of sensors.
 */
void chdrv_group_i2c_start_nb(ch_group_t *grp_ptr) {

	for (int bus_num = 0; bus_num < grp_ptr->num_i2c_buses; bus_num++) {
		chdrv_i2c_queue_t *q = &(grp_ptr->i2c_queue[bus_num]);

		if ((q->len != 0) && !(q->running)) {
			chdrv_group_i2c_irq_handler(grp_ptr, bus_num);
		}
	}
}

/*!
 * \brief Continue a non-blocking readout
 *
 * \param grp_ptr 				pointer to the ch_group_t config structure for a group of sensors
 * \param i2c_bus_index		index value identifying I2C bus within group
 *
 * Call this function once from your I2C interrupt handler each time it executes.
 * It will call the user's callback routine (grp_ptr->io_complete_callback) when all transactions 
 * are complete.
 */
void chdrv_group_i2c_irq_handler(ch_group_t *grp_ptr, uint8_t i2c_bus_index) {
	int i;
	int transactions_pending;
	chdrv_i2c_queue_t *q = &(grp_ptr->i2c_queue[i2c_bus_index]);
	chdrv_i2c_transaction_t *t = &(q->transaction[q->idx]);
	ch_dev_t *dev_ptr = q->transaction[q->idx].dev_ptr;


    chbsp_program_disable(dev_ptr);					// de-assert PROG pin, possibly only briefly

    if (q->idx < q->len) {
    	dev_ptr = q->transaction[q->idx].dev_ptr;
		q->running = 1;

		if (t->type == CHDRV_NB_TRANS_TYPE_EXTERNAL) {
			/* Externally-requested transfer */

			(q->idx)++;
			chbsp_external_i2c_irq_handler(t);
    		t->xfer_num++;			// count this transfer

		} else if (t->type == CHDRV_NB_TRANS_TYPE_PROG) {
			/* Programming interface transfer */

			/* programming interface has max transfer size - check if still more to do during this transaction */
			uint8_t total_xfers = (t->nbytes + (CH_PROG_XFER_SIZE - 1)) / CH_PROG_XFER_SIZE;


			if (t->xfer_num < total_xfers) {
				/* still need to complete this transaction */

				uint16_t bytes_left;
				uint16_t xfer_bytes;

				bytes_left = (t->nbytes - (t->xfer_num * CH_PROG_XFER_SIZE));

				if (t->rd_wrb) {			// only read operations supported for now

					if (grp_ptr->i2c_drv_flags & I2C_DRV_FLAG_RESET_AFTER_NB) {
			    		chbsp_i2c_reset(dev_ptr);					// reset I2C bus if BSP says it's needed */
					}

					chbsp_program_enable(dev_ptr);					// assert PROG pin

	        		uint8_t message[] = { (0x80 | CH_PROG_REG_CTL), 0x09 };      // read burst command

	        		if (bytes_left > CH_PROG_XFER_SIZE) {
	                		xfer_bytes = CH_PROG_XFER_SIZE;
	        		} else {
	                		xfer_bytes = bytes_left;
	        		}
	        		chdrv_prog_write(dev_ptr, CH_PROG_REG_ADDR, (t->addr + (t->xfer_num * CH_PROG_XFER_SIZE)));
	        		chdrv_prog_write(dev_ptr, CH_PROG_REG_CNT, (xfer_bytes - 1));
	        		(void) chdrv_prog_i2c_write(dev_ptr, message, sizeof(message));
	        		(void) chdrv_prog_i2c_read_nb(dev_ptr, (t->databuf + (t->xfer_num * CH_PROG_XFER_SIZE)), xfer_bytes);
				}

	    		t->xfer_num++;			// count this transfer

	    		/* if this is the last transfer in this transaction, advance queue index */
	    		if (t->xfer_num >= total_xfers) {
					(q->idx)++;
	    		}

			}

		} else {
			/* Standard transfer */
			if (t->rd_wrb) {
				(q->idx)++;
				chbsp_i2c_mem_read_nb(t->dev_ptr, t->addr, t->databuf, t->nbytes);

			} else {
				(q->idx)++;
				chbsp_i2c_mem_write_nb(t->dev_ptr, t->addr, t->databuf, t->nbytes);
			}
    		t->xfer_num++;			// count this transfer
		}
		transactions_pending = 1;
	} else {

		if (q->idx >= 1) {
			dev_ptr = q->transaction[(q->idx - 1)].dev_ptr;		// get dev_ptr for previous completed transaction
			if (dev_ptr != NULL) {
			    chbsp_program_disable(dev_ptr);					// de-assert PROG pin for completed transaction

					if (grp_ptr->i2c_drv_flags & I2C_DRV_FLAG_RESET_AFTER_NB) {
			    		chbsp_i2c_reset(dev_ptr);				// reset I2C bus if BSP requires it
					}
			}
		}

		q->len = 0;
		q->idx = 0;
		q->running = 0;
		transactions_pending = 0;

		for (i = 0; i < grp_ptr->num_i2c_buses; i++) {
			if (grp_ptr->i2c_queue[i].len) {
				transactions_pending = 1;
				break;
			}
		}
	}

	if (!transactions_pending) {
		ch_io_complete_callback_t func_ptr = grp_ptr->io_complete_callback;

		if (func_ptr != NULL) {
			(*func_ptr)(grp_ptr);
		}
	}

}


/*!
 * \brief Wait for an individual sensor to finish start-up procedure.
 * 
 * \param dev_ptr 			pointer to the ch_dev_t config structure for a sensor
 * \param timeout_ms 	number of milliseconds to wait for sensor to finish start-up before returning failure
 *
 * \return 0 if startup sequence finished, non-zero if startup sequence timed out or sensor is not connected
 *
 * After the sensor is programmed, it executes an internal start-up and self-test sequence. This function waits the
 * specified time in milliseconds for the sensor to finish this sequence.
 */
int chdrv_wait_for_lock(ch_dev_t *dev_ptr, uint16_t timeout_ms) {
	uint32_t start_time = chbsp_timestamp_ms();
	int ch_err = !(dev_ptr->sensor_connected);

	while (!ch_err && !(dev_ptr->get_locked_state(dev_ptr))) {
		chbsp_delay_ms(10);
		ch_err = ((chbsp_timestamp_ms() - start_time) > timeout_ms);
	}

#ifdef CHDRV_DEBUG
	if (ch_err) {
		char cbuf[80];
		snprintf(cbuf, sizeof(cbuf), "Sensor %hhu initialization timed out or missing\n", dev_ptr->io_index);
		chbsp_print_str(cbuf);
	}
#endif

	return ch_err;
}

/*!
 * \brief Wait for all sensors to finish start-up procedure.
 *
 * \param grp_ptr 		pointer to the ch_group_t config structure for a group of sensors
 *
 * \return 0 if startup sequence finished on all detected sensors, non-zero if startup sequence 
 * timed out on any sensor(s).
 *
 * After each sensor is programmed, it executes an internal start-up and self-test sequence. This 
 * function waits for all sensor devices to finish this sequence.  For each device, the maximum 
 * time to wait is \a CHDRV_FREQLOCK_TIMEOUT_MS milliseconds.
 */
int chdrv_group_wait_for_lock(ch_group_t *grp_ptr) {
	int ch_err = 0;

	for (uint8_t i = 0; i < grp_ptr->num_ports; i++) {
		ch_dev_t *dev_ptr = grp_ptr->device[i];

		if (dev_ptr->sensor_connected) {
			ch_err |= chdrv_wait_for_lock(dev_ptr, CHDRV_FREQLOCK_TIMEOUT_MS);
		}
	}
	return ch_err;
}

/*!
 * \brief Start a measurement in hardware triggered mode.
 * 
 * \param grp_ptr 		pointer to the ch_group_t config structure for a group of sensors
 *
 * \return 0 if success, non-zero if \a grp_ptr pointer is invalid
 *
 * This function starts a triggered measurement on each sensor in a group, by briefly asserting the INT line to each device.  
 * Each sensor must have already been placed in hardware triggered mode before this function is called.
 */
int chdrv_group_hw_trigger(ch_group_t *grp_ptr) {
	int ch_err = !grp_ptr;

	if (!ch_err) {
		//Disable pin interrupt before triggering pulse
		chbsp_group_io_interrupt_disable(grp_ptr);

		// Generate pulse
		chbsp_group_set_io_dir_out(grp_ptr);
		chbsp_group_io_set(grp_ptr);
		chbsp_delay_us(5); 					// Pulse needs to be a minimum of 800ns long
		chbsp_group_io_clear(grp_ptr);
		chbsp_group_set_io_dir_in(grp_ptr);

		// Delay a bit before re-enabling pin interrupt to avoid possibly triggering on falling-edge noise
		chbsp_delay_us(10);

		chbsp_group_io_interrupt_enable(grp_ptr);
	}
	return ch_err;
}

/*!
 * \brief Start a measurement in hardware triggered mode on one sensor.
 *
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor
 *
 * \return 0 if success, non-zero if \a dev_ptr pointer is invalid
 *
 * This function starts a triggered measurement on a single sensor, by briefly asserting the INT line to the device.
 * The sensor must have already been placed in hardware triggered mode before this function is called.
 *
 * \note This function requires implementing the optional BSP functions to control the INT pin direction
 *       and level for individual sensors (\a chbsp_set_io_dir_in(), \a chbsp_set_io_dir_out(), \a chbsp_io_set(), and
 *       \a chbsp_io_clear()).
 */
int chdrv_hw_trigger(ch_dev_t *dev_ptr) {
	int ch_err = !dev_ptr;

	if (!ch_err) {
		//Disable pin interrupt before triggering pulse
		chbsp_io_interrupt_disable(dev_ptr);

		// Generate pulse
		chbsp_set_io_dir_out(dev_ptr);
		chbsp_io_set(dev_ptr);
		chbsp_delay_us(5); 					// Pulse needs to be a minimum of 800ns long  // XXX need define
		chbsp_io_clear(dev_ptr);
		chbsp_set_io_dir_in(dev_ptr);

		// Delay a bit before re-enabling pin interrupt to avoid possibly triggering on falling-edge noise
		chbsp_delay_us(10);			// XXX need define

		chbsp_io_interrupt_enable(dev_ptr);
	}
	return ch_err;
}

/*!
 * \brief Write to a sensor programming register.
 * 
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor
 * \param reg_addr 	sensor programming register address.
 * \param data 		8-bit or 16-bit data to transmit.
 *
 * \return 0 if write to sensor succeeded, non-zero otherwise
 *
 * This local function writes a value to a sensor programming register.
 */
int chdrv_prog_write(ch_dev_t *dev_ptr, uint8_t reg_addr, uint16_t data) {
	/* Set register address write bit */
	reg_addr |= 0x80;
	
	/* Write the register address, followed by the value to be written */
	uint8_t message[] = { reg_addr, (uint8_t) data, (uint8_t) (data >> 8) };

	/* For the 2-byte registers, we also need to also write MSB after the LSB */
	return chdrv_prog_i2c_write(dev_ptr, message, (1 + CH_PROG_SIZEOF(reg_addr)));
}

/*!
 * \brief Write to sensor memory.
 * 
 * \param dev_ptr	pointer to the ch_dev_t config structure for a sensor
 * \param addr		sensor programming register start address
 * \param message	pointer to data to transmit
 * \param nbytes	number of bytes to write
 *
 * \return 0 if write to sensor succeeded, non-zero otherwise
 *
 * This function writes to sensor memory using the low-level programming interface.  The type 
 * of write is automatically determined based on data length and target address alignment.
 */
int chdrv_prog_mem_write(ch_dev_t *dev_ptr, uint16_t addr, uint8_t *message, uint16_t nbytes) {
	int ch_err = (nbytes == 0);

	if (!ch_err) {
		ch_err = chdrv_prog_write(dev_ptr, CH_PROG_REG_ADDR, addr);
	}

	if (nbytes == 1 || (nbytes == 2 && !(addr & 1))) {
		uint16_t data = *((uint16_t*) message);
		if (!ch_err) {
			ch_err = chdrv_prog_write(dev_ptr, CH_PROG_REG_DATA, data);
		}
		if (!ch_err) {
			uint8_t opcode = (0x03 | ((nbytes == 1) ? 0x08 : 0x00));			// XXX need define

			ch_err = chdrv_prog_write(dev_ptr, CH_PROG_REG_CTL, opcode);
		}
	} else {
		static const uint8_t burst_hdr[2] = {0xC4, 0x0B};			// XXX need define

		if (!ch_err) {
			ch_err = chdrv_prog_write(dev_ptr, CH_PROG_REG_CNT, (nbytes - 1));
		}
		if (!ch_err) {
			ch_err = chdrv_prog_i2c_write(dev_ptr, (uint8_t *) burst_hdr, sizeof(burst_hdr));
		}
		if (!ch_err) {
			ch_err = chdrv_prog_i2c_write(dev_ptr, message, nbytes);
		}
	}
	return ch_err;
}

/*!
 * \brief Read from a sensor programming register.
 * 
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor
 * \param reg_addr 	sensor programming register address
 * \param data	 	pointer to a buffer where read bytes will be placed
 *
 * \return 0 if read from sensor succeeded, non-zero otherwise
 *
 * This local function reads a value from a sensor programming register.
 */
static int chdrv_prog_read(ch_dev_t *dev_ptr, uint8_t reg_addr, uint16_t *data) {
	uint8_t nbytes = CH_PROG_SIZEOF(reg_addr);

	uint8_t read_data[2];
	uint8_t message[1] = { 0x7F & reg_addr };

	int ch_err = chdrv_prog_i2c_write(dev_ptr, message, sizeof(message));

	if (!ch_err) {
		ch_err = chdrv_prog_i2c_read(dev_ptr, read_data, nbytes);
	}

	if (!ch_err) {
		*data = read_data[0];
		if (nbytes > 1) {
			*data |= (((uint16_t) read_data[1]) << 8);
		}
	}

	return ch_err;
}
/*!
 * \brief Transfer firmware to the sensor on-chip memory.
 * 
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor
 *
 * \return 0 if firmware write succeeded, non-zero otherwise
 *
 * This local function writes the sensor firmware image to the device.
 */
/*!
 */
static int chdrv_write_firmware(ch_dev_t *dev_ptr) {
	ch_fw_load_func_t func_ptr = dev_ptr->api_funcs.fw_load;	// pointer to firmware load function
	int ch_err = ((func_ptr == NULL) || (!dev_ptr->sensor_connected));

#ifdef CHDRV_DEBUG
	char cbuf[80];
	uint32_t prog_time;

	if (!ch_err) {
		chbsp_print_str("chdrv_write_firmware\n");
	}
#endif

	if (!ch_err) {
#ifdef CHDRV_DEBUG
		chbsp_print_str("Programming Chirp sensor...\n");
		prog_time = chbsp_timestamp_ms();
#endif
		if (func_ptr != NULL) {
			ch_err = (*func_ptr)(dev_ptr);
		} else {
			ch_err = 1;			// indicate error
		}
}

#ifdef CHDRV_DEBUG
	if (!ch_err) {
		prog_time = chbsp_timestamp_ms() - prog_time;
		snprintf(cbuf, sizeof(cbuf), "Wrote %u bytes in %lu ms.\n", CH101_FW_SIZE, prog_time);
		chbsp_print_str(cbuf);
	}
#endif

	return ch_err;
}

/*!
 * \brief Initialize sensor memory contents.
 * 
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor
 *
 * \return 0 if memory write succeeded, non-zero otherwise
 *
 * This local function initializes memory locations in the Chirp sensor, as required by the firmware image.
 */
static int chdrv_init_ram(ch_dev_t *dev_ptr) {
	int ch_err = ! dev_ptr || ! dev_ptr->sensor_connected;

#ifdef CHDRV_DEBUG
	char cbuf[80];
	uint32_t prog_time;
	if (!ch_err) {
		chbsp_print_str("chdrv_init_ram\n");
	}
#endif

	if ((!ch_err) && (dev_ptr->get_fw_ram_init_size() != 0)) {		// if size is not zero, ram init data exists
		uint16_t ram_address;
		uint16_t ram_bytecount;

		ram_address = dev_ptr->get_fw_ram_init_addr();
		ram_bytecount = dev_ptr->get_fw_ram_init_size();

		if (!ch_err) {
#ifdef CHDRV_DEBUG
			chbsp_print_str("Loading RAM init data...\n");
			prog_time = chbsp_timestamp_ms();
#endif
			ch_err = chdrv_prog_mem_write(dev_ptr, ram_address, (uint8_t *) dev_ptr->ram_init, ram_bytecount);
#ifdef CHDRV_DEBUG
			if (!ch_err) {
				prog_time = chbsp_timestamp_ms() - prog_time;
				snprintf(cbuf, sizeof(cbuf), "Wrote %u bytes in %lu ms.\n", sizeof(message), prog_time);
				chbsp_print_str(cbuf);
			}
#endif
		}
	}
	return ch_err;
}

/*!
 * \brief Reset and halt a sensor
 * 
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor
 * 
 * \return 0 if write to sensor succeeded, non-zero otherwise
 *
 * This function resets and halts a sensor device by writing to the control registers.
 *
 * In order for the device to respond, the PROG pin for the device must be asserted before this function is called. 
 */
static int chdrv_reset_and_halt(ch_dev_t * dev_ptr) {
	int ch_err = chdrv_prog_write(dev_ptr, CH_PROG_REG_CPU, 0x40);    	// reset asic			// XXX need define

	ch_err |= chdrv_prog_write(dev_ptr, CH_PROG_REG_CPU, 0x11);   		// halt asic and disable watchdog;			// XXX need define

	return ch_err;
}

/*!
 * \brief Detect a connected sensor. 
 * 
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor
 * 
 * \return 1 if sensor is found, 0 if no sensor is found
 *
 * This function checks for a sensor sensor on the I2C bus by attempting to reset, halt, and read from the device using
 * the programming interface I2C address (0x45). 
 *
 * In order for the device to respond, the PROG pin for the device must be asserted before this function is called. 
 */
int chdrv_prog_ping(ch_dev_t *dev_ptr) {
	// Try a dummy write to the sensor to make sure it's connected and working
	uint16_t tmp;
	int ch_err;

    ch_err = chdrv_reset_and_halt(dev_ptr);

    ch_err |= chdrv_prog_read(dev_ptr, CH_PROG_REG_PING, &tmp);

#ifdef CHDRV_DEBUG
	if (!ch_err) {
		char cbuf[80];
		snprintf(cbuf, sizeof(cbuf), "Test I2C read: %04X\n", tmp);
		chbsp_print_str(cbuf);
	}
#endif

	return !(ch_err);
}

/*!
 * \brief Detect, program, and start a sensor.
 *
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor
 *
 * \return 0 if write to sensor succeeded, non-zero otherwise
 *
 * This function probes the I2C bus for the device.  If it is found, the sensor firmware is programmed into the 
 * device, and the application I2C address is set.  Then the sensor is reset and execution starts.
 *
 * Once started, the sensor device will begin an internal initialization and self-test sequence.  The \a chdrv_wait_for_lock()
 * function may be used to wait for this sequence to complete.
 *
 * \note This routine will leave the PROG pin de-asserted when it completes.
 */
int chdrv_detect_and_program(ch_dev_t *dev_ptr) {
	int ch_err = ! dev_ptr;
	if (ch_err) {
		return ch_err;
	}

	chbsp_program_enable(dev_ptr);					// assert PROG pin

	if (chdrv_prog_ping(dev_ptr)) {					// if device found
		dev_ptr->sensor_connected = 1;

		// Call device discovery hook routine, if any
		chdrv_discovery_hook_t hook_ptr = dev_ptr->group->disco_hook;
		if (hook_ptr != NULL) {
			ch_err = (*hook_ptr)(dev_ptr);		// hook routine can return error, will abort device init
		}

#ifdef CHDRV_DEBUG
		char cbuf[80];
		if (!ch_err) {
			uint16_t prog_stat = UINT16_MAX;
			ch_err = chdrv_prog_read(dev_ptr, CH_PROG_REG_STAT, &prog_stat);
			snprintf(cbuf, sizeof(cbuf), "PROG_STAT: 0x%02X\n", prog_stat);
			chbsp_print_str(cbuf);
		}
#endif

		ch_err = chdrv_init_ram(dev_ptr) ||                // init ram values
				 chdrv_write_firmware(dev_ptr) ||          // transfer program
				 chdrv_reset_and_halt(dev_ptr); 			// reset asic, since it was running mystery code before halt

#ifdef CHDRV_DEBUG
		if (!ch_err) {
			snprintf(cbuf, sizeof(cbuf), "Changing I2C address to %u\n", dev_ptr->i2c_address);
			chbsp_print_str(cbuf);
		}
#endif

		if (!ch_err ) {
			ch_err = chdrv_prog_mem_write(dev_ptr, 0x01C5, &dev_ptr->i2c_address, 1);			// XXX need define
		}

		/* Run charge pumps */
		if (!ch_err) {
			uint16_t write_val;
			write_val = 0x0200;			// XXX need defines
            ch_err |= chdrv_prog_mem_write(dev_ptr, 0x01A6, (uint8_t *)&write_val, 2);		// PMUT.CNTRL4 = HVVSS_FON
			chbsp_delay_ms(5);
			write_val = 0x0600;
            ch_err = chdrv_prog_mem_write(dev_ptr, 0x01A6, (uint8_t *)&write_val, 2);		// PMUT.CNTRL4 = (HVVSS_FON | HVVDD_FON)
			chbsp_delay_ms(5);
			write_val = 0x0000;
			ch_err |= chdrv_prog_mem_write(dev_ptr, 0x01A6, (uint8_t *)&write_val, 2);		// PMUT.CNTRL4 = 0
		}

		if (!ch_err ) {
			ch_err = chdrv_prog_write(dev_ptr, CH_PROG_REG_CPU, 2);	// Exit programming mode and run the chip
		}
	} else {
		dev_ptr->sensor_connected = 0;				// prog_ping failed - no device found
	}

	chbsp_program_disable(dev_ptr);				// de-assert PROG pin

	if (ch_err) { 								// if error, reinitialize I2C bus associated with this device
		chbsp_debug_toggle(CHDRV_DEBUG_PIN_NUM);
		chbsp_i2c_reset(dev_ptr);
	}

	if (ch_err) {
		dev_ptr->sensor_connected = 0;     		// only marked as connected if no errors
	}

	return ch_err;
}

/*!
 * \brief Put the sensors on a bus into an idle state
 *
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor, used to identify the target I2C bus
 *
 * \return 0 if successful, non-zero on error
 *
 * This function loads a tiny idle loop program to all ASICs on a given i2c bus.  This function assumes that all of the devices
 * on the given bus are halted in programming mode (i.e. PROG line is asserted).
 *
 * \note This routine writes to all devices simultaneously, so I2C signalling (i.e. ack's) on the bus may be driven by multiple
 * slaves at once.
 */
int chdrv_set_idle(ch_dev_t * dev_ptr) {
	static const uint16_t idle_loop[2] = {0x4003,0xFFFC}; 		// XXX need define

	int ch_err = chdrv_prog_mem_write(dev_ptr, 0xFFFC, (uint8_t *) &idle_loop[0], sizeof(idle_loop));
	if (!ch_err) {
		ch_err = chdrv_reset_and_halt(dev_ptr);
	}

	// keep wdt stopped after we exit programming mode
	uint16_t val = 0x5a80;			// XXX need define
	if (!ch_err) {
		ch_err = chdrv_prog_mem_write(dev_ptr, 0x0120, (uint8_t *)&val, sizeof(val));			// XXX need define
	}

	return ch_err;
}

/*!
 * \brief Detect, program, and start all sensors in a group.
 *
 * \param grp_ptr 		pointer to the ch_group_t config structure for a group of sensors
 *
 * \return 0 for success, non-zero if write(s) failed to any sensor initially detected as present
 *
 * This function probes the I2C bus for each device in the group.  For each detected sensor, the sensor firmware is 
 * programmed into the device, and the application I2C address is set.  Then the sensor is reset and execution starts.
 *
 * Once started, each sensor device will begin an internal initialization and self-test sequence.  The 
 * \a chdrv_group_wait_for_lock() function may be used to wait for this sequence to complete on all devices in the group.
 *
 * \note This routine will leave the PROG pin de-asserted for all devices in the group when it completes.
 */
int chdrv_group_detect_and_program(ch_group_t *grp_ptr) {
	int ch_err = 0;

	for (uint8_t i = 0; i < grp_ptr->num_ports; i++) {
		ch_dev_t *dev_ptr = grp_ptr->device[i];

		ch_err = chdrv_detect_and_program(dev_ptr);

		if (!ch_err && dev_ptr->sensor_connected) {
			grp_ptr->sensor_count++;
		}

		if (ch_err) {
			break;
		}
	}
	return ch_err;
}


/*!
 * \brief Initialize data structures and hardware for sensor interaction.
 *
 * \param grp_ptr 		pointer to the ch_group_t config structure for a group of sensors
 * \param num_ports 	the total number of physical sensor ports available
 *
 * \return 0 if hardware initialization is successful, non-zero otherwise
 *
 * This function is called internally by \a chdrv_group_start().
 */
int chdrv_group_prepare(ch_group_t* grp_ptr) {
	int ch_err = ! grp_ptr;
	uint8_t i;

	if (!ch_err) {
		grp_ptr->sensor_count = 0;

		for (i = 0; i < grp_ptr->num_i2c_buses; i++) {
			grp_ptr->i2c_queue[i].len = 0;
			grp_ptr->i2c_queue[i].idx = 0;
			grp_ptr->i2c_queue[i].read_pending = 0;
			grp_ptr->i2c_queue[i].running = 0;
		}

		chbsp_group_pin_init(grp_ptr);

		ch_err = chbsp_i2c_init();
	}

	return ch_err;
}

/*!
 * \brief Initalize and start a group of sensors.
 *
 * \param grp_ptr 		pointer to the ch_group_t config structure for a group of sensors
 * \param num_ports 	the total number of physical sensor ports available
 *
 * \return 0 if successful, 1 if device doesn't respond
 *
 * This function resets each sensor in programming mode, transfers the firmware image to the sensor's on-chip memory,
 * changes the sensor's application I2C address from the default, then starts the sensor and sends a timed pulse on the INT line
 * for real-time clock calibration.
 *
 * This function assumes firmware-specific initialization has already been performed for each a ch_dev_t
 * descriptor for each sensor in the group.  See \a ch_init().
 */
#define CH_PROG_XFER_RETRY 4
int chdrv_group_start(ch_group_t *grp_ptr) {
	int ch_err = ! grp_ptr;
	int i;
	uint8_t prog_tries = 0;
#ifdef CHDRV_DEBUG
	uint8_t i;
	char cbuf[80];
	const uint32_t start_time = chbsp_timestamp_ms();
#endif

	if (!ch_err) {
		ch_err = chdrv_group_prepare(grp_ptr);
	}

	if (ch_err) {
		return ch_err;
	}

RESET_AND_LOAD:
	do {
		chbsp_reset_assert();
		for (i = 0; i < grp_ptr->num_ports; i++) {
			chbsp_program_enable(grp_ptr->device[i]);
		}
		chbsp_delay_ms(1);
		chbsp_reset_release();

		 /* For every i2c bus, set the devices idle in parallel, then disable programming mode for all devices on that bus
		  * This is kludgey because we don't have a great way of iterating over the i2c buses */
		ch_dev_t * c_prev = grp_ptr->device[0];
		chdrv_set_idle(c_prev);
		for (i = 0; i < grp_ptr->num_ports; i++) {
			ch_dev_t * c = grp_ptr->device[i];

			if (c->i2c_bus_index != c_prev->i2c_bus_index) {
				chdrv_set_idle(c);
			}

			chbsp_program_disable(c);
			c_prev = c;
		}

		ch_err = chdrv_group_detect_and_program(grp_ptr);

	} while (ch_err && prog_tries++ < CH_PROG_XFER_RETRY);

	if (!ch_err) {
		ch_err = (grp_ptr->sensor_count == 0);
#ifdef CHDRV_DEBUG
		if (ch_err) {
			chbsp_print_str("No Chirp sensor devices are responding\n");
		} else {
			snprintf(cbuf, sizeof(cbuf), "Sensor count: %u, %lu ms.\n", grp_ptr->sensor_count, chbsp_timestamp_ms() - start_time);
			chbsp_print_str(cbuf);
			for (i = 0; i < grp_ptr->num_ports; i++) {
				if (grp_ptr->device[i].sensor_connected)
				{
					snprintf(cbuf, sizeof(cbuf), "Chirp sensor initialized on I2C addr %u:%u.\n", 
							 grp_ptr->device[i].i2c_bus_index, grp_ptr->device[i].i2c_address);
					chbsp_print_str(cbuf);
				}
			}
		}
#endif
	}

	if (!ch_err) {
		ch_err = chdrv_group_wait_for_lock(grp_ptr);
		if(ch_err && prog_tries++ < CH_PROG_XFER_RETRY+1) {
			goto RESET_AND_LOAD;
		}
	}

	if (!ch_err) {
#ifdef CHDRV_DEBUG
		snprintf(cbuf, sizeof(cbuf), "Frequency locked, %lu ms\n", chbsp_timestamp_ms() - start_time);
		chbsp_print_str(cbuf);
#endif

		chbsp_delay_ms(1);

		chdrv_group_measure_rtc(grp_ptr);

#ifdef CHDRV_DEBUG
		snprintf(cbuf, sizeof(cbuf), "RTC calibrated, %lu ms\n", chbsp_timestamp_ms() - start_time);
		chbsp_print_str(cbuf);

		for (i = 0; i < grp_ptr->num_ports; i++) {
			if (grp_ptr->device[i].sensor_connected)
			{
				snprintf(cbuf, sizeof(cbuf), "Cal result: %u\n", grp_ptr->device[i].rtc_cal_result);
				chbsp_print_str(cbuf);
			}
		}
#endif
	}

	return ch_err;
}

/*!
 * \brief Perform a soft reset on a sensor
 *
 * \param dev_ptr 		pointer to the ch_dev_t config structure for a sensor
 *
 * \return 0 if successful, non-zero otherwise
 *
 * This function performs a soft reset on an individual sensor by writing to a special control register.
 */
int chdrv_soft_reset(ch_dev_t *dev_ptr) {
	int ch_err = RET_ERR;

	if (dev_ptr->sensor_connected) {
		chbsp_program_enable(dev_ptr);

		ch_err = chdrv_reset_and_halt(dev_ptr);

		if (!ch_err) {
			ch_err = chdrv_init_ram(dev_ptr) || chdrv_reset_and_halt(dev_ptr);
		}
		if (!ch_err) {
			ch_err = chdrv_prog_mem_write(dev_ptr, 0x01C5, &dev_ptr->i2c_address, 1) ||			// XXX need define
					 chdrv_prog_write(dev_ptr, CH_PROG_REG_CPU, 2);    // Exit programming mode and run the chip
		}

		if (!ch_err) {
			chbsp_program_disable(dev_ptr);
		}
	}
	return ch_err;
}

/*!
 * \brief Perform a hard reset on a group of sensors.
 *
 * \param grp_ptr 		pointer to the ch_group_t config structure for a group of sensors
 *
 * \return 0 if successful, non-zero otherwise
 *
 * This function performs a hardware reset on each device in a group of sensors by asserting each device's RESET_N pin.
 */
int chdrv_group_hard_reset(ch_group_t *grp_ptr) {
    chbsp_reset_assert();
    chbsp_delay_us(1);
    chbsp_reset_release();

    for (uint8_t i = 0; i < grp_ptr->num_ports; i++) {
        ch_dev_t *dev_ptr = grp_ptr->device[i];

        int ch_err = chdrv_soft_reset(dev_ptr);

		if (ch_err) {
			dev_ptr->sensor_connected = 0;     // only marked as connected if no errors
		}
    }
    return 0;
}

/*!
 * \brief Perform a soft reset on a group of sensor devices
 *
 * \param grp_ptr 		pointer to the ch_group_t config structure for a group of sensors
 *
 * \return 0 if successful, non-zero otherwise
 *
 * This function performs a soft reset on each device in a group of sensors by writing to a special control register.
 */
int chdrv_group_soft_reset(ch_group_t *grp_ptr) {
	int ch_err = 0;

	for (uint8_t i = 0; i < grp_ptr->num_ports; i++) {
		ch_dev_t *dev_ptr = grp_ptr->device[i];
		ch_err |= chdrv_soft_reset(dev_ptr);
	}

	return ch_err;
}


/*!
 * \brief Register a hook routine to be called after device discovery.
 *
 * \param grp_ptr 		pointer to the ch_group_t config structure for a group of sensors
 * \param hook_func_ptr	address of hook routine to be called
 *
 * This function sets a pointer to a user supplied hook routine, which will be called from the
 * Chirp driver when each device is discovered on the I2C bus, before the device is initialized.
 *
 * This function should be called between \a ch_init() and \a ch_group_start().
 */
void chdrv_discovery_hook_set(ch_group_t *grp_ptr, chdrv_discovery_hook_t hook_func_ptr){

	grp_ptr->disco_hook = hook_func_ptr;
}

