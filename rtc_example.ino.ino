#define RCC_APB1ENR_PWREN (1 << 28)
#define RCC_LSI_ON (1 << 0)
#define RCC_RTC_LSI (1 << 9)
#define RCC_BDCR_RTCSEL (3 << 8)
#define PWR_CR_DBP (1 << 8)
#define RTC_EN (1 << 15)
#define RTC_INIT (1 << 0)

#define TIMEOUT 100000

int number_to_bcd(int number) {
    
    int temp = number % 10;
    temp += (number / 10) * 16;

    return temp;
}

int bcd_to_number(int bcd) {
    
    int temp = bcd & 0xF;
    temp += ((bcd >> 4) & 0xF) * 10;
    return temp;
}



void getTime(unsigned long *s, unsigned long *m, unsigned long *h) {
   unsigned long time_reg = RTC->TR;
   unsigned long secs = time_reg & 0x7F;
   unsigned long mins = (time_reg & 0x7F00) >> 8;
   unsigned long hours = (time_reg & 0x7F0000) >> 16;
   *s = bcd_to_number(secs);
   *m = bcd_to_number(mins);
   *h = bcd_to_number(hours);
}

void setTime(unsigned long s, unsigned long m, unsigned long h) {
   unsigned long time_reg = 0;

   time_reg = number_to_bcd(s);
   time_reg |= (number_to_bcd(m) << 8);
   time_reg |= (number_to_bcd(h) << 16);

   RTC->TR = time_reg;
}

void setup() {
  Serial.begin(9600);
  unsigned long timeout = TIMEOUT;
  // put your setup code here, to run once:
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  RCC->CSR |= RCC_LSI_ON; // LSI ON
  PWR->CR |= PWR_CR_DBP; // enable RTC access

  while (((PWR->CR & PWR_CR_DBP) == 0) && timeout--);

  if (timeout == 0) {
    Serial.print("Timeout");
  }
  timeout = TIMEOUT;
  
  unsigned long tmpreg1 = (RCC->BDCR & RCC_BDCR_RTCSEL);

  if (tmpreg1 == 0) {
      tmpreg1 = (RCC->BDCR & ~(RCC_BDCR_RTCSEL));
      /* RTC Clock selection can be changed only if the Backup Domain is reset */
      RCC->BDCR |= (1 << 16);
      RCC->BDCR &= ~(1 << 16);
      /* Restore the Content of BDCR register */
      RCC->BDCR = tmpreg1;
  }

  RCC->CSR |= RCC_RTC_LSI;
  
  RCC->BDCR &= ~(RTC_EN); // disable RTC 
  RCC->BDCR |= (RCC_RTC_LSI | RTC_EN); // enable RTC

  RTC->WPR = 0xCA; // disable write protection
  RTC->WPR = 0x53; // disable write protection

  if ((RTC->ISR & RTC_ISR_INITF) == 0) {
    RTC->ISR = RTC_INIT;

    while (((RTC->ISR & RTC_ISR_INITF) == 0) && timeout--);
  }
  // LSI frew is 32kHz, generate clock 1Hz
  RTC->PRER = (128 << 16) | (256); 

  setTime(0, 0, 0);
}


void loop() {
  // put your main code here, to run repeatedly:
  unsigned long s, m, h;
  char buf1[20];
  
  getTime(&s, &m, &h);

  Serial.print("Hodiny: ");
  sprintf(buf1, "%02d:%02d:%02d",  h, m, s);
  Serial.print(buf1);
  Serial.println();
  delay(100);

}
