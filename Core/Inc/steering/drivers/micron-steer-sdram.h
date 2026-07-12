#ifndef MICRON_STEER_H
#define MICRON_STEER_H

#include <stdint.h>

#define MICRON_STEER_SDRAM_REFRESH_COUNT (0x5C8U) /*!< SDRAM refresh counter */
#define MICRON_STEER_SDRAM_TIMEOUT (0xFFFFU)      /*!< SDRAM timeout for read/write access */

#define MICRON_STEER_SDRAM_BURST_LENGTH_1 (0x00000000U)             /*!< Burst Length 1 */
#define MICRON_STEER_SDRAM_BURST_LENGTH_2 (0x00000001U)             /*!< Burst Length 2 */
#define MICRON_STEER_SDRAM_BURST_LENGTH_4 (0x00000002U)             /*!< Burst Length 4 */
#define MICRON_STEER_SDRAM_BURST_LENGTH_8 (0x00000004U)             /*!< Burst Length 8 */
#define MICRON_STEER_SDRAM_BURST_TYPE_SEQUENTIAL (0x00000000U)      /*!< Burst Type Sequential */
#define MICRON_STEER_SDRAM_BURST_TYPE_INTERLEAVED (0x00000008U)     /*!< Burst Type Interleaved */
#define MICRON_STEER_SDRAM_CAS_LATENCY_2 (0x00000020U)              /*!< CAS Latency 2 */
#define MICRON_STEER_SDRAM_CAS_LATENCY_3 (0x00000030U)              /*!< CAS Latency 3 */
#define MICRON_STEER_SDRAM_OPERATING_MODE_STANDARD (0x00000000U)    /*!< Operating Mode Standard */
#define MICRON_STEER_SDRAM_WRITEBURST_MODE_PROGRAMMED (0x00000000U) /*!< Write Burst Mode Programmed */
#define MICRON_STEER_SDRAM_WRITEBURST_MODE_SINGLE (0x00000200U)     /*!< Write Burst Mode Single */

#define MICRON_STEER_SDRAM_NORMAL_MODE_CMD (0x00000000U)      /*!< Normal Mode */
#define MICRON_STEER_SDRAM_CLK_ENABLE_CMD (0x00000001U)       /*!< Clock Enable Command */
#define MICRON_STEER_SDRAM_PALL_CMD (0x00000002U)             /*!< Precharge All Command */
#define MICRON_STEER_SDRAM_AUTOREFRESH_MODE_CMD (0x00000003U) /*!< Auto Refresh Command */
#define MICRON_STEER_SDRAM_LOAD_MODE_CMD (0x00000004U)        /*!< Load Mode Command */
#define MICRON_STEER_SDRAM_SELFREFRESH_MODE_CMD (0x00000005U) /*!< Self Refresh Command */
#define MICRON_STEER_SDRAM_POWERDOWN_MODE_CMD (0x00000006U)   /*!< Power Down Command */

/*!
 * \brief MicronSteer SDRAM context structure definition
 *
 * This structure is used to store the configuration parameters for the MicronSteer SDRAM.
 * It includes information about the target bank, refresh mode, refresh rate, burst length, burst type, CAS latency, operation mode, and write burst mode.
 */
struct MicronSteerSdramContext {
    uint32_t target_bank;      /*!< Target Bank */
    uint32_t refresh_mode;     /*!< Refresh Mode */
    uint32_t refresh_rate;     /*!< Refresh Rate */
    uint32_t burst_length;     /*!< Burst Lengt */
    uint32_t burst_type;       /*!< Burst Type */
    uint32_t cas_latency;      /*!< CAS Latency */
    uint32_t operation_mode;   /*!< Operation Mode */
    uint32_t write_burst_mode; /*!< Write Burst Mode */
};

/*!
 * \brief MicronSteer SDRAM return code enumeration
 *
 * This enumeration defines the possible return codes for the MicronSteer SDRAM functions.
 */
enum MicronSteerSdramReturnCode {
    MICRON_STEER_SDRAM_RC_OK,    /*!< Operation is OK */
    MICRON_STEER_SDRAM_RC_ERROR, /*!< Operation is ERROR */
};

struct MicronSteerSdramCommand {
    uint32_t command_mode;             /*!< Command Mode */
    uint32_t target_bank;              /*!< Target Bank */
    uint32_t auto_refresh_number;      /*!< Auto Refresh Number */
    uint32_t mode_register_definition; /*!< Mode Register Definition */
};

/*!
 * \brief Callback used to wait a specified number of milliseconds
 *
 * \param[in] delay_ms The number of milliseconds to wait
 */
typedef void (*micron_steer_sdram_api_delay_callback)(uint32_t delay_ms);

/*!
 * \brief Callback used to send a command to the SDRAM
 *
 * \param[in] cmd The command to send
 *
 * \return The return code of the operation
 */
typedef enum MicronSteerSdramReturnCode (*micron_steer_sdram_api_send_command_callback)(struct MicronSteerSdramCommand *command);

/*!
 * \brief Callback used to program the refresh rate of the SDRAM
 *
 * \param[in] refresh_rate The refresh rate to program
 *
 * \return The return code of the operation
 */
typedef enum MicronSteerSdramReturnCode (*micron_steer_sdram_api_program_refresh_rate_callback)(uint32_t refresh_rate);

#endif /* MICRON_STEER_H */
