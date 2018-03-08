#define MIDICHANNEL   0
#define MAXSENSE     64
#define INP_PINS      8
#define OUT_PINS     10
#define SUS_PED      28  
#define LED_PIN      13

//tabla para seleccionar entrada del mux
int muxTable[16][4] = {{LOW ,LOW ,LOW ,LOW},
                       {HIGH,LOW ,LOW ,LOW},
                       {LOW ,HIGH,LOW ,LOW},
                       {HIGH,HIGH,LOW ,LOW},
                       {LOW ,LOW ,HIGH,LOW},
                       {HIGH,LOW ,HIGH,LOW},
                       {LOW ,HIGH,HIGH,LOW},
                       {HIGH,HIGH,HIGH,LOW},
                       {LOW ,LOW ,LOW ,HIGH},
                       {HIGH,LOW ,LOW ,HIGH},
                       {LOW ,HIGH,LOW ,HIGH},
                       {HIGH,HIGH,LOW ,HIGH},
                       {LOW ,LOW ,HIGH,HIGH},
                       {HIGH,LOW ,HIGH,HIGH},
                       {LOW ,HIGH,HIGH,HIGH},
                       {HIGH,HIGH,HIGH,HIGH}};

/*
//0=ciclos
//1=notificado
//2=nota
*/
int pads[ OUT_PINS/2 ][ INP_PINS*2 ][ 3 ];

int grp_id ;

void setup(void){

    //abrimos los puertos digitales para controlar el mux
    int i;
    for( i=2; i<6; i++ )
        pinMode(i,OUTPUT);
    
    //abrimos los puertos digitales para energizar las teclas
    for( i=0; i<10; i++ ){
        pinMode(i+30,OUTPUT);
        digitalWrite(i+30,LOW);
    }
    
    //pin de entrada primer MUX
    pinMode( 6, INPUT );

    //tabla de notas midi
    int midi_num=24;//partimos desde el C1
    for( grp_id=0; grp_id<OUT_PINS/2; grp_id++ )
        for( i=0; i<INP_PINS; i ++ ){
            pads[grp_id][i][0] = pads[grp_id][i][1] = 0;
            pads[grp_id][i][2] = midi_num++;
            pads[grp_id][i+INP_PINS][0] = pads[grp_id][i+INP_PINS][1] = 0;
            pads[grp_id][i+INP_PINS][2] = midi_num+39;
        }
    grp_id = 4;

    //me molesta que esté siempre encendido
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN,LOW);

    //sustain pedal
    pinMode( SUS_PED, INPUT_PULLUP  );

    //baudios MIDI
    Serial.begin(31250);
    //Serial.begin(57600);
    //Serial.begin(230400);
}

int sustainPedal = LOW;
int pin_i;

void loop() {

    if( digitalRead( SUS_PED ) != sustainPedal )
        MIDI_TX(176,64,127-(sustainPedal = !sustainPedal)*127);
        
    if(  ++grp_id == 5 )
        grp_id = 0;

    //subimos el primer switch para las ocho entradas
    digitalWrite(grp_id*2+30,HIGH);
  
    //y leemos el primer switch
    for( pin_i=0; pin_i<INP_PINS; pin_i++ ){
        //primera mitad del teclado
        if( !digitalRead_(pin_i) ){
            //damos de baja la tecla
            if( pads[grp_id][pin_i][0] && pads[grp_id][pin_i][1] ){
                MIDI_TX(144,pads[grp_id][pin_i][2],0);
            }
            pads[grp_id][pin_i][0] = pads[grp_id][pin_i][1] = 0;
        }else{  //incrementamos la cantidad de ciclos de la tecla
            pads[grp_id][pin_i][0] = min( pads[grp_id][pin_i][0] + 1 , MAXSENSE );
        }
        //segunda mitad del teclado
        if( !digitalRead_(pin_i+INP_PINS) ){
            if( pads[grp_id][pin_i+INP_PINS][0] && pads[grp_id][pin_i+INP_PINS][1] ){
                //NOTE OFF
                //MIDI_TX(144,pads[grp_id][pin_i+INP_PINS][2],0);
                MIDI_TX(128,pads[grp_id][pin_i+INP_PINS][2],0);
            }
            pads[grp_id][pin_i+INP_PINS][0] = pads[grp_id][pin_i+INP_PINS][1] = 0;
        }else{  //incrementamos la cantidad de ciclos de la tecla
            pads[grp_id][pin_i+INP_PINS][0] = min( pads[grp_id][pin_i+INP_PINS][0] + 1 , MAXSENSE );
        }
    }
    
    //bajamos el primer switch para las ocho entradas
    digitalWrite(grp_id*2+30,LOW);
    //subimos el segundo switch para las ocho entradas
    digitalWrite(grp_id*2+31,HIGH);
  
    //y leemos el segundo switch
    for( pin_i=0; pin_i<INP_PINS; pin_i++ ){
        //primera mitad del teclado        
        if( digitalRead_(pin_i) && !pads[grp_id][pin_i][1] ){
            pads[grp_id][pin_i][1] = 1;
            //NOTE ON
            MIDI_TX(144,pads[grp_id][pin_i][2] , map( pads[grp_id][pin_i][0], MAXSENSE, 1 , 1, 127 ) );
        }
        //segunda mitad del teclado
        if( digitalRead_(pin_i+INP_PINS) && !pads[grp_id][pin_i+INP_PINS][1] ){
            pads[grp_id][pin_i+INP_PINS][1] = 1;
            //NOTE ON
            MIDI_TX(144,pads[grp_id][pin_i+INP_PINS][2] , map( pads[grp_id][pin_i+INP_PINS][0], MAXSENSE, 1 , 1, 127 ) );
        }
    }
    
    digitalWrite(grp_id*2+31,LOW);
    
};

int digitalRead_(int pad){
    digitalWrite(2, muxTable[pad][0] );
    digitalWrite(3, muxTable[pad][1] );
    digitalWrite(4, muxTable[pad][2] );
    digitalWrite(5, muxTable[pad][3] );
    return digitalRead(6);
};

void MIDI_TX(unsigned char CMD, unsigned char NOTE, unsigned char VELOCITY) {
    Serial.write(CMD|MIDICHANNEL);
    Serial.write(NOTE);
    Serial.write(VELOCITY);
    
    digitalWrite(LED_PIN,HIGH);
    delay(1);
    digitalWrite(LED_PIN,LOW);
};

