#include "stm32_module/filter.h"

// glibc include
#include <stdint.h>

// freertos include
#include "FreeRTOS.h"
#include "queue.h"

/* constructor ---------------------------------------------------------------*/
void MovingAverageFilter_ctor(MovingAverageFilter *const self,
                              float *const filter_buffer,
                              const int window_size) {
  module_assert(IS_NOT_NULL(self));
  module_assert(IS_NOT_NULL(filter_buffer));
  module_assert(IS_NOT_NEGATIVE(window_size));

  self->data_queue_ = xQueueCreateStatic(
      window_size, sizeof(float), (uint8_t *)filter_buffer, &self->queue_cb_);
  self->window_size_ = window_size;
  self->sum_ = 0;
}

/* member function -----------------------------------------------------------*/
void MovingAverageFilter_add_data(MovingAverageFilter *const self,
                                  const float data) {
  module_assert(IS_NOT_NULL(self));

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
}

float MovingAverageFilter_get_filtered_data(MovingAverageFilter *const self) {
  module_assert(IS_NOT_NULL(self));

  if (xPortIsInsideInterrupt() &&
      uxQueueMessagesWaitingFromISR(self->data_queue_) != self->window_size_) {
    return 0.0;
  } else if (uxQueueMessagesWaiting(self->data_queue_) != self->window_size_) {
    return 0.0;
  }

  return self->sum_ / self->window_size_;
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
