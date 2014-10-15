#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */

#define USART_BAUDRATE 9600
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define BUFF_LEN 13

PROGMEM const char usbHidReportDescriptor[52] = { /* USB report descriptor, size must match usbconfig.h */
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x06,                    // USAGE (Keyboard)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
	0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
	0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,                    //   REPORT_SIZE (1)
	0x95, 0x08,                    //   REPORT_COUNT (8)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
	0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
	0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0xc0                           // END_COLLECTION
};

static const uchar  keyReport[256][2] PROGMEM = {
	{0, 0}, {0, 20}, {0, 0x2c}, {0, 0x28}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20},
	{0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20},
	{0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20},
	{0, 0x62}, {0, 0x60}, {0, 0x5c}, {0, 6}, {0, 0x5a}, {0, 4}, {0, 0x5e}, {0, 8}, {0, 0x59}, {0, 0x61}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20},
<<<<<<< Updated upstream

	{0, 20}, {0, 0x5d}, {0, 7}, {0, 0x5b}, {0, 5}, {0, 0x5f}, {0, 9}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20},
	{0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20},
	{0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20},
	{0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}, {0, 20}
=======
	{0, 20}, {0, 0x5d}, {0, 6}, {0, 0x5b}, {0, 5}, {0, 0x5f}, {0, 9}
>>>>>>> Stashed changes

};

static uchar    reportBuffer[2];
static uchar    idleRate;   /* repeat rate for keyboards, never used for mice */

static void buildReport(uchar key) {
	*(int *)reportBuffer = pgm_read_word(keyReport[key]);
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;

	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
		DBG1(0x50, &rq->bRequest, 1);   /* debug output: print our request */
		if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we only have one report type, so don't look at wValue */
			usbMsgPtr = (void *)&reportBuffer;
			buildReport(0);
			return sizeof(reportBuffer);
		}else if(rq->bRequest == USBRQ_HID_GET_IDLE){
			usbMsgPtr = &idleRate;
			return 1;
		}else if(rq->bRequest == USBRQ_HID_SET_IDLE){
			idleRate = rq->wValue.bytes[1];
		}
	}else{
		/* no vendor specific requests implemented */
	}
	return 0;   /* default for not implemented requests: return no data back to host */
}

void USART0Init(void) {
	UBRR0H = (uint8_t)(UBRR_VALUE>>8);
	UBRR0L = (uint8_t)UBRR_VALUE;
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
	UCSR0B |= (1<<RXEN0)|(0<<TXEN0);
}

char buffer[BUFF_LEN+1];
uchar bl = 0;
uchar bc = 0;

int __attribute__((noreturn)) main(void) {
	USART0Init();
	wdt_enable(WDTO_1S);

	usbInit();
	cli();
	usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
	_delay_ms(250);
	usbDeviceConnect();
	sei();

	OCR0A = 0xFF;
	TCCR0A = 0x02;
	TIFR0 |= 0x01;
	TIMSK0 = 0x01;
	TCCR0B = 0x05;

	uchar c;

	for(;;){                /* main event loop */
		while ( !(UCSR0A & (1<<RXC0)) ) {}
		c = UDR0;
		while (c == 2) {
			if (bl == BUFF_LEN) break;

			while (c != 3) {
				while ( !(UCSR0A & (1<<RXC0)) ) {}
				c = UDR0;
				buffer[bl] = c;
				bl ++;

				if (bl == BUFF_LEN) break;
			}
		}
	}
}

SIGNAL(TIMER0_OVF_vect)
{
	wdt_reset();
	usbPoll();

	if (bl) {
		if(usbInterruptIsReady()){
			if (bl <= bc) {
				bl = bc = 0;
				buildReport(0);
				usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
			} else {
				buildReport(buffer[bc]);
				usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));

				bc ++;
			}
		}
	}

}
