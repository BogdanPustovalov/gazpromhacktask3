#ifndef ENSEMBLE_DETECTOR_H
#define ENSEMBLE_DETECTOR_H

#include <cmath>
#include <array>
#include <algorithm>
#include <cstddef> // Для size_t

/**
 * @brief Класс для обнаружения аномалий на основе ансамбля методов:
 * 1. Slew Rate (скорость изменения)
 * 2. Standard Deviation (стандартное отклонение в скользящем окне)
 * 3. Kalman Filter Innovation (ошибка предсказания фильтра Калмана)
 */
template<std::size_t WINDOW_SIZE = 20>
class EnsembleDetector {
public:
    struct Result {
        double val;         // Текущее значение
        double slew_err;    // Разница с предыдущим значением
        double std_val;     // Текущее стандартное отклонение
        double kalman_err;  // Ошибка фильтра Калмана
        bool alarm;         // Итоговый флаг тревоги
    };

private:
    // Параметры Slew Rate
    double slew_threshold;
    double last_val;
    bool has_last_val;
    
    // Параметры скользящего окна (STD)
    std::array<double, WINDOW_SIZE> buffer;
    std::size_t buf_idx;
    std::size_t count;
    double std_threshold;
    
    // Параметры фильтра Калмана
    double k_x;      // Состояние (значение)
    double k_v;      // Скорость изменения
    double k_p_00, k_p_01, k_p_10, k_p_11; // Матрица ковариации ошибки
    double k_q_val;  // Шум процесса
    double k_r;      // Шум измерения
    double kalman_threshold;
    bool is_kf_init;
    
    // Параметры подтверждения (Persistence)
    int persistence_limit;
    int suspect_counter;

public:
    /**
     * @param slew_thresh Порог для изменения между соседними точками
     * @param std_thresh Порог для стандартного отклонения в окне
     * @param kalman_thresh Порог для ошибки инновации Калмана
     * @param persistence Кол-во последовательных аномалий для срабатывания
     * @param k_q Шум процесса фильтра Калмана (динамика системы)
     * @param k_r Шум измерения фильтра Калмана (доверие датчику)
     */
    EnsembleDetector(double slew_thresh, double std_thresh, double kalman_thresh,
                    int persistence = 2, double k_q = 0.0001, double k_r = 0.1)
        : slew_threshold(slew_thresh)
        , last_val(0.0)
        , has_last_val(false)
        , buf_idx(0)
        , count(0)
        , std_threshold(std_thresh)
        , k_x(0.0)
        , k_v(0.0)
        , k_p_00(1.0), k_p_01(0.0), k_p_10(0.0), k_p_11(1.0)
        , k_q_val(k_q)
        , k_r(k_r)
        , kalman_threshold(kalman_thresh)
        , is_kf_init(false)
        , persistence_limit(persistence)
        , suspect_counter(0)
    {
        buffer.fill(0.0);
    }

private:
    // Внутренний шаг фильтра Калмана
    double kalman_step(double val) {
        if (!is_kf_init) {
            k_x = val;
            k_v = 0.0;
            is_kf_init = true;
            return 0.0;
        }
        
        // Прогноз
        double x_pred = k_x + k_v;
        double v_pred = k_v;
        k_p_00 += k_p_11 + k_q_val;
        
        // Инновация (ошибка предсказания)
        double innovation = val - x_pred;
        double s = k_p_00 + k_r;
        double k_gain_x = k_p_00 / s;
        double k_gain_v = k_p_01 / s;
        
        // Обновление состояния
        k_x = x_pred + k_gain_x * innovation;
        k_v = v_pred + k_gain_v * innovation;
        k_p_00 *= (1.0 - k_gain_x);
        
        return std::abs(innovation);
    }
    
    // Вычисление стандартного отклонения в буфере
    double calculate_std() const {
        if (count <= 1) return 0.0;
        
        double sum = 0.0;
        for (std::size_t i = 0; i < count; ++i) {
            sum += buffer[i];
        }
        double mean = sum / count;
        
        double sq_sum = 0.0;
        for (std::size_t i = 0; i < count; ++i) {
            double diff = buffer[i] - mean;
            sq_sum += diff * diff;
        }
        return std::sqrt(sq_sum / count);
    }

public:
    /**
     * @brief Обработать новую точку данных
     * @param val Значение с датчика
     * @return Структура Result с расчетами и флагом тревоги
     */
    Result update(double val) {
        Result result;
        result.val = val;
        
        // 1. Slew Rate Check
        result.slew_err = has_last_val ? std::abs(val - last_val) : 0.0;
        bool slew_alarm = result.slew_err > slew_threshold;
        
        // 2. Rolling STD Check
        buffer[buf_idx] = val;
        buf_idx = (buf_idx + 1) % WINDOW_SIZE;
        count = std::min(count + 1, (std::size_t)WINDOW_SIZE);
        result.std_val = calculate_std();
        bool std_alarm = result.std_val > std_threshold;
        
        // 3. Kalman Innovation Check
        result.kalman_err = kalman_step(val);
        bool kalman_alarm = result.kalman_err > kalman_threshold;
        
        // Логика подтверждения тревоги (Persistence)
        if (slew_alarm || kalman_alarm || std_alarm) {
            suspect_counter++;
        } else {
            suspect_counter = 0;
        }
        
        // Тревога считается активной, если аномалия длится от 1 шага до лимита подтверждения
        result.alarm = (suspect_counter >= 1) && (suspect_counter <= persistence_limit);
        
        last_val = val;
        has_last_val = true;
        
        return result;
    }
    
    /**
     * @brief Сброс всех внутренних состояний и фильтров
     */
    void reset() {
        has_last_val = false;
        buf_idx = 0;
        count = 0;
        is_kf_init = false;
        suspect_counter = 0;
        buffer.fill(0.0);
        k_p_00 = 1.0; k_p_01 = 0.0;
        k_p_10 = 0.0; k_p_11 = 1.0;
    }
};

#endif