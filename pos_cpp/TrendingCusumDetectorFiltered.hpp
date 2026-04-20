#ifndef TRENDING_CUSUM_DETECTOR_FILTERED_H
#define TRENDING_CUSUM_DETECTOR_FILTERED_H

#include <algorithm>
#include <cmath>

/**
 * @brief
 * 
 */
class TrendingCusumDetectorFiltered {
public:
    struct Result {
        bool alarm;
        double s_high;
        double s_low;
    };

private:
    double target_slope;
    double K;
    double H;
    
    double s_high;
    double s_low;
    
    double last_val;
    double smoothed_val;
    double alpha;
    double decay;
    
    bool is_init;

public:
    /**
     * @param target_slp
     * @param sigma_slope
     * @param h_factor
     * @param k_factor
     * @param alpha_val
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
        , decay(0.99)
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
    
    /**
     * @brief
     */
    void reset() {
        is_init = false;
        s_high = 0.0;
        s_low = 0.0;
    }
};

#endif