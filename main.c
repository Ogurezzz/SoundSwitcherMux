#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t period=0;
uint32_t recievedData=0;

void NextChannel(){
    uint8_t chanSel = PORTB & 0x07;
    chanSel++;
    if (chanSel<4) chanSel=4;
    if (chanSel>7) chanSel=0;
    PORTB = ((PORTB & 0xf8)|chanSel);
}
void SwitchOff(){
    if(PORTB & 0x07){
        PORTB = ((PORTB & 0xf8)|0x00);
    }else{
        PORTB = ((PORTB & 0xf8)|0x04);
    }
}

//Прерывание по сигналу на кнопке или ИК-приемнику.
ISR(PCINT0_vect){
    if(PINB&(1<<PINB3)){
        NextChannel();
        _delay_ms(500);
    }else{
        if((PINB&(1<<PINB4))==0){
            uint8_t ticks = TCNT0;
            TCCR0B |= (3<<CS00);        //Включаем/перевключаем таймер. Предделитель 64
            if (ticks>80){              //Если это длительный пакет на входе. >5мс
                recievedData=0;         //Обнуляем данные
            }else if (ticks>36){           //Пакет >2мс, но меньше 5
                recievedData = (recievedData<<1)|1;
            }else if (ticks>5){
                recievedData = (recievedData<<1)|0;
            }
            TCNT0=0;                    //Обнуляем счетчик.
        }
        //_delay_ms(200);
    }
}

//Прерывание по переполнению таймера.
ISR (TIM0_OVF_vect){
    //static uint32_t lastData = 0;
    TCCR0B &= ~(7<<CS00);                //Выключаем таймер.
    TCNT0=0;
    //if (lastData==recievedData){
        switch (recievedData)
        {
        case 0x18000:
            NextChannel();
            break;
        case 0x0E0E040BF:
            SwitchOff();
            break;
        default:
            //if(recievedData) NextChannel();
            break;
        }
        //lastData = 0;
    //}else{
    //    lastData = recievedData;
    //}
    recievedData=0;
    _delay_ms(500);
}

int main(void)
{
    //Настраиваем прерывание
    SREG |=  (1<<7);
    GIMSK = (1<<PCIE);                  //Включаем прерывание по изменению пина
    PCMSK = (1<<PCINT3)|(1<<PCINT4);    //На порту PB4 (IRприемник) и PB3 (по кнопке).
    //Настройка таймера
    TCCR0B |= (4<<CS00);                //Предделитель 256
    TIMSK0 |= (1<<TOIE0);               //Прерывание по переполнению.
    //Настройка порта   
    DDRB = 0x07;                        //PB0,PB1,PB2 - На выход. Остальное на вход.
    //_delay_ms(50);
    PORTB = 0x10;                       //Вкл. Подтяжку для ИК приемника.

    while(1){
    }
}

