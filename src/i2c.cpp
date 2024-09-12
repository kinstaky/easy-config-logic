#include "i2c.h"

#include "stdio.h"

const int I2CWAIT = 2;
const int AI2CREG = 0;
const int AI2COUT = 1;


void I2C_Start(volatile uint32_t *mapped)  {
	// printf("I2C Start\n");
	// I2C start
	uint32_t mval = 7;   // SDA = 1; SCL = 1; CTRL = 1
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);

	mval = 6;   // SDA = 0; SCL = 1; CTRL = 1
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);

	return;
}


void I2C_Stop(volatile uint32_t *mapped)  {
	// printf("I2CStop\n");
	// I2C stop
	uint32_t mval = 4;   // SDA = 0; SCL = 0; CTRL = 1
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);

	mval = 6;   // SDA = 0; SCL = 1; CTRL = 1
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);

	mval = 7;   // SDA = 1; SCL = 1; CTRL = 1
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);

	return;
}

int I2C_Slave_Ack(volatile uint32_t *mapped) {
	// printf("I2C slave ack\n");
	// I2C acknowledge
	uint32_t mval = 0x0000;   // clear SCL and CTRL to give slave control of SDA
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);

	mval = 2;   // set SCL
	mapped[AI2CREG] = mval;
	// usleepWrap(I2CWAIT, mapped);
	usleep(I2CWAIT);

	mval = mapped[AI2COUT];
	// printf("%d    %d    %d    %d\n", mval & 0x1, (mval & 0x2)>>1, (mval & 0x4)>>2, (mval & 0x8)>>3);
	int ret = (mval & 0x8) >> 3;
	usleep(I2CWAIT);

	mval = 0x0000;   // clear SCL and CTRL to give slave control of SDA
	// for PLL
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);
	// now can read SDA bit for ACK

	return ret;
}

void I2C_Master_Ack(volatile uint32_t *mapped) {
	// I2C acknowledge
	uint32_t mval = 0x0004;   // clear SCL and SDA but not CTRL to keep control of SDA
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);

	mval = 6;   // set SCL
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);

	return;
}

void I2C_Master_No_Ack(volatile uint32_t *mapped) {
	// I2C acknowledge
	uint32_t mval = 0x0004;   // clear SCL and SDA but not CTRL to keep control of SDA
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);

	mval = 3;   // set SCL  and SDA
	mapped[AI2CREG] = mval;
	usleep(I2CWAIT);
	return;
}



void I2C_Byte_Send(volatile unsigned int *mapped, uint8_t data) {
	// I2C byte send
	// SDA is captured during the low to high transition of SCL
	uint32_t mval = 4;   // SDA = 0; SCL = 0; CTRL = 1
	for (size_t i = 0; i < 8; ++i) {
		// printf("Sending a bit\n");
		mval = mval & 0x0005;   // clear SCL
		mapped[AI2CREG] = mval;
		usleep(I2CWAIT);

		if (data & (0x1 << (7-i))) mval = 5;	// SDA = 1; SCL = 0; CTRL = 1
		else mval = 4;	   // SDA = 0; SCL = 0; CTRL = 1
		mapped[AI2CREG] = mval;
		usleep(I2CWAIT);

		mval = mval | 0x0002;   // set SCL
		mapped[AI2CREG] = mval;
		usleep(I2CWAIT);
   }

	// for PLL
	mval = mval & 0x0005;   // clear SCL
	mapped[AI2CREG] = mval;
	return;
}


void Enable_Rj45(volatile uint32_t *mapped, uint32_t index, uint8_t enable) {
	I2C_Start(mapped);

	uint8_t address;
	switch (index) {
		case 0:
			address = 0b01001000;
			break;
		case 1:
			address = 0b01000000;
			break;
		case 2:
			address = 0b01001010;
			break;
		case 3:
			address = 0b01000010;
			break;
		case 4:
			address = 0b01001100;
			break;
		case 5:
			address = 0b01000100;
			break;
		default:
			fprintf(stderr, "Error: Invalid index %u\n", index);
			return;
	}
	I2C_Byte_Send(mapped, address);
	I2C_Slave_Ack(mapped);

	I2C_Byte_Send(mapped, enable);
	I2C_Slave_Ack(mapped);

	I2C_Stop(mapped);
}