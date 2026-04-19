#include "TrendingCusumDetectorFiltered.hpp"
#include "pros_model2.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <cstdio>  
#include <cstdlib> 
#include <cstring>

// Настройки буферов (без динамической памяти)
const int MAX_ROWS = 100000; 
const int TIMESTAMP_LEN = 24;

// Глобальные статические массивы (память выделяется при запуске программы)
double sensor_data[MAX_ROWS];
char timestamps[MAX_ROWS][TIMESTAMP_LEN]; 
int total_points = 0;

const int STEP_TIME_MS = 20;

/**
 * Функция загрузки данных из CSV формата:
 * TimeStamp,Значение
 */
void load_data_pure_c(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Ошибка открытия файла %s\n", filename);
        return;
    }

    char buffer[512]; 

    // 1. Пропускаем заголовок (первую строку)
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        fclose(fp);
        return;
    }

    // 2. Читаем строки данных
    while (fgets(buffer, sizeof(buffer), fp) != NULL && total_points < MAX_ROWS) {
        // Убираем символы переноса строки в конце (\r или \n)
        buffer[strcspn(buffer, "\r\n")] = 0;

        // Ищем разделитель (запятую)
        char* comma_ptr = strchr(buffer, ',');
        if (comma_ptr != NULL) {
            // Разделяем строку на две части, заменяя запятую на терминатор
            *comma_ptr = '\0';
            char* value_ptr = comma_ptr + 1;

            // Копируем TimeStamp (до 23 символов + \0)
            strncpy(timestamps[total_points], buffer, TIMESTAMP_LEN - 1);
            timestamps[total_points][TIMESTAMP_LEN - 1] = '\0';

            // Преобразуем значение (Положение КПВ) в double
            sensor_data[total_points] = atof(value_ptr);
            
            total_points++;
        }
    }

    fclose(fp);
    printf("Успешно загружено %d точек из файла %s\n", total_points, filename);
}

int main() {
    // 1. Загрузка данных (укажите правильное имя вашего файла)
    load_data_pure_c("PositionKPV.csv"); 

    if (total_points == 0) {
        printf("Данные не найдены. Проверьте путь к файлу.\n");
        return 1;
    }

    // 2. Подготовка файла для результатов
    FILE* csv_out = fopen("output_results.csv", "w");
    if (!csv_out) {
        printf("Ошибка создания выходного файла!\n");
        return 1;
    }
    fprintf(csv_out, "TimeStamp,Value,Alarm\n");

    // 3. Инициализация детекторов (на стеке)
    TrendingCusumDetectorFiltered cusum_det(0.0, 0.05, 50.0, 2.0, 0.2);

    double model_input[3];
    double model_output[2];

    printf("[Running] Обработка данных...\n");

    // 4. Основной цикл (имитация реального времени)
    for (int i = 0; i < total_points; ++i) {
        auto start_step = std::chrono::steady_clock::now();

        double current_val = sensor_data[i];
        const char* current_ts = timestamps[i];

        // Обновление математики
        auto res_csm = cusum_det.update(current_val);

        // Подготовка вектора для модели
        model_input[0] = current_val;    
        model_input[2] = res_csm.s_high;    
        model_input[3] = res_csm.s_low;       

        // Классификация (внешняя функция score)
        score(model_input, model_output);

        bool is_anomaly = (model_output[1] > 0.5);

        // Запись в CSV
        fprintf(csv_out, "%s,%.3f,%d\n", current_ts, current_val, (is_anomaly ? 1 : 0));

        // Вывод при обнаружении аномалии
        if (is_anomaly) {
            printf("[ALARM] %s | Val: %.3f | Prob: %.2f\n", 
                    current_ts, current_val, model_output[1]);
        }

        // Контроль времени шага 20 мс
        auto end_step = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_step - start_step).count();
        long sleep_time_us = (STEP_TIME_MS * 1000) - (long)elapsed;

        if (sleep_time_us > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us));
        }
    }

    fclose(csv_out);
    printf("[Done] Обработка завершена. Данные сохранены в 'output_results.csv'\n");

    return 0;
}