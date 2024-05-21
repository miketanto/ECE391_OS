#include "rtc.h"
// Reference: https://wiki.osdev.org/RTC

volatile int flag = 1;               //init the flag to 1 for read

/* rtc_init
 *   Description: Initialize the RTC by sending initialization commands to the RTC controller,
 *                and enable the RTC interrupt on the PIC at IRQ8
 *   Inputs: none
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Initialize the RTC, enable the RTC interrupt on the PIC at IRQ8
*/
void rtc_init(void) {
    uint8_t rate =  0x06;           //RATE=1024 HZ              //
    
    enable_irq(RTC_IRQ_NUM);        // enable RTC interrupt on PIC    //
    
    outb(REG_B, RTC_REG_NUM);       // select register B, and disable NMI
    char prev = inb(CMOS_RW);       // read the current value of register B
    outb(REG_B, RTC_REG_NUM);       // set the index again (a read will reset the index to register D)
    outb( prev | 0x40, CMOS_RW);     // write the previous value ORed with 0x40. This turns on bit 6 of register

    outb(REG_A, RTC_REG_NUM);       //set to reg A               //
    prev=inb();                     //init val of A             //
    outb(REG_A, RTC_REG_NUM);
    outb( (prev & 0xF0) | rate, CMOS_RW); // Set frequency to 1024;
    flag =1;                        //init the flag to 1 for read
};


/* rtc_handler
 *   Description: Handle the RTC interrupt, throw away contents for now
 *   Inputs: none
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Throw away RTC interrupt contents for now
*/
void rtc_handler(void) {            // we have to read reg C even if we don't care about the interrupt
    outb(REG_C, RTC_REG_NUM);       // select register C
    inb(CMOS_RW);                   // TODO: throw away contents for now
    //printf("r");
    flag =1;                        //flag =1 shows that interrupt happened
    send_eoi(RTC_IRQ_NUM);          // send EOI to PIC
}

/* rtc_open
 *   Description: Initializes RTC frequency to 2HZ
 *   Inputs: filename
 *   Outputs: none
 *   Return Value: 0
*/
int32_t rtc_open( const uint8_t* filename){ 
    cli();
    int8_t rate = 0x0F;
    outb(REG_A, RTC_REG_NUM);                   // select register A, and disable NMI
    char prev = inb(CMOS_RW);                   // read the current value of register A
    outb(REG_A, RTC_REG_NUM);                   // set the index again (a read will reset the index to register D)
    outb( (prev & 0xF0) | rate, CMOS_RW);       // write only our rate to A 
    sti();
    return 0;
}

/* rtc_close
 *   Description: Dose nothing 
 *   Inputs: fd
 *   Outputs: none
 *   Return Value: 0
*/
int32_t rtc_close(int32_t fd){
    return 0;
}

/* rtc_read
 *   Description: Block until the next RTC interrupt
 *   Inputs: fd, buf, nbytes
 *   Outputs: none
 *   Return Value: 0
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    //printf("in read");    
    flag=0;                 //set flag to 0 to wait for interrup to change it
    while(flag !=1){}       //rtc intr will call ahndler and set flag to 1 to exit loop
    return 0;
}

/* rtc_write
 *   Description: Will change the frequency to given valid frequency
 *   Inputs: fd, buf, nbytes
 *   Outputs: none
 *   Return Value: 0 if frequency was succesfully changged, -1 if frequency was unable to be changed
*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    int freq =*((uint32_t*)buf);                             //get the frequency in int form from the buffer
    int x;                                                      //x will hold the return value
    if(((freq & (freq-1)) !=0) | (freq==0) | (freq > 1024)){     //if the freq is not a power of 2, or is =0, or is greater than 1024, the freq is invalid
        x=-1;
    }
    else{                       //if its a valid freq we can change the freq
        if(freq==2){            //find freq to change to and set freq to the correct value, same for all the fallowing if statements
            freq = 0x0F;
        }
        else if(freq==4){
            freq = 0x0E;
        }
        else if(freq ==8){
            freq = 0x0D;
        }
        else if(freq == 16){
            freq = 0x0C;
        }
        else if(freq== 32){
            freq = 0x0B;
        }
        else if(freq==64){
            freq = 0x0A;
        }
        else if(freq==128){
            freq=0x09;
        }
        else if(freq == 256){
            freq=0x08;
        }
        else if(freq == 512){
            freq =0x07;
        }
        else{
            freq = 0x06;
        }
        //change freq here
        cli();
        outb(REG_A, RTC_REG_NUM);                 // select register A, and disable NMI
        char prev = inb(CMOS_RW);                 // read the current value of register A
        outb(REG_A, RTC_REG_NUM);                 // set the index again (a read will reset the index to register D)
        outb( (prev & 0xF0) | freq, CMOS_RW);     // write only our rate(freq) to A 
        sti();
        x=0;                    //set return value to 0
    }
    return x;

}
