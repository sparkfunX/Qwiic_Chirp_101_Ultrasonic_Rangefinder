/*
Copyright © 2016-2019, Chirp Microsystems.  All rights reserved.
All rights reserved.

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
 or by mail at 2560 Ninth Street, Suite 220, Berkeley, CA 94710.
*/



/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CHIRP_SMARTSONIC_H
#define __CHIRP_SMARTSONIC_H

#include "soniclib.h"
#include "chirp_board_config.h"

/* Standard symbols used in board support package - use values from config header */
#define CHBSP_MAX_DEVICES 		CHIRP_MAX_NUM_SENSORS
#define CHBSP_NUM_I2C_BUSES 	CHIRP_NUM_I2C_BUSES

/* Length of real-time clock calibration pulse, in milliseconds */
#define CHBSP_RTC_CAL_PULSE_MS	100			// length of pulse applied to sensor INT line during clock cal

/* I2C Address assignments for each possible device */
#define CHIRP_I2C_ADDRS		{45, 43, 44, 42 }
#define CHIRP_I2C_BUSES		{ 0,  0,  1,  1 }

/* IRQ assignments */
#define TWI1_IRQn           FLEXCOM1_IRQn
#define TWI3_IRQn           FLEXCOM3_IRQn

/* Processor sleep mode */
#define	PROC_SLEEP_MODE		SAM_PM_SMODE_SLEEP_WFI		/* wait for interrupt */

/* Structure to track non-blocking I2C transaction data */
typedef struct {
	uint8_t		*buf_ptr;		/* pointer to data buffer */
	uint16_t	num_bytes;		/* number of bytes to transfer */
} i2c_trans_data_t;


extern uint32_t chirp_pin_prog[CHBSP_MAX_DEVICES];
extern uint32_t chirp_pin_io[CHBSP_MAX_DEVICES];
extern uint32_t chirp_pin_io_irq[CHBSP_MAX_DEVICES];
extern uint32_t chirp_led_pins[];

extern i2c_trans_data_t	i2c_nb_transactions[CHBSP_NUM_I2C_BUSES];		// array of structures to track non-blocking I2C transactions

extern void Measure_power(void);	
extern void ADC0_init(void);
extern void ext_int_init(void);
extern void find_sensors(void);
extern void sensor_led_on(uint32_t pin);
extern void sensor_led_off(uint32_t pin);
extern void indicate_alive(void);
extern uint32_t Measure_Idd(unsigned int count);
extern void	 ext_MotionINT_handler(void);
extern void	 ext_ChirpINT0_handler(void);
extern void	 ext_ChirpINT1_handler(void);
extern void	 ext_ChirpINT2_handler(void);
extern void	 ext_ChirpINT3_handler(void);

extern volatile int MotionINT_trigger;
extern volatile int ChirpINT0_trigger;
extern volatile int ChirpINT1_trigger;
extern volatile int ChirpINT2_trigger;
extern volatile int ChirpINT3_trigger;

extern ch_io_int_callback_t io_int_callback_ptr;	// pointer to sensor I/O interrupt callback function

#endif /* __CHIRP_SMARTSONIC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
