#include <SFML/Graphics.h>
#include <SFML/Window.h>
#include <math.h>
#include <stdio.h>
#include <complex.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <immintrin.h>

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
    const __m256d four = _mm256_set1_pd (4.0);
    const __m256d width = _mm256_set1_pd (WIDTH);
    const __m256d height = _mm256_set1_pd (HEIGHT);
    const __m256d offsetX_vec = _mm256_set1_pd (offsetX);
    const __m256d offsetY_vec = _mm256_set1_pd (offsetY);
    const __m256d zoom_vec = _mm256_set1_pd (zoom);
    const __m256d half_width = _mm256_set1_pd (WIDTH / 2.0);
    const __m256d half_height = _mm256_set1_pd (HEIGHT / 2.0);

    // Маски для сравнения
    const __m256d escape_radius = _mm256_set1_pd (RAD);
    const __m256i max_iter_vec = _mm256_set1_epi32 (MAX_ITER);
    
    for  (int y = 0; y < HEIGHT; y++) {
                // Предварительные вычисления для y
                __m256d y_vec = _mm256_set1_pd (y);
        __m256d ci_base = _mm256_sub_pd (y_vec , half_height);
        ci_base = _mm256_mul_pd (ci_base , four);
        ci_base = _mm256_div_pd (ci_base , height);
        ci_base = _mm256_div_pd (ci_base , zoom_vec);
        __m256d ci = _mm256_add_pd (ci_base , offsetY_vec);
        
        for  (int x = 0; x < WIDTH; x += 4) {
            // Подготовка 4 значений x
            __m256d x_vec = _mm256_set_pd (x+3 , x+2 , x+1 , x);
            __m256d cr_base = _mm256_sub_pd (x_vec , half_width);
            cr_base = _mm256_mul_pd (cr_base , four);
            cr_base = _mm256_div_pd (cr_base , width);
            cr_base = _mm256_div_pd (cr_base , zoom_vec);
            __m256d cr = _mm256_add_pd (cr_base , offsetX_vec);
            
            // Инициализация zr и zi нулями
            __m256d zr = _mm256_setzero_pd ();
            __m256d zi = _mm256_setzero_pd ();
            
            // Вектор для подсчета итераций  (используем 32-битные целые)
            __m256i iter = _mm256_setzero_si256 ();
            __m256i mask = _mm256_set1_epi32 (-1);
            
            for  (int n = 0; n < MAX_ITER; n++) {
                // Проверяем только те пиксели , которые еще не вышли за границу
                if  (_mm256_testz_si256 (mask , mask)) break;
                
                // Вычисляем zr² и zi²
                __m256d zr2 = _mm256_mul_pd (zr , zr);
                __m256d zi2 = _mm256_mul_pd (zi , zi);
                
                // Проверяем условие |z| <= 2
                __m256d magnitude = _mm256_add_pd (zr2 , zi2);
                __m256d cmp = _mm256_cmp_pd (magnitude , escape_radius , _CMP_LE_OS);
                
                // Обновляем маску
                mask = _mm256_castpd_si256 (cmp);
                
                // Увеличиваем счетчик итераций только для тех пикселей , которые еще в множестве
                iter = _mm256_sub_epi32 (iter , _mm256_castpd_si256 (cmp));
                
                // Вычисляем новое значение zi
                __m256d zrzi = _mm256_mul_pd (zr , zi);
                zi = _mm256_add_pd (_mm256_add_pd (zrzi , zrzi) , ci);
                
                // Вычисляем новое значение zr
                zr = _mm256_add_pd (_mm256_sub_pd (zr2 , zi2) , cr);
            }
            
             float iter_arr[4] = {};
            _mm256_storeu_ps (iter_arr , _mm256_cvtepi32_ps (iter));
            
              for  (int i = 0; i < 4; i++) {
                if  (x + i >= WIDTH) break;
                
                float t = iter_arr[i] / MAX_ITER    ;

                unsigned char r =  (unsigned char) (sqrt (t) * 255);
                unsigned char g =  (unsigned char) (t * t * t * 255);
                unsigned char b =  (unsigned char) (sin (3.1415 * t) * 255);

                // unsigned char r = 0;
                // unsigned char g =  (unsigned char) (sin  (3.1415 * t)* 255);
                // unsigned char b =  (unsigned char) (sin  (3.1415 * t) * 255);

                int index =  (y * WIDTH + x + i) * 4;
                pixels[index] = r;
                pixels[index + 1] = g;
                pixels[index + 2] = b;
                pixels[index + 3] = 255;
            }   
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
