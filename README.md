TREINADOR DE CÓDIGO MORSE - CW TRAINER

É um decodificador e treinador de Código Morse que transforma os sinais de uma chave telegráfica (manipulador CW) em texto no display OLED, com retorno sonoro (sidetone).

Você transmite código Morse usando uma chave telegráfica (manipulador)

O Arduino decodifica automaticamente os sinais (dits e dahs)

As letras decodificadas aparecem em tempo real no display OLED

Um buzzer emite o som correspondente (sidetone) para você ouvir o que está transmitindo

O sistema se adapta automaticamente à sua velocidade de transmissão

Pressione a chave por 3 segundos para limpar toda a tela

Pino D2 - CHAVE TELEGRÁFICA (MANIPULADOR CW)

Um lado da chave no pino D2

Outro lado da chave no GND

O pino D2 já está configurado com PULL-UP interno (não precisa de resistor externo)

Pino D9 - BUZZER OU ALTO-FALANTE (SAÍDA DE ÁUDIO)

Buzzer passivo (piezo) ou pequeno alto-falante de 4ou 8Ω

Um lado no pino D9

Outro lado no GND

Para alto-falante, use um resistor capacitor de 10uf 16v

Pino D13 - LED INDICADOR (onboard)

Acende junto com o áudio durante a transmissão

Display OLED I2C (SH1106 128x64)

Arduino 5V  ------ VCC (Display)
Arduino GND ------ GND (Display)
Arduino A4  ------ SDA (Display)
Arduino A5  ------ SCL (Display)
Display OLED 128x64 com controlador SH1106

Comunicação via protocolo I2C

Alimentação: 5V (ou 3.3V dependendo do modelo)
