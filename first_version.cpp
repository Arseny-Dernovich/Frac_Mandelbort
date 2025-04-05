#include <SFML/Graphics.h>
#include <SFML/Window.h>
#include <math.h>
#include <stdio.h>
#include <complex>
using namespace std;
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

const int  WIDTH  =  1000;
const int  HEIGHT =  1000;                

#ifdef FUN                                      
     double MAX_ITER   =     1;           
     double RAD        =     0.1     ;
#else
     double MAX_ITER   =     512;           
     double RAD        =     100;
#endif                                  

void mandelbrot (sfUint8* pixels , double offsetX , double offsetY , double zoom)
{
    for  (int y = 0; y < HEIGHT; y++) {
        for  (int x = 0; x < WIDTH; x++) {

            double cr =  (x - WIDTH / 2.0) * 4.0 / WIDTH / zoom + offsetX;
            double ci =  (y - WIDTH / 2.0) * 4.0 / HEIGHT / zoom + offsetY;
            complex<double> z = 0;
            complex<double> c = complex<double>(cr, ci);
            int iter = 0;

            while  (abs (z) <= 2 && iter < MAX_ITER) {
                z = z * z + c;
                iter++;
            }

                // double smooth = iter + 1 - log2 (log2 (cabs (z)));
                double t = iter / MAX_ITER;


                unsigned char r =  (unsigned char) (sqrt (t) * 255);
                unsigned char g =  (unsigned char) (t * t * t * 255);
                unsigned char b =  (unsigned char) (sin (3.1415 * t) * 255);

                int index =  (y * WIDTH + x) * 4;
                pixels[index] = r;
                pixels[index + 1] = g;
                pixels[index + 2] = b;
                pixels[index + 3] = 255;
        }
    }
}




int main ()
{
    sfRenderWindow* window = NULL;
    sfVideoMode mode = {WIDTH , HEIGHT , 32};
    window = sfRenderWindow_create (mode , "Mandelbrot" , sfResize | sfClose , NULL);

    sfTexture* texture = sfTexture_create (WIDTH , HEIGHT);
    sfSprite* sprite = sfSprite_create ();
    sfSprite_setTexture (sprite , texture , sfTrue);
    
    sfFont* font = sfFont_createFromFile ("arialmt.ttf");
    sfText* text = sfText_create ();
    sfText_setFont (text , font);
    sfText_setCharacterSize (text , 20);
    sfText_setColor (text , sfWhite);

    sfUint8* pixels =  (sfUint8*) calloc  (sizeof  (sfUint8)  , WIDTH * HEIGHT * 4 );
    double offsetX = -0.7 , offsetY = 0.0 , zoom = 1.0;
    
    sfClock* clock = sfClock_create ();
    while  (sfRenderWindow_isOpen (window)) {
        sfEvent event;
        while  (sfRenderWindow_pollEvent (window , &event)) {
            if  (event.type == sfEvtClosed) sfRenderWindow_close (window);
        }

        
        if  (sfKeyboard_isKeyPressed (sfKeyW)) offsetY -= 0.1 / zoom;
        if  (sfKeyboard_isKeyPressed (sfKeyS)) offsetY += 0.1 / zoom;
        if  (sfKeyboard_isKeyPressed (sfKeyA)) offsetX -= 0.1 / zoom;
        if  (sfKeyboard_isKeyPressed (sfKeyD)) offsetX += 0.1 / zoom;
        if  (sfKeyboard_isKeyPressed (sfKeyEqual)) zoom *= 1.1;
        if  (sfKeyboard_isKeyPressed (sfKeyDash)) zoom /= 1.1;

        mandelbrot (pixels , offsetX , offsetY , zoom);
        sfTexture_updateFromPixels (texture , pixels , WIDTH , HEIGHT , 0 , 0);


        sfTime time = sfClock_getElapsedTime (clock);  
        float fps = 1.0f / sfTime_asSeconds (time);   
        sfClock_restart (clock); 
        char fps_text[32] = "";
        sprintf (fps_text , "FPS: %.2f" , fps);
        sfText_setString (text , fps_text);

        sfRenderWindow_clear (window , sfBlack);
        sfRenderWindow_drawSprite (window , sprite , NULL);
        sfRenderWindow_drawText (window , text , NULL);
        sfRenderWindow_display (window);

        #ifdef FUN
            if  (MAX_ITER < 512) {
                MAX_ITER += 1;
                RAD += 0.05;
            }
        
        #endif
    }

    sfClock_destroy (clock);
    sfText_destroy (text);
    sfFont_destroy (font);
    sfSprite_destroy (sprite);
    sfTexture_destroy (texture);
    sfRenderWindow_destroy (window);
    free (pixels);

    return 0;
}
