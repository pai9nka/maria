#ifndef SIMPLE_KALMAN_H
#define SIMPLE_KALMAN_H
#include <Arduino.h>

class SimpleKalman {
  private:
    float _err_measure, _err_estimate, _q;
    float _current_estimate = 0, _last_estimate = 0, _kalman_gain = 0;
  public:
    SimpleKalman(float e_me, float e_est, float q) {
      _err_measure = e_me; _err_estimate = e_est; _q = q;
    }
    float update(float value) {
      _kalman_gain = _err_estimate / (_err_estimate + _err_measure);
      _current_estimate = _last_estimate + _kalman_gain * (value - _last_estimate);
      _err_estimate = (1.0 - _kalman_gain) * _err_estimate + fabsf(_last_estimate - _current_estimate) * _q;
      _last_estimate = _current_estimate;
      return _current_estimate;
    }
};
#endif
