// XXX need file header

#include "soniclib.h"
#include "ch_common.h"
#include "chirp_bsp.h"


uint8_t ch_common_set_mode(ch_dev_t *dev_ptr, ch_mode_t mode) {
	uint8_t ret_val = 0;
	uint8_t	opmode_reg;
	uint8_t	period_reg;
	uint8_t	tick_interval_reg;

	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		opmode_reg = CH101_COMMON_REG_OPMODE;
		period_reg = CH101_COMMON_REG_PERIOD;
		tick_interval_reg = CH101_COMMON_REG_TICK_INTERVAL;
	} else {
		opmode_reg = CH201_COMMON_REG_OPMODE;
		period_reg = CH201_COMMON_REG_PERIOD;
		tick_interval_reg = CH201_COMMON_REG_TICK_INTERVAL;
	}

	if (dev_ptr->sensor_connected) {
		switch (mode) {
			case CH_MODE_IDLE:
				chdrv_write_byte(dev_ptr, opmode_reg, CH_MODE_IDLE);
				chdrv_write_byte(dev_ptr, period_reg, 0);
				chdrv_write_word(dev_ptr, tick_interval_reg, 2048);		// XXX need define
				break;

			case CH_MODE_FREERUN:
				chdrv_write_byte(dev_ptr, opmode_reg, CH_MODE_FREERUN);
					// XXX need to set period / tick interval (?)
				break;

			case CH_MODE_TRIGGERED_TX_RX:
				chdrv_write_byte(dev_ptr, opmode_reg, CH_MODE_TRIGGERED_TX_RX);
				break;

			case CH_MODE_TRIGGERED_RX_ONLY:
				chdrv_write_byte(dev_ptr, opmode_reg, CH_MODE_TRIGGERED_RX_ONLY);
				break;

			default:
				ret_val = RET_ERR;				// return non-zero to indicate error
				break;
		}
	}

	return ret_val;
}

uint8_t ch_common_fw_load(ch_dev_t *dev_ptr) {
	uint8_t	ch_err = 0;
	uint16_t prog_mem_addr;
	uint16_t fw_size;

	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		prog_mem_addr = CH101_PROG_MEM_ADDR;
		fw_size 	  = CH101_FW_SIZE;
	} else {
		prog_mem_addr = CH201_PROG_MEM_ADDR;
		fw_size 	  = CH201_FW_SIZE;
	}

	ch_err = chdrv_prog_mem_write(dev_ptr, prog_mem_addr, (uint8_t *) dev_ptr->firmware, fw_size);
	return ch_err;
}


uint8_t ch_common_set_sample_interval(ch_dev_t *dev_ptr, uint16_t interval_ms) {
	uint8_t	period_reg;
	uint8_t	tick_interval_reg;
	uint8_t ret_val = 0;

	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		period_reg 		  = CH101_COMMON_REG_PERIOD;
		tick_interval_reg = CH101_COMMON_REG_TICK_INTERVAL;
	} else {
		period_reg 		  = CH201_COMMON_REG_PERIOD;
		tick_interval_reg = CH201_COMMON_REG_TICK_INTERVAL;
	}

	if (dev_ptr->sensor_connected) {
		uint32_t sample_interval = dev_ptr->rtc_cal_result * interval_ms / dev_ptr->group->rtc_cal_pulse_ms;
		uint32_t period = (sample_interval / 2048) + 1;				// XXX need define

		if (period > UINT8_MAX) {		/* check if result fits in register */
			ret_val = 1;
		}

		if (ret_val == 0) {
			uint32_t tick_interval = sample_interval / period;

#ifdef CHDRV_DEBUG
			char cbuf[80];
			snprintf(cbuf, sizeof(cbuf), "Set period=%lu, tick_interval=%lu\n", period, tick_interval);
			chbsp_print_str(cbuf);
#endif

			chdrv_write_byte(dev_ptr, period_reg, (uint8_t) period);
			chdrv_write_word(dev_ptr, tick_interval_reg, (uint16_t) tick_interval);
		}
	}

	return ret_val;
}

