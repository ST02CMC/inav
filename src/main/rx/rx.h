/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "common/time.h"
#include "config/parameter_group.h"

#define STICK_CHANNEL_COUNT 4

#define PWM_RANGE_ZERO 0 // FIXME should all usages of this be changed to use PWM_RANGE_MIN?
#define PWM_RANGE_MIN 1000
#define PWM_RANGE_MAX 2000
#define PWM_RANGE_MIDDLE (PWM_RANGE_MIN + ((PWM_RANGE_MAX - PWM_RANGE_MIN) / 2))

#define PWM_PULSE_MIN   750       // minimum PWM pulse width which is considered valid
#define PWM_PULSE_MAX   2250      // maximum PWM pulse width which is considered valid

#define MIDRC_MIN 1200
#define MIDRC_MAX 1700

#define RXFAIL_STEP_TO_CHANNEL_VALUE(step) (PWM_PULSE_MIN + 25 * step)
#define CHANNEL_VALUE_TO_RXFAIL_STEP(channelValue) ((constrain(channelValue, PWM_PULSE_MIN, PWM_PULSE_MAX) - PWM_PULSE_MIN) / 25)
#define MAX_RXFAIL_RANGE_STEP ((PWM_PULSE_MAX - PWM_PULSE_MIN) / 25)

#define DEFAULT_SERVO_MIN 1000
#define DEFAULT_SERVO_MIDDLE 1500
#define DEFAULT_SERVO_MAX 2000

#define DELAY_50_HZ (1000000 / 50)
#define DELAY_10_HZ (1000000 / 10)
#define DELAY_5_HZ (1000000 / 5)

typedef enum {
    RX_FRAME_PENDING = 0,               // No new data available from receiver
    RX_FRAME_COMPLETE = (1 << 0),       // There is new data available
    RX_FRAME_FAILSAFE = (1 << 1)        // Receiver detected loss of RC link. Only valid when RX_FRAME_COMPLETE is set as well
} rxFrameState_e;

typedef enum {
    RX_TYPE_NONE        = 0,
    RX_TYPE_PWM         = 1,
    RX_TYPE_PPM         = 2,
    RX_TYPE_SERIAL      = 3,
    RX_TYPE_MSP         = 4,
    RX_TYPE_SPI         = 5,
    RX_TYPE_UIB         = 6
} rxReceiverType_e;

typedef enum {
    SERIALRX_SPEKTRUM1024 = 0,
    SERIALRX_SPEKTRUM2048 = 1,
    SERIALRX_SBUS = 2,
    SERIALRX_SUMD = 3,
    SERIALRX_SUMH = 4,
    SERIALRX_XBUS_MODE_B = 5,
    SERIALRX_XBUS_MODE_B_RJ01 = 6,
    SERIALRX_IBUS = 7,
    SERIALRX_JETIEXBUS = 8,
    SERIALRX_CRSF = 9
} rxSerialReceiverType_e;

#define MAX_SUPPORTED_RC_PPM_CHANNEL_COUNT          16
#define MAX_SUPPORTED_RC_PARALLEL_PWM_CHANNEL_COUNT  8
#define MAX_SUPPORTED_RC_CHANNEL_COUNT              18

#define NON_AUX_CHANNEL_COUNT 4
#define MAX_AUX_CHANNEL_COUNT (MAX_SUPPORTED_RC_CHANNEL_COUNT - NON_AUX_CHANNEL_COUNT)

#if MAX_SUPPORTED_RC_PARALLEL_PWM_CHANNEL_COUNT > MAX_SUPPORTED_RC_PPM_CHANNEL_COUNT
#define MAX_SUPPORTED_RX_PARALLEL_PWM_OR_PPM_CHANNEL_COUNT MAX_SUPPORTED_RC_PARALLEL_PWM_CHANNEL_COUNT
#else
#define MAX_SUPPORTED_RX_PARALLEL_PWM_OR_PPM_CHANNEL_COUNT MAX_SUPPORTED_RC_PPM_CHANNEL_COUNT
#endif

