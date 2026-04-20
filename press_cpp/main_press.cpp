#include "EnsembleDetector.hpp"
#include "RobustSpikeDetector.hpp"
#include "TrendingCusumDetectorFiltered.hpp"
#include "TempModel.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <cstdio>
#include <cstdlib>
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

    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        fclose(fp);
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL && total_points < MAX_ROWS) {
        char* comma_ptr = strchr(buffer, ',');
        if (comma_ptr != NULL) {
            size_t ts_len = comma_ptr - buffer;
            if (ts_len >= TIMESTAMP_LEN) ts_len = TIMESTAMP_LEN - 1;
            
            strncpy(timestamps[total_points], buffer, ts_len);
            timestamps[total_points][ts_len] = '\0';

            temperature_data[total_points] = atof(comma_ptr + 1);
            
            total_points++;
        }
    }

    fclose(fp);
    printf("Загружено %d точек данных из файла.\n", total_points);
}

int main() {
    load_data_pure_c("PressureMaslaMagistral.csv");
    if (total_points == 0) {
        printf("Нет данных для обработки. Выход.\n");
        return 1;
    }

    FILE* csv_out = fopen("output_results.csv", "w");
    if (!csv_out) {
        printf("Ошибка создания выходного файла!\n");
        return 1;
    }
    fprintf(csv_out, "TimeStamp,Value,Alarm\n");

    EnsembleDetector<500> ensemble_det(1.0, 0.5, 2.0); 
    RobustSpikeDetector   spike_det(2.0, 4.0, 3);
    TrendingCusumDetectorFiltered cusum_det(0.0, 0.05, 50.0, 2.0, 0.2);

    double model_input[4];
    double model_output[2];

    printf("[Running] Запуск цикла реального времени (20 мс)...\n");

    for (int i = 0; i < total_points; ++i) {
        auto start_step = std::chrono::steady_clock::now();

        double current_val = temperature_data[i];
        const char* current_ts = timestamps[i];

        auto res_ens = ensemble_det.update(current_val);
        auto res_spk = spike_det.update(current_val);
        auto res_csm = cusum_det.update(current_val);

        model_input[0] = current_val;          
        model_input[1] = res_spk.slew_rate;    
        model_input[2] = res_csm.s_high;    
        model_input[3] = res_csm.s_low;       

        score(model_input, model_output);

        bool is_anomaly = (model_output[1] > 0.5);

        fprintf(csv_out, "%s,%.3f,%d\n", current_ts, current_val, (is_anomaly ? 1 : 0));

        if (is_anomaly) {
            printf("[ALARM] Step: %d | Val: %.3f | Prob: %.2f | S_High: %.1f\n", 
                    i, current_val, model_output[1], res_csm.s_high);
        }

        auto end_step = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_step - start_step).count();
        
        long sleep_time_us = (STEP_TIME_MS * 1000) - (long)elapsed;

        if (sleep_time_us > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us));
        } else {
        }
    }

    fclose(csv_out);
    printf("[Done] Обработка завершена. Результаты в 'output_results.csv'\n");

    return 0;
}