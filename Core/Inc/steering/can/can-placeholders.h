/*!
 * \file can-placeholders.h
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Messages the pages need that libcan does not define yet (D16, §5.7).
 *
 * \details Their UIDataSource exists and their page fields are wired, so
 *     the pages show "--" until the message arrives. When libcan adds one:
 *     put its frame ID in the router's source table, decode it in
 *     can_communications_router_api_receive_primary, add it to the FDCAN
 *     filters, and delete its line here. Search for TODO(libcan).
 *
 *     Expected (next libcan update):
 *     - Inverters: status (run / derate / fault), control enable as read by
 *       each inverter, DC link voltage, motor speed (rpm), delivered torque, version.
 *     - Tyre pressure, disc temperature (per wheel).
 *     - Cooling tanks: temperature and level, left and right.
 *     - Commanded brake bias.
 *     - Pedal sensor voltages: APPS 1-3, brake sensor.
 *     - GPS: latitude, longitude, speed.
 *     - AS computer status and version; selected mission (#11); EBS armed state.
 *     - PIPO: fuse status per line, charger input voltage.
 *     - Shutdown nodes: inverter interlock, SDC relay.
 *     - Suspension travel per corner (SuspensionFront/Rear carry heave and roll today).
 *     - LV state reason.
 */

#ifndef CAN_PLACEHOLDERS_H
#define CAN_PLACEHOLDERS_H

#include <stdint.h>

/*!
 * \brief Frame ID of a source whose message is not in libcan yet: matches no frame.
 */
#define CAN_PLACEHOLDER_MESSAGE_ID (UINT32_MAX)

#endif // CAN_PLACEHOLDERS_H
