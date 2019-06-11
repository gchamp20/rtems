/**
 * @file
 *
 * @ingroup bsp_clock
 *
 * @brief Raspberry Pi clock support.
 */

/*
 * BCM2835 Clock driver
 *
 * Copyright (c) 2013 Alan Cudmore
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *
 *  http://www.rtems.org/license/LICENSE
 *
*/

#include <rtems.h>
#include <bsp.h>
#include <bsp/irq.h>
#include <bsp/raspberrypi.h>

/* This is defined in ../../../shared/clockdrv_shell.h */
void Clock_isr(rtems_irq_hdl_param arg);

#define PRI_BASE_ADDRESS     0x3F000000UL
#define RPI_GPIO_BASE       ( PRI_BASE_ADDRESS + 0x200000 )

typedef struct {
    volatile uint32_t GPFSEL[6]; ///< Function selection registers.
    volatile uint32_t Reserved_1;
    volatile uint32_t GPSET[2];
    volatile uint32_t Reserved_2;
    volatile uint32_t GPCLR[2];
    volatile uint32_t Reserved_3;
    volatile uint32_t GPLEV[2];
    volatile uint32_t Reserved_4;
    volatile uint32_t GPEDS[2];
    volatile uint32_t Reserved_5;
    volatile uint32_t GPREN[2];
    volatile uint32_t Reserved_6;
    volatile uint32_t GPFEN[2];
    volatile uint32_t Reserved_7;
    volatile uint32_t GPHEN[2];
    volatile uint32_t Reserved_8;
    volatile uint32_t GPLEN[2];
    volatile uint32_t Reserved_9;
    volatile uint32_t GPAREN[2];
    volatile uint32_t Reserved_A;
    volatile uint32_t GPAFEN[2];
    volatile uint32_t Reserved_B;
    volatile uint32_t GPPUD[1];
    volatile uint32_t GPPUDCLK[2];
    //Ignoring the reserved and test bytes
} RPI_GPIO_t;

#define RPI_GPIO ((volatile RPI_GPIO_t *)RPI_GPIO_BASE)

static void rpi_gpio_sel_fun(uint32_t pin, uint32_t func) {
    int offset = pin / 10;
    uint32_t val = RPI_GPIO->GPFSEL[offset];// Read in the original register value.

    int item = pin % 10;
    val &= ~(0x7 << (item * 3));
    val |= ((func & 0x7) << (item * 3));
    RPI_GPIO->GPFSEL[offset] = val;
}

static void rpi_gpio_set_val(uint32_t pin, uint32_t val) {
    uint32_t offset = pin / 32;
    uint32_t mask = (1 << (pin % 32));

    if (val) {
        RPI_GPIO->GPSET[offset] |= mask;
    } else {
        RPI_GPIO->GPCLR[offset] |= mask;
    }
}

static int32_t LED_STATUS = 0;
static void raspberrypi_clock_at_tick(void)
{
   BCM2835_REG(BCM2835_TIMER_CLI) = 0;

	LED_STATUS = LED_STATUS ^ 0x01;
	rpi_gpio_set_val(21, LED_STATUS);
}

static void raspberrypi_clock_handler_install(void)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  sc = rtems_interrupt_handler_install(
    BCM2835_IRQ_ID_TIMER_0,
    "Clock",
    RTEMS_INTERRUPT_UNIQUE,
    (rtems_interrupt_handler) Clock_isr,
    NULL
  );
  if (sc != RTEMS_SUCCESSFUL) {
    rtems_fatal_error_occurred(0xdeadbeef);
  }

	printk("installing int. handler\n");
	rpi_gpio_sel_fun(21, 1);
}

static void raspberrypi_clock_initialize(void)
{
   BCM2835_REG(BCM2835_TIMER_CTL) = 0x003E0000;
   BCM2835_REG(BCM2835_TIMER_LOD) =
                rtems_configuration_get_microseconds_per_tick() - 1;
   BCM2835_REG(BCM2835_TIMER_RLD) =
                rtems_configuration_get_microseconds_per_tick() - 1;
   BCM2835_REG(BCM2835_TIMER_DIV) = BCM2835_TIMER_PRESCALE;
   BCM2835_REG(BCM2835_TIMER_CLI) = 0;
   BCM2835_REG(BCM2835_TIMER_CTL) = 0x003E00A2;
}

static void raspberrypi_clock_cleanup(void)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  /* Remove interrupt handler */
  sc = rtems_interrupt_handler_remove(
    BCM2835_IRQ_ID_TIMER_0,
    (rtems_interrupt_handler) Clock_isr,
    NULL
  );
  if (sc != RTEMS_SUCCESSFUL) {
    rtems_fatal_error_occurred(0xdeadbeef);
  }
}

#define Clock_driver_support_at_tick() raspberrypi_clock_at_tick()

#define Clock_driver_support_initialize_hardware() raspberrypi_clock_initialize()

#define Clock_driver_support_install_isr(isr, old_isr) \
  do {                                                 \
    raspberrypi_clock_handler_install();               \
    old_isr = NULL;                                    \
  } while (0)

#define Clock_driver_support_shutdown_hardware() raspberrypi_clock_cleanup()

#define CLOCK_DRIVER_USE_DUMMY_TIMECOUNTER

#include "../../../shared/clockdrv_shell.h"
