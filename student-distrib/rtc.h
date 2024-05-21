#include "types.h"
#include "lib.h"
#include "i8259.h"

#define RTC_IRQ_NUM 8
#define RTC_REG_NUM 0x70        // used to specify an index or "register number", and to disable NMI
#define CMOS_RW     0x71        // used to read or write from/to that byte of CMOS configuration space

#define REG_A       0x8A
#define REG_B       0x8B        // RTC register B
#define REG_C       0x8C        // RTC register C

// Initialize RTC
void rtc_init(void);

// RTC interrupt handler
extern void rtc_handler(void);

//RTC open
int32_t rtc_open( const uint8_t* filename);

//RTC close
int32_t rtc_close(int32_t fd);

//RTC read
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

//RTC write
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
