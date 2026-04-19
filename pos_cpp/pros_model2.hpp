#ifndef TEMP_MODEL_HPP
#define TEMP_MODEL_HPP

/**
 * @brief Функция классификации (модель m2cgen)
 * 
 * @param input  Массив входных признаков (минимум 3 элемента):
 *               input[0] - Текущее значение
 *               input[1] - Скорость изменения (не используется в данной версии модели)
 *               input[2] - Показатель CUSUM (S_high)
 * @param output Массив выходных вероятностей (2 элемента):
 *               output[0] - Вероятность нормального состояния (Class 0)
 *               output[1] - Вероятность аномалии (Class 1)
 */
#ifdef __cplusplus
#endif

void score(double * input, double * output);

#ifdef __cplusplus
#endif

#endif // TEMP_MODEL_HPP