// XXX need comment block
// XXX    note uses actual num_samples, even for CH201
uint8_t ch_common_set_num_samples(ch_dev_t *dev_ptr, uint16_t num_samples ) {
	uint8_t max_range_reg;
	uint8_t ret_val = 1;		// default is error (not connected or num_samples too big)

	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		max_range_reg = CH101_COMMON_REG_MAX_RANGE;
	} else {
		max_range_reg = CH201_COMMON_REG_MAX_RANGE;
		num_samples /= 2;					// each internal count for CH201 represents 2 physical samples
	}

	if (dev_ptr->sensor_connected && (num_samples <= UINT8_MAX)) {
		ret_val = chdrv_write_byte( dev_ptr, max_range_reg, num_samples );
	}

	return ret_val;
}


uint8_t ch_common_set_max_range(ch_dev_t *dev_ptr, uint16_t max_range_mm) {
	uint8_t ret_val;
	uint32_t num_samples;
	uint16_t max_num_samples;


	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		max_num_samples = CH101_COMMON_NUM_SAMPLES;
	} else {
		max_num_samples = CH201_COMMON_NUM_SAMPLES;
	}

	ret_val = (!dev_ptr->sensor_connected);


	if (!ret_val) {
		num_samples = ch_common_mm_to_samples(dev_ptr, max_range_mm);

		if (num_samples > max_num_samples) {
			num_samples = max_num_samples;
			dev_ptr->max_range = ch_samples_to_mm(dev_ptr, num_samples);	// store reduced max range
		} else {
			dev_ptr->max_range = max_range_mm;							// store user-specified max range
		}


#ifdef CHDRV_DEBUG
		char cbuf[80];
		snprintf(cbuf, sizeof(cbuf), "num_samples=%lu\n", num_samples);
		chbsp_print_str(cbuf);
#endif
	}

	if (!ret_val) {
		ret_val = ch_common_set_num_samples(dev_ptr, (uint8_t) num_samples);
	}

	if (!ret_val) {
		dev_ptr->num_rx_samples = num_samples;
	} else {
		dev_ptr->num_rx_samples = 0;
	}

#ifdef CHDRV_DEBUG
	printf("Set samples: ret_val: %u  dev_ptr->num_rx_samples: %u\n", ret_val, dev_ptr->num_rx_samples);
#endif
	return ret_val;
}


uint16_t ch_common_mm_to_samples(ch_dev_t *dev_ptr, uint16_t num_mm) {
	uint8_t err;
	uint16_t scale_factor;
	uint32_t num_samples = 0;
	uint32_t divisor1;
	uint32_t divisor2 = (dev_ptr->group->rtc_cal_pulse_ms * CH_SPEEDOFSOUND_MPS);

	err = (!dev_ptr) || (!dev_ptr->sensor_connected);

	if (!err) {
		if (dev_ptr->part_number == CH101_PART_NUMBER) {
			divisor1 = 0x2000;			// (4*16*128)  XXX need define(s)
		} else {
			divisor1 = 0x4000;			// (4*16*128*2)  XXX need define(s)
		}

		if (dev_ptr->scale_factor == 0) {
			ch_common_store_scale_factor(dev_ptr);
		}

		scale_factor = dev_ptr->scale_factor;
	}

	if (!err) {
		// Two steps of division to avoid needing a type larger than 32 bits
		// Ceiling division to ensure result is at least enough samples to meet specified range
		num_samples = ((dev_ptr->rtc_cal_result * scale_factor) + (divisor1 - 1)) / divisor1;
		num_samples = ((num_samples * num_mm) + (divisor2 - 1)) / divisor2;
		err = num_samples > UINT16_MAX;
	}

	if (!err) {
		if (dev_ptr->part_number == CH201_PART_NUMBER) {
			num_samples *= 2;			// each internal count for CH201 represents 2 physical samples
		}

		/* Adjust for oversampling, if used */
		num_samples <<= dev_ptr->oversample;
	}
	if (err) {
		num_samples = 0;		// return zero if error
	}

	return (uint16_t) num_samples;
}


