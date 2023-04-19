#ifndef FILTER_H
#define FILTER_H

// freertos include
#include "FreeRTOS.h"
#include "queue.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* class ---------------------------------------------------------------------*/
typedef struct moving_average_filter {
  QueueHandle_t data_queue_;

  StaticQueue_t queue_cb_;

  int window_size_;

  int sum_;
} MovingAverageFilter;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for MovingAverageFilter.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] filter_buffer The buffer for storing data, must have length larger
 * than window_size.
 * @param[in] window_size The size of the filter.
 * @return None.
 * @note User is resposible for managing memory for filter_buffer.
 */
void MovingAverageFilter_ctor(MovingAverageFilter* const self,
                              float* const filter_buffer,
                              const int window_size);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function for adding data to the filter.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] data The data to be added.
 * @return None.
 */
void MovingAverageFilter_add_data(MovingAverageFilter* const self,
                                  const float data);

/**
 * @brief Function for getting the filtered data.
 *
 * @param[in,out] self The instance of the class.
 * @return float The filtered data.
 */
float MovingAverageFilter_get_filtered_data(MovingAverageFilter* const self);

/* class ---------------------------------------------------------------------*/
/**
 * @brief Class for managing 1 dimensional kalman filter.
 *
 */
typedef struct {
  float Q_;
  float R_;
  float x_;
  float P_;
} KalmanFilter1D;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for KalmanFilter1D.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] Q The process noise covariance.
 * @param[in] R The measurement noise covariance.
 * @param[in] x0 The initial state.
 * @param[in] P0 The initial covariance.
 * @return None.
 */
void KalmanFilter1D_ctor(KalmanFilter1D* const self, float Q, float R, float x0,
                         float P0);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function for updating the state of kalman filter.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] z The measurement.
 * @return The updated state.
 */
float KalmanFilter1D_update(KalmanFilter1D* const self, float z);

/**
 * @brief Function for getting the current state of kalman filter.
 *
 * @param[in,out] self The instance of the class.
 * @return float The current state.
 */
float KalmanFilter1D_get_state(KalmanFilter1D* const self);

/**
 * @brief Function for getting the current covariance of kalman filter.
 *
 * @param[in,out] self The instance of the class.
 * @return float The current covariance.
 */
float KalmanFilter1D_get_covariance(KalmanFilter1D* const self);

#endif  // FILTER_H
