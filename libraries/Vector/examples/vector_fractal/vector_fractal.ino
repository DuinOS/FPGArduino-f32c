/*************************************************** 
(c)EMARD
LICENSE=BSD

Vectorized fractal calculation (Mandelbrot)
based on Emanuel's source
****************************************************/
#include <Compositing.h>
#include <Vector.h>

#define ANZCOL 256
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SPRITE_MAX 10

Compositing c2;
Vector V;

// crude malloc()
pixel_t *bitmap = (pixel_t *)0x80080000;
pixel_t color_map[ANZCOL];

#define emu_set_pix(x,y,c) bitmap[x + SCREEN_WIDTH*y] = color_map[c]

void alloc_bitmap()
{
  int i, x,y;

  c2.init();
  c2.alloc_sprites(SPRITE_MAX);
  bitmap = (pixel_t *) malloc(SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(pixel_t)); // alloc memory for video
  c2.sprite_from_bitmap(SCREEN_WIDTH, SCREEN_HEIGHT, bitmap); // create c2 sprite
  c2.sprite_refresh(); // show it on screen
  *c2.cntrl_reg = 0b11000000; // vgatextmode: enable video, yes bitmap, no text mode, no cursor

  for(i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++) bitmap[i] = 0;  // clear screen
}

void fractal()
{
  Vector_RAM Mac(SCREEN_WIDTH), Mbc(SCREEN_WIDTH), Mbc_inc(SCREEN_WIDTH), Mresult(SCREEN_WIDTH);
  Vector_REG Vaz(0), Vbz(1), Vha(2), Vhb(3), Vhc(4), Vac(5), Vbc(6), Vtmp(7);
  float ac;
  int x, y, i;
  float spalt_x, spalt_y;
  float aecke, becke, seite_x, seite_y;
  float linke_ecke, rechte_ecke, untere_ecke, obere_ecke;
  int pixdone[SCREEN_WIDTH]; // 0 if pix is not yet calculated

  // set color map
  for(i = 0; i < ANZCOL; i++)
    color_map[i] = RGB2PIXEL(rand());

  linke_ecke = -1.45;
  rechte_ecke = 0.60;

  obere_ecke = 1.15;
  untere_ecke = -1.15;

  aecke = linke_ecke;
  becke = obere_ecke;

  seite_x = rechte_ecke - linke_ecke;
  seite_y = untere_ecke - obere_ecke;

  spalt_x = seite_x / (float) SCREEN_WIDTH;
  spalt_y = seite_y / (float) SCREEN_HEIGHT;

  ac = aecke;
  for(x = 0; x < SCREEN_WIDTH; x++)
  {
    Mac.vh->data[x].f = ac;
    ac += spalt_x;
    Mbc.vh->data[x].f = becke;
    Mbc_inc.vh->data[x].f = spalt_y;
  }
  int tstart = millis();
  Vac = Mac;
  Vbz = Mbc; // use Vbz as temporary
  for(y = 0; y < SCREEN_HEIGHT; y++)
  {
    // printf("processing screen line %d\n", y);
    Vaz = Vbz - Vbz; // 0
    Vbc = Vaz + Vbz;
    // set vectors to zero before each horizontal line
    Vbz = Vaz - Vaz; // 0
    Vha = Vaz - Vaz; // 0
    Vhb = Vaz - Vaz; // 0
    // set no pixels are done yet in this line
    int pixinline=0;
    for(x = 0; x < SCREEN_WIDTH; x++)
      pixdone[x] = 0;
    // calculate the line using vectors
    for(i = 0; i < ANZCOL && pixinline < SCREEN_WIDTH; i++)
    {
      // main fractal loop is completely done in vector registers (FAST)
      Vhc = Vaz * Vbz;
      Vtmp = Vha - Vhb;
      Vaz = Vtmp + Vac;
      Vtmp = Vhc + Vhc;
      Vbz = Vtmp + Vbc;
      Vha = Vaz * Vaz;
      Vhb = Vbz * Vbz;
      Vtmp = Vha + Vhb;
      // only I/O is to store temporery result in RAM to plot pixels
      Mresult = Vtmp;
      for(x = 0; x < SCREEN_WIDTH; x++)
      {
      	if(pixdone[x] == 0)
        {
          // if pixel is not yet placed, check calculation results
          //if(vresult->data[x].f > 2.0)
          if(Mresult.vh->data[x].part.sign == 0 && Mresult.vh->data[x].part.exponent > 127) // same as above but faster
          {
            emu_set_pix(x, y, i);
            pixdone[x] = 1; // pixel is placed, won't check anymore for this line
            pixinline++;
          } // if > 2.0
        } // if pixdone
      } // for screen width
    } // for anzcol
    // increment bc for the next line
    Vaz = Mbc_inc;
    Vbz = Vbc + Vaz;
  } // for y
  int tstop = millis();

  printf("time %d ms\n", (tstop-tstart));
}

void setup(void)
{
  alloc_bitmap();
  fractal();
}

void loop(void)
{
  delay(1000);
}