uint16_t ch_common_samples_to_mm(ch_dev_t *dev_ptr, uint16_t num_samples) {
	uint32_t	num_mm = 0;
	uint32_t	op_freq = dev_ptr->op_frequency;

	if (op_freq != 0) {
		num_mm = ((uint32_t) num_samples * CH_SPEEDOFSOUND_MPS * 8 * 1000) / (op_freq * 2);
	}

	/* Adjust for oversampling, if used */
	num_mm >>= dev_ptr->oversample;

	return (uint16_t) num_mm;
}



uint8_t ch_common_set_static_range(ch_dev_t *dev_ptr, uint16_t samples) {
	uint8_t ret_val = 1;  	// default is error return

	if (dev_ptr->part_number == CH101_PART_NUMBER) {			// CH101 only
		if (dev_ptr->sensor_connected) {
			ret_val = chdrv_write_byte(dev_ptr, CH101_COMMON_REG_STAT_RANGE, samples);

			if (!ret_val) {
				dev_ptr->static_range = samples;
			}
		}
	}
	return ret_val;
}

uint32_t ch_common_get_range(ch_dev_t *dev_ptr, ch_range_t range_type) {
	uint8_t		tof_reg;
	uint32_t	range = CH_NO_TARGET;
	uint16_t 	time_of_flight;
	uint16_t 	scale_factor;
	int 		err;

	if (dev_ptr->sensor_connected) {

		if (dev_ptr->part_number == CH101_PART_NUMBER) {
			tof_reg = CH101_COMMON_REG_TOF;
		} else {
			tof_reg = CH201_COMMON_REG_TOF;
		}

		err = chdrv_read_word(dev_ptr, tof_reg, &time_of_flight);

		if (!err && (time_of_flight != UINT16_MAX)) { // If object detected

			if (dev_ptr->scale_factor == 0) {
				ch_common_store_scale_factor(dev_ptr);
			}
			scale_factor = dev_ptr->scale_factor;

			if (scale_factor != 0) {
				uint32_t num = (CH_SPEEDOFSOUND_MPS * dev_ptr->group->rtc_cal_pulse_ms * (uint32_t) time_of_flight);
				uint32_t den = ((uint32_t) dev_ptr->rtc_cal_result * (uint32_t) scale_factor) >> 11;		// XXX need define

				range = (num / den);

				if (range_type == CH_RANGE_ECHO_ONE_WAY) {
					range /= 2;
				}

				/* Adjust for oversampling, if used */
				range >>= dev_ptr->oversample;

			}
		}
	}
	return range;
}


uint16_t ch_common_get_amplitude(ch_dev_t *dev_ptr) {
	uint8_t  amplitude_reg;
	uint16_t amplitude = 0;

	if (dev_ptr->sensor_connected) {
		if (dev_ptr->part_number == CH101_PART_NUMBER) {
			amplitude_reg = CH101_COMMON_REG_AMPLITUDE;
		} else {
			amplitude_reg = CH201_COMMON_REG_AMPLITUDE;
		}

		chdrv_read_word(dev_ptr, amplitude_reg, &amplitude);
	}

	return amplitude;
}


uint8_t ch_common_get_locked_state(ch_dev_t *dev_ptr) {
	uint8_t ready_reg;
	uint8_t lock_mask;
	uint8_t ret_val = 0;

	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		ready_reg = CH101_COMMON_REG_READY;
		lock_mask = CH101_COMMON_READY_FREQ_LOCKED;
	} else {
		ready_reg = CH201_COMMON_REG_READY;
		lock_mask = CH201_COMMON_READY_FREQ_LOCKED;
	}

	if (dev_ptr->sensor_connected) {
		uint8_t ready_value = 0;
		chdrv_read_byte(dev_ptr, ready_reg, &ready_value);
		if (ready_value & lock_mask) {
			ret_val = 1;
		}
	}
	return ret_val;
}

