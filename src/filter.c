#include "stm32_module/filter.h"

// glibc include
#include <stdint.h>

// freertos include
#include "FreeRTOS.h"
#include "queue.h"

/* virtual function redirection ----------------------------------------------*/
inline ModuleRet Filter_update(Filter *const self, const float data,
                               float *const filtered_data) {
  return self->vptr_->update(self, data, filtered_data);
}

inline ModuleRet Filter_get_filtered_data(Filter *const self,
                                          float *const filtered_data) {
  return self->vptr_->get_filtered_data(self, filtered_data);
}

/* virtual function definition -----------------------------------------------*/
// pure virtual function for Filter base class
ModuleRet __Filter_update(Filter *const self, const float data,
                          float *const filtered_data) {
  (void)self;
  (void)data;
  (void)filtered_data;

  module_assert(0);
  return ModuleError;
}

// pure virtual function for Filter base class
ModuleRet __Filter_get_filtered_data(Filter *const self,
                                     float *const filtered_data) {
  (void)self;
  (void)filtered_data;

  module_assert(0);
  return ModuleError;
}

/* constructor ---------------------------------------------------------------*/
void Filter_ctor(Filter *const self, Filter *const chained_filter) {
  module_assert(IS_NOT_NULL(self));

  // assign base virtual function
  static struct FilterVtbl vtbl_base = {
      .update = __Filter_update,
      .get_filtered_data = __Filter_get_filtered_data,
  };
  self->vptr_ = &vtbl_base;

  // initialize member variable
  self->chained_filter_ = chained_filter;
}

/* virtual function redirection ----------------------------------------------*/
inline ModuleRet MovingAverageFilter_update(MovingAverageFilter *const self,
                                            const float data,
                                            float *const filtered_data) {
  return self->super_.vptr_->update((Filter *)self, data, filtered_data);
}

inline ModuleRet MovingAverageFilter_get_filtered_data(
    MovingAverageFilter *const self, float *const filtered_data) {
  return self->super_.vptr_->get_filtered_data((Filter *)self, filtered_data);
}

/* virtual function definition -----------------------------------------------*/
// from Filter base class
ModuleRet __MovingAverageFilter_update(Filter *const _self, float data,
                                       float *const filtered_data) {
  module_assert(IS_NOT_NULL(_self));
  // module_assert(IS_IN_VALUE_RANGE(data));

  MovingAverageFilter *const self = (MovingAverageFilter *)_self;
  // process chained filter first
  if (self->super_.chained_filter_ != NULL) {
    ModuleRet ret = Filter_update(self->super_.chained_filter_, data, &data);
    if (ret != ModuleOK) {
      return ret;
    }
  }

  // calculate moving average
  // if queue is full, remove the oldest data from the queue
  UBaseType_t queue_size;
  if (xPortIsInsideInterrupt()) {
    queue_size = uxQueueMessagesWaitingFromISR(self->data_queue_);
  } else {
    queue_size = uxQueueMessagesWaiting(self->data_queue_);
  }
  if (queue_size == (UBaseType_t)self->window_size_) {
    float oldest_value;
    if (xPortIsInsideInterrupt()) {
      xQueueReceiveFromISR(self->data_queue_, &oldest_value, NULL);
    } else {
      xQueueReceive(self->data_queue_, &oldest_value, 0);
    }
    self->sum_ -= oldest_value;
  }
  // add new data to the queue
  self->sum_ += data;
  if (xPortIsInsideInterrupt()) {
    xQueueSendToBackFromISR(self->data_queue_, &data, NULL);
  } else {
    xQueueSendToBack(self->data_queue_, &data, 0);
  }
  // if queue is full, calculate moving average
  if (queue_size == (UBaseType_t)self->window_size_) {
    if (filtered_data != NULL) {
      *filtered_data = self->sum_ / self->window_size_;
    }
    return ModuleOK;
  }
  return ModuleBusy;
}

// from Filter base class
ModuleRet __MovingAverageFilter_get_filtered_data(Filter *const _self,
                                                  float *const filtered_data) {
  module_assert(IS_NOT_NULL(_self));
  module_assert(IS_NOT_NULL(filtered_data));

  MovingAverageFilter *const self = (MovingAverageFilter *)_self;
  if ((xPortIsInsideInterrupt() &&
       uxQueueMessagesWaitingFromISR(self->data_queue_) ==
           (UBaseType_t)self->window_size_) ||
      uxQueueMessagesWaiting(self->data_queue_) ==
          (UBaseType_t)self->window_size_) {
    *filtered_data = self->sum_ / self->window_size_;
    return ModuleOK;
  }
  return ModuleBusy;
}

/* constructor ---------------------------------------------------------------*/
void MovingAverageFilter_ctor(MovingAverageFilter *const self,
                              float *const filter_buffer, const int window_size,
                              Filter *const chained_filter) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(filter_buffer));
  module_assert(IS_NOT_NEGATIVE(window_size));

  // construct inherited class and redirect virtual function
  Filter_ctor((Filter *)self, chained_filter);
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
inline ModuleRet NormalizeFilter_update(NormalizeFilter *const self,
                                        const float data,
                                        float *const filtered_data) {
  return self->super_.vptr_->update((Filter *)self, data, filtered_data);
}

inline ModuleRet NormalizeFilter_get_filtered_data(NormalizeFilter *const self,
                                                   float *const filtered_data) {
  return self->super_.vptr_->get_filtered_data((Filter *)self, filtered_data);
}

/* virtual function definition -----------------------------------------------*/
// from Filter base class
ModuleRet __NormalizeFilter_update(Filter *const _self, float data,
                                   float *const filtered_data) {
  module_assert(IS_NOT_NULL(_self));
  // module_assert(IS_IN_VALUE_RANGE(data));

  NormalizeFilter *const self = (NormalizeFilter *)_self;
  // process chained filter first
  if (self->super_.chained_filter_ != NULL) {
    ModuleRet ret = Filter_update(self->super_.chained_filter_, data, &data);
    if (ret != ModuleOK) {
      return ret;
    }
  }

  // update bounds
  if (self->upper_bound_ < data) {
    self->upper_bound_ = data;
  }
  if (self->lower_bound_ > data) {
    self->lower_bound_ = data;
  }

  // normalize data
  self->filtered_data_ =
      (data - self->lower_bound_) / (self->upper_bound_ - self->lower_bound_);
  *filtered_data = self->filtered_data_;
  return ModuleOK;
}

// from Filter base class
ModuleRet __NormalizeFilter_get_filtered_data(Filter *const _self,
                                              float *const filtered_data) {
  module_assert(IS_NOT_NULL(_self));
  module_assert(IS_NOT_NULL(filtered_data));

  NormalizeFilter *const self = (NormalizeFilter *)_self;
  *filtered_data = self->filtered_data_;
  return ModuleOK;
}

/* constructor ---------------------------------------------------------------*/
void NormalizeFilter_ctor(NormalizeFilter *const self, const float lower_bound,
                          const float upper_bound,
                          Filter *const chained_filter) {
  module_assert(IS_NOT_NULL(self));
  // module_assert(IS_IN_VALUE_RANGE(lower_bound));
  // module_assert(IS_IN_VALUE_RANGE(upper_bound));

  // construct inherited class and redirect virtual function
  Filter_ctor((Filter *)self, chained_filter);
  static struct FilterVtbl vtbl = {
      .update = __NormalizeFilter_update,
      .get_filtered_data = __NormalizeFilter_get_filtered_data,
  };
  self->super_.vptr_ = &vtbl;

  // initialize member variable
  self->lower_bound_ = lower_bound;
  self->upper_bound_ = upper_bound;
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
