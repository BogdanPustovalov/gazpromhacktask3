#ifndef ROBUST_SPIKE_DETECTOR_H
#define ROBUST_SPIKE_DETECTOR_H

#include <cmath>

/**
 * @brief
 */
class RobustSpikeDetector {
public:
    struct Result {
        bool is_alarm;
        double slew_rate;
    };

private:
    double slew_min;
    double k_factor;
    int max_duration;
    
    double last_val;
    int anomaly_counter;
    
    double k_x;
    double k_p;
    double k_q;
    double k_r;
    bool is_init;

public:
    /**
     * @param slew_minimum
     * @param k_fact
     * @param max_spike_dur
     */
    RobustSpikeDetector(double slew_minimum = 1.0, 
                       double k_fact = 4.0, 
                       int max_spike_dur = 3)
        : slew_min(slew_minimum)
        , k_factor(k_fact)
        , max_duration(max_spike_dur)
        , last_val(0.0)
        , anomaly_counter(0)
        , k_x(0.0)
        , k_p(1.0)
        , k_q(0.01)
        , k_r(0.1)
        , is_init(false)
    {}
    
    /**
     * @brief
     * @param val
     * @return
     */
    Result update(double val) {
        Result result;
        
        if (!is_init) {
            k_x = val;
            last_val = val;
            is_init = true;
            result.is_alarm = false;
            result.slew_rate = 0.0;
            return result;
        }
        
        double prediction_err = std::abs(val - k_x);
        
        result.slew_rate = std::abs(val - last_val);
        
        bool is_suspicious = (result.slew_rate > slew_min) && 
                            (prediction_err > (k_factor * k_r));
        
        if (is_suspicious) {
            anomaly_counter++;
        } else {
            anomaly_counter = 0;
        }
        
        result.is_alarm = (0 < anomaly_counter) && (anomaly_counter <= max_duration);
        
        k_p += k_q;
        double k_gain = k_p / (k_p + k_r);
        k_x = k_x + k_gain * (val - k_x);
        k_p *= (1.0 - k_gain);
        
        last_val = val;
        return result;
    }
    
    /**
     * @brief
     */
    void reset() {
        is_init = false;
        anomaly_counter = 0;
        k_p = 1.0;
    }
};

#endif