void ch_common_prepare_pulse_timer(ch_dev_t *dev_ptr) {
	uint8_t cal_trig_reg;

	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		cal_trig_reg = CH101_COMMON_REG_CAL_TRIG;
	} else {
		cal_trig_reg = CH201_COMMON_REG_CAL_TRIG;
	}

	chdrv_write_byte(dev_ptr, cal_trig_reg, 0);
}

void ch_common_store_pt_result(ch_dev_t *dev_ptr) {
	uint8_t pt_result_reg;
	uint16_t rtc_cal_result;

	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		pt_result_reg = CH101_COMMON_REG_CAL_RESULT;
	} else {
		pt_result_reg = CH201_COMMON_REG_CAL_RESULT;
	}

	chdrv_read_word(dev_ptr, pt_result_reg, &rtc_cal_result);
	dev_ptr->rtc_cal_result = rtc_cal_result;
}

void ch_common_store_op_freq(ch_dev_t *dev_ptr){
	uint8_t	 tof_sf_reg;
	uint16_t raw_freq;		// aka scale factor
	uint32_t freq_counter_cycles;
	uint32_t num;
	uint32_t den;
	uint32_t op_freq;

	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		tof_sf_reg = CH101_COMMON_REG_TOF_SF;
		freq_counter_cycles = CH101_FREQCOUNTERCYCLES;
	} else {
		tof_sf_reg = CH201_COMMON_REG_TOF_SF;
		freq_counter_cycles = CH201_FREQCOUNTERCYCLES;
	}

	chdrv_read_word(dev_ptr, tof_sf_reg, &raw_freq);

	num = (uint32_t)(((dev_ptr->rtc_cal_result)*1000U) / (16U * freq_counter_cycles)) * (uint32_t)(raw_freq);
	den = (uint32_t)(dev_ptr->group->rtc_cal_pulse_ms);
	op_freq = (num/den);

	dev_ptr->op_frequency = op_freq;
}


void ch_common_store_bandwidth(ch_dev_t *dev_ptr) {
/*
 * Not supported in current GPR firmware
 */
}

void ch_common_store_scale_factor(ch_dev_t *dev_ptr) {
	uint8_t	err;
	uint8_t	tof_sf_reg;
	uint16_t scale_factor;

	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		tof_sf_reg = CH101_COMMON_REG_TOF_SF;
	} else {
		tof_sf_reg = CH201_COMMON_REG_TOF_SF;
	}

	err = chdrv_read_word(dev_ptr, tof_sf_reg, &scale_factor);
	if (!err) {
		dev_ptr->scale_factor = scale_factor;
	} else {
		dev_ptr->scale_factor = 0;
	}
}


// XXX  Need comment block
uint8_t ch_common_set_thresholds(ch_dev_t *dev_ptr, ch_thresholds_t *thresholds_ptr) {

	uint8_t	thresh_len_reg = 0;		// offset of register for this threshold's length
	uint8_t thresh_level_reg;	// threshold level reg (first in array)
	uint8_t max_num_thresholds;
	int ret_val = 1;		// default return = error
	uint8_t	thresh_num;
	uint8_t thresh_len;
	uint16_t thresh_level;
	uint16_t start_sample = 0;

	if (dev_ptr->sensor_connected) {
		
		if (dev_ptr->part_number == CH101_PART_NUMBER) {
			return ret_val;		// NOT SUPPORTED in CH101

		} else {
			thresh_level_reg = CH201_COMMON_REG_THRESHOLDS;
			max_num_thresholds = CH201_COMMON_NUM_THRESHOLDS;
		}

		for (thresh_num = 0; thresh_num < max_num_thresholds; thresh_num++) {

			if (thresh_num < (max_num_thresholds - 1)) {
				uint16_t next_start_sample = thresholds_ptr->threshold[thresh_num + 1].start_sample;

				thresh_len = (next_start_sample - start_sample);
				start_sample  = next_start_sample;
			} else {
				thresh_len = 0;
			}

			if (dev_ptr->part_number == CH201_PART_NUMBER) {
				if (thresh_num == 0) {
					thresh_len_reg = CH201_COMMON_REG_THRESH_LEN_0;
				} else if (thresh_num == 1) {
					thresh_len_reg = CH201_COMMON_REG_THRESH_LEN_1;
				} else if (thresh_num == 2) {
					thresh_len_reg = CH201_COMMON_REG_THRESH_LEN_2;
				} else if (thresh_num == 3) {
					thresh_len_reg = CH201_COMMON_REG_THRESH_LEN_3;
				} else if (thresh_num == 4) {
					thresh_len_reg = CH201_COMMON_REG_THRESH_LEN_4;
				} else if (thresh_num == 5) {
					thresh_len_reg = 0;			// last threshold does not have length field - assumed to extend to end of data
				}
			}

			if (thresh_len_reg != 0) {
				chdrv_write_byte(dev_ptr, thresh_len_reg, thresh_len); 	// set the length field (if any) for this threshold
			}
			// write level to this threshold's entry in register array
			thresh_level = thresholds_ptr->threshold[thresh_num].level;
			chdrv_write_word(dev_ptr, (thresh_level_reg + (thresh_num * sizeof(uint16_t))), thresh_level);
		}

		ret_val = 0;	// return OK
	}
	return ret_val;
}


