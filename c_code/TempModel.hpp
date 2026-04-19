#ifndef MODEL_H
#define MODEL_H

/**
 * Инструкция по использованию модели:
 * 
 * Входные данные (input):
 * Массив из 4-х элементов double. Судя по коду:
 * input[0] - Признак 1
 * input[1] - Признак 2
 * input[2] - Признак 3
 * input[3] - Признак 4
 * 
 * Выходные данные (output):
 * Массив из 2-х элементов double.
 * output[0] - Вероятность/вес Класса 0 (Норма)
 * output[1] - Вероятность/вес Класса 1 (Аномалия)
 */

#ifdef __cplusplus
#endif

/**
 * Функция предсказания модели
 * @param input  Указатель на массив входных признаков (минимум 4 элемента)
 * @param output Указатель на массив для записи результата (минимум 2 элемента)
 */
void score(double * input, double * output);

#ifdef __cplusplus
#endif

#endif // MODEL_H