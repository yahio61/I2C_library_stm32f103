/*
 * agile_i2c.c
 *
 */
#include "agile_i2c.h"
#include "main.h"
#include "stdint.h"





/**
  * @brief 	Bu fonksiyon I2C1'i baslatir.
  *
  * @param 	secilenMod: SM_mod (100khz) veya FM_mod (400khz) hilzari secilebilir
  * 		PCLK1: PCLK1'in frekansi buraya yazilir.
  *
  * @retval Yok
  */

void i2c_baslat(enum mod_sec secilenMod, int8_t PCLK1)
{

	// Tpclk = 41.66 clock periyodu


	/* Clock hatlarinin acilmasi */
	RCC->APB1ENR |= (1 << 21); 								// I2C1 clock hattini aktiflestir
	RCC->APB2ENR |= (1<<3);  								// GPIOB clock hattini aktiflestir
	HAL_Delay(5);											// 5ms bekle

	/* GPIOB'nin ayarlanmasi */
	GPIOB->CRL |= (3<<26) | (3<<24) | (3<<28) | (3<<30); 	// PB6 icin alternatif fonksiyon output open drain 50khz modu
															// PB7 icin alternatif fonksiyon output open drain 50khz modu

	/* I2C1'in ayarlanmasi */
	I2C1->CR1 |= (1<<15);  						// I2C1 hattini resetle

	I2C1->CR2 |= (PCLK1 << 0);					// APB1 PCLK1 frekansi.

	if(secilenMod == 0)
	{
		I2C1->CR1 &= ~(1<<15);  				// Reset kaldir, I2C1 Standard mod (max 100khz)

		int16_t periyot_PCLK = 0;
		periyot_PCLK = 1000 / PCLK1;			// PCLK1'in periyodu nano saniye cinsine cevrildi
		I2C1->CCR |= 5000 / PCLK1; 				// Thigh periyodu 5000 ns saniye oldugu icin 100 ile carpildi
		I2C1->TRISE |=(1000/periyot_PCLK)+1;	// Rise periyodu maximum 1000 ns. Bu yuzden 300 ile carpildi
	}
	else if(secilenMod == 1)
	{
		I2C1->CR1 &= ~(1<<15);  				// Reset kaldir
		I2C1->CCR |= (1<<15);					// I2C1 Fast mod (max 400khz)
		I2C1->CCR |= (1<<14);					// Fast mod Duty Cycle 1 => Thigh/Tlow = 9/16

		int16_t periyot_PCLK = 0;
		periyot_PCLK = 1000 / PCLK1;			// PCLK1'in periyodu nano saniye cinsine cevrildi
		I2C1->CCR |= 100 / periyot_PCLK;		// Thigh periyodu 100 ns saniye oldugu icin 100 ile carpildi
		I2C1->TRISE |=(300/periyot_PCLK)+1;		// Rise periyodu maximum 300 ns. Bu yuzden 300 ile carpildi
	}

	I2C1->CR1 |= (1<<0); 						// I2C1'i baslat

}




/**
  * @brief 	Bu fonksiyon istenilen chip'in istenilen adresine veri yazmayi saglar.
  *
  * @param 	slaveAdd: veri gonderilmek istenen chip'in adresidir.
  * 		regAdd: chip'in veri yazilmak istenen registerinin adresidir.
  * 		veri: yazilmak istenen veridir. 8 bit olmalidir.
  *
  * @retval Yok
  */

void i2c_yaz(int8_t slaveAdd, int8_t regAdd, int8_t veri )
{
	I2C1 -> CR1 |= 0x0100;                  	// Start biti gonder
	while (!(I2C1 -> SR1 & 0x0001));   			// Start bitinin gonderilmesini bekle

	I2C1 -> DR = slaveAdd ;                 	// Slave adresini gonder
	while (!(I2C1 -> SR1 & 0x0002));    		// Adresin gonderilmesini bekle
	int temp = I2C1 -> SR2;                		// Flag temizlemek icin SR2 register'ini oku
	UNUSED(temp);

	I2C1 -> DR = regAdd;                  		// Register adresini  gonder
	while (!(I2C1 -> SR1 & 0x0080));   			// DR register'inin bosalmasini bekle
	while (!(I2C1 -> SR1 & 0x0004));   			// Byte'in gonderilmesini bekle

	I2C1 -> DR = veri;  	          			// Veri'yi gönder
	while (!(I2C1 -> SR1 & 0x0080));   			// DR register’inin bosalmasini bekle
	while (!(I2C1 -> SR1 & 0x0004));   			// Byte’in gonderilmesini bekle

	I2C1->CR1 |=  (1<<9);						// Stop biti gonder.

}




/**
  * @brief 	Bu fonksiyon istenilen chip'in istenilen adresinden istenilen sayida
  * 		veri alinmasini saglar.
  *
  * @param 	slaveAdd: veri alinmak istenen chip'in adresidir.
  * 		regAdd: chip'in veri alinmak istenen register'inin adresidir.
  * 		byteSayisi: alinmak istenen byte sayisi.
  * 		veri: alinan verilerin yazilacagi dizinin ilk teriminin adresi.
  *
  * @retval Yok
  */

void i2c_veriAl(int8_t slaveAdd, int8_t regAdd, int8_t byteSayisi, char *veri)
{
	I2C1 -> CR1 |= 0x0100;                  	// Start biti gonder
	while (!(I2C1 -> SR1 & 0x0001));   			// Start bitinin gonderilmesini bekle

	I2C1 -> DR = slaveAdd ;	         			// Slave adresini gonder
	while (!(I2C1 -> SR1 & 0x0002));    		// Adresin gonderilmesini bekle
	int temp = I2C1 -> SR2;                		// Flag temizlemek icin SR2 register’ini oku

	I2C1 -> DR = regAdd;                    	// Chip adresini  gönder
	while (!(I2C1 -> SR1 & 0x0080));   			// DR register’inin bosalmasini bekle
	while (!(I2C1 -> SR1 & 0x0004));   			// Byte’in gonderilmesini bekle

	I2C1-> CR1 |= 0x0100;                  		// Start biti gönder
	while (!(I2C1 -> SR1 & 0x0001));  			// Start bitinin gonderilmesini bekle

	I2C1 -> DR = slaveAdd | 1;		 			// Slave adresini gonder
	while (!(I2C1 -> SR1 & 0x0002));  			// Adresin gonderilmesini bekle
	temp = I2C1 -> SR2;             			// Flag temizlemek icin SR2 register’ini oku

	I2C1->CR1 |= 0x400;							// I2C1 ACK acildi

	while (byteSayisi > 0)
	{
		if(byteSayisi == 1)
		{
			I2C1->CR1 &= ~(1<<10);				// I2C1 ACK kapat
			I2C1->CR1 |=  (1<<9);				// Stop biti gonder
			while(!(I2C1->SR1 & (1<<6) )); 		// Dr register'ine veri gelmesini bekle
			*(veri++) = I2C1->DR;  				// Gelen byte'i diziye yaz
			break;
		}

		else
		{
			while(!(I2C1->SR1 & (1<<6) ));		// Dr register'ine veri gelmesini bekle
			*(veri++) = I2C1->DR;				// Gelen byte'i diziye yaz
		}

		byteSayisi--;							// Byte sayisini 1 eksilt
	}
}
