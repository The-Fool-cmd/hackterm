#ifndef CORE_RESULT_H
#define CORE_RESULT_H

/**
 * @brief Result codes for core operations.
 */
typedef enum {
    CORE_OK = 0,           /**< Command succeeded */
    CORE_ERR_NOT_FOUND,    /**< Server not found */
    CORE_ERR_NOT_LINKED,   /**< Server not directly linked */
    CORE_ERR_FILE,         /**< File error */
    CORE_ERR_INVALID_ARG,  /**< Invalid argument */
    CORE_ERR_UNKNOWN       /**< Generic failure */
} CoreResult;

#endif /* CORE_RESULT_H */
