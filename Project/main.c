

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <main.h>
#include "leds.h"
#include "spi_comm.h"
#include "sensors/proximity.h"
#include "motors.h"
#include "audio/play_melody.h"

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

/* ── Tuning speeds */
#define BASE_SPEED        500
#define WALL_TURN_SPEED        400
#define CORNER_TURN_SPEED        300
#define WALL_TOO_CLOSE    700
#define WALL_TOO_FAR       400
#define FRONT_OBSTACLE      150.0

/* Sensor index aliases */
#define IR_FRONT_RIGHT      0   /* ps0 */
#define IR_RIGHT            2   /* ps2 */
#define IR_FRONT_LEFT       7   /* ps7 */

int main(void)
{
    halInit();
    chSysInit();
    mpu_init();

    // Proximity
    messagebus_init(&bus, &bus_lock, &bus_condvar);
    proximity_start(0);
    calibrate_ir();

    // LED
    clear_leds();
    spi_comm_start();
    set_body_led(1);

    // Motors
    motors_init();

    dac_start();
    playMelodyStart();

    void playMelody(song_selection_t choice, play_melody_option_t option, melody_t*
    external_melody);

    /* Infinite loop */
    while (1)
    {
    	//playMelody(MARIO, ML_SIMPLE_PLAY, NULL);
        /* Read proximity sensors*/
        int front_right = get_prox(IR_FRONT_RIGHT);   /* ps0 */
        int right       = get_prox(IR_RIGHT);          /* ps2 */
        int front_left  = get_prox(IR_FRONT_LEFT);    /* ps7 */

        int left_speed, right_speed;

        /* Corner turn procedure*/
        if (front_right > FRONT_OBSTACLE || front_left > FRONT_OBSTACLE)
        {
            left_speed  = 0;
            right_speed =  CORNER_TURN_SPEED;
            set_front_led(1);   /* Front LED on to indicate turning            */
        }

        /*Wall turning - too close */
        else if (right > WALL_TOO_CLOSE || front_right > WALL_TOO_CLOSE)
        {
            left_speed  = WALL_TURN_SPEED;
            right_speed = BASE_SPEED ;
            set_front_led(0);
        }

        /* wall turning - too far*/
        else if (right < WALL_TOO_FAR)
        {
            left_speed  = BASE_SPEED;
            right_speed = WALL_TURN_SPEED;
            set_front_led(0);
        }

        /*normal driving*/
        else
        {
            left_speed  = BASE_SPEED;
            right_speed = BASE_SPEED;
            set_front_led(0);

        }

        left_motor_set_speed(left_speed);
        right_motor_set_speed(right_speed);

        /*delay for sensors*/
        chThdSleepMilliseconds(50);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;
void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
