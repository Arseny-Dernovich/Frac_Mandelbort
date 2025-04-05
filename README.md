  # Fractal Mandelbrot

  ## Задача
  Наша задача заключалась в том , чтобы сперва написать программу на си , которая визуализирует множество Мандельброта с помощью SFML , обрабатывая за  одну итерацию цикла только одну точку множества , 
  что оказалось не эффективным . Потом , зная архитектуру своей компьютерной машины , стоит оптимизировать рассчёт точек множетсва , путём использования векторных инструкций SSE/AVX2/AVX512 (в зависимости от того , поддерживает ли процессор данные расширения).

  ## Зависимость кода от определённой компютерной машины
  Так как существует большое разнообразие процессоров со своей архитектурой и со своими поддерживаемыми расширениями , то , используя те или иные расширения на своём железе , не факт , что другое железо тоже поддерживает эти расширения .
  Характеристики моего процессора для данной задачи:модель 2th Gen Intel® Core™ i9-12900H , SSE И AVX2 поддерживается , AVX512 не поддерживается.
  Исходя из этого моя прогамма **last_version** становится машиннозависимой. Эта проблема решается переписыванием этой программы под различные компютерные машины.
  
  ## Сборка и запуск программ

  ### Вот так создаётся исполняемый файл для первой версии
  
  `g++ -O3 first_version.cpp -o mandelbrot -lcsfml-graphics -lcsfml-window -lcsfml-system -lm`

  ### И так создаётся исполняемый файл для последней версии
  
  `g++ -mavx2 -O3 last_version.cpp -o mandelbrot1 -lcsfml-graphics -lcsfml-window -lcsfml-system -lm -DFUN`

  ### Запуск первой версии

  `./mandelbrot`

  ### Запуск посследней версии

  `./mandelbrot1`
  

  ## Анализ первой версии
  Первая версия это задачи , у меня обрабатывает за одну итерацию одну точку множества. На выходе я получаю визуализация множества Мандельброта , используя графическую библиотеку SFML.
  Оценивать производительность будем по показаниям FPS. Условия при которых происходили опыты : температура окружающей среды 25 градусов Цельсия , ноутбук подключен к зарядному устройству и при запуске этой программы другие программы , требующие высокой производительности не запущенны , 
  размеры выводимого окошка 1000 * 1000 , максимальное количество итераций 512.
  В итоге результат  около 3,6 FPS.


  ```c++
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
 ```

  ![Github](https://github.com/Arseny-Dernovich/Frac_Mandelbort/blob/main/Pictures_for_README/%D0%A1%D0%BD%D0%B8%D0%BC%D0%BE%D0%BA%20%D1%8D%D0%BA%D1%80%D0%B0%D0%BD%D0%B0_20250405_152420.png)

  ## Анализ последней версии с интринсиками
  Эта оптимизированная версия расчёта множества Мандельброта. Оптимизация заключается в использовании векторных инструкций AVX2 , которые могут хранить 4 числа типа double / 8 чисел типа float , 
  что позволяет нам обрабатывать за одну итерацию цикла сразу 4 или 8 точек ( я использую double , поэтому по 4 точки).  Оценивать производительность также  будем по показаниям FPS. Условия при которых происходили опыты : температура окружающей среды 25 градусов Цельсия , ноутбук подключен к зарядному устройству и при запуске этой программы другие программы , требующие высокой производительности не запущенны , 
  размеры выводимого окошка 1000 * 1000 , максимальное количество итераций 512.
  В результате FPS стало примерно 25.8.


  ![Github](https://github.com/Arseny-Dernovich/Frac_Mandelbort/blob/main/Pictures_for_README/%D0%A1%D0%BD%D0%B8%D0%BC%D0%BE%D0%BA%20%D1%8D%D0%BA%D1%80%D0%B0%D0%BD%D0%B0_20250405_152457.png)
  
  ```c++
    const __m256d four = _mm256_set1_pd (4.0);
    const __m256d width = _mm256_set1_pd (WIDTH);
    const __m256d height = _mm256_set1_pd (HEIGHT);
    const __m256d offsetX_vec = _mm256_set1_pd (offsetX);
    const __m256d offsetY_vec = _mm256_set1_pd (offsetY);
    const __m256d zoom_vec = _mm256_set1_pd (zoom);
    const __m256d scaleY_vec = _mm256_set1_pd(scaleY);
    const __m256d scaleX_vec = _mm256_set1_pd(scaleX);
    const __m256d half_width = _mm256_set1_pd (WIDTH / 2.0);
    const __m256d half_height = _mm256_set1_pd (HEIGHT / 2.0);

    // Матрица поворота
    const __m256d cos_angle = _mm256_set1_pd (cos(angle));
    const __m256d sin_angle = _mm256_set1_pd (sin(angle));

    // Маски для сравнения
    const __m256d escape_radius = _mm256_set1_pd (RAD);
    const __m256i max_iter_vec = _mm256_set1_epi32 (MAX_ITER);
    
    for (int y = 0; y < HEIGHT; y++) {

        __m256d y_vec = _mm256_set1_pd (y);
        __m256d ci_base = _mm256_sub_pd (y_vec , half_height);
        ci_base = _mm256_mul_pd (ci_base , four);
        ci_base = _mm256_div_pd (ci_base , height);
        ci_base = _mm256_div_pd (ci_base , zoom_vec);
        ci_base = _mm256_div_pd(ci_base, scaleY_vec);
        ci_base = _mm256_add_pd (ci_base , offsetY_vec);
        
        for (int x = 0; x < WIDTH; x += 4) {

            __m256d x_vec = _mm256_set_pd (x+3 , x+2 , x+1 , x);
            __m256d cr_base = _mm256_sub_pd (x_vec , half_width);
            cr_base = _mm256_mul_pd (cr_base , four);
            cr_base = _mm256_div_pd (cr_base , width);
            cr_base = _mm256_div_pd (cr_base , zoom_vec);
            cr_base = _mm256_div_pd(cr_base, scaleX_vec);   
            cr_base = _mm256_add_pd (cr_base , offsetX_vec);
            
            // Вращение точки (cr, ci) вокруг центра
            __m256d cr_rotated = _mm256_sub_pd (_mm256_mul_pd (cr_base, cos_angle),
                                                _mm256_mul_pd (ci_base, sin_angle));
            __m256d ci_rotated = _mm256_add_pd (_mm256_mul_pd (cr_base, sin_angle),
                                                _mm256_mul_pd (ci_base, cos_angle));

            __m256d zr = _mm256_setzero_pd ();
            __m256d zi = _mm256_setzero_pd ();
            
            __m256i iter = _mm256_setzero_si256 ();
            __m256i mask = _mm256_set1_epi32 (-1);
            
            for (int n = 0; n < MAX_ITER; n++) {

                if (_mm256_testz_si256 (mask , mask)) break;
                
                __m256d zr2 = _mm256_mul_pd (zr , zr);
                __m256d zi2 = _mm256_mul_pd (zi , zi);
                
                __m256d magnitude = _mm256_add_pd (zr2 , zi2);
                __m256d cmp = _mm256_cmp_pd (magnitude , escape_radius , _CMP_LE_OS);
                
                mask = _mm256_castpd_si256 (cmp);
                iter = _mm256_sub_epi32 (iter , _mm256_castpd_si256 (cmp));
                
                __m256d zrzi = _mm256_mul_pd (zr , zi);
                zi = _mm256_add_pd (_mm256_add_pd (zrzi , zrzi) , ci_rotated);
                zr = _mm256_add_pd (_mm256_sub_pd (zr2 , zi2) , cr_rotated);
            }
```
### Интересные фичи
В свои программы я добавил много интересных возможностей 
## Возможности визуализатора Мандельброта

| Возможность           | Клавиши управления       | Описание                                                                 |
|-----------------------|--------------------------|--------------------------------------------------------------------------|
|  Зум                | `+`, `-`                 | Приближение и отдаление изображения множества Мандельброта              |
|  Перемещение        | `W`, `A`, `S`, `D`       | Навигация по изображению (вверх, влево, вниз, вправо)                   |
|  Вращение           | `#define FUN`            | Автоматическое вращение множества при активации макроса `FUN`           |
|  Масштаб X / Y      | `←`, `→`, `↑`, `↓`       | Растягивание и сжатие множества по осям X и Y                           |

Также при активации макроса FUN , изображение множества будет появляться постепенно.
Чтобы использовать возможности связанные с вращением и постепенного роста множества Мандельброта необходимо при компиляции указать `-DFUN`.

## Заключение
Подведём итоги нашего эксперимента с оптимизаций . На основании полученных данных FPS для каждой версии (для первой версии FPS 3.6 , для последней версии FPS 25,8) мы получили ускорение программы чуть больше чем в 7 раз.
Из этого можно сделать вывод , зная возможности своеё компьютерной машины и архитектуры можно ускорять свои программы в несколько раз.
