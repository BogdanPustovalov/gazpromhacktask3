#ifndef ENSEMBLE_DETECTOR_H
#define ENSEMBLE_DETECTOR_H

#include <cmath>
#include <array>
#include <algorithm>
#include <cstddef> // Для size_t

/**
 * @brief
 */
template<std::size_t WINDOW_SIZE = 20>
class EnsembleDetector {
public:
    struct Result {
        double val;
        double slew_err;
        double std_val;
        double kalman_err;
        bool alarm;
    };

private:
    double slew_threshold;
    double last_val;
    bool has_last_val;
    
    std::array<double, WINDOW_SIZE> buffer;
    std::size_t buf_idx;
    std::size_t count;
    double std_threshold;
    
    double k_x;
    double k_v;
    double k_p_00, k_p_01, k_p_10, k_p_11;
    double k_q_val;
    double k_r;
    double kalman_threshold;
    bool is_kf_init;
    
    int persistence_limit;
    int suspect_counter;

public:
    /**
     * @param slew_thresh
     * @param std_thresh
     * @param kalman_thresh
     * @param persistence
     * @param k_q
     * @param k_r
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
    double kalman_step(double val) {
        if (!is_kf_init) {
            k_x = val;
            k_v = 0.0;
            is_kf_init = true;
            return 0.0;
        }
        
        double x_pred = k_x + k_v;
        double v_pred = k_v;
        k_p_00 += k_p_11 + k_q_val;
        
        double innovation = val - x_pred;
        double s = k_p_00 + k_r;
        double k_gain_x = k_p_00 / s;
        double k_gain_v = k_p_01 / s;
        
        k_x = x_pred + k_gain_x * innovation;
        k_v = v_pred + k_gain_v * innovation;
        k_p_00 *= (1.0 - k_gain_x);
        
        return std::abs(innovation);
    }
    
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
     * @brief
     * @param val
     * @return
     */
    Result update(double val) {
        Result result;
        result.val = val;
        
        result.slew_err = has_last_val ? std::abs(val - last_val) : 0.0;
        bool slew_alarm = result.slew_err > slew_threshold;
        
        buffer[buf_idx] = val;
        buf_idx = (buf_idx + 1) % WINDOW_SIZE;
        count = std::min(count + 1, (std::size_t)WINDOW_SIZE);
        result.std_val = calculate_std();
        bool std_alarm = result.std_val > std_threshold;
        
        result.kalman_err = kalman_step(val);
        bool kalman_alarm = result.kalman_err > kalman_threshold;
        
        if (slew_alarm || kalman_alarm || std_alarm) {
            suspect_counter++;
        } else {
            suspect_counter = 0;
        }
        
        result.alarm = (suspect_counter >= 1) && (suspect_counter <= persistence_limit);
        
        last_val = val;
        has_last_val = true;
        
        return result;
    }
    
    /**
     * @brief
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