// XXX Need comment block

uint8_t ch_common_get_thresholds(ch_dev_t *dev_ptr, ch_thresholds_t *thresholds_ptr) {
	uint8_t	thresh_len_reg = 0;		// offset of register for this threshold's length
	uint8_t thresh_level_reg;	// threshold level reg (first in array)
	uint8_t max_num_thresholds;
	uint8_t ret_val = 1;		// default = error return
	uint8_t thresh_num;
	uint8_t	thresh_len = 0;		// number of samples described by each threshold
	uint16_t	start_sample = 0;	// calculated start sample for each threshold

	if (dev_ptr->sensor_connected && (thresholds_ptr != NULL)) {
		
		if (dev_ptr->part_number == CH101_PART_NUMBER) {
			return ret_val;		// NOT SUPPORTED in CH101
			
		} else {
			thresh_level_reg = CH201_COMMON_REG_THRESHOLDS;
			max_num_thresholds = CH201_COMMON_NUM_THRESHOLDS;
		}

		for (thresh_num = 0; thresh_num < max_num_thresholds; thresh_num++) {

			if (dev_ptr->part_number == CH201_PART_NUMBER) {
				if (thresh_num == 0) {
					thresh_len_reg = CH201_COMMON_REG_THRESH_LEN_0;
				} else if (thresh_num == 1) {
					thresh_len_reg = CH201_COMMON_REG_THRESH_LEN_1;
				} else if (thresh_num == 2) {
					thresh_len_reg = CH201_COMMON_REG_THRESH_LEN_2;
				} else if (thresh_num == 3) {
					thresh_len_reg = CH201_COMMON_REG_THRESH_LEN_3;
				} else if (thresh_num == 4) {
					thresh_len_reg = CH201_COMMON_REG_THRESH_LEN_4;
				} else if (thresh_num == 5) {
					thresh_len_reg = 0;			// last threshold does not have length field - assumed to extend to end of data
				}
			}

			if (thresh_len_reg != 0) {
				// read the length field register for this threshold
				chdrv_read_byte(dev_ptr, thresh_len_reg, &thresh_len);
			} else {
				thresh_len = 0;
			}

			thresholds_ptr->threshold[thresh_num].start_sample = start_sample;
			start_sample += thresh_len;				// increment start sample for next threshold

			// get level from this threshold's entry in register array
			chdrv_read_word(dev_ptr, (thresh_level_reg + (thresh_num * sizeof(uint16_t))), 
						    &(thresholds_ptr->threshold[thresh_num].level));

		}
		ret_val = 0;	// return OK
	}
	return ret_val;
}


