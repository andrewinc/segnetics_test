/** RGB LED */
#define G_R 5
#define G_G 6
#define G_B 7

// pins for USB read-write
#define RX 3
#define TX 2
#define RX_INT 0
#define TX_INT 1


//delay(ms) for USB read-write
#define delay_tx_rx 3000


// analog pin for read voltage
#define EXT 5

// dip and buttons
#define ALARM 8
#define ERR_ON 9
#define ERR_OFF 10


//MODE CONST
#define M_NONE 0
#define M_OK 1
#define M_USB 2
#define M_USB_TXRX 3
#define M_ALARM 4
#define M_NO_Q 5

int mode;   //текущий режим
volatile unsigned long time_tr_rx; //время последнего измненения TX или RX
volatile unsigned long lasttime; //последнее время
bool msg_act;//журнал. имеется ли неквитированное событие

void setup() {
  mode=M_NONE;
  
  pinMode(G_R, OUTPUT);
  pinMode(G_G, OUTPUT); 
  pinMode(G_B, OUTPUT);
  
  pinMode(RX, INPUT);
  pinMode(TX, INPUT);
  
  pinMode(ALARM, INPUT);
  pinMode(ERR_ON, INPUT); 
  pinMode(ERR_OFF, INPUT);
  msg_act=false;

  time_tr_rx=0;
  attachInterrupt(TX_INT, interrupt_tx, CHANGE);
  attachInterrupt(RX_INT, interrupt_rx, CHANGE);
}


//при изменениий на пинах TX и RX запоминаем время срабатывания в time_tr_rx
void interrupt_txrx(){
  time_tr_rx=millis();
}
void interrupt_tx(){ interrupt_txrx(); }
void interrupt_rx(){ interrupt_txrx(); }


/** true если внешнее питание. не USB */
bool is_external(){ 
  analogReference(EXTERNAL); 
  int ext= analogRead(EXT); 
  analogReference(DEFAULT); 
  int def= analogRead(EXT); 
  return (ext==def) && (1023==def); 
}


void check(){
  //возбуждение ошибки и квитирование
  if (HIGH==digitalRead(ERR_ON)) msg_act=true; //возбуждение (в журнал)
  else
    if (HIGH==digitalRead(ERR_OFF)) msg_act=false;//квитирование

  //сигнал активной ошибки => mode
  if (HIGH==digitalRead(ALARM)) mode=M_ALARM;
  else { 
    //сигнал неквитированной ошибки => mode
    if(msg_act) mode=M_NO_Q;
    else {//зел. M_USB, зел.миг.M_USB_TXRX или синий M_OK
      if (is_external()) mode=M_OK;
      else {
        //в time_tr_rx время последнего изменения TX или RX. Если прошло больше чем delay_tx_rx то сбросить время
        if (lasttime>(time_tr_rx+delay_tx_rx)) time_tr_rx=0;
        
        //Если время !=0 то индикация передачи
        mode=(0!=time_tr_rx)?M_USB_TXRX:M_USB;
      }//передача USB или ожидание
    }//всё квитировано  
  }//!ALARM  
}

void loop() {
  
  //период индикации
  switch(mode) {
    case M_OK:       digitalWrite(G_R,HIGH); digitalWrite(G_G,HIGH);digitalWrite(G_B,LOW);  break;
    case M_USB:      digitalWrite(G_R,HIGH); digitalWrite(G_G,LOW); digitalWrite(G_B,HIGH); break;
    case M_USB_TXRX: digitalWrite(G_R,HIGH); digitalWrite(G_G,LOW); digitalWrite(G_B,HIGH); 
      delay(10); digitalWrite(G_G,HIGH); delay(100);
    break;//TXRX
    
    case M_ALARM:  digitalWrite(G_R,LOW); digitalWrite(G_G,HIGH); digitalWrite(G_B,HIGH); 
      delay(10); digitalWrite(G_R,HIGH); delay(100);
    break;//красн. часто мерцает активная ошибка
    
    case M_NO_Q:  digitalWrite(G_R,LOW); digitalWrite(G_G,HIGH); digitalWrite(G_B,HIGH); 
      delay(10); digitalWrite(G_R,HIGH); delay(1000);
    break;//красн. редко мерцает неквитир. запись
   
    case M_NONE:
    default:          digitalWrite(G_R,HIGH); digitalWrite(G_G,HIGH);digitalWrite(G_B,HIGH); break;
  }
  lasttime=millis();
  check();
}
