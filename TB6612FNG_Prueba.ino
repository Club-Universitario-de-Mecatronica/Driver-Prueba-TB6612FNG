#define PWMA 5
#define IA1 8
#define IA2 9

#define PWMB 6
#define IB1 11
#define IB2 10

#define STBY 4

#define JSX A0
#define JSY A1



void setup() {
  
  // Motor A
  pinMode(PWMA, OUTPUT);
  pinMode(IA1, OUTPUT);
  pinMode(IA2, OUTPUT);
  pinMode(STBY, OUTPUT);

  // Motor B
  pinMode(PWMB, OUTPUT);
  pinMode(IB1, OUTPUT);
  pinMode(IB2, OUTPUT);

  // Joystick
  pinMode(JSX, INPUT);
  pinMode(JSY, INPUT);


  Serial.begin(115200);

  digitalWrite(STBY, HIGH);
  
}

void cambiarVelocidadPWM(int PWM, int value) {

  int v = map(value, 0, 1023, 0, 180);

  int V = constrain(v, 0, 180);

  analogWrite(PWM, V);
}


void girarDerecha(int velocidad) {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, velocidad);

  digitalWrite(IA1, LOW);
  digitalWrite(IA2, LOW);
  digitalWrite(IB1, LOW);
  digitalWrite(IB2, HIGH);
}

void girarIzquierda(int velocidad) {

  analogWrite(PWMB, 0);
  analogWrite(PWMA, velocidad);

  digitalWrite(IA1, LOW);
  digitalWrite(IA2, HIGH);

  digitalWrite(IB1, LOW);
  digitalWrite(IB2, LOW);

}

void adelante(int velocidad) {

  cambiarVelocidadPWM(PWMA, velocidad);
  cambiarVelocidadPWM(PWMB, velocidad);

  // Motor A
  digitalWrite(IA1, HIGH);
  digitalWrite(IA2, LOW);

  // Motor B
  digitalWrite(IB1, HIGH);
  digitalWrite(IB2, LOW);

}

void atras(int velocidad) {

  cambiarVelocidadPWM(PWMA, velocidad);
  cambiarVelocidadPWM(PWMB, velocidad);

  // Motor A
  digitalWrite(IA1, LOW);
  digitalWrite(IA2, HIGH);

  // Motor B
  digitalWrite(IB1, LOW);
  digitalWrite(IB2, HIGH);

}

void stop() {

  // Detiene ambos motores
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);

  // Motor A
  digitalWrite(IA1, LOW);
  digitalWrite(IA2, LOW);

  // Motor B
  digitalWrite(IB1, LOW);
  digitalWrite(IB2, LOW);

}




void loop() {
  
  int sumaX = 0;
  int sumaY = 0;


  for (size_t i = 0; i < 10 ; i++) {
    sumaX += analogRead(JSX);
    sumaY += analogRead(JSY);
  }

  int x = sumaX / 10;
  int y = sumaY / 10;

  Serial.print("X: ");
  Serial.print(x);
  Serial.print(" Y: ");
  Serial.println(y);

  bool xCentro = x > 500 && x < 540;
  bool yCentro = y > 487 && y < 527;



  // Si esta quieto entonces no hacer nada
  if (xCentro && yCentro) {

    stop();

  } else {

    if (x < 500 && yCentro) {

      // Derecha
      Serial.println("Derecha");
      int velocidad = map(x, 500, 0, 0, 255); 
      velocidad = constrain(velocidad, 0, 255);
      girarDerecha(velocidad);

    } else if (x > 540 && yCentro) {

      // Izquierda
      Serial.println("Izquierda");
      int velocidad = map(x, 540, 1023, 0, 255);
      velocidad = constrain(velocidad, 0, 255);
      girarIzquierda(velocidad);

    } else if (xCentro && y > 527) {

      Serial.println("Adelante");
      adelante(y);

    } else if (xCentro && y < 487) {

      Serial.println("Atras");      
      int velocidad = 1023 - y;
      atras(velocidad);
    }

  }

}
