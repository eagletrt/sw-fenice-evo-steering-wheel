#include "test_flash.h"

#if EXTERNAL_FLASH_ENABLED == 1

uint8_t aTxBuffer[] = "ciao";

__IO uint8_t *mem_addr;
uint32_t address = 0;
uint16_t index1; /*index1 counter of bytes used when reading/writing 256 bytes buffer */
uint16_t index2; /*index2 counter of 256 bytes buffer used when reading/writing the 1Mbytes extended buffer */

void ReadMode(void);
void EnterQuadMode(void);
void EnableMemMappedQuadMode(void);

void print(const char *fmt, ...);

void Flash_Init()
{
    /* Enter Quad Mode 4-4-4 --------------------------------------------- */
    EnterQuadMode();
    ReadMode();
    /* Enable Memory mapped in Quad mode -------------------------------- */
    EnableMemMappedQuadMode();
    /* Writing and Reading Sequence of 1Mbyte --------------------------------------- */
    mem_addr = (__IO uint8_t *)(OCTOSPI1_BASE + address);
}

void Flash_Test()
{
    int errori;

    errori = manual_write_and_read(mem_addr, BUFFERSIZE, aTxBuffer);

    if (errori == 0)
    {
        print("Successful");
    }
    else
    {
        print("Error");
    }
    mem_addr = (__IO uint8_t *)(OCTOSPI1_BASE + address);

    errori = string_write_and_read(mem_addr, BUFFERSIZE, aTxBuffer);

    if (errori == 0)
    {
        print("Successful");
    }
    else
    {
        print("Error");
    }
}

int manual_write_and_read(__IO uint8_t *mem_addr, int BUFFERSIZE, uint8_t aTxBuffer[])
{
    uint16_t index1; /*index1 counter of bytes used when reading/writing 256 bytes buffer */
    __IO uint8_t *mem_addr_iniziale = mem_addr;

    for (index1 = 0; index1 < BUFFERSIZE; index1++)
    {
        *mem_addr = aTxBuffer[index1];
        mem_addr++;
    }
    mem_addr = mem_addr_iniziale;

    for (index1 = 0; index1 < BUFFERSIZE; index1++)
    {
        if (*mem_addr != aTxBuffer[index1])
        {
            return 1;
        }
        mem_addr++;
    }

    return 0;
}

int string_write_and_read(__IO uint8_t *mem_addr, int BUFFERSIZE, uint8_t aTxBuffer[])
{
    uint8_t *addr = mem_addr;
    memcpy(addr, aTxBuffer, BUFFERSIZE);

    uint8_t confronto[BUFFERSIZE];

    memcpy(confronto, mem_addr, BUFFERSIZE);

    if (memcmp(confronto, aTxBuffer, BUFFERSIZE) != 0)
    {
        return 1;
    }

    return 0;
}

/*Function to Enable Memory mapped mode in Quad mode 4-4-4*/
void EnableMemMappedQuadMode(void)
{
    OSPI_RegularCmdTypeDef sCommand;
    OSPI_MemoryMappedTypeDef sMemMappedCfg;
    sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
    sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
    sCommand.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
    sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = HAL_OSPI_DATA_4_LINES;
    sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
    sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    sCommand.Address = 0;
    sCommand.NbData = 1;
    /* Memory-mapped mode configuration for Quad Read mode 4-4-4*/
    sCommand.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
    sCommand.Instruction = FAST_READ_QUAD;
    sCommand.DummyCycles = FAST_READ_QUAD_DUMMY_CYCLES;
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) !=
        HAL_OK)
    {
        Error_Handler();
    }
    /* Memory-mapped mode configuration for Quad Write mode 4-4-4*/
    sCommand.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
    sCommand.Instruction = QUAD_WRITE;
    sCommand.DummyCycles = WRITE_QUAD_DUMMY_CYCLES;
    sCommand.DQSMode = HAL_OSPI_DQS_ENABLE;
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) !=
        HAL_OK)
    {
        Error_Handler();
    }
    /*Disable timeout counter for memory mapped mode*/
    sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;
    /*Enable memory mapped mode*/
    if (HAL_OSPI_MemoryMapped(&hospi1, &sMemMappedCfg) != HAL_OK)
    {
        Error_Handler();
    }
}
/*Function to configure the external memory in Quad mode 4-4-4*/
void EnterQuadMode(void)
{
    OSPI_RegularCmdTypeDef sCommand;
    sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction = ENTER_QUAD_MODE;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles = ENTER_QUAD_DUMMY_CYCLES;
    sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    /*Enter QUAD mode*/
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) !=
        HAL_OK)
    {
        Error_Handler();
    }
}

void ReadMode(void)
{
    /*
    OSPI_RegularCmdTypeDef sCommand;
    sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction = 0x4B;
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
    sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles = ENTER_QUAD_DUMMY_CYCLES;
    sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) !=
        HAL_OK) {
      Error_Handler();
    }
    */

    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET);
    uint8_t write_data[5] = {0x4B, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t read_data[8] = {0};
    HAL_OSPI_Transmit(&hospi1, write_data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
    HAL_OSPI_Receive(&hospi1, read_data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
    print("read data: %x\n", read_data[0]);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET);
}

#endif
