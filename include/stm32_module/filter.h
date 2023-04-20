#ifndef FILTER_H
#define FILTER_H

// freertos include
#include "FreeRTOS.h"
#include "queue.h"

// stm32_module include
#include "stm32_module/module_common.h"

/* macro ---------------------------------------------------------------------*/
// assert macro
#define IS_IN_VALUE_RANGE(VAL) ((VAL) >= 0.0F && (VAL) <= 1.0F)

/* abstract class ------------------------------------------------------------*/
// forward declaration
struct FilterVtbl;

/**
 * @brief Abstract class for managing filter.
 *
 */
typedef struct filter {
  // virtual table
  struct FilterVtbl* vptr_;

  // member variable
  struct filter* chained_filter_;
} Filter;

/// @brief Virtual table for Filter.
struct FilterVtbl {
  ModuleRet (*update)(Filter*, float, float*);

  ModuleRet (*get_filtered_data)(Filter*, float*);
};

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for Filter.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] chained_filter The chained filter will be processed before this
 * filter, NULL if no chained filter.
 * @return None.
 */
void Filter_ctor(Filter* const self, Filter* const chained_filter);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function for adding new data to the filter and returns the current
 * filtered data.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] data The data to be added.
 * @param[out] filtered_data The filtered data, NULL if no need to get filtered
 * data.
 * @return ModuleRet Error code.
 */
ModuleRet Filter_update(Filter* const self, const float data,
                        float* const filtered_data);

/**
 * @brief Function for getting the current filtered data.
 *
 * @param[in,out] self The instance of the class.
 * @param[out] filtered_data The filtered data.
 * @return ModuleRet Error code.
 */
ModuleRet Filter_get_filtered_data(Filter* const self,
                                   float* const filtered_data);

/* class ---------------------------------------------------------------------*/
typedef struct moving_average_filter {
  // inherited class
  Filter super_;

  // member variable
  QueueHandle_t data_queue_;

  StaticQueue_t queue_cb_;

  int window_size_;

  float sum_;
} MovingAverageFilter;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for MovingAverageFilter.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] filter_buffer The buffer for storing data, must have length larger
 * than window_size.
 * @param[in] window_size The size of the filter.
 * @param[in] chained_filter The chained filter will be processed before this
 * filter, NULL if no chained filter.
 * @return None.
 * @note User is resposible for managing memory for filter_buffer.
 */
void MovingAverageFilter_ctor(MovingAverageFilter* const self,
                              float* const filter_buffer, const int window_size,
                              Filter* const chained_filter);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function for adding new data to the filter and returns the current
 * filtered data.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] data The data to be added.
 * @param[out] filtered_data The filtered data, NULL if no need to get filtered
 * data.
 * @return ModuleRet Error code.
 */
ModuleRet MovingAverageFilter_update(MovingAverageFilter* const self,
                                     const float data,
                                     float* const filtered_data);

/**
 * @brief Function for getting the current filtered data.
 *
 * @param[in,out] self The instance of the class.
 * @param[out] filtered_data The filtered data.
 * @return ModuleRet Error code.
 */
ModuleRet MovingAverageFilter_get_filtered_data(MovingAverageFilter* const self,
                                                float* const filtered_data);

/* class ---------------------------------------------------------------------*/
typedef struct normalize_filter {
  // inherited class
  Filter super_;

  float filtered_data_;

  float lower_bound_;

  float upper_bound_;
} NormalizeFilter;

/* constructor ---------------------------------------------------------------*/
/**
 * @brief Constructor for NormalizeFilter.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] lower_bound The initial lower bound of the filter.
 * @param[in] upper_bound The initial upper bound of the filter.
 * @param[in] chained_filter The chained filter will be processed before this
 * filter, NULL if no chained filter.
 * @return None.
 */
void NormalizeFilter_ctor(NormalizeFilter* const self, const float lower_bound,
                          const float upper_bound,
                          Filter* const chained_filter);

/* member function -----------------------------------------------------------*/
/**
 * @brief Function for adding new data to the filter and returns the current
 * filtered data.
 *
 * @param[in,out] self The instance of the class.
 * @param[in] data The data to be added.
 * @param[out] filtered_data The filtered data, NULL if no need to get filtered
 * data.
 * @return ModuleRet Error code.
 */
ModuleRet NormalizeFilter_update(NormalizeFilter* const self, const float data,
                                 float* const filtered_data);

/**
 * @brief Function for getting the current filtered data.
 *
 * @param[in,out] self The instance of the class.
 * @param[out] filtered_data The filtered data.
 * @return ModuleRet Error code.
 */
ModuleRet NormalizeFilter_get_filtered_data(NormalizeFilter* const self,
                                            float* const filtered_data);

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