extern const char rcChannelLetters[];

extern int16_t rcData[MAX_SUPPORTED_RC_CHANNEL_COUNT];       // interval [1000;2000]

#define MAX_MAPPABLE_RX_INPUTS 8

#define RSSI_SCALE_MIN 1
#define RSSI_SCALE_MAX 255
#define RSSI_SCALE_DEFAULT 100

typedef struct rxChannelRangeConfig_s {
    uint16_t min;
    uint16_t max;
} rxChannelRangeConfig_t;
PG_DECLARE_ARRAY(rxChannelRangeConfig_t, NON_AUX_CHANNEL_COUNT, rxChannelRangeConfigs);

typedef struct rxConfig_s {
    uint8_t receiverType;                   // RC receiver type (rxReceiverType_e enum)
    uint8_t rcmap[MAX_MAPPABLE_RX_INPUTS];  // mapping of radio channels to internal RPYTA+ order
    uint8_t serialrx_provider;              // Type of UART-based receiver (rxSerialReceiverType_e enum). Only used if receiverType is RX_TYPE_SERIAL
    uint8_t sbus_inversion;                 // default sbus (Futaba, FrSKY) is inverted. Support for uninverted OpenLRS (and modified FrSKY) receivers.
    uint8_t halfDuplex;                     // allow rx to operate in half duplex mode on F4, ignored for F1 and F3.
    uint8_t rx_spi_protocol;                // type of SPI receiver protocol (rx_spi_protocol_e enum). Only used if receiverType is RX_TYPE_SPI
    uint32_t rx_spi_id;
    uint8_t rx_spi_rf_channel_count;
    uint8_t spektrum_sat_bind;              // number of bind pulses for Spektrum satellite receivers
    uint8_t spektrum_sat_bind_autoreset;    // whenever we will reset (exit) binding mode after hard reboot
    uint8_t rssi_channel;
    uint8_t rssi_scale;
    uint8_t rssiInvert;
    uint16_t midrc;                         // Some radios have not a neutral point centered on 1500. can be changed here
    uint16_t mincheck;                      // minimum rc end
    uint16_t maxcheck;                      // maximum rc end
    uint16_t rx_min_usec;
    uint16_t rx_max_usec;
    uint8_t rcSmoothing;                    // Enable/Disable RC filtering
} rxConfig_t;

PG_DECLARE(rxConfig_t, rxConfig);

#define REMAPPABLE_CHANNEL_COUNT (sizeof(((rxConfig_t *)0)->rcmap) / sizeof(((rxConfig_t *)0)->rcmap[0]))

struct rxRuntimeConfig_s;
typedef uint16_t (*rcReadRawDataFnPtr)(const struct rxRuntimeConfig_s *rxRuntimeConfig, uint8_t chan); // used by receiver driver to return channel data
typedef uint8_t (*rcFrameStatusFnPtr)(void);

typedef struct rxRuntimeConfig_s {
    uint8_t channelCount;                  // number of rc channels as reported by current input driver
    timeUs_t rxRefreshRate;
    timeUs_t rxSignalTimeout;
    bool requireFiltering;
    rcReadRawDataFnPtr rcReadRawFn;
    rcFrameStatusFnPtr rcFrameStatusFn;
} rxRuntimeConfig_t;

extern rxRuntimeConfig_t rxRuntimeConfig; //!!TODO remove this extern, only needed once for channelCount
extern uint16_t rssi;

void rxInit(void);
bool rxUpdateCheck(timeUs_t currentTimeUs, timeDelta_t currentDeltaTime);
bool rxIsReceivingSignal(void);
bool rxAreFlightChannelsValid(void);
void calculateRxChannelsAndUpdateFailsafe(timeUs_t currentTimeUs);

void parseRcChannels(const char *input);

void updateRSSI(timeUs_t currentTimeUs);
void resetAllRxChannelRangeConfigurations(void);

void suspendRxSignal(void);
void resumeRxSignal(void);

uint16_t rxGetRefreshRate(void);
