#include "diag.h"
#include "main.h"

#include "objects.h"
#include "flash_api.h"

// Decide starting flash address for storing application data
// User should pick address carefully to avoid corrupting image section
#define FLASH_APP_BASE  0x7E000

void main(void)
{
    flash_t         flash;
    uint32_t        val32_to_write = 0x13572468;
    uint32_t        val32_to_read;
    uint32_t        address = FLASH_APP_BASE;

    int result = 0;

    flash_read_word(&flash, address, &val32_to_read);
    flash_erase_sector(&flash, address);
    flash_write_word(&flash, address, val32_to_write);
    flash_read_word(&flash, address, &val32_to_read);

    DBG_8195A("Read Data 0x%x\n", val32_to_read);

    // verify result
    result = (val32_to_write == val32_to_read) ? 1 : 0;
    printf("\r\nResult is %s\r\n", (result) ? "success" : "fail");

    for(;;);
}