// XXX need comment block
uint8_t  ch_common_get_iq_data(ch_dev_t *dev_ptr, ch_iq_sample_t *buf_ptr, uint16_t start_sample, uint16_t num_samples, 
							   ch_io_mode_t mode) {
	uint16_t   iq_data_addr;
	uint16_t   max_samples;
	ch_group_t *grp_ptr = dev_ptr->group;
	int        error = 1;

	if (dev_ptr->part_number == CH101_PART_NUMBER) {
		iq_data_addr = CH101_COMMON_REG_DATA;
		max_samples  = CH101_COMMON_NUM_SAMPLES;
	} else {
		iq_data_addr = CH201_COMMON_REG_DATA;
		max_samples  = CH201_COMMON_NUM_SAMPLES;
	}

	iq_data_addr += (start_sample * sizeof(ch_iq_sample_t));

	if ((num_samples != 0) && ((start_sample + num_samples) <= max_samples)) {
		uint16_t num_bytes = (num_samples * sizeof(ch_iq_sample_t));

		if (mode == CH_IO_MODE_BLOCK) {

#ifdef USE_STD_I2C_FOR_IQ
			/* blocking transfer - use standard I2C interface */
			error = chdrv_burst_read(dev_ptr, iq_data_addr, (uint8_t *) buf_ptr, num_bytes);
#else
			/* blocking transfer - use low-level programming interface for speed */

			int num_transfers = (num_bytes + (CH_PROG_XFER_SIZE - 1)) / CH_PROG_XFER_SIZE;
    		int bytes_left = num_bytes;       // remaining bytes to read

			/* Convert register offsets to full memory addresses */
			if (dev_ptr->part_number == CH101_PART_NUMBER) {
				iq_data_addr += CH101_DATA_MEM_ADDR + CH101_COMMON_I2CREGS_OFFSET;
			} else {
				iq_data_addr += CH201_DATA_MEM_ADDR + CH201_COMMON_I2CREGS_OFFSET;
			}

			chbsp_program_enable(dev_ptr);					// assert PROG pin

    		for (int xfer = 0; xfer < num_transfers; xfer++) {
        		int bytes_to_read;
        		uint8_t message[] = { (0x80 | CH_PROG_REG_CTL), 0x09 };      // read burst command

        		if (bytes_left > CH_PROG_XFER_SIZE) {
                		bytes_to_read = CH_PROG_XFER_SIZE;
        		} else {
                		bytes_to_read = bytes_left;
        		}
        		chdrv_prog_write(dev_ptr, CH_PROG_REG_ADDR, (iq_data_addr + (xfer * CH_PROG_XFER_SIZE)));
        		chdrv_prog_write(dev_ptr, CH_PROG_REG_CNT, (bytes_to_read - 1));
        		error = chdrv_prog_i2c_write(dev_ptr, message, sizeof(message));
        		error |= chdrv_prog_i2c_read(dev_ptr, (uint8_t *)(buf_ptr + (xfer * CH_PROG_XFER_SIZE)), bytes_to_read);

        		bytes_left -= bytes_to_read;
    		}
    		chbsp_program_disable(dev_ptr);					// de-assert PROG pin
#endif	// USE_STD_I2C_FOR_IQ

		} else {
			/* non-blocking transfer - queue a read transaction (must be started using ch_io_start_nb() ) */

			if (grp_ptr->i2c_drv_flags & I2C_DRV_FLAG_USE_PROG_NB) {
				/* Use low-level programming interface to read data */

				/* Convert register offsets to full memory addresses */
				if (dev_ptr->part_number == CH101_PART_NUMBER) {
					iq_data_addr += (CH101_DATA_MEM_ADDR + CH101_COMMON_I2CREGS_OFFSET);
				} else {
					iq_data_addr += (CH201_DATA_MEM_ADDR + CH201_COMMON_I2CREGS_OFFSET);
				}

				error = chdrv_group_i2c_queue(grp_ptr, dev_ptr, 1, CHDRV_NB_TRANS_TYPE_PROG, iq_data_addr, num_bytes, 
					                      	(uint8_t *) buf_ptr);
			} else {
				/* Use regular I2C register interface to read data */
				error = chdrv_group_i2c_queue(grp_ptr, dev_ptr, 1, CHDRV_NB_TRANS_TYPE_STD, iq_data_addr, num_bytes, 
											  (uint8_t*) buf_ptr);
			}
		}
	}

	return error;
}





