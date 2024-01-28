// Array que conté imatges 8x8 (bytes)
byte images[40] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, //barra horitzontal
		  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // contingut barra barra o botó
		  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, // Barra vertical
		  0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, // Botó premut
		  0x3C, 0x24, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81 // Tasa de café
		};
		
// El byte p és la page, el byte y es la columna.
// El char img és un índex a l'array de bytes de les imatges. Conté imatges 8x8 (8 bytes cada imatge)
void showImg(byte p, byte y, char img){
   byte *aux = &images[img*8];
   for (int i = 0; i < 8; ++i)
   {
      writeByte (p, y++, *aux++);
   }
}

void writeTxt(byte page, byte y, char * s) {
   int i=0;
   while (*s!='\n' && *s!='\0') 
   {
      putchGLCD(page, y+i, *(s++));
      i++;
   };
}