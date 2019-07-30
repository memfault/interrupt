#ifndef SIMPLE_SERVICE_H
#define SIMPLE_SERVICE_H

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief   Macro for defining a ble_simple_service instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_SIMPLE_SERVICE_DEF(_name)                                                               \
static ble_simple_service_t _name;                                                                  \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     2,                                                     \
                     ble_simple_service_on_ble_evt, &_name)

// Simple service:                     E54B0001-67F5-479E-8711-B3B99198CE6C
//   Button 1 press characteristic:    E54B0002-67F5-479E-8711-B3B99198CE6C

// The bytes are stored in little-endian format, meaning the
// Least Significant Byte is stored first
// (reversed from the order they're displayed as)

// Base UUID: E54B0000-67F5-479E-8711-B3B99198CE6C
#define BLE_UUID_SIMPLE_SERVICE_BASE_UUID  {0x6C, 0xCE, 0x98, 0x91, 0xB9, 0xB3, 0x11, 0x87, 0x9E, 0x47, 0xF5, 0x67, 0x00, 0x00, 0x4B, 0xE5}

// Service & characteristics UUIDs
#define BLE_UUID_SIMPLE_SERVICE_UUID        0x0001
#define BLE_UUID_BUTTON_1_PRESS_CHAR_UUID   0x0002

typedef struct ble_simple_service_s ble_simple_service_t;

typedef enum
{
    BLE_BUTTON_1_PRESS_EVT_NOTIFICATION_ENABLED,
    BLE_BUTTON_1_PRESS_EVT_NOTIFICATION_DISABLED,
} ble_simple_evt_type_t;

typedef struct
{
    ble_simple_evt_type_t evt_type;
} ble_simple_evt_t;

typedef void (*ble_simple_evt_handler_t) (ble_simple_service_t * p_simple_service, ble_simple_evt_t * p_evt);

typedef struct ble_simple_service_s
{
    uint16_t                         conn_handle;
    uint16_t                         service_handle;
    uint8_t                          uuid_type;
    ble_simple_evt_handler_t         evt_handler;
    ble_gatts_char_handles_t         button_1_press_char_handles;

} ble_simple_service_t;

uint32_t ble_simple_service_init(ble_simple_service_t * p_simple_service, ble_simple_evt_handler_t app_evt_handler);

void ble_simple_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

void button_1_characteristic_update(ble_simple_service_t * p_simple_service, uint8_t *button_action);

#endif /* SIMPLE_SERVICE_H */
