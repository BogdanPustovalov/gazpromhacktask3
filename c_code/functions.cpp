#include <cmath>
#include <array>
#include <algorithm>

template<size_t WINDOW_SIZE = 20>
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
    size_t buf_idx;
    size_t count;
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
        for (size_t i = 0; i < count; ++i) {
            sum += buffer[i];
        }
        double mean = sum / count;
        
        double sq_sum = 0.0;
        for (size_t i = 0; i < count; ++i) {
            double diff = buffer[i] - mean;
            sq_sum += diff * diff;
        }
        return std::sqrt(sq_sum / count);
    }

public:
    Result update(double val) {
        Result result;
        result.val = val;
        
        result.slew_err = has_last_val ? std::abs(val - last_val) : 0.0;
        bool slew_alarm = result.slew_err > slew_threshold;
        
        buffer[buf_idx] = val;
        buf_idx = (buf_idx + 1) % WINDOW_SIZE;
        count = std::min(count + 1, WINDOW_SIZE);
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
    
    // Simplified Kalman
    double k_x;
    double k_p;
    double k_q;
    double k_r;
    bool is_init;

public:
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
    
    void reset() {
        is_init = false;
        anomaly_counter = 0;
        k_p = 1.0;
    }
};


class TrendingCusumDetectorFiltered {
public:
    struct Result {
        bool alarm;
        double s_high;
        double s_low;
    };

private:
    double target_slope;
    double K;      // мертвая зона
    double H;      // порог
    
    double s_high;
    double s_low;
    
    double last_val;
    double smoothed_val;
    double alpha;
    double decay;
    
    bool is_init;

public:
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
        , decay(0.99)
        , is_init(false)
    {}
    
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
        
        smoothed_val = alpha * val + (1.0 - alpha) * smoothed_val;
        double current_slope = smoothed_val - last_val;
        last_val = smoothed_val;
        
        double diff = current_slope - target_slope;
        
        s_high = std::max(0.0, (s_high + diff - K) * decay);
        s_low = std::max(0.0, (s_low - diff - K) * decay);
        
        result.alarm = (s_high > H) || (s_low > H);
        result.s_high = s_high;
        result.s_low = s_low;
        
        return result;
    }
    
    void reset() {
        is_init = false;
        s_high = 0.0;
        s_low = 0.0;
    }
};