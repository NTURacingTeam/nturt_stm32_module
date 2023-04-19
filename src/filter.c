#include "stm32_module/filter.h"

// glibc include
#include <stdint.h>

// freertos include
#include "FreeRTOS.h"
#include "queue.h"

/* virtual function redirection ----------------------------------------------*/
inline float Filter_update(Filter *const self, const float data) {
  return self->vptr_->update(self, data);
}

inline float Filter_get_filtered_data(Filter *const self) {
  return self->vptr_->get_filtered_data(self);
}

/* virtual function definition -----------------------------------------------*/
// pure virtual function for Filter base class
float __Filter_update(Filter *const self, const float data) {
  (void)self;
  (void)data;

  module_assert(0);
  return 0.0;
}

float __Filter_get_filtered_data(Filter *const self) {
  (void)self;

  module_assert(0);
  return 0.0;
}

/* constructor ---------------------------------------------------------------*/
void Filter_ctor(Filter *const self) {
  module_assert(IS_NOT_NULL(self));

  // assign base virtual function
  static struct FilterVtbl vtbl_base = {
      .update = __Filter_update,
      .get_filtered_data = __Filter_get_filtered_data,
  };
  self->vptr_ = &vtbl_base;
}

/* virtual function redirection ----------------------------------------------*/
inline float MovingAverageFilter_update(MovingAverageFilter *const self,
                                        const float data) {
  return self->super_.vptr_->update((Filter *)self, data);
}

inline float MovingAverageFilter_get_filtered_data(
    MovingAverageFilter *const self) {
  return self->super_.vptr_->get_filtered_data((Filter *)self);
}

/* virtual function definition -----------------------------------------------*/
// from Filter base class
float __MovingAverageFilter_update(Filter *const _self, const float data) {
  module_assert(IS_NOT_NULL(_self));
  module_assert(IS_IN_VALUE_RANGE(data));

  MovingAverageFilter *const self = (MovingAverageFilter *)_self;
  if (xPortIsInsideInterrupt()) {
    if (uxQueueMessagesWaitingFromISR(self->data_queue_)) {
      int oldest_value;
      xQueueReceiveFromISR(self->data_queue_, &oldest_value, 0);
      self->sum_ -= oldest_value;
    }

    self->sum_ += data;
    xQueueSendToBackFromISR(self->data_queue_, &data, NULL);
  } else {
    if (uxQueueMessagesWaiting(self->data_queue_)) {
      int oldest_value;
      xQueueReceive(self->data_queue_, &oldest_value, 0);
      self->sum_ -= oldest_value;
    }

    self->sum_ += data;
    xQueueSendToBack(self->data_queue_, &data, 0);
  }

  return self->sum_ / self->window_size_;
}

// from Filter base class
float __MovingAverageFilter_get_filtered_data(Filter *const _self) {
  module_assert(IS_NOT_NULL(_self));

  MovingAverageFilter *const self = (MovingAverageFilter *)_self;
  return self->sum_ / self->window_size_;
}

/* constructor ---------------------------------------------------------------*/
void MovingAverageFilter_ctor(MovingAverageFilter *const self,
                              float *const filter_buffer,
                              const int window_size) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(filter_buffer));
  module_assert(IS_NOT_NEGATIVE(window_size));

  // construct inherited class and redirect virtual function
  Filter_ctor((Filter *)self);
  static struct FilterVtbl vtbl = {
      .update = __MovingAverageFilter_update,
      .get_filtered_data = __MovingAverageFilter_get_filtered_data,
  };
  self->super_.vptr_ = &vtbl;

  // initialize member variable
  self->data_queue_ = xQueueCreateStatic(
      window_size, sizeof(float), (uint8_t *)filter_buffer, &self->queue_cb_);
  self->window_size_ = window_size;
  self->sum_ = 0;
}

/* virtual function redirection ----------------------------------------------*/
inline float NormalizeFilter_update(NormalizeFilter *const self,
                                    const float data) {
  return self->super_.vptr_->update((Filter *)self, data);
}

inline float NormalizeFilter_get_filtered_data(NormalizeFilter *const self) {
  return self->super_.vptr_->get_filtered_data((Filter *)self);
}

/* virtual function definition -----------------------------------------------*/
// from Filter base class
float __NormalizeFilter_update(Filter *const _self, float data) {
  module_assert(IS_NOT_NULL(_self));
  module_assert(IS_IN_VALUE_RANGE(data));

  // process chained filter first
  NormalizeFilter *const self = (NormalizeFilter *)_self;
  if (self->chained_filter_ != NULL) {
    data = Filter_update(self->chained_filter_, data);
  }

  // update bounds
  if (data > self->upper_bound_) {
    self->upper_bound_ = data;
  } else if (self->filtered_data_ < self->lower_bound_) {
    self->lower_bound_ = data;
  }

  // normalize data
  self->filtered_data_ =
      (data - self->lower_bound_) / (self->upper_bound_ - self->lower_bound_);
  return self->filtered_data_;
}

// from Filter base class
float __NormalizeFilter_get_filtered_data(Filter *const _self) {
  module_assert(IS_NOT_NULL(_self));

  NormalizeFilter *const self = (NormalizeFilter *)_self;
  return self->filtered_data_;
}

/* constructor ---------------------------------------------------------------*/
void NormalizeFilter_ctor(NormalizeFilter *const self, const float upper_bound,
                          const float lower_bound,
                          Filter *const chained_filter) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_IN_VALUE_RANGE(upper_bound));
  module_assert(IS_IN_VALUE_RANGE(lower_bound));

  // construct inherited class and redirect virtual function
  Filter_ctor((Filter *)self);
  static struct FilterVtbl vtbl = {
      .update = __NormalizeFilter_update,
      .get_filtered_data = __NormalizeFilter_get_filtered_data,
  };
  self->super_.vptr_ = &vtbl;

  // initialize member variable
  self->upper_bound_ = upper_bound;
  self->lower_bound_ = lower_bound;
  self->chained_filter_ = chained_filter;
}

/* constructor ---------------------------------------------------------------*/
void KalmanFilter1D_ctor(KalmanFilter1D *const self, float Q, float R, float x0,
                         float P0) {
  self->Q_ = Q;
  self->R_ = R;
  self->x_ = x0;
  self->P_ = P0;
}

/* member function -----------------------------------------------------------*/
float KalmanFilter1D_update(KalmanFilter1D *const self, float z) {
  // predict step
  float x_pred = self->x_;
  float P_pred = self->P_ + self->Q_;

  // update step
  float K = P_pred / (P_pred + self->R_);
  self->x_ = x_pred + K * (z - x_pred);
  self->P_ = (1 - K) * P_pred;

  return self->x_;
}

float KalmanFilter1D_get_state(KalmanFilter1D *const self) { return self->x_; }

float KalmanFilter1D_get_covariance(KalmanFilter1D *const self) {
  return self->P_;
}
