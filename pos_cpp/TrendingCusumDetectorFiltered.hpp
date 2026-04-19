#ifndef TRENDING_CUSUM_DETECTOR_FILTERED_H
#define TRENDING_CUSUM_DETECTOR_FILTERED_H

#include <algorithm> // Для std::max
#include <cmath>

/**
 * @brief Детектор аномалий на основе метода CUSUM (накопленная сумма)
 * с предварительной фильтрацией и учетом тренда (наклона).
 */
class TrendingCusumDetectorFiltered {
public:
    struct Result {
        bool alarm;     // Флаг превышения порога H
        double s_high;  // Накопленная сумма для положительного отклонения
        double s_low;   // Накопленная сумма для отрицательного отклонения
    };

private:
    double target_slope;
    double K;      // Мертвая зона (Dead band)
    double H;      // Порог срабатывания (Threshold)
    
    double s_high;
    double s_low;
    
    double last_val;
    double smoothed_val;
    double alpha;  // Коэффициент сглаживания EMA
    double decay;  // Коэффициент затухания накопленной суммы
    
    bool is_init;

public:
    /**
     * @param target_slp Ожидаемый наклон (тренд) данных
     * @param sigma_slope Среднеквадратичное отклонение наклона (шум)
     * @param h_factor Множитель порога (обычно 4.0 - 10.0)
     * @param k_factor Множитель мертвой зоны (обычно 0.5 - 2.0)
     * @param alpha_val Коэффициент фильтра EMA (0.0 - 1.0)
     */
    TrendingCusumDetectorFiltered(double target_slp, 
                                 double sigma_slope, 
                                 double h_factor = 50.0, 
                                 double k_factor = 2.0, 
                                 double alpha_val = 0.2)
        : target_slope(target_slp)
        , K(k_factor * sigma_slope)
        , H(h_factor * sigma_slope)
        , s_high(0.0)
        , s_low(0.0)
        , last_val(0.0)
        , smoothed_val(0.0)
        , alpha(alpha_val)
        , decay(0.99) // Затухание для предотвращения бесконечного накопления дрейфа
        , is_init(false)
    {}
    
    /**
     * @brief Обработка нового значения
     * @param val Текущее значение датчика
     * @return Структура Result с текущими суммами и флагом тревоги
     */
    Result update(double val) {
        Result result;
        
        if (!is_init) {
            smoothed_val = val;
            last_val = val;
            is_init = true;
            result.alarm = false;
            result.s_high = 0.0;
            result.s_low = 0.0;
            return result;
        }
        
        // 1. Экспоненциальное сглаживание (фильтр низких частот)
        smoothed_val = alpha * val + (1.0 - alpha) * smoothed_val;
        
        // 2. Расчет текущего наклона (производной)
        double current_slope = smoothed_val - last_val;
        last_val = smoothed_val;
        
        // 3. Отклонение от целевого наклона
        double diff = current_slope - target_slope;
        
        // 4. Расчет накопленных сумм CUSUM (Leaky Integrator)
        // Добавляем diff, вычитаем мертвую зону K и применяем затухание decay
        s_high = std::max(0.0, (s_high + diff - K) * decay);
        s_low = std::max(0.0, (s_low - diff - K) * decay);
        
        // 5. Проверка порога
        result.alarm = (s_high > H) || (s_low > H);
        result.s_high = s_high;
        result.s_low = s_low;
        
        return result;
    }
    
    /**
     * @brief Сброс состояния детектора (обнуление накопленных сумм)
     */
    void reset() {
        is_init = false;
        s_high = 0.0;
        s_low = 0.0;
    }
};

#endif