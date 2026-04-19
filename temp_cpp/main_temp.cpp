#include "EnsembleDetector.hpp"
#include "RobustSpikeDetector.hpp"
#include "TrendingCusumDetectorFiltered.hpp"
#include "TempModel.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <cstdio>  // Вместо iostream используем стандартный Си-ввод/вывод
#include <cstdlib> // Для atof (перевод текста в число)
#include <cstring>
const int MAX_ROWS = 100000; 
const int TIMESTAMP_LEN = 24;
double temperature_data[MAX_ROWS];
static char timestamps[MAX_ROWS][TIMESTAMP_LEN]; 
int total_points = 0;
const int STEP_TIME_MS = 20;

void load_data_pure_c(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Ошибка открытия файла %s\n", filename);
        return;
    }

    char buffer[512]; 

    // Пропускаем заголовок
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        fclose(fp);
        return;
    }

    // Читаем строки
    while (fgets(buffer, sizeof(buffer), fp) != NULL && total_points < MAX_ROWS) {
        // Ищем запятую
        char* comma_ptr = strchr(buffer, ',');
        if (comma_ptr != NULL) {
            // 1. Извлекаем TimeStamp (все до запятой)
            size_t ts_len = comma_ptr - buffer;
            if (ts_len >= TIMESTAMP_LEN) ts_len = TIMESTAMP_LEN - 1;
            
            strncpy(timestamps[total_points], buffer, ts_len);
            timestamps[total_points][ts_len] = '\0'; // Завершаем строку

            // 2. Извлекаем Температуру (все после запятой)
            temperature_data[total_points] = atof(comma_ptr + 1);
            
            total_points++;
        }
    }

    fclose(fp);
    printf("Загружено %d точек данных из файла.\n", total_points);
}

int main() {
    // 1. Инициализация (загрузка входных данных)
    load_data_pure_c("TempMaslaMagistral.csv");
    if (total_points == 0) {
        printf("Нет данных для обработки. Выход.\n");
        return 1;
    }

    // 2. Подготовка файла для записи результатов
    FILE* csv_out = fopen("output_results.csv", "w");
    if (!csv_out) {
        printf("Ошибка создания выходного файла!\n");
        return 1;
    }
    // Записываем заголовки в выходной файл
    fprintf(csv_out, "TimeStamp,Value,Alarm\n");

    // 3. Создание детекторов на стеке (без динамической памяти)
    EnsembleDetector<500> ensemble_det(1.0, 0.5, 2.0); 
    RobustSpikeDetector   spike_det(2.0, 4.0, 3);
    TrendingCusumDetectorFiltered cusum_det(0.0, 0.05, 50.0, 2.0, 0.2);

    // Буферы для модели классификатора
    double model_input[4];
    double model_output[2];

    printf("[Running] Запуск цикла реального времени (20 мс)...\n");

    // 4. Основной цикл обработки
    for (int i = 0; i < total_points; ++i) {
        auto start_step = std::chrono::steady_clock::now();

        double current_val = temperature_data[i];
        const char* current_ts = timestamps[i];

        // Обновление математических детекторов
        auto res_ens = ensemble_det.update(current_val);
        auto res_spk = spike_det.update(current_val);
        auto res_csm = cusum_det.update(current_val);

        // Формирование признаков для модели (СТРОГО по порядку обучения)
        // 0: val, 1: slew, 2: s_high, 3: s_low
        model_input[0] = current_val;          
        model_input[1] = res_spk.slew_rate;    
        model_input[2] = res_csm.s_high;    
        model_input[3] = res_csm.s_low;       

        // Вызов классификатора m2cgen
        score(model_input, model_output);

        bool is_anomaly = (model_output[1] > 0.5);

        // --- ЗАПИСЬ В CSV ---
        // Сохраняем: шаг, значение, показатели CUSUM, вероятности модели и итоговый флаг
        fprintf(csv_out, "%s,%.3f,%d\n", current_ts, current_val, (is_anomaly ? 1 : 0));

        // Вывод аларма в консоль
        if (is_anomaly) {
            printf("[ALARM] Step: %d | Val: %.3f | Prob: %.2f | S_High: %.1f\n", 
                    i, current_val, model_output[1], res_csm.s_high);
        }

        // Контроль периода 20 мс
        auto end_step = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_step - start_step).count();
        
        long sleep_time_us = (STEP_TIME_MS * 1000) - (long)elapsed;

        if (sleep_time_us > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us));
        } else {
            // Если вычисления + запись заняли больше 20 мс
            // (на x86 это случится только при очень медленной записи на диск)
            // printf("[Warning] Overrun: %ld us\n", (long)elapsed);
        }
    }

    // 5. Завершение работы
    fclose(csv_out);
    printf("[Done] Обработка завершена. Результаты в 'output_results.csv'\n");

    return 0;
}